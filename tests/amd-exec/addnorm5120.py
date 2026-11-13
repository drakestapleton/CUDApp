#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Private-output acceptance: exact residual5120 -> rmsnorm5120 equivalence."""
import argparse, hashlib, json, runpy, shutil, struct, subprocess
from pathlib import Path
import numpy as np
from gdnstate_w import PIN,sha,section,bf16
NAME='addnorm5120'
NAMES=('h','delta','weight','h_new','y')
CASES=('random-1','random-2','zero','cancel','tiny','large','subnormal','exceptional')


def fixture(kind):
    rng=np.random.default_rng(int(kind[-1]) if kind.startswith('random-') else 7)
    h=rng.uniform(-1,1,5120).astype(np.float32); d=rng.uniform(-0.5,0.5,5120).astype(np.float32)
    w=bf16(rng.uniform(-1.5,1.5,5120).astype(np.float32)); w[:3]=(-1,0,1)
    if kind=='zero': h.fill(0); d.fill(0)
    elif kind=='cancel': d=-h
    elif kind=='tiny': h*=np.float32(2.0**-16); d*=np.float32(2.0**-16)
    elif kind=='large': h*=np.float32(2.0**70); d*=np.float32(2.0**70)
    elif kind=='subnormal':
        bits=np.array([0,0x80000000,1,0x80000001,0x007fffff,0x807fffff,0x00800000,0x80800000],dtype=np.uint32)
        h=np.resize(bits,5120).view(np.float32); d=np.resize(bits[::-1],5120).view(np.float32)
    elif kind=='exceptional':
        h[:8]=np.array([0x7f800000,0xff800000,0x7fc00000,0x7fa12345,0xffa12345,0x7f7fffff,0xff7fffff,0x80000000],dtype=np.uint32).view(np.float32)
        d[:8]=h[::-1][:8]
    blobs=(h.tobytes(),d.tobytes(),w.tobytes(),b'\xa5'*20480,b'\xa5'*20480)
    spans=[('kernarg',0,40)]; pointers=[]; cursor=128
    for name,blob in zip(NAMES,blobs): spans.append((name,cursor,len(blob))); pointers.append(cursor); cursor+=len(blob)+128
    image=bytearray(b'\xa5')*cursor; struct.pack_into('<5Q',image,0,*pointers)
    for p,blob in zip(pointers,blobs): image[p:p+len(blob)]=blob
    return image,spans


def residual_reference(image,spans):
    ptr={name:p for name,p,n in spans}
    h=np.frombuffer(image,dtype='<f4',count=5120,offset=ptr['h']); d=np.frombuffer(image,dtype='<f4',count=5120,offset=ptr['delta'])
    with np.errstate(all='ignore'): return bf16(np.add(bf16(h),bf16(d)))


def check_oracle(actual,initial,spans,ref):
    ptr={name:p for name,p,n in spans}; residual=residual_reference(initial,spans)
    observed=np.frombuffer(actual,dtype='<f4',count=5120,offset=ptr['h_new'])
    assert np.array_equal(observed.view(np.uint32),residual.view(np.uint32)),'independent residual bit mismatch'
    if not np.isfinite(residual).all(): return 'nonfinite: residual exact; norm exact against original sequence'
    words=residual.view(np.uint32).tolist(); weights=struct.unpack_from('<5120I',initial,ptr['weight'])
    fma,add,mul,finite,point,imul,rsqbound=(ref[n] for n in ('fma','add','mul','finite','point','imul','rsqbound'))
    lanes=[0]*64
    for lane in range(64):
        for chunk in range(20):
            for j in range(4):
                a=words[chunk*256+lane*4+j]; lanes[lane]=fma(a,a,lanes[lane])
    for shift in (32,16,8,4,2,1): lanes=[add(lanes[i],lanes[i^shift]) for i in range(64)]
    variance=add(mul(lanes[0],0x394ccccd),0x358637bd)
    if variance&0x7f800000==0x7f800000: return 'overflow: residual exact; norm exact against original sequence'
    inv=rsqbound(finite(variance)); outputs=struct.unpack_from('<5120I',actual,ptr['y'])
    for x,w,y in zip(words,weights,outputs):
        lo,hi=imul(imul(point(x),inv),point(add(0x3f800000,w)))
        assert y&0x7f800000!=0x7f800000 and lo<=finite(y)<=hi,'independent norm interval mismatch'
    return 'residual exact; all norm outputs within instruction-boundary RSQ interval'


