#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Run inside bwrap. Mutate temporary Omega compositions, never tracked files."""
import os
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
bridge = Path(os.environ['OMEGA_BRIDGE'])
env = dict(os.environ, OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
build = root / 'build/api-rest'
cases = [
 ('launch-low-half', 'ar_launch', ['om/apirest/launch.om'], 'uint and get q 0 4294967295', 'uint get q 0'),
 ('cache-invalid', 'ar_cache', ['om/apirest/cache.om'], 'if eq get a add b get q 2 0', 'if ne get a add b get q 2 0'),
 ('pinned-victim', 'ar_victim', ['om/apirest/victim.om'], 'if eq get x add b get q 3 0', 'if ne get x add b get q 3 0'),
 ('upload-byte-count', 'ar_mstep', ['om/apirest/data.om', 'om/api/apish_shp.om', 'om/apirest/mstep.om'], 'put q 12 mul get q 11 2', 'put q 12 mul get q 11 4'),
 ('swiglu-copy-order', 'ar_pfcopy', ['om/apirest/data.om', 'om/api/apish_shp.om', 'om/apirest/pfcopy.om'], 'src = swsrc uint p uint get q 0', 'src = swsrc uint get q 0 uint p'),
 ('swiglu-ratio-order', 'ar_pfratio', ['om/apirest/data.om', 'om/api/apish_shp.om', 'om/apirest/pfratio.om'], 'p = swsrc uint p uint get q 0', 'p = swsrc uint get q 0 uint p'),
 ('slot-pin-wrap', 'ar_slot', ['om/apirest/data.om', 'om/api/apish_slot.om', 'om/apirest/slot.om'], 'uint and get rec 1 4294967295', 'uint get rec 1'),
]
subprocess.run(['sh', 'tests/api_rest_run.sh'], cwd=root, check=True)
with tempfile.TemporaryDirectory(prefix='api-rest-mutants-') as tmp:
 tmp = Path(tmp)
 for name, unit, parts, before, after in cases:
  text = '\n'.join((root / p).read_text() for p in parts)
  if text.count(before) != 1:
   raise SystemExit(f'FAIL {name}: mutation site count {text.count(before)}')
  src = tmp / (name + '.om'); src.write_text(text.replace(before, after))
  c = tmp / (name + '.c')
  with c.open('w') as out:
   subprocess.run(['sh', str(bridge / 'tools/bridge-seeded'), '--runtime-views', 'c', str(src)], cwd=root, env=env, stdout=out, check=True)
  obj = tmp / (name + '.o')
  subprocess.run(['cc', '-std=gnu11', '-O2', '-ffp-contract=off', '-c', str(c), '-o', str(obj)], check=True)
  objects = [str(p) for p in sorted(build.glob('*.o')) if p.name != 'cudapp_om_' + unit + '.o']
  exe = tmp / name
  subprocess.run(['cc', '-o', str(exe), *objects, str(obj), str(root/'build/libomega_gpu.a'), '-lm', '-lpthread'], check=True)
  p = subprocess.run([str(exe)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
  if p.returncode == 0:
   raise SystemExit(f'FAIL mutant survived: {name}')
  print(f'KILLED {name} rc={p.returncode} {p.stdout.strip()}', flush=True)
print(f'PASS api-rest mutants killed={len(cases)}')
