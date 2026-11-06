#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Mutate copies only; regenerate each with the pinned seeded compiler and exercise the real replay path."""
import os
import pathlib
import shlex
import subprocess

root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(os.environ.get('CODEGEN_BASE_OUT', root / 'build/codegen-base-om'))
objects = shlex.split(os.environ['CODEGEN_BASE_OBJECTS'])
flags = shlex.split('-std=gnu11 -O2 -Wall -Wextra -Werror -ffp-contract=off -ffunction-sections -fdata-sections -Isrc')
cc = shlex.split(os.environ.get('CC', 'cc'))
mutants = [
    ('cg_fields', 'put rec 6 or get raw 6 shl get raw 7 32', 'put rec 6 or get raw 6 shl get raw 7 31'),
    ('cg_replay', 'if gt get ins b maxop', 'if ge get ins b maxop'),
    ('cg_life', 'put life add b 1 ulong e', 'put life add b 1 ulong s'),
    ('cg_invar', 'ret 6', 'ret 0'),
    ('cg_budget', 'if gt mul decl threads 65536', 'if gt mul decl threads 65535'),
]
for unit, old, new in mutants:
    source = (root / 'om/codegen' / (unit + '.om')).read_text()
    if source.count(old) != 1:
        raise SystemExit('FAIL: mutant site is not unique: ' + unit)
    d = out / ('mutant-' + unit)
    d.mkdir(exist_ok=True)
    om = d / (unit + '.om')
    om.write_text(source.replace(old, new))
    env = dict(os.environ, OMEGA_UNITS=str(om), OMEGA_UNITS_DEST=str(d))
    with (d / 'generate.log').open('w') as log:
        subprocess.run(['sh', 'om/units.sh'], cwd=root, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
    obj = d / (unit + '.o')
    providers = sorted(d.glob('cudapp_*_' + unit + '.c'))
    assert len(providers) == 1, providers
    subprocess.run(cc + flags + ['-c', str(providers[0]), '-o', str(obj)], cwd=root, check=True)
    replace = providers[0].with_suffix('.o').name
    selected = [str(obj) if pathlib.Path(o).name == replace else o for o in objects]
    assert sum(pathlib.Path(o).name == replace for o in objects) == 1
    exe = d / 'differential'
    subprocess.run(cc + flags + ['tests/codegen_base/differential.c'] + selected + ['--coverage', '-Wl,--gc-sections', '-o', str(exe)], cwd=root, check=True)
    with (d / 'run.log').open('w') as log:
        result = subprocess.run([str(exe)], cwd=root, stdout=log, stderr=subprocess.STDOUT)
    if result.returncode == 0:
        raise SystemExit('FAIL: mutant survived: ' + unit)
    print('KILLED', unit, 'rc', result.returncode, flush=True)
print('CODEGEN BASE MUTANTS PASS: 5 killed')
