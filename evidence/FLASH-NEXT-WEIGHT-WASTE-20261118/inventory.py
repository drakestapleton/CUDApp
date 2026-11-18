import json,subprocess,struct,collections,re,sys,hashlib
from concurrent.futures import ThreadPoolExecutor
REV=sys.argv[1]
B=f'https://huggingface.co/Qwen/Qwen3.8-Flash-Next-FP8/resolve/{REV}/'
ix=json.load(open('index.json'))['weight_map']
shards=sorted(set(ix.values()))
def get(u,*a):
    for _ in range(4):
        r=subprocess.run(['curl','-sS','-L','-m','180',*a,u],capture_output=True)
        if r.returncode==0 and r.stdout: return r.stdout
    raise RuntimeError(u)
def hdr(sh):
    n=struct.unpack('<Q',get(B+sh,'-r','0-7'))[0]
    return sh,get(B+sh,'-r',f'8-{7+n}')
def cat(k):
    if k.startswith('model.visual'): return 'vision tower'
    if k.startswith('mtp.'): return 'MTP head'
    if 'ngram_embedding' in k: return 'n-gram table'
    if '.mlp.experts.' in k: return 'routed experts (scales)' if k.endswith('scale_inv') else 'routed experts (weights)'
    if k=='lm_head.weight': return 'lm_head'
    if k.endswith('embed_tokens.weight'): return 'token embedding'
    if '.ple.' in k: return 'PLE projections'
    if 'hyper_connection' in k: return 'gated residual'
    if '.linear_attn.' in k: return 'GDN'
    if '.self_attn.' in k: return 'QSA attention + indexer'
    if '.mlp.' in k: return 'router + shared expert'
    return 'other'
by=collections.Counter(); dt=collections.defaultdict(collections.Counter); hh=hashlib.sha256(); nt=0
with ThreadPoolExecutor(8) as ex:
    for sh,raw in sorted(ex.map(hdr,shards)):
        hh.update(raw)
        for k,v in json.loads(raw).items():
            if k=='__metadata__': continue
            a,b=v['data_offsets']; c=cat(k); by[c]+=b-a; dt[c][v['dtype']]+=b-a; nt+=1
print('revision',REV,'shards',len(shards),'tensors',nt,'headers_sha256',hh.hexdigest())
tot=sum(by.values())
for c,n in by.most_common(): print(f'{c:28s} {n:>15d} B {n/2**30:8.3f} GiB  {dict(dt[c])}')
print(f'{"TOTAL":28s} {tot:>15d} B {tot/2**30:8.3f} GiB')
json.dump({'revision':REV,'bytes':by,'dtypes':dt},open('inventory.json','w'),indent=1)
