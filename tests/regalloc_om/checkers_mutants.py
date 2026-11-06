#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Run after checkers.sh, inside its device-free namespace. Compile refusal is failure."""
import os
from pathlib import Path
import subprocess
ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/checkers-om'
BRIDGE = Path(os.environ['OMEGA_BRIDGE'])
OMC = os.environ['OMEGA_OMC']
CASES = [
    ('emit-var-pending', 'cg_em', 'pn = uint 3\n', 'pn = uint 2\n'),
    ('emit-predicate-preserve', 'cg_em', 'if eq pred 0\n', 'if eq pred 1\n'),
    ('prefetch-held', 'cg_em', 'put w 1 or get w 1 ulong shl uint 1 sb\n', 'put w 1 get w 1\n'),
    ('consume-release', 'cg_em', 'put w 1 and get w 1 xor mk 4294967295\n', 'put w 1 get w 1\n'),
    ('loop-capacity', 'cg_em', 'if ge a 4\n', 'if ge a 3\n'),
    ('interval-equal-start', 'cg_em', 'ge c b lt c endval\n', 'gt c b lt c endval\n'),
    ('join-bssy-opcode', 'cg_reconv', 'ne rc_g w 2 54 ', 'ne rc_g w 2 55 '),
    ('join-exit-opcode', 'cg_reconv', 'if ne rc_g w 3 46\n', 'if ne rc_g w 3 47\n'),
    ('join-depth-decrement', 'cg_reconv', 'put w 0 rc_u sub rc_g w 0 1\n', 'put w 0 get w 0\n'),
    ('join-relative-immediate', 'cg_reconv', 'ret sub add rc_g w 1 1 rc_g w 0\n', 'ret sub rc_g w 1 rc_g w 0\n'),
]
def run(cmd, log, **kw):
    with log.open('wb') as f:
        p = subprocess.run(cmd, cwd=ROOT, stdout=f, stderr=subprocess.STDOUT, **kw)
    if p.returncode:
        raise RuntimeError(f'NOT_KILLED rc={p.returncode}: {log}')
def main():
    reference = (OUT/'oracle.out').read_bytes()
    rows=[]
    for name, stem, before, after in CASES:
        work=OUT/'mutants'/name
        work.mkdir(parents=True, exist_ok=True)
        source=(ROOT/'om/codegen'/f'{stem}.om').read_text()
        if source.count(before)!=1: raise RuntimeError(f'{name}: nonunique anchor')
        om=work/f'{stem}.om'; om.write_text(source.replace(before,after))
        generated=work/f'{stem}.c'
        with generated.open('wb') as f, (work/'compile.log').open('wb') as e:
            p=subprocess.run(['sh',str(BRIDGE/'tools/bridge-seeded'),'c',str(om)],cwd=ROOT,
                             env=dict(os.environ,OMEGA_SEEDED_OMC=OMC),stdout=f,stderr=e)
        if p.returncode: raise RuntimeError(f'{name}: NOT_KILLED compiler rc={p.returncode}')
        units=[str(generated) if x==stem else f'src/cudapp_om_{x}.c' for x in ('cg_em','cg_reconv')]
        binary=work/'checkers'
        run([os.environ.get('CC','cc'),'-std=gnu11','-O2','-ffunction-sections','-fdata-sections','-Isrc',
             'tests/regalloc_om/checkers.c',*units,'-Wl,--gc-sections','-o',str(binary)],work/'link.log')
        run([str(binary)],work/'exercise.out')
        if (work/'exercise.out').read_bytes()==reference: raise RuntimeError(f'{name}: SURVIVED')
        rows.append(f'{name}\tcompile=0\texercise=0\tdifferential=1')
        print('KILLED',rows[-1],flush=True)
    (OUT/'mutants.tsv').write_text('\n'.join(rows)+'\n')
    print(f'PASS checker recompiled Omega semantic mutants={len(rows)}')
if __name__=='__main__': main()
