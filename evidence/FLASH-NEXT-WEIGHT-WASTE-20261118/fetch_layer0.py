import json,subprocess,sys,numpy as np,zlib,lzma,hashlib
B='https://huggingface.co/Qwen/Qwen3.8-Flash-Next-FP8/resolve/main/'
H=json.load(open('hdrs.json'))
P='model.language_model.layers.0.mlp.experts.'
def fetch(name):
    for sh,(n,h) in H.items():
        if name in h:
            a,b=h[name]['data_offsets']; o=8+n
            d=subprocess.run(['curl','-sS','-L','-m','300','--retry','3','-r',f'{o+a}-{o+b-1}',B+sh],capture_output=True).stdout
            assert len(d)==b-a,(name,len(d)); return np.frombuffer(d,dtype=np.uint8 if h[name]['dtype']=='F8_E4M3' else np.uint16).reshape(h[name]['shape'])
    raise KeyError(name)
ex=[int(x) for x in sys.argv[1:]]
out={}
for e in ex:
    for m in ('gate_proj','up_proj','down_proj'):
        out[f'{e}.{m}']=fetch(f'{P}{e}.{m}.weight'); out[f'{e}.{m}.s']=fetch(f'{P}{e}.{m}.weight_scale_inv')
np.savez('l0.npz',**out)
print('saved',len(out),'sha256',hashlib.sha256(open('l0.npz','rb').read()).hexdigest())
