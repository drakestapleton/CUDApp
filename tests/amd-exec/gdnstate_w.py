#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Private-output executor acceptance and independent NumPy scalar-order oracle."""
import argparse
import hashlib
import json
from pathlib import Path
import runpy
import shutil
import struct
import subprocess
import numpy as np

PIN='2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f'
NAME='gdnstate_w'
SIZES=(16384,24576,3145728,3145728,24576,192,192)
NAMES=('qk','value','oldstate','newstate','output','decay','beta')
CASES=('random-1','random-2','random-3','zero','cancel','subnormal','impulse','exceptional')


def sha(p):
    with Path(p).open('rb') as f: return hashlib.file_digest(f,'sha256').hexdigest()


def bf16(x):
    bits=np.asarray(x,dtype=np.float32).view(np.uint32)
    result=(bits+0x7fff+((bits>>16)&1))&np.uint32(0xffff0000)
    return np.where((bits&0x7fffffff)>0x7f800000,np.uint32(0x7fc00000),result).astype(np.uint32).view(np.float32)


def fixture(kind):
    seed=int(kind.split('-')[1]) if kind.startswith('random-') else 7
    rng=np.random.default_rng(seed)
    qk=bf16(rng.uniform(-0.125,0.125,(2,16,128)).astype(np.float32))
    value=bf16(rng.uniform(-0.25,0.25,(48,128)).astype(np.float32))
    old=rng.uniform(-0.25,0.25,(48,128,128)).astype(np.float32)
    decay=rng.uniform(0.125,0.875,48).astype(np.float32)
    beta=bf16(rng.uniform(0.125,0.875,48).astype(np.float32))
    if kind=='zero': old.fill(0); value.fill(0); qk.fill(0)
    elif kind=='cancel':
        old[:]=np.where(np.arange(128)[:,None]%2,0.5,-0.5).astype(np.float32)
        qk.fill(0.125); value.fill(0); decay.fill(1); beta.fill(1)
    elif kind=='subnormal':
        words=np.array([0,0x80000000,1,0x80000001,0x007fffff,0x807fffff,0x00800000,0x80800000],dtype=np.uint32)
        old[:]=np.resize(words,old.size).reshape(old.shape).view(np.float32)
        value[:]=np.resize(words,value.size).reshape(value.shape).view(np.float32)
        qk.fill(0.125); decay.fill(0.5); beta.fill(0.5)
    elif kind=='impulse':
        old.fill(0); value.fill(0); qk.fill(0)
        for head in range(48):
            old[head,head%128,(head*17)%128]=np.float32((head+1)/64)
            value[head,(head*7)%128]=np.float32((head+1)/128)
        for head in range(16): qk[0,head,head*7]=0.5; qk[1,head,head*5]=0.25
    elif kind=='exceptional':
        words=np.array([0x7f800000,0xff800000,0x7fc00000,0x7fa12345,0xffa12345,0x7f7fffff,0xff7fffff,0x80000000],dtype=np.uint32)
        for head in range(48): old[head,head%128,:8]=words.view(np.float32)
        value[:8,0]=words.view(np.float32); decay[0]=0; beta[1]=0
    buffers=(qk.tobytes(),value.tobytes(),old.tobytes(),b'\xa5'*SIZES[3],b'\xa5'*SIZES[4],decay.tobytes(),beta.tobytes())
    spans=[('kernarg',0,56)]; pointers=[]; cursor=128
    for name,blob in zip(NAMES,buffers):
        spans.append((name,cursor,len(blob))); pointers.append(cursor); cursor+=len(blob)+128
    image=bytearray(b'\xa5')*cursor
    struct.pack_into('<7Q',image,0,*pointers)
    for p,b in zip(pointers,buffers): image[p:p+len(b)]=b
    return image,spans


def oracle(image,spans):
    ptr={name:p for name,p,n in spans}
    def arr(name,shape): return np.frombuffer(image,dtype='<f4',count=int(np.prod(shape)),offset=ptr[name]).reshape(shape)
    qk=arr('qk',(2,16,128)); q=np.repeat(qk[0],3,axis=0); k=np.repeat(qk[1],3,axis=0)
    value=arr('value',(48,128)); old=arr('oldstate',(48,128,128))
    decay=arr('decay',(48,)); beta=arr('beta',(48,))
    with np.errstate(all='ignore'):
        decayed=np.multiply(old,decay[:,None,None]); prediction=np.zeros((48,128),dtype=np.float32)
        for r in range(128): prediction=np.add(prediction,np.multiply(k[:,r,None],decayed[:,r,:]))
        delta=np.multiply(beta[:,None],np.subtract(value,prediction))
        updated=np.add(decayed,np.multiply(k[:,:,None],delta[:,None,:]))
        core=np.zeros((48,128),dtype=np.float32)
        for r in range(128): core=np.add(core,np.multiply(q[:,r,None],updated[:,r,:]))
        core=bf16(core)
    return {'newstate':updated,'output':core}


