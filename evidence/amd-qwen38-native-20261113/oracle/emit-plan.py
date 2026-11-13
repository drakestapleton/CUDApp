"""Harness: write an OMPLAN01 plan (FORMAT.md) from real2's own Card.emit, then decode the file independently and
compare it with the recorded launch list. Python never touches weight bytes: buffers name (file, offset, length).
Usage: emit-plan.py DRIVER WEIGHTS OUT.plan [--controls] [--capacity=N] [--fixture-json=PATH]
--capacity=N sizes the KV cache, RoPE table and attention partitions for N positions (Card's own default is
prompt+generated = 48). Prompt, generation and golden ids are unchanged; the override is applied here, not in Card.
--fixture-json=PATH replaces the driver's prompt_ids/generated_ids with the fixture's before anything is recorded
(Card source untouched); its capacity applies unless --capacity is given. The sidecar records the fixture's identity.
"""
import ctypes,hashlib,importlib.util,json,lzma,base64,os,struct,sys,types
from pathlib import Path
import torch

driver_path,weights,out=Path(sys.argv[1]),Path(sys.argv[2]),Path(sys.argv[3]);controls='--controls' in sys.argv
capacity=next((int(a.split('=',1)[1]) for a in sys.argv[4:] if a.startswith('--capacity=')),None)
fixture_path=next((Path(a.split('=',1)[1]) for a in sys.argv[4:] if a.startswith('--fixture-json=')),None)
fixture=None
if fixture_path is not None:
    fixture_raw=fixture_path.read_bytes();fixture=json.loads(fixture_raw)
    for k in ('prompt_ids','generated_ids'):
        if not (isinstance(fixture.get(k),list) and fixture[k] and all(isinstance(t,int) and t>=0 for t in fixture[k])):raise SystemExit('fixture %s: %s must be a non-empty list of token ids'%(fixture_path,k))
    if capacity is None:capacity=fixture.get('capacity')
os.environ['AMDSERVE_WEIGHTS']=str(weights);os.environ['AMDSERVE_OUTPUT']=str(out)+'.log'

# Card allocates on 'cuda'; build the same objects on CPU. Weights are zero-stride stand-ins carrying true shape/dtype.
_to,_zeros,_tensor=torch.Tensor.to,torch.zeros,torch.tensor
def cpu(kw):
    if str(kw.get('device'))=='cuda':kw['device']='cpu'
    return kw
torch.zeros=lambda *a,**k:_zeros(*a,**cpu(k));torch.tensor=lambda *a,**k:_tensor(*a,**cpu(k))
torch.Tensor.to=lambda self,*a,**k:_to(self,*(('cpu',)+a[1:] if a and str(a[0])=='cuda' else a),**cpu(k))
torch.cuda.init=lambda:None;torch.cuda.synchronize=lambda:None

index=json.loads((weights/'model.safetensors.index.json').read_text())['weight_map']
headers={}
def header(file):
    if file not in headers:
        with (weights/file).open('rb') as f:
            n=struct.unpack('<Q',f.read(8))[0];headers[file]=(8+n,json.loads(f.read(n)))
    return headers[file]
DT={'BF16':torch.bfloat16,'F8_E4M3':torch.float8_e4m3fn,'F32':torch.float32}
stand={}
class FakeOpen:
    def __init__(self,path,**k):self.file=Path(path).name
    def __enter__(self):return self
    def __exit__(self,*a):pass
    def get_tensor(self,name):
        base,h=header(self.file);meta=h[name]
        t=_zeros(1,dtype=DT[meta['dtype']]).expand(meta['shape'])
        stand[name]=(self.file,base+meta['data_offsets'][0],meta['data_offsets'][1]-meta['data_offsets'][0],meta['dtype'],tuple(meta['shape']))
        return t
sys.modules['safetensors']=types.SimpleNamespace(safe_open=FakeOpen)

spec=importlib.util.spec_from_file_location('real2driver',driver_path);drv=importlib.util.module_from_spec(spec);spec.loader.exec_module(drv)

class Recorder:
    def __init__(self):self.calls=[]
    def launch(self,name,gx,gy,*args):drv.check_bounds(name,args);self.calls.append((name,gx,gy,args))
rec=Recorder()
if fixture is not None:
    # Only the ids change; the driver's top5 references belong to its own fixture and are not used by the emitter.
    drv.GOLD=dict(drv.GOLD,prompt_ids=list(fixture['prompt_ids']),generated_ids=list(fixture['generated_ids']))
if capacity is not None:
    need=len(drv.GOLD['prompt_ids'])+len(drv.GOLD['generated_ids'])
    if capacity<need:raise SystemExit('capacity %d is below the %d positions this run needs'%(capacity,need))
    # Card writes self.cap in __init__; this class property keeps the requested capacity instead.
    drv.Card.cap=property(lambda self:capacity,lambda self,value:None)
card=drv.Card(torch,rec,'controls' if controls else 'omega')

