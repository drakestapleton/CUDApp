# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import os,pathlib,subprocess,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
src=(root/'om/graph/table.om').read_text(); bridge=pathlib.Path(os.environ['OMEGA_BRIDGE']);cc=os.environ.get('CC','cc')
env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
old='end\nret i\nend\nfn grpslot'
mutants=[('original-tail',old,'end\nret add r sub add 128 j p\nend\nfn grpslot'),('tail-zero',old,'end\nret 0\nend\nfn grpslot'),('tail-plus-one',old,'end\nret add i 1\nend\nfn grpslot'),('pair-boundary','if lt j p','if le j p'),('rotary-half','p = div r 2','p = div r 4')]
with tempfile.TemporaryDirectory() as td:
 d=pathlib.Path(td)
 def run(s,name):
  om=d/'table.om';om.write_text(s);c=d/'table.c'
  with c.open('w') as out:subprocess.run(['sh',str(bridge/'tools/bridge-seeded'),'--runtime-views','c',str(om)],env=env,stdout=out,check=True)
  subprocess.run([cc,'-O2','-I'+str(root/'src'),str(root/'tests/test_graph_table_perm.c'),str(c),'-o',str(d/'test')],check=True)
  return subprocess.run([str(d/'test')]).returncode
 if run(src,'green'):raise SystemExit('FAIL original')
 for name,a,b in mutants:
  assert src.count(a)==1
  if run(src.replace(a,b),name)==0:raise SystemExit('FAIL survived '+name)
  print('KILLED '+name)
print('PASS graph table five compiled mutants')
