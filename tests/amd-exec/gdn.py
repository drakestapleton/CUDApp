#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""GDN query/key normalization and FP32 recurrent-state reference instruments."""
import base64
from fractions import Fraction
from decimal import Decimal,localcontext
from functools import lru_cache
import pathlib
import runpy
import struct
import sys
HERE=pathlib.Path(__file__).parent
ref=runpy.run_path(str(HERE/'token.py'))
finite,round32,mul,add,point,imul,rsqbound,execute,export=(ref[n] for n in ('finite','round32','mul','add','point','imul','rsqbound','execute','export'))
bf16=ref['ref']['bf16']
QSCALE=0x3db504f3
EPS=bf16(0x358637bd)<<16
with localcontext() as context:
    context.prec=120
    assert round32(Fraction(Decimal(1)/Decimal(128).sqrt()))==QSCALE
QK_CASES=((1,'zero'),(2,'basis'),(32,'varied'))
STATE_CASES=((1,'zero'),(1,'no-beta'),(1,'no-decay'),(16,'dense'))
QK_CONTROLS=('gdnqknorm-no-qscale','gdnqknorm-wrong-epsilon','gdnqknorm-drop-upper','gdnqknorm-no-norm-round')
STATE_CONTROLS=('gdnstate-no-decay','gdnstate-no-beta','gdnstate-wrong-key-head','gdnstate-wrong-row-stride','gdnstate-no-core-round')

def ibf16(value):
    return tuple(finite(bf16(round32(x))<<16) for x in value)


def qk_fixture(active,kind):
    xp,op=64,64+4096*4+64
    image=bytearray([0xa5])*(op+4096*4+64)
    struct.pack_into('<2Q',image,0,xp,op)
    intervals=[]
    for head in range(32):
        x=[round32(Fraction((i*7+head*3)%37-18,16)) for i in range(128)]
        if kind=='zero': x=[0]*128
        elif kind=='basis': x=[0x3f800000 if i==head else 0 for i in range(128)]
        else: x[:4]=[0,0x80000000,0x00010000,0x80010000]
        struct.pack_into('<128I',image,xp+head*512,*x)
        if head>=active: continue
        lanes=[add(bf16(mul(x[i],x[i]))<<16,bf16(mul(x[i+64],x[i+64]))<<16) for i in range(64)]
        for shift in (32,16,8,4,2,1): lanes=[add(lanes[i],lanes[i^shift]) for i in range(64)]
        variance=bf16(add(bf16(lanes[0])<<16,EPS))<<16
        inv=ibf16(rsqbound(finite(variance)))
        for i,bits in enumerate(x):
            value=ibf16(imul(point(bits),inv))
            if head<16: value=imul(value,point(QSCALE))
            intervals.append([op+head*512+i*4,str(value[0]),str(value[1])])
    return dict(label=f'qk_heads={active}/32 D=128 kind={kind}',memory=base64.b64encode(image).decode(),
                intervals=intervals,args=[['ptr',xp],['ptr',op]],gx=33 if active==32 else active,gy=2,
                timing_grid=[32,1],input_bytes=4096*4)


def qk_value(key,row,query):
    negative=(row*3+key)%4<2
    return (0x3c000000 if query else QSCALE) | (0x80000000 if negative else 0)


def state_scalars(head,kind):
    decay=0 if kind=='no-decay' else (0x3f400000,0x3f000000)[head%2]
    beta=0 if kind=='no-beta' else (0x3e800000,0x3f000000,0x3f400000)[head%3]
    return decay,beta


@lru_cache(None)
def state_head(head,kind):
    # Test values repeat after12 heads; every physical head/state word is still checked.
    key=head//3
    old=[0 if kind=='zero' else round32(Fraction((r*7+c*13+(head%4)*17)%31-15,256)) for r in range(128) for c in range(128)]
    values=[round32(Fraction((c*11+head*3)%29-14,16)) for c in range(128)]
    decay,beta=state_scalars(head,kind)
    new=[mul(a,decay) for a in old]
    kv=[0]*128
    for r in range(128):
        k=qk_value(key,r,False)
        for c in range(128): kv[c]=add(kv[c],mul(new[r*128+c],k))
    delta=[mul(add(v,a^0x80000000),beta) for v,a in zip(values,kv)]
    output=[0]*128
    for r in range(128):
        k,q=qk_value(key,r,False),qk_value(key,r,True)
        for c in range(128):
            i=r*128+c
            new[i]=add(new[i],mul(k,delta[c]))
            output[c]=add(output[c],mul(new[i],q))
    return old,values,new,[bf16(b)<<16 for b in output]