# Buffers: one per distinct storage. Weight storages come from the checkpoint; the rest are zero-filled device memory,
# except the RoPE table, which the harness computes exactly as Card does and ships as plan data.
bufs=[];by_ptr={};data_pool=bytearray()
def buffer_for(t):
    ptr=t.untyped_storage().data_ptr()
    if ptr in by_ptr:return by_ptr[ptr]
    raise KeyError('tensor storage not registered')
def add(ptr,entry):by_ptr[ptr]=len(bufs);bufs.append(entry)
for name,t in card.weights.items():
    file,off,length,dtype,shape=stand[name]
    if t.ndim==1 or t.dtype==torch.float32:
        assert dtype=='BF16' and len(shape)==1,(name,dtype,shape)
        mutated=controls and (name.endswith(('input_layernorm.weight','post_attention_layernorm.weight','q_norm.weight','k_norm.weight')) or name==drv.PFX+'norm.weight')
        if mutated:
            # CONTROLS plan only: the omitted norm +1 mutation Card applies in mode controls, shipped as plan data (kind 3).
            with (weights/file).open('rb') as f:f.seek(off);raw=f.read(length)
            vec=(torch.frombuffer(bytearray(raw),dtype=torch.bfloat16).float()-1).numpy().tobytes()
            add(t.untyped_storage().data_ptr(),dict(name=name,bytes=len(vec),kind=3,file=None,off=len(data_pool),len=len(vec),src=[file,off,length]));data_pool+=vec
        else:
            add(t.untyped_storage().data_ptr(),dict(name=name,bytes=length*2,kind=2,file=file,off=off,len=length))
    else:
        add(t.untyped_storage().data_ptr(),dict(name=name,bytes=length,kind=1,file=file,off=off,len=length))
cs=card.cs.contiguous().numpy().tobytes();cs_off=len(data_pool);data_pool+=cs
add(card.cs.untyped_storage().data_ptr(),dict(name='rope_cs',bytes=len(cs),kind=3,file=None,off=cs_off,len=len(cs)))

card.emit();calls=rec.calls
for name,gx,gy,args in calls:
    for a in args:
        if isinstance(a,torch.Tensor) and a.untyped_storage().data_ptr() not in by_ptr:
            s=a.untyped_storage();add(s.data_ptr(),dict(name='act%d'%len(bufs),bytes=s.nbytes(),kind=0,file=None,off=0,len=0))
tok_buf=buffer_for(card.tok)

