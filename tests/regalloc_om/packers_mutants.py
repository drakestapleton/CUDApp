#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Run this complete process under the device-free bwrap contract."""
import os
from pathlib import Path
import subprocess
ROOT = Path(__file__).resolve().parents[2]
os.chdir(ROOT)
OUT = ROOT / 'build/packers-om/mutants'
OUT.mkdir(parents=True, exist_ok=True)
SOURCE = (ROOT / 'om/codegen/cg_pack_h.om').read_text()
CASES = [
    ('a-column-low-bit', 'mul shr cc 3 4 and cc 1\n', 'mul shr cc 3 4 xor and cc 1 1\n'),
    ('a-row-lane-stride', 'mul and rr 7 32', 'mul and rr 7 16'),
    ('b-group-stride', 'add get job 0 mul g 16', 'add get job 0 mul g 8'),
    ('padding-value', 'put d x ushort 0\n', 'put d x ushort 1\n'),
    ('bf16-tie-even', 'add add x 32767 and shr x 16 1', 'add add x 32767 ulong 0'),
    ('row0-padding-zero', 'ph_z16 d blk c1\n', 'ph_z16 d blk 16\n'),
]
bridge = Path(os.environ['OMEGA_BRIDGE']) / 'tools/bridge-seeded'
env = dict(os.environ, OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
expected = (ROOT / 'build/packers-om/oracle.out').read_bytes()
summary=[]
for name, old, new in CASES:
    if SOURCE.count(old) != 1: raise RuntimeError(f'{name}: ambiguous anchor')
    work = OUT / name
    work.mkdir(exist_ok=True)
    source=work/'mutant.om'; source.write_text(SOURCE.replace(old,new))
    generated=work/'mutant.c'
    with generated.open('wb') as stdout, (work/'compile.log').open('wb') as stderr:
        compiled=subprocess.run(['sh',str(bridge),'--runtime-views','c',str(source)],env=env,stdout=stdout,stderr=stderr)
    if compiled.returncode: raise RuntimeError(f'{name}: NOT_KILLED compiler rc={compiled.returncode}')
    binary=work/'packers'
    with (work/'link.log').open('wb') as log:
        subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O2','-ffunction-sections','-fdata-sections','-Isrc',
            'tests/regalloc_om/packers.c','src/omega_blackwell_codegen.c','src/cudapp_om_cg_pack_len.c',str(generated),
            '-Wl,--gc-sections','-o',str(binary)],stdout=log,stderr=subprocess.STDOUT,check=True)
    with (work/'output').open('wb') as output:
        exercised=subprocess.run([str(binary),'--mutant'],stdout=output,stderr=subprocess.PIPE)
    (work/'exercise.log').write_bytes(exercised.stderr)
    if exercised.returncode: raise RuntimeError(f'{name}: NOT_KILLED exercise rc={exercised.returncode}')
    if (work/'output').read_bytes()==expected: raise RuntimeError(f'{name}: SURVIVED')
    summary.append(f'{name}\tcompile=0\texercise=0\tcomparison=different')
    print(f'KILLED {summary[-1]}',flush=True)
(OUT/'mutants.tsv').write_text('\n'.join(summary)+'\n')
print(f'PASS fragment recompiled Omega semantic mutants={len(summary)}')