def state_fixture(keys,kind):
    sizes=(4096*4,48*128*4,48*128*128*4,48*128*128*4,48*128*4,48*4,48*4)
    pointers=[]; end=64
    for size in sizes: pointers.append(end); end+=size+64
    qp,vp,sp,np,op,dp,bp=pointers
    image=bytearray([0xa5])*end
    struct.pack_into('<7Q',image,0,*pointers)
    for key in range(16):
        for part in range(2):
            bits=[qk_value((key%4),r,part==0) for r in range(128)]
            struct.pack_into('<128I',image,qp+(part*16+key)*512,*bits)
    exact=[]
    for head in range(48):
        old,values,new,output=state_head(head%12,kind)
        struct.pack_into('<16384I',image,sp+head*65536,*old)
        struct.pack_into('<128I',image,vp+head*512,*values)
        decay,beta=state_scalars(head,kind)
        struct.pack_into('<I',image,dp+head*4,decay)
        struct.pack_into('<I',image,bp+head*4,beta)
        if head>=keys*3: continue
        exact.extend([np+head*65536+i*4,b,4] for i,b in enumerate(new))
        exact.extend([op+head*512+i*4,b,4] for i,b in enumerate(output))
    return dict(label=f'value_heads={keys*3}/48 state=128x128 kind={kind}',memory=base64.b64encode(image).decode(),
                intervals=[],exact=exact,args=[['ptr',p] for p in pointers],gx=4,gy=17 if keys==16 else keys,
                timing_grid=[3,16],input_bytes=sizes[0]+sizes[1]+sizes[2]+sizes[5]+sizes[6])


def mutate(src,out):
    qk=(src/'gdnqknorm.isa').read_text()
    changes=((QK_CONTROLS[0],'L '+str(QSCALE),'L '+str(0x3f800000)),
             (QK_CONTROLS[1],'L '+str(EPS),'L '+str(0x3f800000)),
             (QK_CONTROLS[2],'I 5 265 9 11 5','I 5 264 8 11 5'))
    for name,old,new in changes:
        assert old in qk,name
        (out/(name+'.isa')).write_text(qk.replace(old,new))
    state=(src/'gdnstate.isa').read_text()
    changes=((STATE_CONTROLS[0],'I 5 276 16 16 5','I 8 272 1 16'),
             (STATE_CONTROLS[1],'I 5 277 18 18 5','I 8 274 1 18'),
             (STATE_CONTROLS[2],'I 2 3 137 29 28','I 2 2 137 29 28'),
             (STATE_CONTROLS[3],'L 512','L 256'))
    for name,old,new in changes:
        assert old in state,name
        (out/(name+'.isa')).write_text(state.replace(old,new,1))
    for name,source,tags in ((QK_CONTROLS[-1],qk,('NORM0','NORM1')),(STATE_CONTROLS[-1],state,('CORE0','CORE1'))):
        lines=source.splitlines(); result=[]; skip=False
        for line in lines:
            if any(line=='# BF16_ROUND_'+tag for tag in tags): skip=True
            if not skip: result.append(line)
            if any(line=='# END_BF16_ROUND_'+tag for tag in tags): skip=False
        (out/(name+'.isa')).write_text('\n'.join(result)+'\n')


def suite(exe,out):
    for name,parameters,factory,controls in (('gdnqknorm',QK_CASES,qk_fixture,QK_CONTROLS),('gdnstate',STATE_CASES,state_fixture,STATE_CONTROLS)):
        cases=[factory(*args) for args in parameters]
        for case in cases:
            widest=execute(exe,out/(name+'.text'),out,case)
            worst=ref['midpoint_error']((out/'token.memory').read_bytes(),case)
            print(f'PASS {name} {case["label"]} interval_escapes=0 worst_midpoint_error={float(worst):.12g} every-output/state all-inputs/guards widest_interval={widest}',flush=True)
        for control in controls:
            try: execute(exe,out/(control+'.text'),out,cases[-1])
            except AssertionError as error:
                assert str(error).startswith('token mismatch'),error
                print(f'CONTROL {control} CAUGHT {str(error).split(":")[0]}',flush=True)
            else: raise AssertionError('control escaped '+control)
        export(name,cases,controls,out,factory,parameters)


if __name__=='__main__':
    if sys.argv[1]=='mutate': mutate(pathlib.Path(sys.argv[2]),pathlib.Path(sys.argv[3]))
    elif sys.argv[1]=='suite': suite(sys.argv[2],pathlib.Path(sys.argv[3]))
    elif sys.argv[1]=='controls': print(*QK_CONTROLS,*STATE_CONTROLS)
    else: raise SystemExit('unknown GDN mode')
