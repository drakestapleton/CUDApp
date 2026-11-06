#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Verification-only Omega source mutants; never edit canonical generated output."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
out = Path(sys.argv[1]).resolve()
parts = [root / 'om/codegen' / name for name in (
    'cg_pfhost_emit.om', 'cg_pfhost_rl.om', 'cg_pfhost_mc.om', 'cg_pfhost.om')]
mutants = (
    ('router-address', 1, 'ph_wide ins st 0 8 mul get cf 0 4 0', 'ph_wide ins st 0 8 mul get cf 0 8 0'),
    ('combine-address', 2, 'ph_wide ins st 1 5 mul get cf 1 4 1', 'ph_wide ins st 1 5 mul get cf 1 8 1'),
)
for name, index, before, after in mutants:
    with tempfile.TemporaryDirectory(prefix='pfhost-mutant-') as work:
        tmp = Path(work)
        src = parts[index].read_text()
        if src.count(before) != 1:
            raise SystemExit('FAIL mutant site must be unique: ' + name)
        mutant = tmp / parts[index].name
        mutant.write_text(src.replace(before, after))
        sources = parts.copy()
        sources[index] = mutant
        env = dict(os.environ, OMEGA_UNITS='cg_pfhost=' + ','.join(map(str, sources)), OMEGA_UNITS_DEST=str(tmp))
        subprocess.run(['sh', 'om/units.sh'], cwd=root, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run([os.environ.get('CC', 'cc'), '-O2', '-Wall', '-Wextra', '-Werror', '-Isrc', '-Itests',
                        'tests/codegen_pfhost_diff.c', str(tmp / 'cudapp_om_cg_pfhost.c'),
                        str(out / 'codegen_pfhost_oracle.o'), str(out / 'libomega_gpu.a'), '-lpthread', '-lm',
                        '-o', str(tmp / 'diff')], cwd=root, check=True)
        run = subprocess.run([str(tmp / 'diff'), '--quick'], cwd=root, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if run.returncode != 1 or not run.stdout.startswith('FAIL '):
            raise SystemExit('FAIL mutant not killed by differential: ' + name + '\n' + run.stdout)
        print('PASS Omega mutant killed: ' + name + ': ' + run.stdout.strip(), flush=True)
print('PASS pfhost Omega source mutants killed', flush=True)
