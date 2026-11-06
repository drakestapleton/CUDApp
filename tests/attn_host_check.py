#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Run only inside the device-hiding sandbox documented in the evidence."""
import argparse,json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--out');p.add_argument('--mutants',action='store_true');a=p.parse_args()
tmp=None if a.out else tempfile.TemporaryDirectory(prefix='attn-host-')
out=Path(a.out or tmp.name).resolve();out.mkdir(parents=True,exist_ok=True);os.chdir(ROOT)
commands=[]
def run(cmd,expect=0):
 cmd=[str(x) for x in cmd];r=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 commands.append({'command':cmd,'rc':r.returncode,'output':r.stdout});(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 if r.stdout:print(r.stdout,end='',flush=True)
 if expect is not None and r.returncode!=expect:raise SystemExit('FAIL command '+str(cmd))
 return r
cc=os.environ.get('CC','cc');flags=['-std=gnu11','-O2','-ffp-contract=off','-Isrc']
units={'attn_host':'om/api/attn_host.om','attn_scanchk':'om/scan/scan_check.om','attn_clock':'om/api/clock_ns.om'}
objects={}
for name in units:
 obj=out/(name+'.o');run([cc,*flags,'-c','src/cudapp_om_'+name+'.c','-o',obj]);objects[name]=obj
fixtures=[]
for prefix in ['om','c']:
 obj=out/(prefix+'.o');extra=['-DHOST(x)='+prefix+'_ ## x']
 if prefix=='c':extra+=['-DCUDAPP_MIGRATION_PENDING_C_'+name+'=1' for name in ['ATTENTION_HOST_POLICY','SCAN_CHECK_REST','CLOCK_SCALARS']]
 run([cc,*flags,*extra,'-c','tests/attn_host_fixture.c','-o',obj]);fixtures.append(obj)
main=out/'main.o';run([cc,*flags,'-c','tests/attn_host_diff.c','-o',main])
def link(dest,replacements=None,omit=None,expect=0):
 objs=dict(objects);objs.update(replacements or {})
 if omit:del objs[omit]
 return run([cc,main,*fixtures,*objs.values(),'-o',dest],expect)
link(out/'diff');run([out/'diff'])
for name in units:
 r=link(out/'missing',omit=name,expect=None)
 if not r.returncode:raise SystemExit('FAIL missing-unit linked '+name)
 print('PASS strong-link red '+name,flush=True)
if a.mutants:
 mutants=[('attn_host','31 4294967264','30 4294967264'),('attn_host','put q 9 ulong mul v 16','put q 9 ulong mul v 8'),('attn_host','put q 9 add add mul get q 3 4 get q 1 idw','put q 9 add add mul get q 3 3 get q 1 idw'),('attn_host','if gt nr get q 3','if lt nr get q 3'),('attn_host','put q 21 ulong mul uint get q 12 4','put q 21 ulong mul uint get q 12 8'),('attn_scanchk','if eq get w i 2141195181','if eq get w i 2141195180'),('attn_scanchk','v = sub v get w sub i 1','v = add v get w sub i 1'),('attn_scanchk','if lt get w i get w sub i 1','if gt get w i get w sub i 1'),('attn_clock','mul s 1000000000','mul s 1000000')]
 env=os.environ.copy();env['OMEGA_SEEDED_OMC']=env['OMEGA_OMC'];rows=[]
 for num,(name,site,replacement) in enumerate(mutants):
  source=(ROOT/units[name]).read_text()
  if source.count(site)!=1:raise SystemExit('FAIL mutant site '+name)
  om=out/('mutant-'+str(num)+'.om');om.write_text(source.replace(site,replacement))
  cmd=['sh',str(Path(env['OMEGA_BRIDGE'])/'tools/bridge-seeded'),'--runtime-views','c',str(om)]
  r=subprocess.run(cmd,env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  commands.append({'command':cmd,'rc':r.returncode,'output':r.stderr})
  if r.returncode:raise SystemExit('FAIL mutant generation '+r.stderr)
  c=om.with_suffix('.c');c.write_text(r.stdout);obj=om.with_suffix('.o');run([cc,*flags,'-c',c,'-o',obj])
  exe=out/('mutant-'+str(num));link(exe,{name:obj});r=run([exe],expect=None)
  if not r.returncode:raise SystemExit('FAIL surviving mutant '+name)
  rows.append({'unit':name,'site':site,'replacement':replacement,'rc':r.returncode,'verdict':'KILLED'});print('KILLED '+name,flush=True)
 (out/'mutants.json').write_text(json.dumps(rows,indent=2)+'\n');print('PASS source mutants killed='+str(len(rows)),flush=True)
print('ATTN-HOST-CHECK PASS',flush=True)
