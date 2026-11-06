#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Two compiling source mutations per policy; the unchanged C oracle kills each."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
env = dict(os.environ)
cc = shlex.split(os.environ.get('CC', 'cc'))
mutants = (
    ('mm', 0, 'gt m 8192', 'gt m 8191'),
    ('mm', 0, 'budget = 1024', 'budget = 1023'),
    ('at', 1, 'gt h 256', 'gt h 255'),
    ('at', 1, 'lt l 4', 'lt l 3'),
    ('ew', 2, 'gt dim 16384', 'gt dim 16383'),
    ('ew', 2, 'ni = 131072', 'ni = 131071'),
    ('bf', 3, 'w = ulong 16', 'w = ulong 32'),
    ('bf', 3, 'put a 11 ulong 96', 'put a 11 ulong 95'),
    ('da', 4, 'gt layers 64', 'gt layers 63'),
    ('da', 4, 'depth = 4', 'depth = 3'),
    ('rt', 5, 'e = 128', 'e = 256'),
    ('rt', 5, 'add ulong 8 get a 2', 'add ulong 7 get a 2'),
)
for unit, family, old, new in mutants:
    name = 'api_' + unit + '_plan'
    source = (root / 'om/components' / (name + '.om')).read_text()
    assert source.count(old) == 1, (name, old)
    with tempfile.TemporaryDirectory(prefix='cudapp-api-plan-') as tmp:
        tmp = Path(tmp)
        om = tmp / (name + '.om')
        om.write_text(source.replace(old, new))
        generated = tmp / ('cudapp_om_' + name + '.c')
        mutant_env = dict(env, OMEGA_UNITS=name + '=' + str(om), OMEGA_UNITS_DEST=str(tmp))
        subprocess.run(['sh', str(root / 'om/units.sh')], env=mutant_env, cwd=root, check=True)
        others = [str(p) for p in sorted((root / 'src').glob('cudapp_om_api_*_plan.c')) if p.name != 'cudapp_om_' + name + '.c']
        exe = tmp / 'mutant'
        subprocess.run(cc + ['-O2', '-Wall', '-Wextra', '-Werror', '-I' + str(root / 'src'), str(root / 'tests/runtime/api_plan_unit.c'), str(generated)] + others + ['-o', str(exe)], check=True)
        result = subprocess.run([str(exe), str(family)], capture_output=True, text=True)
        assert result.returncode != 0 and 'API PLAN differential FAIL' in result.stderr, result
        print('API PLAN mutant killed:', name, old, '=>', new, flush=True)
print('API PLAN 12 source mutants PASS')
