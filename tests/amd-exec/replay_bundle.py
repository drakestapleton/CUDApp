#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Outside verification/transport instrument; never a model runtime dependency."""
import hashlib
import json
import pathlib
import runpy
import shutil
import subprocess
import sys

ROOT=pathlib.Path(__file__).resolve().parents[2]
SRC=ROOT/'backend/amd/gfx942'

def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()

def bundle(evidence,dest,golden,config):
    tokens=json.loads(golden.read_text())
    capacity=len(tokens['prompt_ids'])+len(tokens['generated_ids'])
    model=json.loads(config.read_text())
    vocab=model.get('text_config',model)['vocab_size']
    report=json.loads((evidence/'report.json').read_text())
    assert report['result']=='PASS' and report['hardware']=='NOT_RUN'
    assert capacity>0 and vocab>0
    for name,pin in report['source_sha256'].items():
        assert sha(ROOT/name)==pin, 'source changed after gate: '+name
    controls=runpy.run_path(str(ROOT/'tests/amd-exec/replay.py'))['MUTANTS']
    assert not dest.exists(),'refusing to overwrite an evidence bundle'
    head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
    # Model-shape constants belong to these specialized kernels. Capacity/vocabulary are caller inputs.
    cache=4*capacity*256*2
    tiles=(capacity+63)//64
    state=48*128*128*4
    definitions={
      'embed5120_step': ([80,1,1],[vocab*5120*2,5120*4,4]),
      'attn256_prep_step': ([24,1,1],[24*512*4,4*256*4,4*256*4,256*4,256*4,capacity*64*4,24*256*4,cache,cache,4]),
      'attn256_part_step': ([tiles,24,1],[24*256*4,cache,cache,24*tiles*1040,24*256*4,4]),
      'attn256_merge_step': ([1,24,1],[24*256*4,cache,cache,24*tiles*1040,24*256*4,4]),
      'gdnconv4_step': ([160,1,1],[10240*4,10240*8,10240*8,10240*8,10240*4,4]),
      'gdnstate_step': ([3,16,1],[2*16*128*4,48*128*4,state,state,48*128*4,48*4,48*4,4]),
      'step_advance': ([1,1,1],[4]),
      'readfirst_probe': ([1,1,1],[64*4,4]),
    }
    manifest=dict(source_head=head,hardware='NOT_RUN',capacity=capacity,vocab=vocab,
        compiler_sha256=report['compiler_sha256'],evidence=dict(path=str(evidence/'report.json'),sha256=sha(evidence/'report.json')),
        input_pins={str(p):sha(p) for p in (golden,config)},
        kernels=[],contract=dict(state_banks='even position A->B; odd B->A; start at position0',
        workspace='capacity-sized allocation, active ceil((position+1)/64)*1040 head stride',
        increment='exactly once after every layer and argmax; replay count must not exceed capacity',
        position='u32 device counter shared by all step kernels',token='u32 device output of argmax_final',
        rope='capacity rows of 32 cos + 32 sin FP32 words; one256-byte row per position'))
    dest.mkdir(parents=True)
    for entry in report['objects']:
        name=entry['name']; grid,sizes=definitions[name]
        args=[]; i=0
        for arg in entry['args']:
            arg=dict(arg)
            if arg['kind']=='ptr': arg['buffer_bytes']=sizes[i]; i+=1
            elif arg['name'] in ('cap','capacity'): arg['value']=capacity
            elif arg['name'] in ('vocab','vocabulary'): arg['value']=vocab
            args.append(arg)
        assert i==len(sizes),name
        names=[name]+[n for n,(base,_,_) in controls.items() if base==name]
        objects=[]
        for variant in names:
            origin=evidence/(variant+'.hsaco')
            if variant==name: assert sha(origin)==entry['sha256']
            shutil.copyfile(origin,dest/origin.name)
            objects.append(dict(file=origin.name,entry=name,sha256=sha(origin),bytes=origin.stat().st_size))
        access=[a for a in report['access'] if a['kernel'] in names]
        manifest['kernels'].append(dict(kernel=name,kernarg_bytes=entry['kernarg_bytes'],args=args,
            grid=grid,block=entry['block'],objects=objects,measured_access_runs=access,
            numerical_comparison='bit-exact original-kernel parity; no new floating arithmetic',hardware='NOT_RUN'))
    probe=json.loads((evidence/'readfirst-probe.json').read_text())
    probe_abi=next(k for k in manifest['kernels'] if k['kernel']=='readfirst_probe')
    probe['args']=[a['name'] for a in probe_abi['args']]
    probe['kernarg_bytes']=probe_abi['kernarg_bytes']
    (dest/'readfirst-probe.json').write_text(json.dumps(probe,indent=2)+'\n')
    (dest/'list.json').write_text(json.dumps(manifest,indent=2)+'\n')
    receipt=dict(report,source_head=head,list_sha256=sha(dest/'list.json'),
                 original_receipt=dict(path=str(evidence/'report.json'),sha256=sha(evidence/'report.json')))
    receipt['objects']=[obj for kernel in manifest['kernels'] for obj in kernel['objects']]
    (dest/'report.json').write_text(json.dumps(receipt,indent=2)+'\n')
    lines=['# Device-stepped kernel objects','',f'Source head: `{head}`',
      f'Capacity: {capacity}; vocabulary: {vocab}. Hardware execution: NOT_RUN.',
      'All grids are workgroups; block=(64,1,1). The measured-access runs below are CPU fixtures, not full-model hardware qualification.',
      'Position/token values are read through VMEM. Keep arguments, buffers and the full RoPE table alive across capture/replay.',
      'Start at position0 with fixed A/B state pointers; increment once after argmax. Stop replay before exhausting capacity.',
      'No host pointer swap, scalar argument change or row copy is required during replay.',
      '', '| Object | SHA256 | Kernarg bytes | Grid |', '|---|---|---:|---|']
    for kernel in manifest['kernels']:
        for obj in kernel['objects']:
            lines.append(f'| {obj["file"]} | `{obj["sha256"]}` | {kernel["kernarg_bytes"]} | {kernel["grid"]} |')
    for kernel in manifest['kernels']:
        lines+=['','## '+kernel['kernel'],'| Arg | Offset | Kind | Buffer bytes |','|---|---:|---|---:|']
        for arg in kernel['args']:
            lines.append('| '+' | '.join(str(arg.get(key,'—')) for key in ('name','offset','kind','buffer_bytes'))+' |')
        lines+=['','Measured CPU access (each address range is independently checked on every access):',
                '```json',json.dumps(kernel['measured_access_runs'],indent=2),'```']
    (dest/'list.md').write_text('\n'.join(lines)+'\n')
    (dest/'SHA256SUMS').write_text(''.join(sha(p)+'  '+p.name+'\n' for p in sorted(dest.iterdir()) if p.is_file()))
    print('BUNDLE',dest,'head='+head,'receipt_sha256='+sha(dest/'report.json'))

if __name__=='__main__':
    bundle(pathlib.Path(sys.argv[1]).resolve(),pathlib.Path(sys.argv[2]).resolve(),
           pathlib.Path(sys.argv[3]).resolve(),pathlib.Path(sys.argv[4]).resolve())
