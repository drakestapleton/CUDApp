import json,subprocess,struct,sys,numpy as np,hashlib
B='https://huggingface.co/Qwen/Qwen3.8-Flash-Next-FP8/resolve/main/'
ix=json.load(open('index.json'))['weight_map']
def get(u,*a): return subprocess.run(['curl','-sS','-L','-m','300','--retry','3',*a,u],capture_output=True).stdout
hc={}
def hdr(sh):
    if sh not in hc:
        n=struct.unpack('<Q',get(B+sh,'-r','0-7'))[0]; hc[sh]=(n,json.loads(get(B+sh,'-r',f'8-{7+n}')))
    return hc[sh]
out={}
for L in (11,24,47):
    for e in (5,130,409):
        for m in ('gate_proj','up_proj','down_proj'):
            for suf,dt in (('weight',np.uint8),('weight_scale_inv',np.uint16)):
                name=f'model.language_model.layers.{L}.mlp.experts.{e}.{m}.{suf}'
                n,h=hdr(ix[name]); a,b=h[name]['data_offsets']; o=8+n
                d=get(B+ix[name],'-r',f'{o+a}-{o+b-1}'); assert len(d)==b-a
                out[f'{L}_{e}.{m}'+('' if suf=='weight' else '.s')]=np.frombuffer(d,dtype=dt).reshape(h[name]['shape'])
np.savez('lx.npz',**out); print('sha256',hashlib.sha256(open('lx.npz','rb').read()).hexdigest())
