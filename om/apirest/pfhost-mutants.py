# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import os,pathlib,subprocess,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
source=(root/'om/apirest/pfhost.om').read_text()
entry=(root/'om/apirest/pfreg.om').read_text()
env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
cc=os.environ.get('CC','cc'); bridge=pathlib.Path(os.environ['OMEGA_BRIDGE'])
mutants=[('limit','65536','65535'),('warp','ulong 32','ulong 31'),('compare','if gt mul','if ge mul'),('poison','-999.0f32','-998.0f32'),('count','add n 1','add n 2'),('skip','len x','0')]
with tempfile.TemporaryDirectory() as td:
 d=pathlib.Path(td)
 def build(src,name):
  files=[]
  for unit,suffix in [('count',''),('reg',entry)]:
   om=d/(unit+'.om');om.write_text(src+'\n'+suffix)
   c=d/(unit+'.c')
   with c.open('w') as out: subprocess.run(['sh',str(bridge/'tools/bridge-seeded'),'--runtime-views','c',str(om)],env=env,stdout=out,check=True)
   files.append(str(c))
  exe=d/name
  subprocess.run([cc,'-O2','-ffp-contract=off','-I'+str(root/'src'),str(root/'tests/test_prefill_host_rest.c'),*files,'-o',str(exe)],check=True)
  return subprocess.run([str(exe)]).returncode
 if build(source,'green'): raise SystemExit('FAIL original')
 p=subprocess.run([cc,'-I'+str(root/'src'),str(root/'tests/test_prefill_host_rest.c'),'-o',str(d/'missing')],capture_output=True)
 if p.returncode==0: raise SystemExit('FAIL missing binding accepted')
 print('PASS strong binding negative control')
 for name,old,new in mutants:
  assert source.count(old)==1
  if build(source.replace(old,new),name)==0: raise SystemExit('FAIL survived '+name)
  print('KILLED '+name)
print('PASS prefill host red control and six mutants')
