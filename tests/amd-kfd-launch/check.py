#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Independent CPU measuring/oracle harness, never a production launcher component.
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[2]
src=root/'backend/amd/kfd/launch'
if len(sys.argv)!=2 or not Path(sys.argv[1]).is_absolute():
    raise SystemExit(64)
omc=Path(sys.argv[1])
pin=(src/'COMPILER').read_text().splitlines()[1].split()[0]
if not omc.is_file() or hashlib.sha256(omc.read_bytes()).hexdigest()!=pin:
    print('REFUSED compiler digest',file=sys.stderr)
    raise SystemExit(69)
out=Path(os.environ.get('AMD_KFD_LAUNCH_OUT',root/'build/amd-kfd-launch/gate')).resolve()
out.mkdir(parents=True,exist_ok=True)
results=[]
def run(label,args,want=0,data=None):
    p=subprocess.run(list(map(str,args)),input=data,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=120)
    row={'case':label,'exit':p.returncode,'expected':want,'stderr':p.stderr.decode(errors='replace')}
    results.append(row)
    print(('PASS' if p.returncode==want else 'FAIL'),label,'exit',p.returncode,'expected',want,flush=True)
    (out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
    if p.returncode!=want:
        print(p.stdout.decode(errors='replace')); print(row['stderr']); raise SystemExit(1)
    return p
parts=['common','elf','device','dispatch','main']
body=''.join((src/(x+'.om')).read_text() for x in parts)+(root/'backend/amd/kfd/cli.om').read_text()
def compile(label,text,target='a64'):
    text=('target x86linux\n' if target=='x86' else '')+'om2\n'+text
    (out/(label+'.om')).write_text(text)
    p=run('compile '+label,[omc],data=text.encode())
    binary=out/label; binary.write_bytes(p.stdout); binary.chmod(0o700); return binary
qemu=shutil.which('qemu-x86_64')
bins={t:compile('launch-'+t,body,t) for t in ('a64','x86')}
wrong=out/'wrong-compiler'; marker=out/'WRONG-EXECUTED'
wrong.write_text('#!/bin/sh\ntouch '+str(marker)+'\n'); wrong.chmod(0o700)
wrongout=out/'wrong-output'; env=dict(os.environ,AMD_KFD_LAUNCH_OUT=str(wrongout))
p=subprocess.run([sys.executable,'-I',__file__,str(wrong)],env=env,capture_output=True)
assert p.returncode==69 and not marker.exists() and not wrongout.exists()
print('CONTROL wrong compiler refused before execution/output',flush=True)
# Build the repository-owned object writer, not an LLVM-generated replacement.
hs=run('compile object writer',[omc],data=(root/'backend/amd/gfx942/hsaco.om').read_bytes())
writer=out/'hsaco'; writer.write_bytes(hs.stdout); writer.chmod(0o700)
objects={}
def elfmeta(data):
    phoff=struct.unpack_from('<Q',data,32)[0]; phnum=struct.unpack_from('<H',data,56)[0]
    shoff=struct.unpack_from('<Q',data,40)[0]; shnum=struct.unpack_from('<H',data,60)[0]
    sections=[struct.unpack_from('<IIQQQQIIQQ',data,shoff+64*i) for i in range(shnum)]
    for sec in sections:
        if sec[1] not in (2,11): continue
        strings=sections[sec[6]]; table=data[strings[4]:strings[4]+strings[5]]
        for p in range(sec[4],sec[4]+sec[5],24):
            name,info,other,shndx,value,size=struct.unpack_from('<IBBHQQ',data,p)
            n=table[name:table.index(0,name)].decode()
            if n.endswith('.kd'):
                for i in range(phnum):
                    typ,flags,off,va,pa,filesz,memsz,align=struct.unpack_from('<IIQQQQQQ',data,phoff+56*i)
                    if typ==1 and va<=value and value+64<=va+filesz:
                        kd=off+value-va
                        return n[:-3],kd,struct.unpack_from('<I',data,kd+8)[0]
    raise AssertionError('descriptor missing')
for table in sorted((root/'backend/amd/gfx942').glob('*.isa')):
    data=run('build '+table.stem,[writer,table]).stdout
    obj=out/(table.stem+'.hsaco'); obj.write_bytes(data)
    name,kd,argbytes=elfmeta(data); objects[name]=(obj,data,kd,argbytes)
    for target,binary in bins.items():
        if target=='x86' and not qemu: continue
        argv=([qemu] if target=='x86' else [])+[binary,'inspect',obj,name]
        p=run(target+' inspect '+name,argv)
        assert ('kernarg_bytes='+str(argbytes)).encode() in p.stdout
vobj,vdata,kd,ka=objects['vadd']
# Record numeric constant and unit-conversion oracle queries/results, not guesses.
calcs={'ELF_magic':int.from_bytes(b'\x7fELF','little'),'ELF_class_data_version_osabi':int.from_bytes(bytes([2,1,1,64]),'little'),'ELF_type_machine':int.from_bytes(struct.pack('<HH',3,224),'little'),'manifest_magic':int.from_bytes(b'OMKFD001','little'),'f32_one':struct.pack('<f',1.0).hex(),'f32_one_plus_one':struct.pack('<f',1.0+1.0).hex()}
(out/'calculation-oracles.json').write_text(json.dumps(calcs,indent=2)+'\n')
mutations={ 'truncated':vdata[:63], 'wrong-machine':vdata[:18]+b'\0\0'+vdata[20:], 'wave32':vdata[:kd+56]+struct.pack('<H',struct.unpack_from('<H',vdata,kd+56)[0]|1024)+vdata[kd+58:], 'negative-entry':vdata[:kd+16]+struct.pack('<q',-1)+vdata[kd+24:] }
for label,data in mutations.items():
    pth=out/(label+'.hsaco'); pth.write_bytes(data)
    for target,binary in bins.items():
        if target=='x86' and not qemu: continue
        run(target+' refuse '+label,([qemu] if target=='x86' else [])+[binary,'inspect',pth,'vadd'],65)
for target,binary in bins.items():
    if target=='x86' and not qemu: continue
    pre=([qemu] if target=='x86' else [])
    run(target+' missing descriptor',pre+[binary,'inspect',vobj,'not_a_kernel'],65)
    if not Path('/dev/kfd').exists():
        p=run(target+' absent KFD',pre+[binary,'vadd',vobj,'vadd'],77)
        assert p.stdout==b'OMEGA device=absent errno=2\n'
    badargs=out/'bad-args'; badargs.write_bytes(b'\0'*48)
    run(target+' refuse argument header',pre+[binary,'run',vobj,'vadd',4,1,badargs,out/'unused'],64)
# Full native host orchestration with a syscall instrument: no GPU opens reach Linux.
run('compile UAPI layout oracle',['cc','-Wall','-Wextra','-Werror',root/'tests/amd-kfd/layout-oracle.c','-o',out/'layout'])
layout=run('UAPI layout',[out/'layout']).stdout
assert layout==(root/'backend/amd/kfd/requests.tsv').read_bytes()
for name in ('trace','publication'):
    run('compile '+name+' instrument',['cc','-Wall','-Wextra','-Werror',root/'tests/amd-kfd-launch'/ (name+'.c'),'-o',out/name])
with tempfile.TemporaryDirectory(prefix='kfd-',dir='/tmp') as tmp:
    node=Path(tmp)/'2'; node.mkdir()
    (node/'gpu_id').write_text('1\n')
    (node/'properties').write_text('gfx_target_version 90402\ndrm_render_minor 128\nsimd_count 32\nsimd_per_cu 4\narray_count 8\nsimd_arrays_per_engine 2\nnum_xcc 2\n')
    run('native simulated orchestration',[out/'trace',tmp,bins['a64'],'vadd',vobj,'vadd'])
    # The generic CLI reuses the ring across its boundary, not only one packet.
    ins=[]
    for name in ('a','b'):
        path=out/('repeat-'+name); path.write_bytes(struct.pack('<f',1.0)*256); ins.append(path)
    raw=bytes(24)+struct.pack('<I',256)
    pairs=b''.join(struct.pack('<QQ',i*8,i) for i in range(3))
    args=out/'repeat.args'
    args.write_bytes(b'OMKFD001'+struct.pack('<QQQQQ',28,1024,200,10_000_000_000,3)+pairs+raw)
    result=out/'repeat.out'
    invocation=[out/'trace',tmp,bins['a64'],'run',vobj,'vadd',4,1,args,*ins,result]
    repeated=run('native generic CLI ring wrap',invocation)
    assert b'packets=200\n' in repeated.stdout and result.read_bytes()==struct.pack('<f',2.0)*256
    args.write_bytes(b'OMKFD001'+struct.pack('<QQQQQ',28,1024,1,1,3)+pairs+raw)
    stopped=run('CONTROL missing device completion',['env','KFD_TEST_NO_COMPLETE=1',*invocation],1)
    assert b'child_exit=75 ' in stopped.stderr
    mutant=body.replace('hput 32 4 get dev 2\n','hput 32 4 add get dev 2 1\n')
    assert mutant!=body
    bad=compile('bad-gpu-record',mutant)
    run('CONTROL wrong GPU record',[out/'trace',tmp,bad,'vadd',vobj,'vadd'],1)
# Exercise packet publication separately at every native instruction, using anonymous
# CPU mappings. The observer verifies body -> header -> wptr -> doorbell, not final bytes alone.
unit=''.join((src/(x+'.om')).read_text() for x in ('common','elf','device','dispatch'))
unit+=(root/'backend/amd/kfd/cli.om').read_text()
unit+='''fn main to int
+t = checked mapnew -1 0 192 3 34
+put launch 0 ulong t
+for i ulong 8
+h = checked mapnew -1 0 4096 3 34
+r = checked mapset t mul i 24 8 ulong h
+end
+b = checked mapnew -1 0 4096 3 34
+put launch 8 ulong b
+r = checked mapset b 0 8 18446744073709551615
+put launch 5 4
+put launch 6 1
+put el 6 64
+r = checked mapset lh 3 0 4 1
+hput 0 8 mapaddr lh 3
+hput 8 8 mapaddr lh 0
+hput 16 8 mapaddr lh 1
+hput 24 8 mapaddr lh 7
+hput 32 8 mapaddr lh 4
+hput 40 8 mapaddr b
+r = checked write 1 hdr 0 48
+packet 0
+ret 0
+end
+'''.replace('\n+','\n')
u=compile('publication-unit',unit)
run('native publication order',[out/'publication',u])
line='r = checked mapwrite lh 3 add slot 4 hdr 4 60\n'
mutant=unit.replace(line,'').replace('r = checked mapset lh 3 slot 4 0x00021502\n','r = checked mapset lh 3 slot 4 0x00021502\n'+line)
assert mutant!=unit
u=compile('publication-early-header',mutant)
run('CONTROL early valid header',[out/'publication',u],1)
summary={'compiler':pin,'checks':len(results),'failures':sum(r['exit']!=r['expected'] for r in results),'qemu':qemu,'device':'NOT_RUN','orchestration':'native CPU syscall simulation only','publication':'native instruction-step observation only','objects':{name:hashlib.sha256(data).hexdigest() for name,(p,data,k,a) in objects.items()},'binaries':{name:hashlib.sha256(p.read_bytes()).hexdigest() for name,p in bins.items()}}
(out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print('AMD-KFD-LAUNCH CPU PASS',json.dumps(summary,sort_keys=True))
print('NOT_RUN every AMD device row: KFD queues, GPU mappings, completion, parity, timing, Qwen inference')
