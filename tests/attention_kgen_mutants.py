#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Recompile perturbed seeded attention units and require differential detection.
Verification oracle only, never part of the production codegen path. Invoke through
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10.
"""
import os
from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
build = (root / sys.argv[1]).resolve()
files = ['attention_emit.om', 'attention_tuned.om', 'attention_general.om', 'attention_ir.om']
mutants = [
    ('descriptor-offset', 'attention_general.om', '775 atrg 0 AN AN AN 856 1', '775 atrg 0 AN AN AN 896 1'),
    ('query-argument-offset', 'attention_general.om', 'get s add A i add 896 mul i 8', 'get s add A i add 904 mul i 8'),
    ('thread-register', 'attention_general.om', 'get s TID 33', 'get s TID 37'),
    ('argument-scalar-offset', 'attention_general.om', 'get s add MAXB i add 928 mul i 4', 'get s add MAXB i add 952 mul i 4'),
    ('stall-control-field', 'attention_emit.om', 'shl st 9 shl wb 14', 'shl st 14 shl wb 14'),
]

def run(cmd, **kw):
    return subprocess.run(cmd, cwd=root, text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, **kw)

if Path('/dev/nvidiactl').exists() or Path('/dev/nvidia0').exists():
    raise SystemExit('FAIL attention mutants require bwrap private /dev')
base = run([str(build / 'attention_diff')])
if base.returncode:
    raise SystemExit('FAIL baseline\n' + base.stdout)
print(base.stdout.strip(), flush=True)
for name, filename, old, new in mutants:
    with tempfile.TemporaryDirectory(prefix='attention-mut-', dir=build) as tmpdir:
        tmp = Path(tmpdir)
        sources = []
        for f in files:
            text = (root / 'om/kgen' / f).read_text()
            if f == filename:
                if text.count(old) != 1:
                    raise SystemExit(f'FAIL {name}: ambiguous mutation site count={text.count(old)}')
                text = text.replace(old, new)
            dest = tmp / f
            dest.write_text(text)
            sources.append(str(dest))
        generated = tmp / 'generated'
        generated.mkdir()
        env = dict(os.environ, OMEGA_UNITS='attention_ir=' + ','.join(sources),
                   OMEGA_UNITS_DEST=str(generated))
        result = run(['sh', 'om/units.sh'], env=env)
        if result.returncode:
            raise SystemExit(f'FAIL {name}: generation failure is not a semantic kill\n{result.stdout}')
        obj, exe = tmp / 'mutant.o', tmp / 'differential'
        cc = os.environ.get('CC', 'cc')
        result = run([cc, '-O2', '-Isrc', '-ffunction-sections', '-fdata-sections',
                      '-c', str(generated / 'cudapp_om_attention_ir.c'), '-o', str(obj)])
        if result.returncode:
            raise SystemExit(f'FAIL {name}: compilation\n{result.stdout}')
        core = {'attention_diff.o', 'omega_blackwell_codegen.o', 'omega_gpu_om_ir.o', 'omega_cubin.o', 'sha256.o'}
        objects = [p for p in sorted(build.glob('*.o')) if p.name in core or
                   (p.name.startswith('cudapp_') and p.name != 'cudapp_om_attention_ir.o')]
        result = run([cc, '--coverage', '-Wl,--gc-sections', '-o', str(exe), str(obj),
                      *map(str, objects), '-lpthread', '-lm'])
        if result.returncode:
            raise SystemExit(f'FAIL {name}: link\n{result.stdout}')
        result = run([str(exe)])
        if result.returncode != 1 or 'attention parity failed ' not in result.stdout:
            raise SystemExit(f'FAIL {name}: survived or crashed rc={result.returncode}\n{result.stdout}')
        print(f'KILLED {name}: rc=1 {result.stdout.strip()}', flush=True)
print('PASS attention seeded semantic mutants: 5 killed', flush=True)
