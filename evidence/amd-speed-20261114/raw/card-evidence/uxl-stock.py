import json,time,statistics,sys,traceback
import torch,transformers
from transformers import pipeline, StoppingCriteria, StoppingCriteriaList
OUT='/workspace/uxl/uxl-stock.json'
R={'what':'AMD stock notebook configuration: transformers pipeline image-text-to-text, Qwen/Qwen3.8-27B BF16, device_map=cuda; lighthouse prompt; greedy; 256 new tokens','torch':torch.__version__,'transformers':transformers.__version__,'rows':[]}
def save(): open(OUT,'w').write(json.dumps(R,indent=1))
try:
    req=json.load(open('/workspace/uxl/CUDApp/evidence/amd-qwen38-native-refusal-20261113/spark-baseline/decode1.request.json'))
    prompt=req['messages'][0]['content']; R['prompt']=prompt
    t0=time.perf_counter()
    pipe=pipeline("image-text-to-text", model="Qwen/Qwen3.8-27B", device_map="cuda")
    R['load_s']=time.perf_counter()-t0; R['dtype']=str(next(pipe.model.parameters()).dtype); save()
    msgs=[{"role":"user","content":[{"type":"text","text":prompt}]}]
    class T(StoppingCriteria):
        def __init__(s): s.t=[]; s.ids=None; s.first_len=None
        def __call__(s,input_ids,scores,**k):
            torch.cuda.synchronize(); s.t.append(time.perf_counter())
            if s.first_len is None: s.first_len=int(input_ids.shape[1])
            s.ids=input_ids[0]
            return False
    for label in ['warmup','run1','run2','run3','run4','run5']:
        tc=T(); torch.cuda.synchronize(); b=time.perf_counter()
        o=pipe(text=msgs, max_new_tokens=256, min_new_tokens=256, do_sample=False, return_full_text=False, generate_kwargs={'stopping_criteria':StoppingCriteriaList([tc])})
        torch.cuda.synchronize(); e=time.perf_counter()
        d=[(y-x)*1000 for x,y in zip(tc.t,tc.t[1:])]
        row={'label':label,'total_s':e-b,'calls':len(tc.t)}
        if d:
            row.update(first_token_s=tc.t[0]-b,decode_ms_median=statistics.median(d),decode_ms_min=min(d),decode_ms_max=max(d),decode_ms=d,prompt_tokens=tc.first_len-1,generated_ids=tc.ids[tc.first_len-1:].tolist())
        row['text_head']=str(o[0].get('generated_text'))[:160]
        R['rows'].append(row); save(); print(label,row.get('decode_ms_median'),row['total_s'],flush=True)
    R['vram_peak_bytes']=torch.cuda.max_memory_allocated(); R['status']='complete'; save()
except Exception:
    R['status']='exception'; R['error']=traceback.format_exc(); save(); raise
