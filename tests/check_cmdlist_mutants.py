#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compile five altered Omega planners; require behavioral (not build) failures."""
import os
from pathlib import Path
import shlex
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
work = root / os.environ.get('OUT_DIR', 'build') / 'om-cmdlist' / 'mutants'
work.mkdir(parents=True, exist_ok=True)
source = (root / 'om/cmdlist/plan.om').read_text()
mutants = [
    ('drop-required-barrier', 'h = clhazards p i get o 3', 'h = ulong 0', 'sim'),
    ('capacity-off-by-one', 'if gt get a 0 get a 1', 'if ge get a 0 get a 1', 'planner'),
    ('wrong-RAW-class', 'h = or h 1', 'h = or h 2', 'planner'),
    ('adjacent-only', 'mul add start j 49', 'mul sub i 1 49', 'planner'),
    ('under-count-pushbuffer', 'put o 0 35', 'put o 0 34', 'planner'),
]
cc = shlex.split(os.environ.get('CC', 'cc'))
for name, old, new, check in mutants:
    if source.count(old) != 1:
        sys.exit('FAIL mutant site is not unique: ' + name)
    directory = work / name
    directory.mkdir(exist_ok=True)
    om = directory / 'plan.om'
    om.write_text(source.replace(old, new))
    env = dict(os.environ, OMEGA_UNITS='cmdlist_plan=' + str(om), OMEGA_UNITS_DEST=str(directory))
    with (directory / 'build.log').open('w') as log:
        subprocess.run(['sh', 'om/units.sh'], cwd=root, env=env, stdout=log, stderr=log, check=True)
        test = root / ('tests/cmdlist_sim_test.c' if check == 'sim' else 'tests/cmdlist_plan_test.c')
        command = cc + ['-std=gnu11', '-O2', '-ffp-contract=off', '-Isrc', '-Itests', str(test), str(directory/'cudapp_om_cmdlist_plan.c')]
        if check == 'sim':
            command += [str(root/os.environ.get('OUT_DIR','build')/'libomega_gpu.a'), '-lpthread', '-lm']
        else:
            command += ['src/cudapp_om_cmdlist_record.c', 'src/cudapp_om_cmdlist_layout.c', 'src/omega_blackwell_qmd.c']
        command += ['-o', str(directory/'check')]
        subprocess.run(command, cwd=root, stdout=log, stderr=log, check=True)
    run = subprocess.run([str(directory/'check')], cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    (directory/'run.log').write_text(run.stdout)
    if run.returncode != 1 or (check == 'sim' and 'FAIL sim data:' not in run.stdout):
        sys.exit('FAIL mutant survived or failed outside the data check: ' + name + '\n' + run.stdout)
    print('PASS red mutant: ' + name + ' rc=1 ' + run.stdout.splitlines()[-1])
print('PASS five compiled Omega mutants rejected by behavior')
