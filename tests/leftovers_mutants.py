# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Change Omega, regenerate with the pinned frontend, exercise the real wrappers."""
import os
from pathlib import Path
import shlex
import subprocess
import sys

root = Path(__file__).resolve().parent.parent
out = Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
source = (root / 'om/components/leftovers.om').read_text()
mutants = [
    ('alphabet', 'ge value 97 le value 122', 'ge value 98 le value 122'),
    ('punctuation', 'eq value 58 eq value 126', 'eq value 59 eq value 126'),
    ('dotdot', 'eq count 0 eq state 2', 'eq count 0 eq state 1'),
    ('capacity', 'ge count sub cap 1', 'gt count sub cap 1'),
    ('ordinal', 'ret value\nend\nret 0\nend\nfn leftover_acquire', 'ret 0\nend\nret 0\nend\nfn leftover_acquire'),
    ('acquire', 'if eq okay 0', 'if ne okay 0'),
    ('poison', 'if eq state 2\nret 1', 'if eq state 1\nret 1'),
    ('context', 'eq present 0', 'eq present 1'),
    ('release', 'lt index 32', 'lt index 31'),
]
bridge = os.environ['OMEGA_BRIDGE']
env = dict(os.environ, OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
for name, old, new in mutants:
    if source.count(old) != 1:
        raise SystemExit(f'FAIL {name}: mutation site not unique')
    directory = out / name
    directory.mkdir(exist_ok=True)
    unit = directory / 'leftovers.om'
    unit.write_text(source.replace(old, new))
    generated = directory / 'leftovers.c'
    with generated.open('w') as stream:
        subprocess.run(['sh', bridge + '/tools/bridge-freest', 'c', str(unit)], env=env, stdout=stream, check=True)
    binary = directory / 'diff'
    subprocess.run(shlex.split(os.environ.get('CC', 'cc')) + ['-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror', '-Isrc', 'tests/leftovers_test.c', str(generated), '-o', str(binary)], cwd=root, check=True)
    with (directory / 'run.log').open('w') as stream:
        result = subprocess.run([str(binary)], stdout=stream, stderr=subprocess.STDOUT)
    if result.returncode != 1:
        raise SystemExit(f'FAIL {name}: expected assertion exit 1, got {result.returncode}')
    print(f'KILLED {name}', flush=True)
print(f'LEFTOVERS MUTANTS PASS killed={len(mutants)}')
