#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Package the completed CPU acceptance evidence for the notebook relay.
This is an outside verification/transport instrument, not a model runtime.
"""
import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import sys
from decimal import Decimal

def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def bundle(evidence,dest):
    root=pathlib.Path(__file__).resolve().parents[2]; src=root/'backend/amd/gfx942'; out=evidence/'exec'
    host=(evidence/'host.log').read_text(); log=(evidence/'exec.log').read_text(); torch=(evidence/'torch.log').read_text()
    assert 'AMD-GFX942-HOST PASS' in host and 'AMD-GFX942-EXEC PASS' in log
    assert 'PASS torch-recurrent-rounding-boundaries kind=dense' in torch
    access=json.loads((out/'token-access.json').read_text())
    head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()
    assert not dest.exists(),'refusing to overwrite an evidence bundle'
    dest.mkdir(parents=True)
    manifest=dict(source_head=head,hardware='NOT_RUN',performance='NOT_RUN',compiler_sha256=re.search(r'PIN omc=([0-9a-f]{64})',log)[1],
                  evidence={n:dict(path=str(evidence/n),sha256=sha(evidence/n)) for n in ('host.log','exec.log','torch.log')},
                  access_evidence=dict(path=str(out/'token-access.json'),sha256=sha(out/'token-access.json')),kernels=[])
    lines=['# Token decode notebook objects','',f'Source head: `{head}`','',
           'Host/functional executor only. MI300X execution, timing, and full-model serving are NOT_RUN.',
           'Grid entries are workgroups; block is (64,1,1). All pointers are device uint64 addresses.',
           'FP32-word BF16 boundaries, state layout, row selection, and operation ordering are defined in PROVENANCE.md sections 21–22.',
           'The attention-prep concrete fixture has position=1 and capacity=4; cache lengths are capacity*4*256*2 bytes.',
           'Maximum offsets below are measured by the per-buffer-checking executor for the full fixture, including safe overdispatch.',
           'The reported numerical error is distance to the midpoint of the independently derived ISA interval, not hardware error.',
           'Sections use the existing v8 run_section(ctx) interface and generate their fixtures/oracles in-cell.',
           'Load a section in a fresh namespace, then supply its matching objects plus the established allocation/launch/copy context.',
           '', '| Object | SHA256 | Kernarg bytes | Grid |', '|---|---|---:|---|']
    for report in access:
        name=report['kernel']; table=(src/(name+'.isa')).read_text(); args=[]; i=0; payload=None
        for line in table.splitlines():
            fields=line.split()
            if fields and fields[0]=='A':
                _,label,offset,size,kind=fields
                a=dict(name=label,offset=int(offset),bytes=int(size),kind='pointer' if kind=='0' else 'u32')
                if kind=='0':
                    b=report['buffers'][i]; measured=report['objects'][0]['access'][i+1]
                    a.update(buffer_bytes=b['bytes'],max_read_byte=measured['max_read_byte'],max_write_byte=measured['max_write_byte']); i+=1
                else:
                    if payload is None: payload=json.loads((out/(name+'-intervals.json')).read_text())
                    a['fixture_value']=payload['cases'][-1]['args'][len(args)][1]
                args.append(a)
        assert i==len(report['buffers'])
        matches=re.findall(r'^PASS '+name+r' (.+) interval_escapes=0 worst_midpoint_error=([^ ]+).*$',log,re.M)
        assert matches,name
        worst=max((error for label,error in matches),key=Decimal)
        controls=[o['object'] for o in report['objects'][1:]]
        for control in controls: assert 'CONTROL '+control+' CAUGHT' in log,control
        assert 'CONTROL ACCESS '+name+' short-input CAUGHT exit=68' in log
        objects=[]
        for obj in report['objects']:
            variant=obj['object']; origin=out/(variant+('.1.hsaco' if variant==name else '.hsaco'))
            filename=variant+'.hsaco'; shutil.copyfile(origin,dest/filename)
            item=dict(file=filename,sha256=sha(origin),entry=name,bytes=origin.stat().st_size,access=obj['access'])
            objects.append(item)
            lines.append(f'| {filename} | `{item["sha256"]}` | {report["kernarg_bytes"]} | {report["grid"]} |')
        section=out/(name+'.py'); shutil.copyfile(section,dest/section.name)
        manifest['kernels'].append(dict(kernel=name,table_sha256=sha(src/(name+'.isa')),section=dict(file=section.name,sha256=sha(section)),
                 kernarg_bytes=report['kernarg_bytes'],args=args,grid=report['grid']+[1],block=report['block'],case=report['case'],
                 interval_escapes=0,worst_midpoint_error=worst,cases=[dict(label=l,worst_midpoint_error=e) for l,e in matches],controls_caught=controls,
                 access_control='short-input: exit 68',objects=objects))
    lines+=['','## Kernarg layouts and measured byte bounds','']
    for k in manifest['kernels']:
        lines+=['### '+k['kernel'],f'Worst interval-midpoint error: {k["worst_midpoint_error"]}; interval escapes: 0.',
                '| Arg | Offset | Kind | Buffer bytes | Max read byte | Max write byte |','|---|---:|---|---:|---:|---:|']
        for a in k['args']:
            lines.append('| '+ ' | '.join(str(a.get(key,'—')) for key in ('name','offset','kind','buffer_bytes','max_read_byte','max_write_byte'))+' |')
        lines.append('')
    (dest/'list.json').write_text(json.dumps(manifest,indent=2)+'\n')
    (dest/'list.md').write_text('\n'.join(lines)+'\n')
    (dest/'SHA256SUMS').write_text(''.join(sha(p)+'  '+p.name+'\n' for p in sorted(dest.iterdir()) if p.is_file()))
    print('BUNDLE',dest,'source_head='+head,'kernels='+str(len(access)))
if __name__=='__main__': bundle(pathlib.Path(sys.argv[1]).resolve(),pathlib.Path(sys.argv[2]).resolve())