# Kernarg size from each code object's kernel descriptor (.kd symbol, kernarg_size u32 at descriptor offset 8).
def kernarg_size(raw,symbol):
    shoff,=struct.unpack_from('<Q',raw,0x28);shentsize,shnum=struct.unpack_from('<HH',raw,0x3a)
    secs=[struct.unpack_from('<IIQQQQIIQQ',raw,shoff+i*shentsize) for i in range(shnum)]
    for s in secs:
        if s[1]!=2:continue
        strtab=secs[s[6]];
        for j in range(s[5]//24):
            st_name,_,_,_,value,_=struct.unpack_from('<IBBHQQ',raw,s[4]+j*24)
            end=raw.index(b'\0',strtab[4]+st_name)
            if raw[strtab[4]+st_name:end]==(symbol+'.kd').encode():
                for p in secs:
                    if p[3]<=value<p[3]+p[5] and p[1]!=8:return struct.unpack_from('<I',raw,p[4]+value-p[3]+8)[0]
    raise KeyError(symbol+'.kd')
names=sorted({c[0] for c in calls});code={}
for n in names:
    sha,data,symbol=drv.OBJ[n];raw=lzma.decompress(base64.b85decode(data));assert hashlib.sha256(raw).hexdigest()==sha
    code[n]=dict(index=len(code),sha256=sha,symbol=symbol,kernarg=kernarg_size(raw,symbol),raw=raw)

def kernarg(name,args):
    blob=bytearray();relocs=[]
    for a in args:
        if isinstance(a,int):
            assert 0<=a<1<<32;blob+=struct.pack('<I',a)
        else:
            while len(blob)%8:blob+=b'\0'
            s=a.untyped_storage();relocs.append((len(blob),by_ptr[s.data_ptr()],a.data_ptr()-s.data_ptr()));blob+=bytes(8)
    size=code[name]['kernarg'];assert len(blob)<=size,(name,len(blob),size);blob+=bytes(size-len(blob))
    return bytes(blob),relocs
files=sorted({b['file'] for b in bufs if b['file']})
strings=bytearray();code_rows=[]
for n in names:code_rows.append((len(strings),len(code[n]['symbol'])));strings+=code[n]['symbol'].encode()
plan=bytearray(b'OMPLAN01'+struct.pack('<7Q',len(names),len(files),len(bufs),len(calls),len(drv.GOLD['prompt_ids']),len(drv.GOLD['generated_ids']),tok_buf))
for o,l in code_rows:plan+=struct.pack('<2Q',o,l)
for b in bufs:plan+=struct.pack('<6Q',b['bytes'],b['kind'],files.index(b['file']) if b['file'] else 0,b['off'],b['len'],2 if b is bufs[tok_buf] else 1)
plan+=b''.join(struct.pack('<i',t) for t in drv.GOLD['prompt_ids'])
while len(plan)%8:plan+=b'\0'
for name,gx,gy,args in calls:
    blob,relocs=kernarg(name,args)
    plan+=struct.pack('<8I',code[name]['index'],gx,gy,1,64,len(blob),len(relocs),0)+blob+b''.join(struct.pack('<IIQ',*r) for r in relocs)
    while len(plan)%8:plan+=b'\0'
spool=len(plan);plan+=strings
dpool=len(plan);plan+=data_pool
plan+=struct.pack('<4Q',spool,len(strings),dpool,len(data_pool))
out.write_bytes(plan)

# Independent decode of the written bytes, compared with the recorder (name, grid, every argument).
def decode(p):
    assert p[:8]==b'OMPLAN01';h=struct.unpack_from('<7Q',p,8);nc,nf,nb,nd,npm,ng,tb=h;q=64
    sp,sl,dp,dl=struct.unpack_from('<4Q',p,len(p)-32)
    cnames=[]
    for i in range(nc):o,l=struct.unpack_from('<2Q',p,q);q+=16;cnames.append(p[sp+o:sp+o+l].decode())
    btab=[struct.unpack_from('<6Q',p,q+48*i) for i in range(nb)];q+=48*nb
    prompt=list(struct.unpack_from('<%di'%npm,p,q));q+=4*npm;q+=(-q)%8
    disp=[]
    for i in range(nd):
        ci,gx,gy,gz,bx,kb,nr,_=struct.unpack_from('<8I',p,q);q+=32;blob=p[q:q+kb];q+=kb
        rel=[struct.unpack_from('<IIQ',p,q+16*j) for j in range(nr)];q+=16*nr;q+=(-q)%8
        for off,bi,bo in rel:
            assert bi<nb and bo<btab[bi][0] and blob[off:off+8]==bytes(8) and off%8==0
        disp.append((cnames[ci],gx,gy,gz,bx,blob,rel))
    assert q==sp,(q,sp)
    return dict(code=cnames,bufs=btab,prompt=prompt,disp=disp,n_gen=ng,tok=tb)
d=decode(bytes(out.read_bytes()))
assert len(d['disp'])==len(calls)
for (name,gx,gy,args),(dn,dgx,dgy,gz,bx,blob,rel) in zip(calls,d['disp']):
    assert dn==code[name]['symbol'] and (gz,bx)==(1,64),(name,dn,gz,bx)
    assert (gx,gy)==(dgx,dgy)
    ri=iter(rel);pos=0
    for a in args:
        if isinstance(a,int):assert struct.unpack_from('<I',blob,pos)[0]==a;pos+=4
        else:
            pos+=(-pos)%8;off,bi,bo=next(ri);s=a.untyped_storage()
            assert off==pos and bi==by_ptr[s.data_ptr()] and bo==a.data_ptr()-s.data_ptr();pos+=8
total=sum(b[0] for b in d['bufs'])
summary=dict(plan=str(out),bytes=len(plan),sha256=hashlib.sha256(plan).hexdigest(),controls=controls,code_objects={n:dict(sha256=code[n]['sha256'],kernarg=code[n]['kernarg']) for n in names},
    files=files,buffers=len(bufs),device_bytes=total,dispatches_per_step=len(calls),prompt=len(d['prompt']),generate=d['n_gen'],tok_buffer=tok_buf,
    by_kernel={n:sum(1 for c in calls if c[0]==n) for n in names},
    control_vectors=[[i,b['src'][0],b['src'][1],b['src'][2]] for i,b in enumerate(bufs) if 'src' in b],
    capacity=card.cap,rope_buffer=by_ptr[card.cs.untyped_storage().data_ptr()],
    kv_buffers=[by_ptr[s.untyped_storage().data_ptr()] for l in sorted(card.states) if len(card.states[l])==2 for s in card.states[l]],
    fixture=None if fixture is None else dict(path=str(fixture_path),sha256=hashlib.sha256(fixture_raw).hexdigest(),name=fixture.get('name'),
        prompt_ids=fixture['prompt_ids'],generated_ids=fixture['generated_ids']))
Path(str(out)+'.json').write_text(json.dumps(summary,indent=1)+'\n')
print('PLAN_DECODE_MATCH',json.dumps({k:summary[k] for k in ('bytes','sha256','buffers','device_bytes','dispatches_per_step','prompt','generate')}))
cdir=out.parent/'code';cdir.mkdir(exist_ok=True)
for n in names:(cdir/(n+'.hsaco')).write_bytes(code[n]['raw'])
args=['omega-launch','plan',out.name]+['code/'+n+'.hsaco' for n in names]+['$AMDSERVE_WEIGHTS/'+f for f in files]
Path(str(out)+'.cmd').write_text(' '.join(args)+'\n')