def check_independent(actual,spans,expected):
    ptr={name:p for name,p,n in spans}
    for name,want in expected.items():
        got=np.frombuffer(actual,dtype='<f4',count=want.size,offset=ptr[name]).reshape(want.shape)
        assert np.array_equal(np.isnan(got),np.isnan(want)),name+' NaN classification'
        finite=~np.isnan(want)
        assert np.array_equal(got.view(np.uint32)[finite],want.view(np.uint32)[finite]),name+' independent finite/infinite bit mismatch'


def mutants(src):
    changes={'wrong-column-block':('I 5 131 1 1 18','I 5 130 1 1 18'),
             'wrong-key-head':('I 2 3 137 29 28','I 2 3 136 29 28'),
             'no-beta':('I 5 277 18 18 5','I 5 242 18 18 5')}
    result={}
    for name,(old,new) in changes.items():
        assert src.count(old)==1,name
        result[name]=src.replace(old,new)
    return result


def section(path,name):
    blob=Path(path).read_bytes(); shoff=struct.unpack_from('<Q',blob,40)[0]
    entsize,count,stridx=struct.unpack_from('<HHH',blob,58)
    entries=[struct.unpack_from('<IIQQQQIIQQ',blob,shoff+i*entsize) for i in range(count)]
    names=blob[entries[stridx][4]:entries[stridx][4]+entries[stridx][5]]
    for entry in entries:
        if names[entry[0]:names.index(0,entry[0])].decode()==name: return blob[entry[4]:entry[4]+entry[5]]
    raise AssertionError(name)