def mutants(source):
    no_store=source
    preadd=source
    for j in range(4):
        store=f'I 7 {j*4} 0 2 0 0 28 0 6 {12+j} 127 0 0\n'
        assert no_store.count(store)==1; no_store=no_store.replace(store,'')
        fma=f'I 5 {268+j} {12+j} 16 59\n'; assert preadd.count(fma)==1
        preadd=preadd.replace(fma,'').replace(f'# FUSED_ELEMENT_{j}\n',f'# FUSED_ELEMENT_{j}\nI 5 {264+j} {8+j} 16 59\n')
    assert source.count('L 897988541')==1
    return {'skip-residual-write':no_store,'norm-preadd':preadd,'wrong-epsilon':source.replace('L 897988541','L 0')}


def main():
    p=argparse.ArgumentParser(); p.add_argument('--omc',required=True,type=Path); p.add_argument('--out',required=True,type=Path); args=p.parse_args()
    if sha(args.omc)!=PIN: raise SystemExit(69)
    root=Path(__file__).resolve().parents[2]; out=args.out.resolve(); out.mkdir(parents=True,exist_ok=False); snapshot=out/'source'; snapshot.mkdir()
    names=('hsaco.om','residual5120.isa','residual5120-gfx942.s','rmsnorm5120.isa','rmsnorm5120-gfx942.s',NAME+'.isa',NAME+'-gfx942.s','exec/fp.om','exec/fma.om','exec/main.om')
    sources=[root/'backend/amd/gfx942'/name for name in names]
    sources += [Path(__file__).with_name(name) for name in ('token_access.py','prep.py','attention.py','sgemm.py','gdnstate_w.py','addnorm5120.py')]
    for source in sources: shutil.copyfile(source,snapshot/source.name)
    pins={source.name:sha(source) for source in snapshot.iterdir()}
    ref=runpy.run_path(str(snapshot/'prep.py')); instrument=runpy.run_path(str(snapshot/'token_access.py'))['instrument']; commands=[]
    def run(argv,stdin=None,stdout=None,expected=0):
        result=subprocess.run([str(a) for a in argv],input=stdin,capture_output=True,timeout=900)
        commands.append(dict(argv=[str(a) for a in argv],exit=result.returncode,stdout_sha256=hashlib.sha256(result.stdout).hexdigest(),stderr=result.stderr.decode(errors='replace')))
        (out/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
        assert result.returncode==expected and not result.stderr,commands[-1]
        if stdout: Path(stdout).write_bytes(result.stdout)
        return result.stdout
    def compile_source(label,text):
        (out/(label+'.om')).write_text(text); exe=out/label; run([args.omc],stdin=text.encode(),stdout=exe); exe.chmod(0o755); return exe
    producer_source=(snapshot/'hsaco.om').read_text(); producer=compile_source('hsaco',producer_source)
    assert run([args.omc],stdin=producer_source.encode())==producer.read_bytes()
    exec_source=''.join((snapshot/name).read_text() for name in ('fp.om','fma.om','main.om')); executor=compile_source('exec',exec_source)
    assert run([args.omc],stdin=exec_source.encode())==executor.read_bytes()
    original=(snapshot/(NAME+'.isa')).read_text()
    variants={name:(snapshot/(name+'.isa')).read_text() for name in ('residual5120','rmsnorm5120',NAME)}
    variants.update({NAME+'-'+name:text for name,text in mutants(original).items()}); texts={}; objects={}
    for name,text in variants.items():
        table=out/(name+'.isa'); table.write_text(text); obj=out/(name+'.hsaco'); run([producer,table],stdout=obj)
        assert run([producer,table])==obj.read_bytes(); raw=out/(name+'.text'); raw.write_bytes(section(obj,'.text')); texts[name]=raw; objects[name]=obj
    for name in ('residual5120','rmsnorm5120',NAME):
        obj=out/(name+'-llvm.o'); run(['llvm-mc-18','-triple=amdgcn-amd-amdhsa','-mcpu=gfx942','-filetype=obj',snapshot/(name+'-gfx942.s'),'-o',obj])
        for sec in ('.text','.note'): assert section(objects[name],sec)==section(obj,sec),(name,sec)
        a,b=section(objects[name],'.rodata'),section(obj,'.rodata'); assert len(a)==len(b)==64 and a[:16]+a[24:]==b[:16]+b[24:],(name,'descriptor')
    print('PASS deterministic seeded producer/executor; original and fused LLVM text/note/descriptor',flush=True)
    memory=out/'memory.bin'; cases=[]; bounds=[]; controls=[]
    def execute(exe,name,data,gx=2,gy=2):
        memory.write_bytes(data); stdout=run([exe,texts[name],memory,gx*64,gy]); return stdout,memory.read_bytes()
    def access(stdout,spans):
        hits=struct.unpack('<'+'Q'*(2*len(spans)),stdout)
        return [dict(name=name,bytes=n,max_read_offset=hits[i*2]-1 if hits[i*2] else None,max_write_offset=hits[i*2+1]-1 if hits[i*2+1] else None) for i,(name,p,n) in enumerate(spans)]
    for kind in CASES:
        data,spans=fixture(kind); ptr={name:p for name,p,n in spans}; baseline=bytearray(data)
        struct.pack_into('<3Q',baseline,0,ptr['h'],ptr['delta'],ptr['h_new']); stdout,baseline=execute(executor,'residual5120',baseline,81); assert not stdout
        baseline=bytearray(baseline); struct.pack_into('<3QI',baseline,0,ptr['h_new'],ptr['weight'],ptr['y'],1)
        stdout,baseline=execute(executor,'rmsnorm5120',baseline); assert not stdout
        baseline=bytearray(baseline); baseline[:40]=data[:40]
        independent=check_oracle(baseline,data,spans,ref)
        exe=executor
        if kind=='random-1': exe=compile_source('access',instrument(exec_source,[(p,n) for name,p,n in spans]))
        stdout,actual=execute(exe,NAME,data); assert actual==baseline,(kind,'fused/two-kernel bit mismatch')
        expected=bytearray(data)
        for name in ('h_new','y'): expected[ptr[name]:ptr[name]+20480]=baseline[ptr[name]:ptr[name]+20480]
        assert actual==expected,(kind,'input/guard changed')
        if kind=='random-1':
            bounds.append(dict(case=kind,buffers=access(stdout,spans))); (out/'fixture.bin').write_bytes(data); (out/'expected.bin').write_bytes(actual)
        else: assert not stdout
        cases.append(dict(case=kind,input_sha256=hashlib.sha256(data).hexdigest(),result_sha256=hashlib.sha256(actual).hexdigest(),independent=independent))
        print('PASS '+kind+' residual/norm all bits equal two-kernel sequence; guards; XY overdispatch; '+independent,flush=True)
    data,spans=fixture('random-1'); expected=(out/'expected.bin').read_bytes()
    for name in mutants(original):
        stdout,actual=execute(out/'access',NAME+'-'+name,data); assert actual!=expected,name+' escaped'
        bounds.append(dict(case=name,buffers=access(stdout,spans))); controls.append(dict(mutant=name,exit=0,result='CAUGHT numerical mismatch'))
        print('CONTROL '+name+' CAUGHT numerical mismatch exit=0',flush=True)
    measured=bounds[0]['buffers']
    for i,(name,p,n) in enumerate(spans):
        last=max(v for v in (measured[i]['max_read_offset'],measured[i]['max_write_offset']) if v is not None)
        shortened=[(p,n) for _,p,n in spans]; shortened[i]=(p,last)
        exe=compile_source('short-'+name,instrument(exec_source,shortened)); memory.write_bytes(data); run([exe,texts[NAME],memory,128,2],expected=68)
        controls.append(dict(buffer=name,exit=68,result='CAUGHT one-byte-short span')); print('CONTROL short-'+name+' CAUGHT exit=68',flush=True)
    memory.unlink()
    report=dict(schema='addnorm5120-acceptance-v1',status='EXERCISED',hardware='NOT_RUN',performance='NOT_RUN',compiler_sha256=PIN,source_sha256=pins,kernarg_size=40,grid_workgroups=[1,1,1],block=[64,1,1],objects={name:dict(file=obj.name,sha256=sha(obj)) for name,obj in objects.items()},cases=cases,bounds=bounds,controls=controls)
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('PASS addnorm5120 cases=%d controls=%d HARDWARE NOT_RUN PERFORMANCE NOT_RUN'%(len(cases),len(controls)),flush=True)

if __name__=='__main__': main()
