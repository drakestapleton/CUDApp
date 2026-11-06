#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Recompile five actual graph-bank semantic changes; the independent C oracle must reject each."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'om/graph/record.om'
MUTANTS=[
 ('low-address-bit','put w mul i 2 uint and get v i 4294967295','put w mul i 2 uint and get v i 4294967294'),
 ('high-address-shift','put w add mul i 2 1 uint shr get v i 32','put w add mul i 2 1 uint shr get v i 31'),
 ('norm-epsilon-binding','put w 10 uint and get v 5 4294967295\nput w 11','put w 10 uint and get v 6 4294967295\nput w 11'),
 ('swiglu-scale-binding','put w 4 uint and get v 2 4294967295','put w 4 uint and get v 3 4294967295'),
 ('gate-length-binding','put w 6 uint and get v 3 4294967295','put w 6 uint and get v 4 4294967295'),
]
def run(cmd,**kw):
 print('+',shlex.join(map(str,cmd)),flush=True)
 return subprocess.run(list(map(str,cmd)),cwd=ROOT,**kw)
source=SOURCE.read_text()
for name,old,new in MUTANTS:
 if source.count(old)!=1:raise SystemExit(f'{name}: expected exactly one mutation site')
 with tempfile.TemporaryDirectory(prefix='model-graph-mutant-') as tmp:
  tmp=Path(tmp);mut=tmp/'record.om';mut.write_text(source.replace(old,new));dest=tmp/'gen'
  dest.mkdir()
  env=dict(os.environ,OMEGA_UNITS=f'model_graph_wire={mut},om/graph/entry_wire.om',OMEGA_UNITS_DEST=str(dest))
  if run(['sh','om/units.sh'],env=env).returncode:raise SystemExit(f'{name}: compilation failed, NOT killed')
  binary=tmp/'check'
  if run(shlex.split(os.environ.get('CC','cc'))+['-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Isrc','tests/test_model_graphs_om.c',dest/'cudapp_om_model_graph_wire.c','-o',binary]).returncode:raise SystemExit(f'{name}: host compilation failed, NOT killed')
  result=run([binary],capture_output=True,text=True)
  print(result.stdout,end='');print(result.stderr,end='')
  if result.returncode!=1 or 'FAIL graph wire' not in result.stderr:raise SystemExit(f'{name}: mutant survived or unrelated failure rc={result.returncode}')
  print(f'MODEL-GRAPHS mutant {name} KILLED',flush=True)
print('MODEL-GRAPHS mutants PASS killed=5 compiled=5')
