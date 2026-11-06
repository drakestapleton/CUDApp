# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Mutate pathcopy.om, regenerate the freestanding binding, and require the differential to die."""
import os
from pathlib import Path
import shlex
import subprocess
import sys

root = Path(__file__).resolve().parent.parent
out = Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
source = (root / 'om/components/pathcopy.om').read_text()
mutants = [
    ('alphabet', 'ge value 97 le value 122', 'ge value 98 le value 122'),
    ('punctuation', 'eq value 58 eq value 126', 'eq value 59 eq value 126'),
    ('slash-state', 'if eq state 2\nret 4', 'if eq state 1\nret 4'),
    ('invalid-byte', 'ret 256\nend\nfn pathstep', 'ret value\nend\nfn pathstep'),
    ('dotdot', 'eq count 0 eq state 2', 'eq count 0 eq state 1'),
]
for name, old, new in mutants:
    if source.count(old) != 1:
        raise SystemExit(f'FAIL {name}: mutation site not unique')
    directory = out / name
    directory.mkdir(exist_ok=True)
    unit = directory / 'pathcopy.om'
    unit.write_text(source.replace(old, new))
    generated = directory / 'cudapp_freest_pathcopy.c'
    env = dict(os.environ)
    env['PATHCOPY_UNIT'] = str(unit)
    env['PATHCOPY_DEST'] = str(generated)
    env['PATHCOPY_ARCH'] = 'a64'
    subprocess.run(['sh', str(root / 'om/native-freest.sh')], env=env, cwd=root, check=True)
    binary = directory / 'diff'
    subprocess.run(
        shlex.split(os.environ.get('CC', 'cc')) + [
            '-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror', '-Isrc',
            'tests/leftovers_test.c', 'src/cudapp_freest_leftovers.c', str(generated),
            '-o', str(binary),
        ],
        cwd=root,
        check=True,
    )
    with (directory / 'run.log').open('w') as stream:
        result = subprocess.run([str(binary)], stdout=stream, stderr=subprocess.STDOUT)
    if result.returncode != 1:
        raise SystemExit(f'FAIL {name}: expected assertion exit 1, got {result.returncode}')
    print(f'KILLED {name}', flush=True)
print(f'PATHCOPY MUTANTS PASS killed={len(mutants)}')
