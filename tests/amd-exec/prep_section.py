#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compact notebook cases using the canonical attention prep/gate interval oracle."""
import base64
from fractions import Fraction
import pathlib
import runpy
import struct
import sys
HERE=pathlib.Path(__file__).parent
ref=runpy.run_path(str(HERE/'token.py'))
execute,export,round32,finite,imul,point,sigmoid=(ref[n] for n in ('execute','export','round32','finite','imul','point','sigmoid'))
fixture,normrope,rope_reference,bf16=(ref['ref'][n] for n in ('fixture','normrope','rope_reference','bf16'))

def prep_case(position):
    cs=[round32(x) for x in rope_reference(position)]
    image,p,q,k,v,qw,kw,cap=fixture(position,cs)
    intervals=[]; half=[]; exact=[]
    for h,row in enumerate(q):
        for d,(lo,hi) in enumerate(normrope(row,qw,cs)): intervals.append([p[6]+h*1024+d*4,str(lo),str(hi)])
    for h,row in enumerate(k):
        for d,bounds in enumerate(normrope(row,kw,cs)):
            lo,hi=sorted(finite(bf16(round32(x))<<16) for x in bounds)
            half.append([p[7]+(h*cap+position)*512+d*2,str(lo),str(hi)])
        exact.extend([p[8]+(h*cap+position)*512+d*2,bf16(x),2] for d,x in enumerate(v[h]))
    sizes=[p[i+1]-p[i]-32 for i in range(8)]+[len(image)-p[8]-32]
    return dict(label=f'QK norm RoPE64/256 KV append position={position} cap={cap}',memory=base64.b64encode(image).decode(),
                args=[['ptr',x] for x in p]+[['u32',position],['u32',cap]],gx=25,gy=2,timing_grid=[24,1],
                input_bytes=sum(sizes[:6]),intervals=intervals,half_intervals=half,exact=exact,
                buffers=[dict(offset=x,bytes=n) for x,n in zip(p,sizes)])

def gate_case(kind):
    p=[128,128+24*2048+32,128+24*2048+32+24*1024+32]
    sizes=[24*2048,24*1024,24*1024]; image=bytearray([0xa5])*(p[2]+sizes[2]+32)
    struct.pack_into('<3Q',image,0,*p); intervals=[]
    for h in range(24):
        x=[round32(Fraction((h*17+d*3)%37-18,8)) for d in range(256)]
        g=[round32(Fraction((h*5+d*7)%41-20,4)) for d in range(256)]
        if kind=='extreme': g[:4]=[0,0x80000000,0x42c80000,0xc2c80000]
        struct.pack_into('<256I',image,p[0]+h*2048+1024,*g); struct.pack_into('<256I',image,p[1]+h*1024,*x)
        for d,(a,b) in enumerate(zip(x,g)):
            lo,hi=imul(point(a),sigmoid(b)); intervals.append([p[2]+h*1024+d*4,str(lo),str(hi)])
    return dict(label='attention sigmoid gate '+kind,memory=base64.b64encode(image).decode(),args=[['ptr',x] for x in p],
                gx=97,gy=2,timing_grid=[96,1],input_bytes=sum(sizes[:2]),intervals=intervals,
                buffers=[dict(offset=x,bytes=n) for x,n in zip(p,sizes)])

SPECS=(('attn256_prep',prep_case,(0,63,64,65,1),tuple('prep-'+s for s in ('no-one','wrong-pair','wrong-position','truncate-cache'))),
       ('attn256_gate',gate_case,('extreme','varied'),('prep-extra-swish','prep-wrong-gate-stride')))
def suite(exe,out):
    for name,factory,parameters,controls in SPECS:
        cases=[factory(arg) for arg in parameters]
        for case in cases:
            width=execute(exe,out/(name+'.text'),out,case)
            worst=ref['midpoint_error']((out/'token.memory').read_bytes(),case)
            print(f'PASS {name} {case["label"]} interval_escapes=0 worst_midpoint_error={float(worst):.12g} widest_interval={width} every-output/input/cache-history/guards=PASS',flush=True)
        for control in controls:
            try: execute(exe,out/(control+'.text'),out,cases[-1])
            except AssertionError as e:
                assert str(e).startswith('token mismatch'),e
                print(f'CONTROL {control} CAUGHT',flush=True)
            else: raise AssertionError('control escaped '+control)
        export(name,cases,controls,out,factory,[(p,) for p in parameters])
if __name__=='__main__': suite(sys.argv[1],pathlib.Path(sys.argv[2]))
