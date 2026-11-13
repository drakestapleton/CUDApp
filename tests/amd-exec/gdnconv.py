#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""BF16 conv4/cache + SiLU oracle, following the reference's dtype boundaries."""
import base64
from fractions import Fraction
import pathlib
import runpy
import struct
import sys

HERE=pathlib.Path(__file__).parent
ref=runpy.run_path(str(HERE/'token.py'))
finite,round32,fma,point,imul,sigmoid,execute,export=(ref[n] for n in ('finite','round32','fma','point','imul','sigmoid','execute','export'))
bf16=ref['ref']['bf16']
CHANNELS=16*128*2+48*128
CASES=((64,'zero'),(128,'range'),(CHANNELS,'history'))
CONTROLS=('gdnconv4-no-input-round','gdnconv4-no-conv-round','gdnconv4-no-silu-round','gdnconv4-wrong-cache-shift')


def fixture(active,kind):
    pointers=[]; end=64
    for size in (CHANNELS*4,CHANNELS*8,CHANNELS*8,CHANNELS*8,CHANNELS*4):
        pointers.append(end); end+=size+64
    xp,wp,sp,np,op=pointers
    image=bytearray([0xa5])*end
    struct.pack_into('<5Q',image,0,*pointers)
    inputs=[round32(Fraction((i*19)%127-63,32)) for i in range(CHANNELS)]
    inputs[:8]=[0x3f808000,0x3f818000,0xbf808000,0xbf818000,1,0x80000001,0x42c80000,0xc2c80000]
    if kind=='range': inputs[8:12]=[0x7f800000,0xff800000,0x7f801234,0xffc01234]
    struct.pack_into('<'+str(CHANNELS)+'I',image,xp,*inputs)
    intervals=[]; exact=[]
    for c in range(CHANNELS):
        weights=[bf16(round32(Fraction((c*7+j*11)%31-15,16))) for j in range(4)]
        old=[0 if kind=='zero' else bf16(round32(Fraction((c*17+j*5)%61-30,16))) for j in range(4)]
        struct.pack_into('<4H',image,wp+c*8,*weights)
        struct.pack_into('<4H',image,sp+c*8,*old)
        if c>=active: continue
        new=old[1:]+[bf16(inputs[c])]
        exact.extend([np+c*8+j*2,x,2] for j,x in enumerate(new))
        total=0
        for a,w in zip(new,weights): total=fma(a<<16,w<<16,total)
        conv=bf16(total)<<16
        if conv & 0x7fffffff >= 0x7f800000:
            answer=0x7f800000 if conv==0x7f800000 else 0x7fc00000
            exact.append([op+c*4,answer,4])
        else:
            endpoints=imul(point(conv),sigmoid(conv))
            ends=[finite(bf16(round32(x))<<16) for x in endpoints]
            intervals.append([op+c*4,str(min(ends)),str(max(ends))])
    return dict(label=f'channels={active}/{CHANNELS} taps=4 kind={kind}',memory=base64.b64encode(image).decode(),
                intervals=intervals,exact=exact,bf16_outputs=[v[0] for v in intervals],args=[['ptr',p] for p in pointers],
                gx=CHANNELS//64+1 if active==CHANNELS else active//64,gy=2,
                timing_grid=[CHANNELS//64,1],input_bytes=CHANNELS*(4+8+8))


def mutate(src,out):
    text=(src/'gdnconv4.isa').read_text()
    for name,tag in zip(CONTROLS[:3],('INPUT','CONV','SILU')):
        start=text.index('# BF16_ROUND_'+tag+'\n')
        stop=text.index('# END_BF16_ROUND_'+tag+'\n',start)+len('# END_BF16_ROUND_'+tag+'\n')
        (out/(name+'.isa')).write_text(text[:start]+text[stop:])
    old='I 5 144 16 18 16'
    assert text.count(old)==1
    (out/(CONTROLS[3]+'.isa')).write_text(text.replace(old,'I 5 128 16 18 16'))


def suite(exe,out):
    cases=[fixture(n,kind) for n,kind in CASES]
    for case in cases:
        widest=execute(exe,out/'gdnconv4.text',out,case)
        print(f'PASS gdnconv4 {case["label"]} BF16-input/conv/SiLU cache-exact every-output all-inputs/guards widest_interval={widest}',flush=True)
    for name in CONTROLS:
        try: execute(exe,out/(name+'.text'),out,cases[-1])
        except AssertionError as error:
            assert str(error).startswith('token mismatch'),error
            print(f'CONTROL {name} CAUGHT {str(error).split(":")[0]}',flush=True)
        else: raise AssertionError(f'control escaped {name}')
    export('gdnconv4',cases,CONTROLS,out,fixture,CASES)
    print(f'PASS GDN convolution channels={CHANNELS} cases={len(CASES)} hardware=NOT_RUN',flush=True)


if __name__=='__main__':
    if sys.argv[1]=='mutate': mutate(pathlib.Path(sys.argv[2]),pathlib.Path(sys.argv[3]))
    elif sys.argv[1]=='suite': suite(sys.argv[2],pathlib.Path(sys.argv[3]))
    else: raise SystemExit('unknown convolution mode')
