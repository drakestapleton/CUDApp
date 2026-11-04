# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External source-mutant verification, not a production dependency.
import hashlib,os,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
source=(root/'om/prefill-shape/admit.om').read_text()
mutants={
 'drop_column_offset':('last = add last 36','last = add last 0'),
 'overflow_offset_boundary':('le last 8388607','le last 8388643'),
 'drop_last_row_group':('sub mul rt 16 8','sub mul rt 16 16'),
 'drop_large_shape':('eq rt 8 eq warps 8','eq rt 7 eq warps 8'),
 'admit_large_overbudget_warps':('eq rt 8 eq warps 8','eq rt 8 eq warps 16'),
 'drop_small_shape':('eq rt 2 eq warps 4','eq rt 3 eq warps 4'),
 'drop_incumbent':('eq rt 4 eq warps 8','eq rt 5 eq warps 8'),
 'admit_unsafe_one_row_group':('eq rt 2 eq warps 4','eq rt 1 eq warps 2'),
 'ignore_warp_count':('eq rt 2 eq warps 4','eq rt 2 eq rt 2'),
}
ident=(root/'src/cudapp_om_prefill_shape.ident').read_text().strip()
pins=dict(part.split('=',1) for part in ident.split() if '=' in part)
assert hashlib.sha256(Path(os.environ['OMEGA_OMC']).read_bytes()).hexdigest()==pins['omc']
env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
bridge=Path(os.environ['OMEGA_BRIDGE'])/'tools/bridge-seeded'
assert hashlib.sha256(bridge.read_bytes()).hexdigest()==pins['frontend']
assert hashlib.sha256((bridge.parent.parent/'src/toc.om').read_bytes()).hexdigest()==pins['converter']
with tempfile.TemporaryDirectory(prefix='prefill-shape-mutants-') as t:
 t=Path(t)
 for name,(old,new) in mutants.items():
  assert source.count(old)==1
  om=t/(name+'.om');om.write_text(source.replace(old,new))
  c=t/(name+'.c');so=t/(name+'.so')
  with c.open('w') as f:subprocess.run(['sh',str(bridge),'c',str(om)],env=env,stdout=f,check=True)
  subprocess.run([os.environ.get('CC','cc'),'-O2','-shared','-fPIC',str(c),'-o',str(so)],check=True)
  p=subprocess.run(['python3',str(root/'om/prefill-shape/check.py'),str(so)],capture_output=True,text=True)
  assert p.returncode!=0,name+' escaped oracle'
  assert 'AssertionError' in p.stderr,(name,p.stderr)
  print(name+': caught',flush=True)
