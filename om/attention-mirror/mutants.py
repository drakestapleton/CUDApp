# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External source-mutant verification, not a production dependency.
import hashlib,os,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
source=(root/'om/attention-mirror/plan.om').read_text()
mutants={
 'lose_plane_slack':('add mul maxpos 2048 8256','mul maxpos 2048'),
 'admit_wrong_shape':('ne qheads 24','eq qheads 24'),
 'lose_position_bound':('if gt rows sub maxpos pos','if lt rows sub maxpos pos'),
 'copy_tiny_requests':('lt rows 1024','lt rows 1'),
 'copy_one_plane':('ret mul plane 2','ret plane'),
}


ident=(root/'src/cudapp_om_attn_mirror_plan.ident').read_text().strip()
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
  subprocess.run([os.environ.get('CC','cc'),'-O2','-shared','-fPIC',str(c),str(root/'src/cudapp_om_attn_mirror_step.c'),'-o',str(so)],check=True)
  p=subprocess.run(['python3',str(root/'om/attention-mirror/check.py'),str(so)],capture_output=True,text=True)
  assert p.returncode!=0,name+' escaped oracle'
  assert 'AssertionError' in p.stderr,(name,p.stderr)
  print(name+': caught',flush=True)

source=(root/'om/attention-mirror/step.om').read_text()
mutants={
 'advance_failed_copy':('if ne ok 1','if gt ok 1'),
 'skip_copy':('ret 3','ret 4'),
 'reuse_too_small':('if gt need have','if lt need have'),
 'accept_short_allocation':('if lt have need','if gt have need'),
}
with tempfile.TemporaryDirectory(prefix='attention-step-mutants-') as t:
 t=Path(t)
 for name,(old,new) in mutants.items():
  assert source.count(old)==1
  om=t/(name+'.om');om.write_text(source.replace(old,new))
  c=t/(name+'.c');so=t/(name+'.so')
  with c.open('w') as f:subprocess.run(['sh',str(bridge),'c',str(om)],env=env,stdout=f,check=True)
  subprocess.run([os.environ.get('CC','cc'),'-O2','-shared','-fPIC',str(c),str(root/'src/cudapp_om_attn_mirror_plan.c'),'-o',str(so)],check=True)
  p=subprocess.run(['python3',str(root/'om/attention-mirror/check.py'),str(so)],capture_output=True,text=True)
  assert p.returncode!=0 and 'AssertionError' in p.stderr,(name,p.stderr)
  print(name+': caught',flush=True)
