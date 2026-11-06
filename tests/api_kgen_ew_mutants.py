#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Mutate Omega sources, regenerate checked bindings, run the C differential oracle.
Run only inside the prescribed bwrap; inherited /dev must hide GPU devices.
"""
import os
from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
build = (root / sys.argv[1]).resolve()
files = ['ew_emit.om', 'ew_scalar.om', 'ew_reconv.om', 'ew_prime.om', 'ew_ir.om']
mutants = [
    ('descriptor-offset', 'ew_emit.om', 'NR NR NR 856', 'NR NR NR 864'),
    ('argument-b-offset', 'ew_emit.om', 'r4 1 NR NR NR 904', 'r4 1 NR NR NR 896'),
    ('cta-register', 'ew_emit.om', 'S2R r4 4 NR NR NR 37', 'S2R r4 4 NR NR NR 33'),
    ('pair-latency', 'ew_emit.om', 'shl ulong 5 9', 'shl ulong 4 9'),
    ('ex2-dispatch', 'ew_ir.om', 'ex2 ins meta', 'xchg ins meta'),
]
def run(cmd, **kw):
    return subprocess.run(cmd, cwd=root, text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, **kw)

if Path('/dev/nvidiactl').exists() or Path('/dev/nvidia0').exists():
    raise SystemExit('FAIL: mutation gate requires bwrap private /dev')
base = run([str(build / 'ew_diff')])
if base.returncode != 0:
    raise SystemExit('FAIL baseline\n' + base.stdout)
print(base.stdout.strip())
for name, filename, old, new in mutants:
    with tempfile.TemporaryDirectory(prefix='ew-mut-', dir=build) as tmp:
        tmp = Path(tmp)
        sources = []
        for f in files:
            text = (root / 'om/kgen' / f).read_text()
            if f == filename:
                if text.count(old) != 1:
                    raise SystemExit(f'FAIL {name}: mutation site count {text.count(old)}')
                text = text.replace(old, new)
            dest = tmp / f
            dest.write_text(text)
            sources.append(str(dest))
        generated = tmp / 'generated'
        generated.mkdir()
        env = dict(os.environ, OMEGA_UNITS='ew_ir=' + ','.join(sources),
                   OMEGA_UNITS_DEST=str(generated))
        result = run(['sh', 'om/units.sh'], env=env)
        if result.returncode:
            raise SystemExit(f'FAIL {name}: generation is not a killed semantic mutant\n{result.stdout}')
        obj, exe = tmp / 'mutant.o', tmp / 'diff'
        result = run([os.environ.get('CC', 'cc'), '-O2', '-Isrc', '-ffunction-sections',
                      '-fdata-sections', '-c', str(generated / 'cudapp_om_ew_ir.c'), '-o', str(obj)])
        if result.returncode:
            raise SystemExit(f'FAIL {name}: compile\n{result.stdout}')
        core = {'ew_diff.o', 'omega_blackwell_codegen.o', 'omega_gpu_om_ir.o', 'omega_cubin.o', 'sha256.o'}
        objects = [p for p in sorted(build.glob('*.o'))
                   if p.name in core or (p.name.startswith('cudapp_') and p.name != 'cudapp_om_ew_ir.o')]
        result = run([os.environ.get('CC', 'cc'), '-Wl,--gc-sections', '-o', str(exe),
                      str(obj), *map(str, objects), '-lpthread', '-lm'])
        if result.returncode:
            raise SystemExit(f'FAIL {name}: link\n{result.stdout}')
        result = run([str(exe)])
        if result.returncode != 1 or not any(line.startswith('FAIL ') for line in result.stdout.splitlines()):
            raise SystemExit(f'FAIL {name}: survived or crashed rc={result.returncode}\n{result.stdout}')
        print(f'KILLED {name}: {result.stdout.strip()}')
print('PASS API EW seeded semantic mutants: 5 killed')
