#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compile source mutants, never mutate checked-in generated files."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
bridge = Path(os.environ['OMEGA_BRIDGE'])
env = dict(os.environ, OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
cc = shlex.split(os.environ.get('CC', 'cc'))
for name, old, new in (
    ('api_policy', 'gt a 8192', 'gt a 8193'),
    ('artifact_policy', 'art_word q0 12 0', 'art_word q0 13 0'),
    ('artifact_parse', 'ge get s 0 9', 'ge get s 0 10'),
):
    source = (root / 'om/components' / (name + '.om')).read_text()
    assert source.count(old) == 1
    with tempfile.TemporaryDirectory(prefix='cudapp-host-policy-') as tmp:
        tmp = Path(tmp)
        om = tmp / (name + '.om')
        om.write_text(source.replace(old, new))
        generated = tmp / (name + '.c')
        with generated.open('w') as out:
            subprocess.run(['sh', str(bridge / 'tools/bridge-seeded'), 'c', str(om)], env=env, stdout=out, check=True)
        other = 'artifact_policy' if name == 'api_policy' else 'api_policy'
        test = 'artifact_text_unit.c' if name == 'artifact_parse' else 'host_policy_unit.c'
        extra = [] if name == 'artifact_parse' else [str(root / 'src' / ('cudapp_om_' + other + '.c'))]
        exe = tmp / 'mutant'
        subprocess.run(cc + ['-O2', '-Wall', '-Wextra', '-Werror', '-I' + str(root / 'src'),
                       str(root / 'tests/runtime' / test), str(generated)] + extra + ['-o', str(exe)], check=True)
        result = subprocess.run([str(exe)], capture_output=True, text=True)
        marker = 'ARTIFACT TEXT' if name == 'artifact_parse' else 'HOST POLICY'
        assert result.returncode != 0 and marker + ' differential FAIL' in result.stderr, result
        print('HOST POLICY mutant caught:', name)
print('HOST POLICY mutants PASS')

source = (root / 'om/components/cm_sort.om').read_text()
old = 'if not cms_less get a j get a sub j 1'
assert source.count(old) == 1
with tempfile.TemporaryDirectory(prefix='cudapp-cm-sort-') as tmp:
    tmp = Path(tmp)
    om = tmp / 'cm_sort.om'
    om.write_text(source.replace(old, 'if cms_less get a j get a sub j 1'))
    generated = tmp / 'cm_sort.c'
    with generated.open('w') as out:
        subprocess.run(['sh', str(bridge / 'tools/bridge-seeded'), 'c', str(om)], env=env, stdout=out, check=True)
    exe = tmp / 'mutant'
    subprocess.run(cc + ['-O2', '-Wall', '-Wextra', '-Werror', '-I' + str(root / 'src'),
                   str(root / 'tests/runtime/cm_sort_unit.c'), str(generated), '-o', str(exe)], check=True)
    result = subprocess.run([str(exe)], capture_output=True, text=True)
    assert result.returncode != 0 and 'CM SORT differential FAIL' in result.stderr, result
    print('CM SORT mutant caught: reversed insertion order')
print('CM SORT mutants PASS')
