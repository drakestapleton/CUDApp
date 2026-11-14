# Instrument (not Omega): HIP graph replay of N step_advance launches per step, the same kernel
# object as the Omega micro plan (driver-real2.py's embedded objects), timed the way real2 times
# a step: perf_counter before hipGraphLaunch, after hipStreamSynchronize. Also times the parts:
# launch call return, synchronize return, and a 4-byte token readback (tok.item()).
# Usage: gap-hipmicro.py DRIVER N STEPS OUT.json
import importlib.util,json,statistics,sys,time
from pathlib import Path
driver,N,S,out=Path(sys.argv[1]),int(sys.argv[2]),int(sys.argv[3]),Path(sys.argv[4])
spec=importlib.util.spec_from_file_location('real2driver',driver);drv=importlib.util.module_from_spec(spec);spec.loader.exec_module(drv)
import torch
hip=drv.Hip(torch)
pos=torch.zeros(1,dtype=torch.int32,device='cuda');tok=torch.zeros(1,dtype=torch.int32,device='cuda')
cap=131072
def emit():
    for _ in range(N):hip.launch('step_advance',1,1,pos,cap)
torch.cuda.synchronize()
g=hip.capture(emit)
rows=[]
try:
    for s in range(S+1):
        t0=time.perf_counter();hip.replay(g);t1=time.perf_counter();hip.sync();t2=time.perf_counter();v=int(tok.item());t3=time.perf_counter()
        if s:rows.append(((t2-t0)*1e6,(t1-t0)*1e6,(t2-t1)*1e6,(t3-t2)*1e6))
finally:hip.release_graph(g)
cols=list(zip(*rows))
res=dict(N=N,steps=S,step_us=list(cols[0]),median_step_us=statistics.median(cols[0]),median_launch_call_us=statistics.median(cols[1]),
         median_sync_us=statistics.median(cols[2]),median_token_readback_us=statistics.median(cols[3]),position=int(pos.item()))
out.write_text(json.dumps(res)+'\n')
print('HIPMICRO',json.dumps({k:v for k,v in res.items() if k!='step_us'}))
