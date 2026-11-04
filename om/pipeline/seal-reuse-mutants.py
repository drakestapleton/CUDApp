# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External source-mutant verification, not a production dependency.
import hashlib,os,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
source=(root/'om/pipeline/seal_reuse.om').read_text()
mutants={
 'ignore_all_bytes':('same = false','same = same'),
 'ignore_count':('same = and eq get cache 0 1 eq ulong get cache 1 n','same = eq get cache 0 1'),
 'ignore_one_byte':('if ne get b i get cache add 64 i\nsame = false\nend','if ne i 1024\nif ne get b i get cache add 64 i\nsame = false\nend\nend'),
 'wrong_normalization':('and ge i 48 lt i 52','and ge i 49 lt i 52'),
 'dirty_refusal':('mc_clear edit out\nif or eq n 0','if or eq n 0'),
}
ident=(root/'src/cudapp_om_closure_reseal.ident').read_text().strip()
pins=dict(part.split('=',1) for part in ident.split() if '=' in part)
assert hashlib.sha256(Path(os.environ['OMEGA_OMC']).read_bytes()).hexdigest()==pins['omc']
paths=ident.split(' path=',1)[1].split('=',1)[1].split(',')
assert paths[-1]=='om/pipeline/seal_reuse.om'
prefix='\n'.join((root/p).read_text() for p in paths[:-1])+'\n'
env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
bridge=Path(os.environ['OMEGA_BRIDGE'])/'tools/bridge-seeded'
assert hashlib.sha256(bridge.read_bytes()).hexdigest()==pins['frontend']
assert hashlib.sha256((bridge.parent.parent/'src/toc.om').read_bytes()).hexdigest()==pins['converter']
with tempfile.TemporaryDirectory(prefix='seal-reuse-mutants-') as t:
 t=Path(t)
 for name,(old,new) in mutants.items():
  assert source.count(old)==1,(name,source.count(old))
  om=t/(name+'.om');om.write_text(prefix+source.replace(old,new))
  c=t/(name+'.c');so=t/(name+'.so')
  with c.open('w') as f:subprocess.run(['sh',str(bridge),'c',str(om)],env=env,stdout=f,check=True)
  subprocess.run([os.environ.get('CC','cc'),os.environ.get('CUDAPP_SEAL_OPT','-O3'),'-shared','-fPIC',str(c),'-o',str(so)],check=True)
  p=subprocess.run(['python3',str(root/'om/pipeline/seal-reuse-check.py'),str(so)],capture_output=True,text=True)
  assert p.returncode!=0,name+' escaped oracle'
  assert 'AssertionError' in p.stderr,(name,p.stderr)
  print(name+': caught',flush=True)
