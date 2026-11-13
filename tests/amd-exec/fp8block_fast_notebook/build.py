#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Outside notebook packaging instrument; no kernel production dependency."""
import base64
import hashlib
import json
from pathlib import Path
import zlib

HERE=Path(__file__).resolve().parent

def sha(data): return hashlib.sha256(data).hexdigest()
def encoded(data): return base64.b85encode(zlib.compress(data,9)).decode()

def main():
    manifest=json.loads((HERE/'list.json').read_text())
    report=json.loads((HERE/'report.json').read_text())
    objects={}
    for kernel in manifest['kernels']:
        for item in kernel['objects']:
            raw=(HERE/item['file']).read_bytes()
            assert sha(raw)==item['sha256']
            name=Path(item['file']).stem
            assert name not in objects
            objects[name]=dict(sha256=item['sha256'],base64=base64.b64encode(raw).decode(),entry=item['entry'])
    timed_kernels=[kernel['name'] for kernel in manifest['kernels']]
    shapes=[[c['M'],c['K']] for c in report['cases'] if c.get('checkpoint')]
    assert len(shapes)==len({tuple(s) for s in shapes})
    controls=[c['mutant'] for c in report['controls'] if 'mutant' in c]
    groups=[dict(id='ctl-'+c,kind='control') for c in controls]
    groups += [dict(id='correct-%dx%d'%tuple(s),kind='correctness',shape=s) for s in shapes]
    groups += [dict(id='tails',kind='tails')]
    groups += [dict(id='time-'+name+'-%dx%d'%tuple(s),kind='timing',shape=s,kernel=name)
               for s in shapes for name in timed_kernels]
    groups += [dict(id='ref-%dx%d'%tuple(s),kind='reference',shape=s) for s in shapes]
    data=dict(source_head=manifest['source_head'],baseline_source_head=manifest['baseline_source_head'],
              timed_kernels=timed_kernels,groups=groups,objects=objects,
              tails=[[c['M'],c['K']] for c in report['cases'] if c['case'].startswith('tail-')])
    template=(HERE/'driver.template.py').read_text()
    assert template.count('@DATA@')==1
    driver=template.replace('@DATA@',encoded(json.dumps(data,separators=(',',':')).encode()))
    compile(driver,'driver-v13.py','exec')
    digest=sha(driver.encode())
    cell=("# Paste the complete cell into the MI300X notebook. No model weights or downloads needed.\n"
          "import base64, hashlib, pathlib, subprocess, sys, zlib\n"
          "try:\n"
          "    _raw=zlib.decompress(base64.b85decode(%r))\n" % encoded(driver.encode())+
          "    assert hashlib.sha256(_raw).hexdigest()==%r, 'driver digest mismatch'\n" % digest+
          "    _driver=pathlib.Path('driver-v13.py'); _driver.write_bytes(_raw)\n"
          "    _run=subprocess.run([sys.executable,str(_driver.resolve())])\n"
          "    if _run.returncode: raise RuntimeError('probe parent exit='+str(_run.returncode))\n"
          "except BaseException:\n"
          "    print('CONTROLS ESCAPED or NOT_RUN; cell wrapper failed',flush=True)\n"
          "    print('RESULT overall FAIL_OR_INCOMPLETE; cell wrapper failed',flush=True)\n"
          "    raise\n")
    compile(cell,'cell-v13.py','exec')
    size=len(cell.encode())
    assert size<60000,('notebook cell size alarm',size)
    (HERE/'driver-v13.py').write_text(driver)
    (HERE/'cell-v13.py').write_text(cell)
    result=dict(file='cell-v13.py',bytes=size,sha256=sha(cell.encode()),driver_sha256=digest,
                source_head=manifest['source_head'],template_sha256=sha(template.encode()),
                source_manifest_sha256=sha((HERE/'list.json').read_bytes()),groups=groups,
                hardware='NOT_RUN',performance='NOT_RUN')
    (HERE/'build.json').write_text(json.dumps(result,indent=2)+'\n')
    print('CELL cell-v13.py bytes=%d sha256=%s groups=%d' % (size,result['sha256'],len(groups)))

if __name__=='__main__': main()
