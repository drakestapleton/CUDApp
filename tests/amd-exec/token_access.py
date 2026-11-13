#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test-only seeded executor instrumentation: reject EVERY out-of-buffer access.
Build one instrument per concrete ABI image; run production and numerical controls.
The instrumentation records actual maximum read/write byte offsets, not a formula.
"""
import base64
import hashlib
import json
import pathlib
import runpy
import struct
import subprocess
import sys
HERE=pathlib.Path(__file__).parent

def specs():
    g=runpy.run_path(str(HERE/'gdn.py'))
    d=runpy.run_path(str(HERE/'decode.py'))
    p=runpy.run_path(str(HERE/'prep_section.py'))
    return [('gdnqknorm',g['qk_fixture'],g['QK_CASES'][-1],g['QK_CONTROLS']),
            ('gdnstate',g['state_fixture'],g['STATE_CASES'][-1],g['STATE_CONTROLS'])]+[
            (n,f,(params[-1],),controls) for n,f,params,controls in d['SPECS']+p['SPECS']]

def ranges(case):
    if 'buffers' in case: return case['buffers']
    p=[v for kind,v in case['args'] if kind=='ptr']; end=len(base64.b64decode(case['memory']))
    return [dict(offset=x,bytes=y-x-64) for x,y in zip(p,p[1:]+[end])]

def instrument(source,spans):
    n=len(spans)
    header=f'global hits ulong {n*2}\nglobal ob uchar 8\nfn touch p ulong w ulong rw ulong to void\n'
    for i,(start,size) in enumerate(spans):
        header+=f'if and ge p {start} lt p {start+size}\nif gt w sub ulong {start+size} p\nexit 68\nend\nv = sub add p w {start}\ni = add ulong {i*2} rw\nif gt v get hits i\nput hits i v\nend\nret\nend\n'
    header+='exit 68\nend\n'
    source=source.replace('fn mread p ulong to ulong\n','fn mread p ulong to ulong\ntouch p 4 0\n',1)
    source=source.replace('fn mbyte p ulong to ulong\n','fn mbyte p ulong to ulong\ntouch p 1 0\n',1)
    source=source.replace('fn mhalf p ulong to ulong\n','fn mhalf p ulong to ulong\ntouch p 2 0\n',1)
    source=source.replace('fn mwrite p ulong v ulong to void\n','fn mwrite p ulong v ulong to void\ntouch p 4 1\n',1)
    end=f'for i ulong {n*2}\nfor j ulong 8\nput ob j uchar and shr get hits i mul j 8 255\nend\nr = write 1 ob 0 8\nif ne r 8\nret 74\nend\nend\n'
    assert source.endswith('ret 0\nend\n')
    return source.replace('global st ulong 18\n',header+'global st ulong 18\n',1)[:-len('ret 0\nend\n')]+end+'ret 0\nend\n'

def suite(omc,src,out):
    source=''.join((src/'exec'/f).read_text() for f in ('fp.om','fma.om','main.om'))
    reports=[]
    for name,factory,params,controls in specs():
        case=factory(*params); buffers=ranges(case); size=sum(8 if k=='ptr' else 4 for k,v in case['args'])
        spans=[(0,size)]+[(b['offset'],b['bytes']) for b in buffers]
        text=instrument(source,spans); path=out/(name+'-access.om'); path.write_text(text)
        exe=out/(name+'-access'); p=subprocess.run([omc],input=text.encode(),capture_output=True,timeout=120)
        assert p.returncode==0 and not p.stderr,(name,p.stderr)
        exe.write_bytes(p.stdout); exe.chmod(0o700)
        objects=[]
        for variant in (name,*controls):
            memory=out/'access.memory'; memory.write_bytes(base64.b64decode(case['memory']))
            p=subprocess.run([str(exe),str(out/(variant+'.text')),str(memory),str(case['gx']*64),str(case['gy'])],capture_output=True,timeout=600)
            assert p.returncode==0 and not p.stderr and len(p.stdout)==16*len(spans),(name,variant,p.returncode,p.stderr)
            values=struct.unpack('<'+str(len(spans)*2)+'Q',p.stdout)
            offsets=[dict(bytes=n,max_read_byte=(values[2*i]-1 if values[2*i] else None),max_write_byte=(values[2*i+1]-1 if values[2*i+1] else None)) for i,(start,n) in enumerate(spans)]
            objects.append(dict(object=variant,access=offsets))
            print('PASS ACCESS',variant,json.dumps(offsets,separators=(',',':')),flush=True)
        # Remove the last actually read byte; allocated padding is not a valid negative control.
        last=objects[0]['access'][1]['max_read_byte']; assert last is not None
        badspans=list(spans); start,n=badspans[1]; badspans[1]=(start,last)
        bad=instrument(source,badspans); p=subprocess.run([omc],input=bad.encode(),capture_output=True,timeout=120)
        assert p.returncode==0 and not p.stderr
        mutant=out/(name+'-access-short'); mutant.write_bytes(p.stdout); mutant.chmod(0o700)
        memory.write_bytes(base64.b64decode(case['memory']))
        p=subprocess.run([str(mutant),str(out/(name+'.text')),str(memory),str(case['gx']*64),str(case['gy'])],capture_output=True,timeout=600)
        assert p.returncode==68 and not p.stdout and not p.stderr,(name,p.returncode,p.stderr)
        print('CONTROL ACCESS',name,'short-input CAUGHT exit=68',flush=True)
        reports.append(dict(kernel=name,case=case['label'],kernarg_bytes=size,grid=case['timing_grid'],block=[64,1,1],buffers=buffers,objects=objects,source_sha256=hashlib.sha256(text.encode()).hexdigest()))
    (out/'token-access.json').write_text(json.dumps(reports,indent=2)+'\n')
if __name__=='__main__': suite(sys.argv[1],pathlib.Path(sys.argv[2]),pathlib.Path(sys.argv[3]))
