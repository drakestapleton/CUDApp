# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External source-mutant verification, not a production dependency.
import hashlib,os,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
source=(root/'om/prefill-shape/reading.om').read_text()
mutants={
 'reading_threshold':('if le rows 64\nret 4','if le rows 63\nret 4'),
 'reading_group':('if le rows 64\nret 0','if le rows 65\nret 0'),
 'reading_wait':('if eq index 2\nret 45','if eq index 2\nret 0'),
 'reading_zero':('if or eq rows 0 gt rows 4294967295','if or eq rows 1 gt rows 4294967295'),
 'reading_index':('if gt index 3','if gt index 4'),

}
ident=(root/'src/cudapp_om_prefill_reading.ident').read_text().strip()
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
  p=subprocess.run(['python3',str(root/'om/prefill-shape/reading-check.py'),str(so)],capture_output=True,text=True)
  assert p.returncode!=0,name+' escaped oracle'
  assert 'AssertionError' in p.stderr,(name,p.stderr)
  print(name+': caught',flush=True)
