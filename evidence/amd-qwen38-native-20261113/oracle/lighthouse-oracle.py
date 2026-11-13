"""Harness (oracle only): run real2's own Card and AMD-library reference on the lighthouse256 fixture to place the
native plan's first divergence (index 109). Card source is not edited; GOLD and capacity are overridden here as in
emit-plan.py. One mode per process, so each gets a fresh device:
  graph     Card, HIP graph replay, free-running greedy (the path the native plan was recorded from)
  forced    Card, HIP graph replay, teacher-forced on the Spark ids: at every generated index, Spark token rank,
            Omega top-1 minus top-2 logit and top-1 minus Spark-token logit
  reference AMD library (transformers, same FP8 checkpoint), free-running greedy, top-5 per index
Usage: lighthouse-oracle.py DRIVER WEIGHTS FIXTURE MODE OUT.json [--capacity=N]   (capacity default: fixture's)
Reading: graph ids equal to the native ids means the plan is faithful to Card. A small margin at the first divergence,
with reference also near-tied there, points to precision; a large margin or reference agreeing with Spark points to a kernel.
"""
import importlib.util,json,os,statistics,sys
from pathlib import Path

driver,weights,fixture_path,mode,out=Path(sys.argv[1]),Path(sys.argv[2]),Path(sys.argv[3]),sys.argv[4],Path(sys.argv[5])
if mode not in ('graph','forced','reference'):raise SystemExit('mode must be graph, forced or reference')
fixture=json.loads(fixture_path.read_bytes())
cap=next((int(a.split('=',1)[1]) for a in sys.argv[6:] if a.startswith('--capacity=')),fixture.get('capacity'))
P,G=fixture['prompt_ids'],fixture['generated_ids']
os.environ['AMDSERVE_WEIGHTS']=str(weights);os.environ['AMDSERVE_OUTPUT']=str(out)+'.log'
spec=importlib.util.spec_from_file_location('real2driver',driver);drv=importlib.util.module_from_spec(spec);spec.loader.exec_module(drv)
# measure()/reference() compare against the driver's own top-5 references; this fixture has none, so those columns are void.
drv.GOLD=dict(drv.GOLD,prompt_ids=list(P),generated_ids=list(G),top5=[[(0,0.0)]]*len(G))
drv.top5_difference=lambda logits,expected:None
if cap is not None:
    if cap<len(P)+len(G):raise SystemExit('capacity below prompt+generated')
    drv.Card.cap=property(lambda self:cap,lambda self,value:None)
import torch
def first_div(ids):return next((i for i,(a,b) in enumerate(zip(ids,G)) if a!=b),None)
def row(i,logits):
    val,idx=logits.float().topk(5);s=G[i];ls=float(logits[s])
    return dict(i=i,spark=s,top5=list(zip(idx.tolist(),[round(v,4) for v in val.tolist()])),
                spark_rank=int((logits.float()>ls).sum()),top1_minus_top2=round(float(val[0]-val[1]),4),top1_minus_spark=round(float(val[0])-ls,4))
drv.prepare()
if mode=='reference':
    r=drv.reference(torch);ids=r['ids']
    res=dict(mode=mode,ids=ids,first_divergence=first_div(ids),match=sum(a==b for a,b in zip(ids,G)),top5=r['top5'],decode_ms_median=r.get('decode_ms_median'))
elif mode=='graph':
    with torch.inference_mode():r=drv.measure(torch,'omega-graph')
    ids=r['ids'];res=dict(mode=mode,capacity=cap,ids=ids,first_divergence=first_div(ids),match=sum(a==b for a,b in zip(ids,G)),top5=r['top5'],decode_ms_median=r['decode_ms_median'])
else:
    card=drv.Card(torch,drv.Hip(torch),'omega');rows=[]
    try:
        with torch.inference_mode():
            for t in P[:-1]:card.forward(t,graph=True)
            logits,_,_=card.forward(P[-1],graph=True);rows.append(row(0,logits))
            for i in range(1,len(G)):
                logits,_,_=card.forward(G[i-1],graph=True);rows.append(row(i,logits))
    finally:
        if card.graph is not None:card.hip.release_graph(card.graph)
    bad=[r for r in rows if r['spark_rank']!=0]
    res=dict(mode=mode,capacity=cap,rows=rows,disagree=[r['i'] for r in bad],
             disagree_max_top1_minus_spark=max((r['top1_minus_spark'] for r in bad),default=0.0),
             median_top1_minus_top2=statistics.median(r['top1_minus_top2'] for r in rows))
res.update(fixture_sha256=__import__('hashlib').sha256(fixture_path.read_bytes()).hexdigest(),driver=str(driver))
out.write_text(json.dumps(res)+'\n')
print('ORACLE',mode,json.dumps({k:v for k,v in res.items() if k not in ('ids','top5','rows')}))
