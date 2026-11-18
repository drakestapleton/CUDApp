import numpy as np,subprocess,sys,hashlib,os
tot=[0,0]; ok=True; bad=0; nt=0
for f in sys.argv[1:]:
    Z=np.load(f)
    for k in Z.files:
        if k.endswith('.s'): continue
        a=Z[k]; a.tofile('t.raw'); r,c=a.shape
        n,z,rt=map(int,subprocess.check_output(['./rowrans','t.raw',str(r),str(c)]).split())
        n2,z2,rt2=map(int,subprocess.check_output(['./rowrans','t.raw',str(r),str(c),'flip']).split())
        tot[0]+=n; tot[1]+=z; ok&=(rt==1); bad+=(rt2==0); nt+=1
os.remove('t.raw')
print(f'tensors={nt} raw={tot[0]} coded={tot[1]} bits/weight={8*tot[1]/tot[0]:.4f} ratio={tot[1]/tot[0]:.4f} all_roundtrip_bit_exact={ok} flipped_bit_detected={bad}/{nt}')
