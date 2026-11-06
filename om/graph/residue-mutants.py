#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compile semantic Omega mutants and require the real differential to turn red.
Run only inside the documented bwrap environment, including every child.
"""
import os
from pathlib import Path
import shlex
import subprocess

root = Path(__file__).resolve().parents[2]
os.chdir(root)
out = Path(os.environ.get('OUT_DIR', 'build')).resolve()
work = out / 'graph-residue-mutants'
work.mkdir(parents=True, exist_ok=True)
def run(args, **kw):
    return subprocess.run(args, check=True, **kw)
expected = subprocess.check_output([str(out / 'graph-residue')])
mutants = [
    ('poison', 'hello_host', 'hello.om', 'uint 4290838510', 'uint 4290838509'),
    ('input-period', 'hello_host', 'hello.om', 'rem i 7', 'rem i 6'),
    ('checker-sub', 'hello_host', 'hello.om', 'bitcast uint add x y', 'bitcast uint sub x y'),
    ('shape-cap', 'model_graph_residue', 'residue.om', 'gt hd 5120', 'gt hd 4096'),
    ('timeline-room', 'model_graph_residue', 'residue.om', 'sub get c 2 get c 0', 'add sub get c 2 get c 0 1'),
    ('perm-stride', 'model_graph_table', 'table.om', 'mul rem div j 4 32 16', 'mul rem div j 4 32 8'),
    ('load-scoreboard', 'hello_plan', 'hello-plan.om', 'wb = 0', 'wb = 1'),
    ('d8-own-round', 'hello_plan', 'hello-plan.om', 'ownk = div add get c 3 1023 1024', 'ownk = div get c 3 1024'),
    ('d8-beyond-sign', 'hello_plan', 'hello-plan.om', 'put d 4 1', 'put d 4 0'),
    ('d8-driver-kib', 'hello_plan', 'hello-plan.om', 'put d 5 div add get c 4 1023 1024', 'put d 5 div get c 4 1000'),
]
for name, unit, filename, old, new in mutants:
    source = (root / 'om/graph' / filename).read_text()
    if source.count(old) != 1:
        raise SystemExit(f'FAIL {name}: mutation site count {source.count(old)}')
    lane = work / name
    lane.mkdir(exist_ok=True)
    om = lane / 'mutant.om'
    om.write_text(source.replace(old, new))
    env = dict(os.environ, OMEGA_UNITS=f'{unit}={om}', OMEGA_UNITS_DEST=str(lane))
    with (lane/'generate.log').open('w') as log:
        run(['sh', 'om/units.sh'], env=env, stdout=log, stderr=log)
    binary = lane / 'test'
    with (lane/'compile.log').open('w') as log:
        run(shlex.split(os.environ.get('CC','cc')) + shlex.split(os.environ['GRAPH_RESIDUE_FLAGS']) +
            ['tests/test_graph_residue.c', str(lane/f'cudapp_om_{unit}.c'), str(out/'libomega_gpu.a'),
             '-lpthread','-lm','-o',str(binary)], stdout=log, stderr=log)
    result = subprocess.run([str(binary)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    (lane/'run.log').write_bytes(result.stdout)
    if result.returncode == 0 and result.stdout == expected:
        raise SystemExit(f'FAIL mutant {name} survived')
    print(f'mutant {name} killed rc={result.returncode}', flush=True)
print(f'PASS graph residue: {len(mutants)} compiled semantic mutants killed')
