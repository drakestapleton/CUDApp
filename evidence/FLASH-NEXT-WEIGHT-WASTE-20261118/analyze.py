import numpy as np,zlib,lzma,sys
Z=np.load(sys.argv[1])
c=np.arange(256); s=np.where(c&128,-1.0,1.0); e=(c>>3)&15; m=c&7
T=s*np.where(e==0,2.0**-6*(m/8),2.0**(e-7.0)*(1+m/8)); T[(c&127)==127]=np.nan
def H(x,n=256):
    p=np.bincount(x.ravel(),minlength=n).astype(float); p=p[p>0]/x.size; return -(p*np.log2(p)).sum()
def deq(k):
    w=T[Z[k]]; sc=Z[k+'.s'].view(np.uint16).astype(np.uint32)
    f=(sc<<16).astype(np.uint32).view(np.float32).astype(np.float64)
    return w*np.kron(f,np.ones((128,128)))[:w.shape[0],:w.shape[1]]
ex=sorted({int(k.split('.')[0]) for k in Z.files})
for mname in ('gate_proj','up_proj','down_proj'):
    allb=np.concatenate([Z[f'{x}.{mname}'].ravel() for x in ex])
    h0=H(allb); mag=H(allb&127,128); used=np.unique(allb).size
    # per-block adaptive order-0 entropy
    hb=[]
    for x in ex:
        a=Z[f'{x}.{mname}']; r,cc=a.shape
        for i in range(0,r,128):
            for j in range(0,cc,128): hb.append(H(a[i:i+128,j:j+128]))
    raw=allb.tobytes()
    print(f'{mname}: bytes={allb.size} order0={h0:.3f} bits mag7={mag:.3f} distinct={used} zero_codes={np.mean((allb&127)==0):.4f} nan={np.mean((allb&127)==127):.5f} perblock_H={np.mean(hb):.3f} zlib9={8*len(zlib.compress(raw,9))/allb.size:.3f} xz={8*len(lzma.compress(raw[:20000000],preset=6))/min(allb.size,20000000):.3f} maxcode_used={np.mean((allb&127)>=120):.5f}')
# neuron importance and structure
for x in ex[:4]:
    g,u,d=deq(f'{x}.gate_proj'),deq(f'{x}.up_proj'),deq(f'{x}.down_proj')
    imp=np.linalg.norm(g,axis=1)*np.linalg.norm(u,axis=1)*np.linalg.norm(d,axis=0); imp/=imp.sum()
    si=np.sort(imp)
    sv=[np.linalg.svd(a,compute_uv=False) for a in (g,u,d)]
    rk=[int(np.searchsorted(np.cumsum(v**2)/np.sum(v**2),q))+1 for v in sv for q in (0.99,)]
    print(f'expert {x}: neuron-importance min/median/max ratio={si[0]/np.median(si):.3f}/1/{si[-1]/np.median(si):.2f} bottom10%share={si[:64].sum():.4f} exact-zero-neurons={(imp==0).sum()} rank@99%energy g,u,d={rk} of 640 | rms g,u,d={g.std():.4f},{u.std():.4f},{d.std():.4f}')
D=[deq(f'{x}.down_proj').ravel() for x in ex]
C=np.corrcoef(np.stack(D)); off=C[~np.eye(len(ex),dtype=bool)]
print('cross-expert down_proj cosine: mean %.4f max %.4f'%(off.mean(),off.max()))
