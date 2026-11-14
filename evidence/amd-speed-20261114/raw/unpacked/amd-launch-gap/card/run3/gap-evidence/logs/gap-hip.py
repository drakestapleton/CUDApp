# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Instrument (not Omega): the published HIP graph arm, lighthouse-oracle.py mode `graph`, keeping the
per-step decode times and all 256 ids instead of only the median. Same driver, Card, kernels and timer:
real2 Card.forward begins perf_counter before hipGraphLaunch and stops after hipStreamSynchronize; host
readback of logits and the token id happen after the timer stops.
Usage: gap-hip.py DRIVER WEIGHTS FIXTURE OUT.json CAPACITY"""
import hashlib,importlib.util,json,os,statistics,sys
from pathlib import Path

driver,weights,fixture_path,out,cap=Path(sys.argv[1]),Path(sys.argv[2]),Path(sys.argv[3]),Path(sys.argv[4]),int(sys.argv[5])
fixture=json.loads(fixture_path.read_bytes())
P,G=fixture['prompt_ids'],fixture['generated_ids']
os.environ['AMDSERVE_WEIGHTS']=str(weights);os.environ['AMDSERVE_OUTPUT']=str(out)+'.log'
spec=importlib.util.spec_from_file_location('real2driver',driver);drv=importlib.util.module_from_spec(spec);spec.loader.exec_module(drv)
drv.GOLD=dict(drv.GOLD,prompt_ids=list(P),generated_ids=list(G),top5=[[(0,0.0)]]*len(G))
drv.top5_difference=lambda logits,expected:None
if cap<len(P)+len(G):raise SystemExit('capacity below prompt+generated')
drv.Card.cap=property(lambda self:cap,lambda self,value:None)
import torch,time
drv.prepare()
# Same-clock record: real2's forward returns (logits.cpu(), tok.item(), elapsed) where elapsed stops after
# hipStreamSynchronize; the wall time of the whole call also covers the logits copy and token read.
walls=[];orig=drv.Card.forward
def timed(self,*a,**k):
    t0=time.perf_counter();r=orig(self,*a,**k);walls.append(((time.perf_counter()-t0)*1000,r[2]));return r
drv.Card.forward=timed
with torch.inference_mode():r=drv.measure(torch,'omega-graph')
dec=walls[len(walls)-len(r['decode_ms']):]
res=dict(forward_wall_ms=[w for w,_ in dec],forward_wall_ms_median=statistics.median(w for w,_ in dec),mode='graph',capacity=cap,ids=r['ids'],decode_ms=r['decode_ms'],decode_ms_median=r['decode_ms_median'],
         capture_ms=r['capture_ms'],timing_mode=r['timing_mode'],fixture_sha256=hashlib.sha256(fixture_path.read_bytes()).hexdigest(),
         driver=str(driver),driver_sha256=hashlib.sha256(driver.read_bytes()).hexdigest())
out.write_text(json.dumps(res)+'\n')
print('GAPHIP',json.dumps({k:v for k,v in res.items() if k not in ('ids','decode_ms')}))
