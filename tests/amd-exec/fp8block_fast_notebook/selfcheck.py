#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Run the complete cell wrapper against injected HIP and torch simulations.
No GPU calls, library kernels or performance measurements occur here.
"""
import argparse
import ast
import base64
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import zlib

HERE=Path(__file__).resolve().parent

def sha(data): return hashlib.sha256(data).hexdigest()

def injected_cell():
    cell=(HERE/'cell-v13.py').read_text()
    blobs=[n.args[0].value for n in ast.walk(ast.parse(cell)) if isinstance(n,ast.Call) and isinstance(n.func,ast.Attribute) and n.func.attr=='b85decode']
    assert len(blobs)==1
    driver=zlib.decompress(base64.b85decode(blobs[0])).decode()
    build=json.loads((HERE/'build.json').read_text())
    assert sha(driver.encode())==build['driver_sha256'] and sha(cell.encode())==build['sha256']
    fake=(HERE/'simulated_device.py').read_text()
    assert driver.count('# SELFCHECK_INSERT')==1
    modified=driver.replace('# SELFCHECK_INSERT',fake)
    blob=base64.b85encode(zlib.compress(modified.encode(),9)).decode()
    assert cell.count(repr(blobs[0]))==1
    cell=cell.replace(repr(blobs[0]),repr(blob))
    assert cell.count(repr(build['driver_sha256']))==1
    cell=cell.replace(repr(build['driver_sha256']),repr(sha(modified.encode())))
    return cell,build


def check_oracle(cell):
    import numpy as np
    blob=next(n.args[0].value for n in ast.walk(ast.parse(cell)) if isinstance(n,ast.Call) and isinstance(n.func,ast.Attribute) and n.func.attr=='b85decode')
    driver=zlib.decompress(base64.b85decode(blob)).decode()
    # Definitions only: __name__ is not __main__; no parent/child/device execution.
    ns={'__name__':'selfcheck_definitions'}
    exec(compile(driver,'selfcheck_definitions','exec'),ns)
    ns['np']=np
    table=ns['fp8_table']()
    assert table[0]==0 and table[128]==0 and np.signbit(table[128])
    assert table[1]==2.0**-9 and table[126]==448 and table[254]==-448
    assert np.isnan(table[127]) and np.isnan(table[255])
    case=ns['fixture'](257,259); expected=ns['oracle'](case)
    data=expected[0].astype(np.float32)
    assert ns['mismatches'](data.tobytes(),expected)==0
    data[-2]=1e30
    assert ns['mismatches'](data.tobytes(),expected)==1
    print('ORACLE SELFCHECK OCP signed zero/subnormal/max/NaNs and last-finite-row corruption CAUGHT',flush=True)


def run(cell,build,root,mode,group=''):
    folder=root/mode; folder.mkdir(parents=True,exist_ok=False)
    script=folder/'cell.py'; script.write_text(cell)
    env=dict(os.environ,OPENBLAS_NUM_THREADS='1',OMP_NUM_THREADS='1')
    for key in ('AMDSERVE_GROUP','AMDSERVE_T0','SELFCHECK_FAULT_GROUP','SELFCHECK_ESCAPE_GROUP','SELFCHECK_CORRUPT_GROUP'): env.pop(key,None)
    if mode!='normal': env[{'fault':'SELFCHECK_FAULT_GROUP','escaped':'SELFCHECK_ESCAPE_GROUP','corrupt':'SELFCHECK_CORRUPT_GROUP'}[mode]]=group
    proc=subprocess.run([sys.executable,str(script)],cwd=folder,env=env,capture_output=True,text=True,timeout=3600)
    (folder/'stdout.txt').write_text(proc.stdout); (folder/'stderr.txt').write_text(proc.stderr)
    assert proc.returncode==0,(proc.returncode,proc.stderr)
    output=(folder/'amdserve-probe-v13.txt').read_text()
    records=[json.loads(line) for line in (folder/'amdserve-probe-v13.txt.state').read_text().splitlines()]
    groups=[g['id'] for g in build['groups']]
    seen=[r['group'] for r in records]
    assert len(seen)==len(set(seen))
    assert 'SIMULATED host; no device query' in output
    assert 'CONTROLS ' in output and 'RESULT overall ' in output
    first_correct=output.index('GROUP correct-')
    assert output.index('CONTROLS ')<first_correct,'controls summary must precede correctness and timing'
    if mode=='normal':
        assert set(seen)==set(groups) and all(r['ok'] for r in records)
        assert 'RESULT overall PASS' in output and 'CONTROLS all CAUGHT' in output
        assert 'FAULT ' not in output and 'ERROR ' not in output
    elif mode=='fault':
        line=next(line for line in output.splitlines() if line.startswith('FAULT group '+group))
        assert 'exit -11' in line and re.search(r'sha256 [0-9a-f]{64}\b',line)
        assert set(seen)==set(groups)-{group} and all(r['ok'] for r in records)
        assert 'RESULT overall FAIL_OR_INCOMPLETE; missing='+group in output
        assert 'CONTROLS ESCAPED or NOT_RUN' in output
    else:
        assert set(seen)==set(groups)
        assert [r['group'] for r in records if not r['ok']]==[group]
        assert 'RESULT overall FAIL_OR_INCOMPLETE' in output
        if mode=='escaped': assert group[4:]+' ESCAPED' in output and 'CONTROLS ESCAPED or NOT_RUN' in output
        else: assert 'all-row oracle mismatches=1' in output
    correct={}
    for line in output.splitlines():
        if line.startswith('OMEGA CORRECTNESS '):
            values=dict(re.findall(r'(\w+)=([^ ]+)',line))
            correct[((int(values['M']),int(values['K'])),values['kernel'])]=values['same_data_sha256']
    timed=[]; reference=[]
    for prefix,rows in (('OMEGA TIMING',timed),('REFERENCE torch.matmul BF16 (AMD library, not Omega)',reference)):
        for line in output.splitlines():
            if not line.startswith(prefix): continue
            values=dict(re.findall(r'(\w+)=([^ ]+)',line))
            shape=(int(values['M']),int(values['K']))
            key=(shape,values.get('kernel','torch'))
            if key in correct: assert values['same_data_sha256']==correct[key]
            assert int(values['rotation_bytes'])>256*(1<<20)
            assert int(values['cold_calls'])%int(values['copies'])==0
            assert float(values['warm_ms'])>0 and float(values['cold_rotation_ms'])>0
            assert 'SIMULATED_NOT_MEASURED' in line
            rows.append((key,values['same_data_sha256']))
    expected={(tuple(g['shape']),g['kernel']) for g in build['groups'] if g['kind']=='timing'}
    assert len(timed)==len(expected) and {key for key,d in timed}==expected
    assert len(reference)==len({shape for shape,kernel in expected})
    refs={key[0]:digest for key,digest in reference}
    for key,digest in timed: assert refs[key[0]]==digest,'Omega/reference data differs'
    if mode=='normal': assert expected<=correct.keys()
    print('SIMULATED_DEVICE mode=%s parent_exit=%d groups_recorded=%d/%d omega_timing_rows=%d reference_rows=%d result=%s' % (mode,proc.returncode,len(seen),len(groups),len(timed),len(reference),'PASS' if mode=='normal' else 'NEGATIVE_CAUGHT'),flush=True)
    summary=[line for line in output.splitlines() if line.startswith(('CONTROLS ','RESULT ','FAULT ','ERROR '))]
    for line in summary: print(line,flush=True)
    return dict(mode=mode,injected_group=group,exit=proc.returncode,result='PASS' if mode=='normal' else 'NEGATIVE_CAUGHT',
                output=str(folder/'amdserve-probe-v13.txt'),output_sha256=sha(output.encode()),
                simulated_only=True,hardware='NOT_RUN',performance='NOT_RUN')


def main():
    parser=argparse.ArgumentParser(); parser.add_argument('--out',required=True,type=Path); args=parser.parse_args()
    args.out.mkdir(parents=True,exist_ok=False)
    cell,build=injected_cell()
    check_oracle(cell)
    ctl=next(g['id'] for g in build['groups'] if g['kind']=='control')
    correct=next(g['id'] for g in build['groups'] if g['kind']=='correctness')
    result=[run(cell,build,args.out,'normal'),run(cell,build,args.out,'fault',ctl),
            run(cell,build,args.out,'escaped',ctl),run(cell,build,args.out,'corrupt',correct)]
    report=dict(cell_sha256=build['sha256'],simulated_device_sha256=sha((HERE/'simulated_device.py').read_bytes()),runs=result,
                hardware='NOT_RUN',performance='NOT_RUN')
    (HERE/'selfcheck.json').write_text(json.dumps(report,indent=2)+'\n')
    print('SELFCHECK PASS normal, child SIGSEGV, escaped mutant, last-finite-row corruption; HARDWARE NOT_RUN PERFORMANCE NOT_RUN',flush=True)

if __name__=='__main__': main()
