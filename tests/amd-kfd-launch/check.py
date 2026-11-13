#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Independent CPU measuring/oracle harness, never a production launcher component.
import hashlib
import json
import os
import re
import statistics
import math
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
    if not label.startswith(('compile ', 'build ')):
        row['stdout']=p.stdout.decode(errors='replace')
    results.append(row)
    print(('PASS' if p.returncode==want else 'FAIL'),label,'exit',p.returncode,'expected',want,flush=True)
    (out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
    if p.returncode!=want:
        print(p.stdout.decode(errors='replace')); print(row['stderr']); raise SystemExit(1)
    return p
reference=root/'tests/amd-kfd-launch/reference'
for line in (reference/'SHA256SUMS').read_text().splitlines():
    digest,name=line.split()
    assert hashlib.sha256((reference/name).read_bytes()).hexdigest()==digest,name
source=(reference/'topology-rocm-6.2.0.c').read_text().splitlines()
assert 'array_count' in source[1146] and 'simd_arrays_per_engine' in source[1148]
assert 'NumArrays' in source[1203] and 'NumShaderBanks' in source[1204]
source=(reference/'topology-rocm-7.2.2.c').read_text().splitlines()
assert 'array_count' in source[1170] and 'simd_arrays_per_engine' in source[1172]
assert 'NumArrays' in source[1233] and 'NumShaderBanks' in source[1234]
print('PRIMARY SOURCE PASS producer mapping and pinned files',flush=True)
parts=['common','elf','device','dispatch','plan','main']
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
# A card model run is releasable only when every exercised object rebuilds here.
required=json.loads((root/'tests/amd-kfd-launch/qwen38-objects.json').read_text())['objects']
def require_model_objects(actual):
    missing=sorted(set(required)-set(actual))
    assert not missing,('model kernels absent from release source',missing)
    for name,row in required.items():
        assert hashlib.sha256(actual[name][1]).hexdigest()==row['sha256'],('model object differs from card fixture',name)
        assert actual[name][3]==row['kernarg'],('model kernarg differs from card fixture',name)
require_model_objects(objects)
first=next(iter(required))
for kind in ('missing','changed'):
    altered=objects.copy()
    if kind=='missing':del altered[first]
    else:
        v=list(altered[first]);v[1]=bytes([v[1][0]^1])+v[1][1:];altered[first]=tuple(v)
    try:require_model_objects(altered)
    except AssertionError:pass
    else:raise AssertionError(('model source control escaped',kind))
(out/'model-object-source-check.json').write_text(json.dumps({'required':required,'all_rebuilt':True,'controls_caught':['missing','changed']},indent=2)+'\n')
print('MODEL SOURCE PASS all required objects rebuilt; missing/changed controls caught',flush=True)
vobj,vdata,kd,ka=objects['vadd']
# Record numeric constant and unit-conversion oracle queries/results, not guesses.
calcs={'ELF_magic':int.from_bytes(b'\x7fELF','little'),'ELF_class_data_version_osabi':int.from_bytes(bytes([2,1,1,64]),'little'),'ELF_type_machine':int.from_bytes(struct.pack('<HH',3,224),'little'),'manifest_magic':int.from_bytes(b'OMKFD001','little'),'f32_one':struct.pack('<f',1.0).hex(),'f32_one_plus_one':struct.pack('<f',1.0+1.0).hex()}
(out/'calculation-oracles.json').write_text(json.dumps(calcs,indent=2)+'\n')
mutations={ 'oversized':vdata[:63], 'wrong-machine':vdata[:18]+b'\0\0'+vdata[20:], 'wave32':vdata[:kd+56]+struct.pack('<H',struct.unpack_from('<H',vdata,kd+56)[0]|1024)+vdata[kd+58:], 'negative-entry':vdata[:kd+16]+struct.pack('<q',-1)+vdata[kd+24:] }
mutations['private-segment-unsupported']=vdata[:kd+4]+struct.pack('<I',20)+vdata[kd+8:]
mutations['queue-pointer-required']=vdata[:kd+56]+struct.pack('<H',struct.unpack_from('<H',vdata,kd+56)[0]|4)+vdata[kd+58:]
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
        assert b'OMEGA device=absent errno=2\n' in p.stdout
        assert b'OMEGA step open_kfd fail errno=2 phase=after detail=0\n' in p.stdout
    badargs=out/'bad-args'; badargs.write_bytes(b'\0'*48)
    run(target+' refuse argument header',pre+[binary,'run',vobj,'vadd',4,1,badargs,out/'unused'],64)
# Full native host orchestration with a syscall instrument: no GPU opens reach Linux.
run('compile UAPI layout oracle',['cc','-Wall','-Wextra','-Werror',root/'tests/amd-kfd/layout-oracle.c','-o',out/'layout'])
layout=run('UAPI layout',[out/'layout']).stdout
assert layout==(root/'backend/amd/kfd/requests.tsv').read_bytes()
for name in ('trace','publication'):
    run('compile '+name+' instrument',['cc','-Wall','-Wextra','-Werror',root/'tests/amd-kfd-launch'/ (name+'.c'),'-o',out/name])
def diagnostics(p,read,signal,output):
    rows={}
    for line in p.stdout.decode().splitlines():
        if line.startswith('OMEGA diag '):
            name,value=line.removeprefix('OMEGA diag ').split()
            rows[name]=int(value)
    expected={'queue_read_pointer':read,'queue_write_pointer':1,'signal_kind':1,'signal_value':signal,
              'queue_properties':2,'read_pointer_base_offset':128,'ring_header':0x21502,'ring_slot_byte_offset':0,'queue_id':7,'doorbell_offset':8,
              'wait_timeout_ns':10_000_000_000,'output_first8_nan_fill_count':8 if output==0x7fc00000 else 0,
              'output_first8_expected_vadd_sum_count':8 if output==0x40000000 else 0}
    for name,off,fmt in [('kernel_code_entry_byte_offset',16,'Q'),('compute_pgm_rsrc1',48,'I'),
                         ('compute_pgm_rsrc2',52,'I'),('compute_pgm_rsrc3',44,'I'),('kernel_code_properties',56,'H'),
                         ('group_segment_size',0,'I'),('private_segment_size',4,'I'),('kernarg_size',8,'I')]:
        expected[name]=struct.unpack_from('<'+fmt,vdata,kd+off)[0]
    expected.update({f'output_word_{i}':output for i in range(8)})
    for line in p.stderr.decode().splitlines():
        if line.startswith('TEST_SIM queue_input_qword_'):
            name,value=line.removeprefix('TEST_SIM ').split('=')
            expected['create_'+name]=int(value)
        if line.startswith('TEST_SIM packet_'):
            name,value=line.removeprefix('TEST_SIM ').split('=')
            expected[name]=int(value)
    assert len([x for x in expected if x.startswith('create_')])==11
    assert all(rows.get(k)==v for k,v in expected.items()), (rows,expected)
    assert rows['packet_kernel_object'] and rows['packet_kernarg_address']
with tempfile.TemporaryDirectory(prefix='kfd-',dir='/tmp') as tmp:
    node=Path(tmp)/'2'; node.mkdir()
    (node/'gpu_id').write_text('1\n')
    (node/'properties').write_text('gfx_target_version 90402\ndrm_render_minor 128\nsimd_count 32\nsimd_per_cu 4\nmax_waves_per_simd 8\narray_count 8\nsimd_arrays_per_engine 2\nnum_xcc 2\n')
    command=[out/'trace',tmp,bins['a64'],'vadd',vobj,'vadd','10000000000']
    lds=bytearray(vdata);struct.pack_into('<I',lds,kd,4096)
    ldsobj=out/'vadd-lds.hsaco';ldsobj.write_bytes(lds)
    p=run('accepted ELF with nonzero LDS uses HSA packet group field',[out/'trace',tmp,bins['a64'],'vadd',ldsobj,'vadd','10000000000'])
    p=run('generic default alarms without refusing a slow completion',['env','KFD_TEST_EXPIRE=1',out/'trace',tmp,bins['a64'],'vadd',vobj,'vadd'])
    assert p.stdout.count(b'OMEGA alarm slow_dispatch_ns=')==1
    twice=body.replace('dispatch\nstepbegin streadback','dispatch\nr = checked mapset lh 7 8 8 1\ndispatch\nstepbegin streadback')
    twice=twice.replace('for i ulong get launch 2 bound 9223372036854775807\nwhile', 'for j ulong get launch 2 bound 9223372036854775807\ni = add j mapget lh 5 56 8\nwhile')
    assert twice!=body
    u=compile('generic-two-dispatches',twice)
    p=run('generic slow alarm re-arms for second dispatch',['env','KFD_TEST_EXPIRE=1',out/'trace',tmp,u,'vadd',vobj,'vadd'])
    assert p.stdout.count(b'OMEGA alarm slow_dispatch_ns=')==2,p.stdout
    mutant=twice.replace('put dispatchalarm 0 0\n','')
    assert mutant!=twice
    u=compile('generic-no-alarm-rearm',mutant)
    p=run('CONTROL generic missing re-arm detected',['env','KFD_TEST_EXPIRE=1',out/'trace',tmp,u,'vadd',vobj,'vadd'])
    assert p.stdout.count(b'OMEGA alarm slow_dispatch_ns=')==1
    mutant=body.replace('gt elapsed 10000000000','gt elapsed 0')
    assert mutant!=body
    u=compile('alarm-threshold-zero',mutant)
    p=run('CONTROL fast dispatch false alarm detected',[out/'trace',tmp,u,'vadd',vobj,'vadd'])
    assert b'OMEGA alarm slow_dispatch_ns=' in p.stdout
    p=run('native simulated orchestration',command)
    assert b'OMEGA alarm slow_dispatch' not in p.stdout
    diagnostics(p,1,0,0x40000000)
    for name,variables,read,signal,output,child_exit in [
        ('unconsumed timeout',['KFD_TEST_NO_COMPLETE=1','KFD_TEST_EXPIRE=1'],0,1,0x7fc00000,75),
        ('output without completion timeout',['KFD_TEST_NO_SIGNAL=1','KFD_TEST_EXPIRE=1'],1,1,0x40000000,75),
        ('completion without output',['KFD_TEST_NO_OUTPUT=1'],1,0,0x7fc00000,76)]:
        p=run('CONTROL diagnostics '+name,['env',*variables,*command],1)
        assert ('child_exit='+str(child_exit)+' ').encode() in p.stderr
        diagnostics(p,read,signal,output)
        if child_exit==75:
            assert b'OMEGA step signal_wait fail errno=110' in p.stdout
    mutant=body.replace('r = checked mapset lh 5 180 4 2\n','r = checked mapset lh 5 180 4 0\n')
    assert mutant!=body
    bad=compile('queue-ptr32',mutant)
    p=run('CONTROL wrong queue pointer width',[out/'trace',tmp,bad,'vadd',vobj,'vadd'],1)
    assert b'queue_image' in p.stderr
    mutant=body.replace('r = checked mapset lh 5 180 4 2\n',
                        'r = checked mapset lh 5 180 4 2\nr = checked mapset lh 5 2040 8 1\n')
    assert mutant!=body
    bad=compile('queue-tail-dirty',mutant)
    p=run('CONTROL nonzero unused queue tail',[out/'trace',tmp,bad,'vadd',vobj,'vadd'],1)
    assert b'queue_image' in p.stderr
    mutant=body.replace('r = allocate 3 mul get launch 10 64 0xe6000002', 'r = allocate 3 mul get launch 10 64 0xa6000002')
    assert mutant!=body
    bad=compile('ring-not-executable',mutant)
    p=run('CONTROL ring lacks execute permission',[out/'trace',tmp,bad,'vadd',vobj,'vadd'],1)
    assert b'KFD_IOC_ALLOC_MEM_FLAGS_EXECUTABLE' in p.stderr
    for mode in ('missing','duplicate','oversized','invalid','empty','large-duplicate'):
        p=run('CONTROL aperture '+mode,['env','KFD_TEST_APERTURE='+mode,*command],1)
        assert b'child_exit=66 ' in p.stderr and b'queued=0' in p.stderr
    for mode in ('large','grow'):
        run('aperture inventory '+mode,['env','KFD_TEST_APERTURE='+mode,*command])
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
# An array-bound-dominant topology distinguishes both required ROCT divisions.
with tempfile.TemporaryDirectory(prefix='kfd-ctx-',dir='/tmp') as tmp:
    node=Path(tmp)/'2';node.mkdir()
    (node/'gpu_id').write_text('1\n')
    (node/'properties').write_text('gfx_target_version 90402\ndrm_render_minor 128\nsimd_count 1216\nsimd_per_cu 4\nmax_waves_per_simd 8\narray_count 4\nsimd_arrays_per_engine 2\nnum_xcc 1\n')
    command=['env','KFD_TEST_CTX_BOUND=1',out/'trace',tmp,bins['a64'],'vadd',vobj,'vadd','10000000000']
    run('CWSR bound uses converted NumShaderBanks',command)
    mutant=body.replace('mul div div get dev 5 get dev 6 get dev 6 512','mul div get dev 5 get dev 6 512')
    assert mutant!=body
    bad=compile('wrong-one-division-context',mutant)
    run('CONTROL missing topology conversion',['env','KFD_TEST_CTX_BOUND=1',out/'trace',tmp,bad,'vadd',vobj,'vadd','10000000000'],1)
# Card-shaped discovery: CPUs, denied gpu_id/properties, and only node 3 readable.
# Denial is injected at syscall entry, so this remains deterministic under root.
with tempfile.TemporaryDirectory(prefix='kfd-',dir='/tmp') as tmp:
    for index in range(4):
        node=Path(tmp)/str(index); node.mkdir()
        (node/'gpu_id').write_text('0\n' if index<2 else '37820\n')
        (node/'properties').write_text('gfx_target_version 90402\ndrm_render_minor 136\nsimd_count 32\nsimd_per_cu 4\nmax_waves_per_simd 8\narray_count 8\nsimd_arrays_per_engine 2\nnum_xcc 2\n')
    command=['env','KFD_TEST_SORT_NODES=1','KFD_TEST_GPU=37820','KFD_TEST_MINOR=136',out/'trace',tmp,bins['a64'],'vadd',vobj,'vadd']
    for deny in ('2/gpu_id','2/properties'):
        args=['env','KFD_TEST_DENY='+deny,*command]
        p=run('node-3-only discovery '+deny,args)
        assert b'OMEGA topology selected node=3 gpu_id=37820 render_minor=136\n' in p.stdout
        assert b'OMEGA topology skip CPU gpu_id=0\n' in p.stdout
        assert b'OMEGA step topology_node fail errno=1 phase=after detail=2\n' in p.stdout
        for step in ('open_kfd','version','render_open','acquire_vm','memory_alloc','memory_map','memory_cpu_map','queue_create','doorbell_map','code_object_load','dispatch','signal_wait','readback'):
            assert ('OMEGA step '+step+' ok errno=0 phase=before').encode() in p.stdout
            assert ('OMEGA step '+step+' ok errno=0 phase=after').encode() in p.stdout
    p=run('CONTROL missing selected render node',['env','KFD_TEST_DENY=2/gpu_id','KFD_TEST_RENDER_ABSENT=1',*command],1)
    assert b'child_exit=74 ' in p.stderr
    assert b'OMEGA step render_open fail errno=2 phase=after detail=136\n' in p.stdout
    mutant=body.replace('r = stepresult fd\nret false','r = checked fd\nret false')
    assert mutant!=body
    bad=compile('fatal-topology-open',mutant)
    p=run('CONTROL inaccessible node must not abort',['env','KFD_TEST_DENY=2/gpu_id',*command[:-4],bad,'vadd',vobj,'vadd'],1)
    assert b'child_exit=74 ' in p.stderr
    mutant=body.replace('emit cpuskip\nret false','emit cpuskip\nexit 66')
    assert mutant!=body
    bad=compile('fatal-cpu-node',mutant)
    p=run('CONTROL CPU node must not abort',[*command[:-4],bad,'vadd',vobj,'vadd'],1)
    assert b'child_exit=66 ' in p.stderr
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
+put el 9 4096
+r = checked mapset lh 3 0 4 1
+hput 0 8 mapaddr lh 3
+hput 8 8 mapaddr lh 0
+hput 16 8 mapaddr lh 1
+hput 24 8 mapaddr lh 7
+hput 32 8 add mapaddr lh 5 56
+hput 40 8 mapaddr b
+r = checked write 1 hdr 0 48
+packet 0
+ret 0
+end
+'''.replace('\n+','\n')
# Reuse the separately gated HSA packet encoder, not a second copy of launcher offsets.
aql_bytes=run('compile independent AQL encoder',[omc],data=(root/'backend/amd/kfd/aql.om').read_bytes()+(root/'backend/amd/kfd/cli.om').read_bytes()).stdout
aql=out/'aql-oracle';aql.write_bytes(aql_bytes);aql.chmod(0o700)
def packet_agrees_with_aql(p,private=0):
    assert len(p.stdout)==112
    ring,code,args,sig,wp,bell=struct.unpack('<6Q',p.stdout[:48])
    kobj,karg=code+64,args
    expected=run('independent AQL nonzero LDS encoding',[aql,2,64,1,1,256,1,1,private,4096,kobj,karg,sig,1,2,2]).stdout
    assert p.stdout[48:]==expected,(p.stdout[48:].hex(),expected.hex())
u=compile('publication-unit',unit)
packet_agrees_with_aql(run('native publication order',[out/'publication',u]))
u=compile('publication-distinct-segments',unit.replace('put el 9 4096','put el 9 4096\nput el 10 20'))
packet_agrees_with_aql(run('generic ABI private20 group4096',['env','EXPECTED_PRIVATE=20',out/'publication',u]),20)
mutant=unit.replace('hput 24 4 get el 10\nhput 28 4 get el 9','hput 24 4 get el 9\nhput 28 4 get el 10')
assert mutant!=unit
u=compile('publication-swapped-segments',mutant)
run('CONTROL swapped group-private segments',[out/'publication',u],1)

line='r = checked mapwrite lh 3 add slot 4 hdr 4 60\n'
mutant=unit.replace(line,'').replace('r = checked mapset lh 3 slot 4 0x00021502\n','r = checked mapset lh 3 slot 4 0x00021502\n'+line)
assert mutant!=unit
u=compile('publication-early-header',mutant)
run('CONTROL early valid header',[out/'publication',u],1)
# Ring capacity follows the dispatch count, the established AQL minimum,
# and the 32-bit CREATE_QUEUE byte-size wire field.
ringunit=''.join((src/(x+'.om')).read_text() for x in ('common','elf','device','dispatch','plan'))
ringunit+=(root/'backend/amd/kfd/cli.om').read_text()
# pw widths are fixed at production call sites. This defensive helper boundary is
# not reachable by plan bytes; exercise it explicitly instead of inventing a plan row.
widthunit=ringunit+'fn main to int\nput ps 2 16\nr = pw 0 9\nret 99\nend\n'
u=compile('plan-helper-word-width',widthunit)
p=run('plan helper refuses impossible input width',[u],64)
assert p.stdout==b'OMEGA refuse plan field=word-width\n'
ringmain='fn main to int\n'
for dispatches,slots in [(1,16),(15,16),(16,32),(20,32),(1126,2048),(33554431,33554432)]:
    ringmain+='put ps 6 '+str(dispatches)+'\nplanring\nif ne get launch 10 '+str(slots)+'\nret 99\nend\n'
ringmain+='ret 0\nend\n'
u=compile('plan-ring-boundaries',ringunit+ringmain)
run('native plan ring capacity boundaries',[u])
u=compile('plan-ring-unrepresentable',ringunit+'fn main to int\nput ps 6 33554432\nplanring\nret 99\nend\n')
run('CONTROL plan ring exceeds UAPI size',[u],64)
# The batch path has its own body-before-header observation and early-header control.
planunit=''.join((src/(x+'.om')).read_text() for x in ('common','elf','device','dispatch','plan'))
planunit+=(root/'backend/amd/kfd/cli.om').read_text()
main=unit[unit.index('fn main to int'):]
setup="""put launch 10 16
put ps 6 1
put ps 20 ulong checked mapnew -1 0 4096 3 34
put ps 0 ulong checked mapnew -1 0 4096 3 34
put ps 2 4096
r = checked mapset long get ps 0 4 4 4
r = checked mapset long get ps 0 8 4 1
put ps 17 ulong checked mapnew -1 0 4096 3 34
r = checked mapset long get ps 17 48 8 64
r = checked mapset long get ps 17 72 8 4096
put ps 19 ulong checked mapnew -1 0 4096 3 34
r = checked mapset t 192 8 ulong lh 0
plantemplate 0
planpublish 0 0"""
planunit+=main.replace('mapnew -1 0 192 3 34','mapnew -1 0 4096 3 34').replace('packet 0',setup)
u=compile('plan-publication-unit',planunit)
packet_agrees_with_aql(run('native batch publication order through plantemplate',[out/'publication',u]))
u=compile('plan-publication-distinct-segments',planunit.replace('r = checked mapset long get ps 17 72 8 4096','r = checked mapset long get ps 17 72 8 4096\nr = checked mapset long get ps 17 80 8 20'))
packet_agrees_with_aql(run('plantemplate ABI private20 group4096',['env','EXPECTED_PRIVATE=20',out/'publication',u]),20)
mutant=planunit.replace('hput 24 4 cm code 80\nhput 28 4 cm code 72','hput 24 4 cm code 72\nhput 28 4 cm code 80')
assert mutant!=planunit
u=compile('plan-publication-swapped-segments',mutant)
run('CONTROL plantemplate-only swapped segments',[out/'publication',u],1)
mutant=planunit.replace('plancopy sent\ncpufence\n','').replace('r = checked mapset lh 5 56 8 add sent get ps 6','plancopy sent\nr = checked mapset lh 5 56 8 add sent get ps 6')
assert mutant!=planunit
u=compile('plan-publication-early-header',mutant)
run('CONTROL batch early valid header',[out/'publication',u],1)
mutant=body.replace('hput 24 4 cm code 80\nhput 28 4 cm code 72','hput 24 4 cm code 72\nhput 28 4 cm code 80')
assert mutant!=body
compile('plan-swapped-segments',mutant)
mutant=body.replace('put dispatchalarm 0 0\n','')
assert mutant!=body
compile('plan-no-alarm-rearm',mutant)
run('native plan CPU contracts',[sys.executable,'-I',root/'tests/amd-kfd-launch/plan-check.py',out])
for receipt in sorted((root/'evidence').glob('amd-qwen38-native-*')):
    state=receipt/'native/state.json'
    if not state.exists():continue
    run('verify archived native evidence '+receipt.name,[sys.executable,'-I',receipt/'verify.py'])
    v=json.loads(state.read_text());prompt=len(v['fixture']['prompt_ids'])
    for row in [v['control'],*v['rows']]:
        text=(receipt/'native'/(row['label']+'.stdout')).read_text()
        steps=[tuple(map(int,m)) for m in re.findall(r'^OMEGA_STEP (\d+) tok=(\d+) us=(\d+)$',text,re.M)]
        assert math.isclose(sum(u for _,_,u in steps[:prompt])/1000,row['prefill_ms'],rel_tol=0,abs_tol=1e-9)
        assert statistics.median(u/1000 for _,_,u in steps[prompt:])==row['decode_ms_median']
    print('RAW TIMING PASS',receipt.name,flush=True)
summary={'compiler':pin,'checks':len(results),'failures':sum(r['exit']!=r['expected'] for r in results),'qemu':qemu,'device':'NOT_RUN','orchestration':'native CPU syscall simulation only','publication':'native instruction-step observation only','objects':{name:hashlib.sha256(data).hexdigest() for name,(p,data,k,a) in objects.items()},'binaries':{name:hashlib.sha256(p.read_bytes()).hexdigest() for name,p in bins.items()}}
(out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print('AMD-KFD-LAUNCH CPU PASS',json.dumps(summary,sort_keys=True))
print('NOT_RUN every AMD device row: KFD queues, GPU mappings, completion, parity, timing, Qwen inference')
