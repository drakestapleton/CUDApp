"""Harness check, independent of emit-plan.py: parse PLAN with FORMAT.md rules only, then
(1) every file-backed buffer is exactly one checkpoint tensor's byte range (from the safetensors headers),
(2) kind 2/4 buffers are 1-D BF16 vectors and their widened values match safetensors' own decoding,
(3) the dispatch table obeys every structural rule L1214 validates,
(4) negative controls: a plan with one corrupted relocation and one with a misaligned weight range are refused,
(5) capacity: the RoPE table, the 32 KV caches, the attention partition grid and every capacity kernel argument
    agree with the capacity recorded beside the plan; checking against any other capacity is refused.
Usage: check-plan.py PLAN WEIGHTS
"""
import hashlib,json,struct,sys
from pathlib import Path
import torch
from safetensors import safe_open
plan_path,weights=Path(sys.argv[1]),Path(sys.argv[2]);summary=json.loads(Path(str(plan_path)+'.json').read_text())
ranges={}
for f in summary['files']:
    with (weights/f).open('rb') as h:
        n=struct.unpack('<Q',h.read(8))[0];hd=json.loads(h.read(n))
    for name,m in hd.items():
        if name!='__metadata__':ranges[(f,8+n+m['data_offsets'][0],m['data_offsets'][1]-m['data_offsets'][0])]=(name,m['dtype'],m['shape'])
def check(p,deep):
    assert p[:8]==b'OMPLAN01';nc,nf,nb,nd,npm,ng,tb=struct.unpack_from('<7Q',p,8);assert npm>=1 and ng>=1 and nf==len(summary['files'])
    sp,sl,dp,dl=struct.unpack_from('<4Q',p,len(p)-32);assert dp==sp+sl and dp+dl==len(p)-32
    q=64;codes=[]
    for i in range(nc):
        o,l=struct.unpack_from('<2Q',p,q);q+=16;assert o+l<=sl;codes.append(p[sp+o:sp+o+l].decode())
    bufs=[struct.unpack_from('<6Q',p,q+48*i) for i in range(nb)];q+=48*nb
    names=[]
    for size,kind,fi,off,length,flags in bufs:
        assert flags in (1,2)
        if kind==0:assert fi==off==length==0;names.append(None)
        elif kind==3:assert fi==0 and off+length<=dl and size==length;names.append(None)
        else:
            assert kind in (1,2) and fi<nf
            key=(summary['files'][fi],off,length);assert key in ranges,('not a tensor range',key)
            name,dtype,shape=ranges[key];names.append((fi,name))
            if kind==1:assert size==length
            else:assert dtype=='BF16' and len(shape)==1 and length%2==0 and size==2*length
    assert bufs[tb][5]==2 and bufs[tb][0]>=4
    q+=4*npm;q+=(-q)%8
    for i in range(nd):
        ci,gx,gy,gz,bx,kb,nr,pad=struct.unpack_from('<8I',p,q);q+=32
        assert ci<nc and gx>=1 and gy>=1 and gz==1 and bx==64 and pad==0
        blob=p[q:q+kb];q+=kb;slots=set()
        for j in range(nr):
            off,bi,bo=struct.unpack_from('<IIQ',p,q);q+=16
            assert off%8==0 and off+8<=kb and blob[off:off+8]==bytes(8) and off not in slots and bi<nb and bo<bufs[bi][0];slots.add(off)
        q+=(-q)%8
    assert q==sp,(q,sp)
    if deep:
        widened=0
        for (size,kind,fi,off,length,flags),nm in zip(bufs,names):
            if kind!=2:continue
            fi_,name=nm
            with safe_open(str(weights/summary['files'][fi_]),framework='pt') as f:t=f.get_tensor(name).float()
            with (weights/summary['files'][fi_]).open('rb') as h:h.seek(off);raw=h.read(length)
            assert torch.equal(torch.frombuffer(bytearray(raw),dtype=torch.bfloat16).float(),t);widened+=1
        # CONTROLS vectors (kind 3) must be exactly the checkpoint vector widened minus 1, decoded by safetensors itself.
        mutated=0
        for bi,file,foff,flen in summary['control_vectors']:
            size,kind,fi,off,length,flags=bufs[bi];assert kind==3
            name,dtype,shape=ranges[(file,foff,flen)]
            with safe_open(str(weights/file),framework='pt') as f:t=f.get_tensor(name).float()-1
            assert torch.equal(torch.frombuffer(bytearray(p[dp+off:dp+off+length]),dtype=torch.float32),t);mutated+=1
        assert mutated==(161 if summary['controls'] else 0),mutated
        return dict(code=len(codes),buffers=nb,file_backed=sum(1 for n in names if n),widened=widened,control_vectors=mutated,dispatches=nd)