def main():
    parser=argparse.ArgumentParser(); parser.add_argument('--omc',required=True,type=Path); parser.add_argument('--out',required=True,type=Path)
    args=parser.parse_args()
    if sha(args.omc)!=PIN: raise SystemExit(69)
    root=Path(__file__).resolve().parents[2]; out=args.out.resolve(); out.mkdir(parents=True,exist_ok=False)
    snapshot=out/'source'; snapshot.mkdir()
    paths=[root/'backend/amd/gfx942'/p for p in ('hsaco.om','gdnstate.isa','gdnstate-gfx942.s',NAME+'.isa',NAME+'-gfx942.s','exec/fp.om','exec/fma.om','exec/main.om')]
    paths += [Path(__file__).with_name(p) for p in ('token_access.py','gdnstate_w.py')]
    for path in paths: shutil.copyfile(path,snapshot/path.name)
    hashes={p.name:sha(p) for p in snapshot.iterdir()}
    instrument=runpy.run_path(str(snapshot/'token_access.py'))['instrument']
    commands=[]
    def run(argv,stdin=None,stdout=None,expected=0):
        p=subprocess.run([str(a) for a in argv],input=stdin,capture_output=True,timeout=1800)
        commands.append(dict(argv=[str(a) for a in argv],exit=p.returncode,stdout_sha256=hashlib.sha256(p.stdout).hexdigest(),stderr=p.stderr.decode(errors='replace')))
        (out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
        assert p.returncode==expected and not p.stderr,commands[-1]
        if stdout: Path(stdout).write_bytes(p.stdout)
        return p.stdout
    def compile_source(label,text):
        (out/(label+'.om')).write_text(text); exe=out/label
        run([args.omc],stdin=text.encode(),stdout=exe); exe.chmod(0o755); return exe
    producer_src=(snapshot/'hsaco.om').read_text(); producer=compile_source('hsaco',producer_src)
    assert run([args.omc],stdin=producer_src.encode())==producer.read_bytes()
    exec_src=''.join((snapshot/p).read_text() for p in ('fp.om','fma.om','main.om'))
    executor=compile_source('exec',exec_src)
    assert run([args.omc],stdin=exec_src.encode())==executor.read_bytes()
    original=(snapshot/(NAME+'.isa')).read_text()
    variants={'gdnstate':(snapshot/'gdnstate.isa').read_text(),NAME:original,**{NAME+'-'+k:v for k,v in mutants(original).items()}}
    texts={}; objects={}
    for label,text in variants.items():
        path=out/(label+'.isa'); path.write_text(text); obj=out/(label+'.hsaco')
        run([producer,path],stdout=obj); assert run([producer,path])==obj.read_bytes()
        raw=out/(label+'.text'); raw.write_bytes(section(obj,'.text')); texts[label]=raw; objects[label]=obj
    for label in ('gdnstate',NAME):
        obj=out/(label+'-llvm.o')
        run(['llvm-mc-18','-triple=amdgcn-amd-amdhsa','-mcpu=gfx942','-filetype=obj',snapshot/(label+'-gfx942.s'),'-o',obj])
        for sec in ('.text','.note'): assert section(objects[label],sec)==section(obj,sec),(label,sec)
        a,b=section(objects[label],'.rodata'),section(obj,'.rodata')
        assert len(a)==len(b)==64 and a[:16]+a[24:]==b[:16]+b[24:],(label,'descriptor')
    print('PASS deterministic seeded producer/executor and LLVM text/note/descriptor',flush=True)
    memory=out/'memory.bin'; cases=[]; bounds=[]; controls=[]
    def execute(exe,name,data):
        memory.write_bytes(data); gx=4 if name=='gdnstate' else 49
        stdout=run([exe,texts[name],memory,gx*64,17]); return stdout,memory.read_bytes()
    def access(stdout,spans):
        hits=struct.unpack('<'+'Q'*(2*len(spans)),stdout)
        return [dict(name=name,bytes=n,max_read_offset=hits[2*i]-1 if hits[2*i] else None,max_write_offset=hits[2*i+1]-1 if hits[2*i+1] else None) for i,(name,p,n) in enumerate(spans)]
    for kind in CASES:
        data,spans=fixture(kind); expected=oracle(data,spans)
        stdout,baseline=execute(executor,'gdnstate',data); assert not stdout
        check_independent(baseline,spans,expected)
        exe=executor
        if kind=='random-1': exe=compile_source('access',instrument(exec_src,[(p,n) for name,p,n in spans]))
        stdout,actual=execute(exe,NAME,data)
        assert actual==baseline,(kind,'partitioned/baseline bit mismatch')
        ptr={name:p for name,p,n in spans}; expected_image=bytearray(data)
        for name in ('newstate','output'):
            n=SIZES[NAMES.index(name)]; p=ptr[name]; expected_image[p:p+n]=baseline[p:p+n]
        assert actual==expected_image,(kind,'input/guard write')
        if kind=='random-1': bounds.append(dict(case=kind,buffers=access(stdout,spans)))
        else: assert not stdout
        record=dict(case=kind,comparison='every byte vs original gdnstate; independent scalar-order FP32 oracle',input_sha256=hashlib.sha256(data).hexdigest(),result_sha256=hashlib.sha256(actual).hexdigest())
        cases.append(record)
        if kind=='random-1':
            (out/'fixture.bin').write_bytes(data); (out/'expected.bin').write_bytes(actual)
            (out/'fixture.json').write_text(json.dumps(dict(buffers=[dict(name=name,offset=p,bytes=n) for name,p,n in spans]),indent=2)+'\n')
        print('PASS '+kind+' all-state/core bits and all inputs/guards; XY overdispatch',flush=True)
    data,spans=fixture('random-1'); expected=(out/'expected.bin').read_bytes()
    access_exe=out/'access'
    for mutant in mutants(original):
        stdout,actual=execute(access_exe,NAME+'-'+mutant,data)
        assert actual!=expected,mutant+' escaped'
        bounds.append(dict(case=mutant,buffers=access(stdout,spans)))
        controls.append(dict(mutant=mutant,exit=0,result='CAUGHT numerical mismatch'))
        print('CONTROL '+mutant+' CAUGHT numerical mismatch exit=0',flush=True)
    measured=bounds[0]['buffers']
    for i,(name,p,n) in enumerate(spans):
        last=max(v for v in (measured[i]['max_read_offset'],measured[i]['max_write_offset']) if v is not None)
        shortened=[(base,length) for _,base,length in spans]; shortened[i]=(p,last)
        exe=compile_source('short-'+name,instrument(exec_src,shortened)); memory.write_bytes(data)
        run([exe,texts[NAME],memory,49*64,17],expected=68)
        controls.append(dict(buffer=name,exit=68,result='CAUGHT one-byte-short span'))
        print('CONTROL short-'+name+' CAUGHT exit=68',flush=True)
    memory.unlink()
    report=dict(schema='gdnstate-w-acceptance-v1',status='EXERCISED',hardware='NOT_RUN',performance='NOT_RUN',compiler_sha256=PIN,source_sha256=hashes,kernarg_size=56,grid_workgroups=[48,16,1],block=[64,1,1],active_lanes=8,columns_per_workgroup=8,objects={name:dict(file=p.name,sha256=sha(p)) for name,p in objects.items()},cases=cases,bounds=bounds,controls=controls)
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('PASS gdnstate_w cases=%d controls=%d HARDWARE NOT_RUN PERFORMANCE NOT_RUN' % (len(cases),len(controls)),flush=True)

if __name__=='__main__': main()
