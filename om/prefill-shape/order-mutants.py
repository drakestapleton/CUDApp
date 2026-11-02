# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External source-mutant verification, not a production dependency.
import hashlib,os,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
source=(root/'om/prefill-shape/order.om').read_text()
mutants={
 'drop_tail_guard':('if eq guard 1','if eq guard 0'),
 'floor_grid':('div add tiles sub size 1 size','div tiles size'),
 'bad_row_group':('at 3 37 size','at 3 37 1'),
 'bad_column_shift':('at 2 0 group','at 2 0 0'),
 'bad_row_mask':('sub size 1','sub size 2'),
}

ident=(root/'src/cudapp_om_prefill_order.ident').read_text().strip()
pins=dict(part.split('=',1) for part in ident.split() if '=' in part)
assert hashlib.sha256(Path(os.environ['OMEGA_OMC']).read_bytes()).hexdigest()==pins['omc']
env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
bridge=Path(os.environ['OMEGA_BRIDGE'])/'tools/bridge-seeded'
assert hashlib.sha256(bridge.read_bytes()).hexdigest()==pins['frontend']
assert hashlib.sha256((bridge.parent.parent/'src/toc.om').read_bytes()).hexdigest()==pins['converter']
with tempfile.TemporaryDirectory(prefix='prefill-order-mutants-') as t:
 t=Path(t)
 for name,(old,new) in mutants.items():
  assert source.count(old)>=1
  om=t/(name+'.om');om.write_text(source.replace(old,new,1))
  c=t/(name+'.c');so=t/(name+'.so')
  with c.open('w') as f:subprocess.run(['sh',str(bridge),'c',str(om)],env=env,stdout=f,check=True)
  subprocess.run([os.environ.get('CC','cc'),'-O2','-shared','-fPIC',str(c),'-o',str(so)],check=True)
  p=subprocess.run(['python3',str(root/'om/prefill-shape/order-check.py'),str(so)],capture_output=True,text=True)
  assert p.returncode!=0,name+' escaped oracle'
  assert 'AssertionError' in p.stderr,(name,p.stderr)
  print(name+': caught',flush=True)
