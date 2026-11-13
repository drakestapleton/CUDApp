#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""RMSNorm5120 instruction-boundary Fraction/Decimal oracle, test only."""
import base64
from fractions import Fraction
import pathlib
import runpy
import struct
import sys

HERE = pathlib.Path(__file__).parent
ref = runpy.run_path(str(HERE/'token.py'))
finite,round32,fma,add,mul,point,imul,rsqbound,execute,export = (
    ref[n] for n in ('finite','round32','fma','add','mul','point','imul','rsqbound','execute','export'))
CASES = ((0,'varied'),(1,'zero'),(2,'varied'),(3,'large'),(1,'varied'))
CONTROLS = ('rmsnorm5120-no-one','rmsnorm5120-wrong-mean','rmsnorm5120-dropped-chunk')
ONE,EPS,MEAN = 0x3f800000,0x358637bd,0x394ccccd
assert round32(Fraction(1,5120)) == MEAN


def fixture(rows,kind):
    xp,wp,op = 64,64+rows*5120*4+64,64+rows*5120*4+64+5120*4+64
    image = bytearray([0xa5])*(op+rows*5120*4+64)
    struct.pack_into('<3QI',image,0,xp,wp,op,rows)
    weights = [round32(Fraction((i*19)%67-33,32)) for i in range(5120)]
    struct.pack_into('<5120I',image,wp,*weights)
    intervals=[]
    for row in range(rows):
        x = [round32(Fraction((i*17+row*5)%97-48,16)) for i in range(5120)]
        if kind=='zero': x=[0]*5120
        elif kind=='large': x=[mul(v,0x43000000) for v in x]
        else: x[:4]=[0,0x80000000,1,0x80000001]
        struct.pack_into('<5120I',image,xp+row*5120*4,*x)
        lanes=[0]*64
        for lane in range(64):
            for chunk in range(20):
                for j in range(4):
                    a=x[chunk*256+lane*4+j]
                    lanes[lane]=fma(a,a,lanes[lane])
        for shift in (32,16,8,4,2,1): lanes=[add(lanes[i],lanes[i^shift]) for i in range(64)]
        variance=add(mul(lanes[0],MEAN),EPS)
        inv=rsqbound(finite(variance))
        for i,(a,w) in enumerate(zip(x,weights)):
            lo,hi=imul(imul(point(a),inv),point(add(ONE,w)))
            intervals.append([op+(row*5120+i)*4,str(lo),str(hi)])
    return dict(label=f'rows={rows} hidden=5120 kind={kind}',memory=base64.b64encode(image).decode(),
                intervals=intervals,args=[['ptr',xp],['ptr',wp],['ptr',op],['u32',rows]],
                gx=rows+1,gy=2,input_bytes=(rows+1)*5120*4+4,timing_grid=[max(1,rows),1])


def mutate(src,out):
    text=(src/'rmsnorm5120.isa').read_text()
    changes={'rmsnorm5120-no-one':('I 5 242 12 12 1','I 8 268 1 12'),
             'rmsnorm5120-wrong-mean':('L '+str(MEAN),'L '+str(0x3b800000)),
             'rmsnorm5120-dropped-chunk':('I 3 147 0 13','I 3 146 0 13')}
    for name,(old,new) in changes.items():
        assert old in text,name
        (out/(name+'.isa')).write_text(text.replace(old,new,1))


def suite(exe,out):
    cases=[fixture(rows,kind) for rows,kind in CASES]
    for case in cases:
        widest=execute(exe,out/'rmsnorm5120.text',out,case)
        print(f'PASS rmsnorm5120 {case["label"]} all-outputs/manual-interval all-inputs/guards overdispatch=XY widest_interval={widest}',flush=True)
    for name in CONTROLS:
        try: execute(exe,out/(name+'.text'),out,cases[-1])
        except AssertionError as error:
            assert str(error).startswith('token mismatch'),error
            print(f'CONTROL {name} CAUGHT {str(error).split(":")[0]}',flush=True)
        else: raise AssertionError(f'control escaped {name}')
    export('rmsnorm5120',cases,CONTROLS,out,fixture,CASES)
    print(f'PASS RMSNorm5120 rows={",".join(map(str,sorted(set(r for r,_ in CASES))))} cases={len(CASES)} hardware=NOT_RUN',flush=True)


if __name__=='__main__':
    if sys.argv[1]=='mutate': mutate(pathlib.Path(sys.argv[2]),pathlib.Path(sys.argv[3]))
    elif sys.argv[1]=='suite': suite(sys.argv[2],pathlib.Path(sys.argv[3]))
    else: raise SystemExit('unknown norm mode')