def capcheck(p,cap):
    # Independent of Card: RoPE recomputed from config.json, sizes from the attention shape (4 KV heads x 256, BF16).
    nc,nf,nb,nd,npm,ng,tb=struct.unpack_from('<7Q',p,8);assert cap>=npm+ng
    sp,sl,dp,dl=struct.unpack_from('<4Q',p,len(p)-32);q=64;codes=[]
    for i in range(nc):o,l=struct.unpack_from('<2Q',p,q);q+=16;codes.append(p[sp+o:sp+o+l].decode())
    bufs=[struct.unpack_from('<6Q',p,q+48*i) for i in range(nb)];q+=48*nb
    cfg=json.loads((weights/'config.json').read_text())['text_config'];theta=cfg['rope_parameters']['rope_theta']
    angles=torch.arange(cap,dtype=torch.float64).unsqueeze(1)/torch.pow(torch.tensor(theta,dtype=torch.float64),torch.arange(32,dtype=torch.float64)*2/64)
    want=torch.cat((angles.cos(),angles.sin()),dim=1).float().numpy().tobytes()
    size,kind,fi,off,length,flags=bufs[summary['rope_buffer']];assert kind==3 and length==len(want) and p[dp+off:dp+off+length]==want,'rope table'
    kv=summary['kv_buffers'];assert len(kv)==2*sum(1 for k in cfg['layer_types'] if k=='full_attention')==32,len(kv)
    for bi in kv:assert bufs[bi][1]==0 and bufs[bi][0]==4*cap*256*2,('kv size',bi,bufs[bi][0])
    q+=4*npm;q+=(-q)%8;seen={}
    at={'attn256_prep_step':80,'attn256_part_step':48,'attn256_merge_step':48,'step_advance':8}
    for i in range(nd):
        ci,gx,gy,gz,bx,kb,nr,pad=struct.unpack_from('<8I',p,q);q+=32;blob=p[q:q+kb];q+=kb+16*nr;q+=(-q)%8;name=codes[ci]
        if name in at:
            assert struct.unpack_from('<I',blob,at[name])[0]==cap,(name,'capacity argument');seen[name]=seen.get(name,0)+1
        if name=='attn256_part_step':assert gx==(cap+63)//64,('partition grid',gx)
    assert seen=={'attn256_prep_step':16,'attn256_part_step':16,'attn256_merge_step':16,'step_advance':1},seen
    return dict(capacity=cap,kv_bytes=sum(bufs[bi][0] for bi in kv),rope_bytes=len(want),partitions=(cap+63)//64)
p=bytes(plan_path.read_bytes());result=check(p,True)
# Negative controls: each must be refused.
q=64+16*struct.unpack_from('<Q',p,8)[0]
r=q+48*next(i for i in range(struct.unpack_from('<Q',p,24)[0]) if struct.unpack_from('<Q',p,q+48*i+8)[0]==1)  # first raw (kind 1) buffer
bad=bytearray(p);struct.pack_into('<Q',bad,r+8*3,struct.unpack_from('<Q',p,r+24)[0]+8)  # its file_offset moved 8 bytes
caught=[]
for label,b in [('weight range shifted',bytes(bad))]:
    try:check(b,False)
    except AssertionError:caught.append(label)
nb=struct.unpack_from('<Q',p,24)[0];npm=struct.unpack_from('<Q',p,40)[0];d0=q+48*nb+4*npm;d0+=(-d0)%8
kb,nr=struct.unpack_from('<2I',p,d0+20);bad=bytearray(p);struct.pack_into('<I',bad,d0+32+kb,4)  # first reloc slot misaligned
try:check(bytes(bad),False)
except AssertionError:caught.append('reloc slot misaligned')
assert caught==['weight range shifted','reloc slot misaligned'],caught
cap=summary.get('capacity')
if cap is None:print('NOT_RUN capacity check: sidecar has no capacity record')
else:
    result['capacity']=capcheck(p,cap)
    for label,other in [('capacity halved',cap//2),('capacity plus 64',cap+64)]:
        try:capcheck(p,other)
        except AssertionError:caught.append(label)
    assert caught[2:]==['capacity halved','capacity plus 64'],caught
fx=summary.get('fixture')
def fixcheck(p,fx):
    # The plan header's prompt/generate counts and its prompt-id table equal the fixture file's own bytes, read again here.
    raw=Path(fx['path']).read_bytes();assert hashlib.sha256(raw).hexdigest()==fx['sha256'],'fixture file changed'
    f=json.loads(raw);nc,nf,nb,nd,npm,ng,tb=struct.unpack_from('<7Q',p,8)
    assert npm==len(f['prompt_ids']) and ng==len(f['generated_ids']),('counts',npm,ng)
    q=64+16*nc+48*nb;assert list(struct.unpack_from('<%di'%npm,p,q))==f['prompt_ids'],'prompt ids'
    assert fx['prompt_ids']==f['prompt_ids'] and fx['generated_ids']==f['generated_ids'],'sidecar ids'
    return dict(fixture=f.get('name'),sha256=fx['sha256'],prompt=npm,generate=ng)
if fx is None:print('NOT_RUN fixture check: plan uses the driver fixture')
else:
    result['fixture']=fixcheck(p,fx)
    bad=bytearray(p);q=64+16*struct.unpack_from('<Q',p,8)[0]+48*struct.unpack_from('<Q',p,24)[0];struct.pack_into('<i',bad,q,struct.unpack_from('<i',p,q)[0]+1)
    try:fixcheck(bytes(bad),fx)
    except AssertionError:caught.append('prompt id changed')
    assert caught[-1]=='prompt id changed',caught
print('PLAN_CHECK PASS',json.dumps(result),'CONTROLS CAUGHT',caught)
