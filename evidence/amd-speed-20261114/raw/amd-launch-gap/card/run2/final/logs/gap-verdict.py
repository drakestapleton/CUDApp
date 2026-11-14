# Instrument (not Omega): per-run ids and timing verdicts for the launch-gap card run.
# Usage: gap-verdict.py LOGDIR PUBLISHED_IDS.json   Prints one line per run and writes verdict.json.
import json,re,statistics,sys
from pathlib import Path
d=Path(sys.argv[1]);pub=json.loads(Path(sys.argv[2]).read_text())['ids']
rows=[];ok=True
for f in sorted(d.glob('omega-*.txt')):
    t=f.read_text()
    steps=[(int(a),int(b),int(c)) for a,b,c in re.findall(r'^OMEGA_STEP (\d+) tok=(-?\d+) us=(\d+)$',t,re.M)]
    m=re.search(r'^OMEGA_PLAN ids=\[([0-9,-]+)\]$',t,re.M)
    ids=[int(x) for x in m.group(1).split(',')] if m else None
    us=[u for _,_,u in steps]
    row=dict(run=f.stem,steps=len(steps),complete='OMEGA_PLAN complete' in t,ids_equal_published=ids==pub,
             first_difference=None if ids is None or ids==pub else next((i for i,(a,b) in enumerate(zip(ids,pub)) if a!=b),len(ids)),
             prefill_us=sum(us[:30]),decode_us=us[30:],decode_median_us=statistics.median(us[30:]) if len(us)>30 else None)
    control='control' in f.stem
    row['verdict']=('CAUGHT' if control and ids is not None and ids!=pub else 'ESCAPED' if control else
                    'PASS' if ids==pub and len(steps)==285 else 'FAIL')
    ok&=row['verdict'] in ('PASS','CAUGHT');rows.append(row)
for f in sorted(d.glob('hip-*.json')):
    v=json.loads(f.read_text())
    row=dict(run=f.stem,ids_equal_published=v['ids']==pub,decode_ms=v['decode_ms'],decode_median_ms=v['decode_ms_median'])
    row['verdict']='PASS' if v['ids']==pub and len(v['decode_ms'])==255 else 'FAIL'
    ok&=row['verdict']=='PASS';rows.append(row)
for r in rows:
    print('VERDICT',r['run'],r['verdict'],'ids_equal_published=%s'%r['ids_equal_published'],
          'decode_median=%s'%(r.get('decode_median_us') if 'decode_median_us' in r else r.get('decode_median_ms')))
(d/'verdict.json').write_text(json.dumps(dict(ok=ok,rows=rows))+'\n')
print('GAP_VERDICT','PASS' if ok else 'FAIL',len(rows),'runs')
