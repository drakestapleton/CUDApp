#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Token-kernel independent numerical oracle and generated notebook instruments."""
import base64
from fractions import Fraction
from functools import lru_cache
import json
import pathlib
import runpy
import struct
import subprocess
import sys
import zlib

HERE = pathlib.Path(__file__).parent
ref = runpy.run_path(str(HERE/'prep.py'))
finite,round32,fma,add,mul,point,imul,iadd,expbound,intervalrcp,rsqbound = (
    ref[n] for n in ('finite','round32','fma','add','mul','point','imul','iadd','expbound','intervalrcp','rsqbound'))
ONE,LOG2E = 0x3f800000,0x3fb8aa3b
COUNTS = (0,1,63,64,65,17408)
CONTROLS = ('mlpsilu-no-silu','mlpsilu-wrong-up')

@lru_cache(None)
def sigmoid(g):
    exponent = mul((g & 0x7fffffff)|0x80000000,LOG2E)
    t = expbound(exponent)
    answer = intervalrcp(iadd(point(ONE),t))
    return imul(answer,t) if g>>31 else answer


def mlp_case(n):
    gp,up,op = 64,64+4*n+64,64+8*n+128
    image = bytearray([0xa5])*(op+4*n+64)
    struct.pack_into('<3QI',image,0,gp,up,op,n)
    gates = [round32(Fraction((i*17)%193-96,8)) for i in range(n)]
    ups = [round32(Fraction((i*29)%257-128,16)) for i in range(n)]
    special = [0,0x80000000,1,0x80000001,0x42c80000,0xc2c80000,0x4b000000,0xcb000000]
    gates[:min(n,len(special))] = special[:n]
    struct.pack_into('<'+str(n)+'I',image,gp,*gates)
    struct.pack_into('<'+str(n)+'I',image,up,*ups)
    intervals = []
    for i,(g,u) in enumerate(zip(gates,ups)):
        lo,hi = imul(imul(point(g),sigmoid(g)),point(u))
        intervals.append([op+4*i,str(lo),str(hi)])
    return dict(label=f'N={n}',memory=base64.b64encode(image).decode(),intervals=intervals,
                args=[['ptr',gp],['ptr',up],['ptr',op],['u32',n]],gx=(n+63)//64+1,gy=2,input_bytes=8*n+4,
                timing_grid=[max(1,(n+63)//64),1])


def check(actual,case):
    before = base64.b64decode(case['memory'])
    assert len(actual)==len(before),'token mismatch memory length'
    untouched = bytearray(actual)
    widest = Fraction(0)
    for off,lo,hi in case['intervals']:
        bits = struct.unpack_from('<I',actual,off)[0]
        lo,hi = Fraction(lo),Fraction(hi)
        assert bits & 0x7fffffff < 0x7f800000 and lo <= finite(bits) <= hi, f'token mismatch {case["label"]} offset={off} bits={bits:08x} interval={lo},{hi}'
        widest = max(widest,hi-lo)
        untouched[off:off+4] = before[off:off+4]
    for off,bits,width in case.get('exact',[]):
        assert int.from_bytes(actual[off:off+width],'little') == bits, f'token mismatch exact offset={off}'
        untouched[off:off+width] = before[off:off+width]
    assert untouched==before,'token mismatch input/guard corruption'
    return widest


def execute(exe,code,out,case):
    path = out/'token.memory'
    path.write_bytes(base64.b64decode(case['memory']))
    p = subprocess.run([exe,str(code),str(path),str(case['gx']*64),str(case['gy'])],capture_output=True,timeout=600)
    assert p.returncode==0 and not p.stdout and not p.stderr,p
    return check(path.read_bytes(),case)


def export(name,cases,controls,out):
    payload = dict(name=name,cases=cases,controls=list(controls),timing=len(cases)-1)
    serialized = json.dumps(payload,separators=(',',':')).encode()
    (out/(name+'-intervals.json')).write_bytes(serialized+b'\n')
    encoded = base64.b64encode(zlib.compress(serialized,9)).decode()
    source = (HERE/'notebook_section.py').read_text()
    generated = '# Generated from tests/amd-exec/token.py and notebook_section.py; do not edit.\nPAYLOAD = '+repr(encoded)+'\n'+source
    compile(generated,name+'.py','exec')
    (out/(name+'.py')).write_text(generated)


def mutate(src,out):
    text = (src/'mlpsilu.isa').read_text()
    changes = {'mlpsilu-no-silu':('I 5 267 8 8 5','I 8 267 1 8'),
               'mlpsilu-wrong-up':('I 7 0 0 2 0 0 20 0 4 0 127 0 9','I 7 0 0 2 0 0 20 0 2 0 127 0 9')}
    for name,(old,new) in changes.items():
        assert text.count(old)==1,name
        (out/(name+'.isa')).write_text(text.replace(old,new))


def suite(exe,out):
    cases = [mlp_case(n) for n in COUNTS]
    for case in cases:
        widest = execute(exe,out/'mlpsilu.text',out,case)
        print(f'PASS mlpsilu {case["label"]} all-outputs/manual-interval all-inputs/guards overdispatch=XY widest_interval={widest}',flush=True)
    for name in CONTROLS:
        try: execute(exe,out/(name+'.text'),out,cases[-1])
        except AssertionError as error:
            assert str(error).startswith('token mismatch'),error
            print(f'CONTROL {name} CAUGHT {str(error).split(":")[0]}',flush=True)
        else: raise AssertionError(f'control escaped {name}')
    export('mlpsilu',cases,CONTROLS,out)
    print(f'PASS token mlpsilu_counts={",".join(map(str,COUNTS))} mlpsilu_cases={len(cases)} hardware=NOT_RUN',flush=True)


if __name__ == '__main__':
    if sys.argv[1]=='mutate': mutate(pathlib.Path(sys.argv[2]),pathlib.Path(sys.argv[3]))
    elif sys.argv[1]=='suite': suite(sys.argv[2],pathlib.Path(sys.argv[3]))
    else: raise SystemExit('unknown token mode')
