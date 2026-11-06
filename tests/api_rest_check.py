#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Device-free residual migration differential and source-mutant gate.
Run inside the device-hiding bwrap invocation recorded in the lane evidence.
"""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--out');p.add_argument('--mutants',action='store_true');a=p.parse_args()
tmp=None if a.out else tempfile.TemporaryDirectory(prefix='api-rest-')
out=Path(a.out if a.out else tmp.name).resolve();out.mkdir(parents=True,exist_ok=True);os.chdir(ROOT)
commands=[]
def run(cmd,expect=0):
    cmd=[str(x) for x in cmd];r=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    commands.append({'command':cmd,'rc':r.returncode,'output':r.stdout});(out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
    if r.stdout:print(r.stdout,end='',flush=True)
    if expect is not None and r.returncode!=expect:raise SystemExit('FAIL command '+str(cmd))
    return r
cc=os.environ.get('CC','cc');flags=['-std=gnu11','-O2','-ffp-contract=off','-ffunction-sections','-fdata-sections','-Isrc','-Itests']
units=dict(re.findall(r'^UNITS="\$UNITS (attn_(?:ds|na|mm)_\w+)=([^"\n]+)"$',(ROOT/'om/units.sh').read_text(),re.M))
objects={}
for name in units:
    obj=out/(name+'.o');run([cc,*flags,'-c','src/cudapp_om_'+name+'.c','-o',obj]);objects[name]=obj
# parse_semver's existing Omega root is outside this residual slice.
obj=out/'artifact_parse.o';run([cc,*flags,'-c','src/cudapp_om_artifact_parse.c','-o',obj]);objects['artifact_parse']=obj
obj=out/'attn_buf.o';run([cc,*flags,'-c','src/cudapp_om_attn_buf.c','-o',obj]);objects['attn_buf']=obj
obj=out/'blackwell_matmul.o';run([cc,*flags,'-c','src/omega_blackwell_matmul.c','-o',obj]);objects['blackwell_matmul']=obj
obj=out/'decode_scalar.o';run([cc,*flags,'-c','src/omega_gpu_decode_attn_api.c','-o',obj])
run([os.environ.get('OBJCOPY','objcopy'),'-G','omega_dattn_bf16_rne',obj]);objects['decode_scalar']=obj
fixtures=[]
for kind,names in [('art',['cubin','layout','version','json']),('mm',['oracle'])]:
    for prefix in ['om','c']:
        obj=out/(kind+'-'+prefix+'.o');extra=['-DREST(x)='+prefix+'_ ## x']
        if prefix=='c':extra+=['-DCUDAPP_MIGRATION_PENDING_C_NATIVE_ARTIFACT_REST=1','-DCUDAPP_MIGRATION_PENDING_C_ARTIFACT=1','-DCUDAPP_MIGRATION_PENDING_C_MATMUL_ORACLE_GUARD=1']
        run([cc,*flags,*extra,'-c','tests/api_rest_'+kind+'_fixture.c','-o',obj])
        # Localize unrelated API exports in the fixture, keeping only accessors.
        keep=[]
        for name in names:keep+=['-G',prefix+'_'+name]
        run([os.environ.get('OBJCOPY','objcopy'),*keep,obj]);fixtures.append(obj)
main=out/'main.o';run([cc,*flags,'-c','tests/api_rest_diff.c','-o',main])
def link(dest,replacements=None,omit=None,expect=0):
    objs=dict(objects);objs.update(replacements or {})
    if omit:del objs[omit]
    return run([cc,'-Wl,--gc-sections',main,*fixtures,*objs.values(),'-lm','-o',dest],expect=expect)
link(out/'diff');run([out/'diff'])
# Strong-reference red control: omitting a migrated implementation must not link.
for name in ['attn_ds_round','attn_na_cubin','attn_mm_dot']:
    r=link(out/'missing',omit=name,expect=None)
    if not r.returncode:raise SystemExit('FAIL missing-unit linked '+name)
    print('PASS strong-link red '+name,flush=True)
if a.mutants:
    mutants=[('attn_ds_round','fn av_drnd u uint to uint\nif gx_fin u','fn av_drnd u uint to uint\nif not gx_fin u'),('attn_ds_slot','p = div r 2','p = div r 3'),('attn_ds_vecf','ret av_permf s d r','ret av_permf s d 256'),('attn_na_cubin','ret ar_eq b off txt','ret true'),('attn_na_layout','if ge len b cap','if ge len b 1'),('attn_na_ver','ret gt get h i get n i','ret lt get h i get n i'),('attn_na_json','if or lt c 32 eq c 127','if lt c 32'),('attn_mm_dot','ret bitcast ulong mul x y','ret bitcast ulong add x y'),('attn_mm_err','if gt rel 0.00001','if gt rel 100.0')]
    env=os.environ.copy();env['OMEGA_SEEDED_OMC']=env['OMEGA_OMC'];rows=[]
    for num,(name,site,replacement) in enumerate(mutants):
        source='\n'.join((ROOT/x).read_text() for x in units[name].split(','))+'\n'
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
print('API-REST-CHECK PASS',flush=True)
