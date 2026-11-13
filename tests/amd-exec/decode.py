#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Token decode independent reference instruments; no production Python dependency."""
import base64
from decimal import Decimal,localcontext
from fractions import Fraction
from functools import lru_cache
import pathlib
import runpy
import struct
import sys
HERE=pathlib.Path(__file__).parent
ref=runpy.run_path(str(HERE/'token.py'))
finite,round32,mul,add,point,imul,iadd,expbound,intervalrcp,rsqbound,execute,export=(ref[n] for n in ('finite','round32','mul','add','point','imul','iadd','expbound','intervalrcp','rsqbound','execute','export'))
bf16=ref['ref']['bf16']
sigmoid=ref['sigmoid']
logbound=runpy.run_path(str(HERE/'log.py'))['logbound']

def alloc(sizes):
    pointers=[]; end=128
    for size in sizes: pointers.append(end); end+=size+64
    image=bytearray([0xa5])*end
    struct.pack_into('<'+str(len(pointers))+'Q',image,0,*pointers)
    return image,pointers

def result(label,image,pointers,sizes,grid,exact=(),intervals=(),bfout=()):
    return dict(label=label,memory=base64.b64encode(image).decode(),args=[['ptr',p] for p in pointers],
                gx=grid[0]+1,gy=grid[1]+1,timing_grid=list(grid),input_bytes=sum(sizes[:-1]),
                exact=list(exact),intervals=list(intervals),bf16_outputs=list(bfout),
                buffers=[dict(offset=p,bytes=n) for p,n in zip(pointers,sizes)])

def ibf(a): return tuple(finite(bf16(round32(x))<<16) for x in a)

def residual_case(kind):
    sizes=(5120*4,5120*4,5120*4); image,p=alloc(sizes); exact=[]
    for i in range(5120):
        a=round32(Fraction((i*17)%131-65,16)); b=round32(Fraction((i*7)%127-63,257))
        if kind=='cancel': b=a^0x80000000
        struct.pack_into('<I',image,p[0]+4*i,a); struct.pack_into('<I',image,p[1]+4*i,b)
        c=bf16(add(bf16(a)<<16,bf16(b)<<16))<<16
        exact.append([p[2]+4*i,c,4])
    return result('BF16 residual '+kind,image,p,sizes,(80,1),exact)

def embedding_case(kind):
    sizes=(5120*2,5120*4); image,p=alloc(sizes); exact=[]
    for i in range(5120):
        b=bf16(round32(Fraction((i*7)%127-63,16)))
        if kind=='bits': b=(0,0x8000,1,0x8001,0x7f80,0xff80,0x7fc1,0x7f81)[i%8]
        struct.pack_into('<H',image,p[0]+2*i,b); exact.append([p[1]+4*i,b<<16,4])
    return result('selected BF16 embedding row '+kind,image,p,sizes,(80,1),exact)

@lru_cache(None)
def intervalexp(a):
    return expbound(round32(a[0]))[0],expbound(round32(a[1]))[1]

@lru_cache(None)
def softplus(x):
    if finite(x)>20: return point(x)
    t=expbound(mul((x&0x7fffffff)|0x80000000,0x3fb8aa3b))
    u=iadd(point(0x3f800000),t)
    bounds=[]
    # The rounded u can have several adjacent values; union their possible paths.
    for bits in range(round32(u[0]),round32(u[1])+1):
        if bits==0x3f800000: y=t
        else:
            d=point(add(bits,0xbf800000))
            y=imul(imul(logbound(bits),point(0x3f317218)),imul(t,intervalrcp(d)))
        bounds.extend(y)
    return iadd((min(bounds),max(bounds)),point(x if finite(x)>0 else 0))

@lru_cache(None)
def param_values(kind):
    values=[]
    for i in range(48):
        a=round32(Fraction((i*11)%37-18,4)); b=round32(Fraction((i*7)%29-14,4))
        A=round32(Fraction(i%7-3,4)); dt=round32(Fraction(i%5-2,8))
        if kind=='tails': a=round32(Fraction((-80,-20,-17,-2,0,2,20,21,80)[i%9])); A=round32(Fraction(i%4)); b=round32(Fraction((-80,0,80)[i%3]))
        values.append((a,b,A,dt))
    return values

def params_case(kind):
    sizes=(48*4,)*6; image,p=alloc(sizes); intervals=[]; bfout=[]
    for i,(a,b,A,dt) in enumerate(param_values(kind)):
        for ptr,v in zip(p,(a,b,A,dt)): struct.pack_into('<I',image,ptr+4*i,v)
        beta=ibf(sigmoid(bf16(b)<<16)); sp=softplus(add(bf16(a)<<16,dt)); ea=expbound(mul(A,0x3fb8aa3b))
        g=imul(ea,sp); g=(-g[1],-g[0]); decay=intervalexp(imul(g,point(0x3fb8aa3b)))
        for ptr,bounds in ((p[4],decay),(p[5],beta)): intervals.append([ptr+4*i,*map(str,bounds)])
        bfout.append(p[5]+4*i)
    return result('GDN decay/beta '+kind,image,p,sizes,(1,1),intervals=intervals,bfout=bfout)

def rmsgate_case(kind):
    sizes=(6144*4,128*4,6144*4,6144*4); image,p=alloc(sizes); intervals=[]; bfout=[]
    weights=[round32(Fraction((i*3)%13+1,8)) for i in range(128)]
    struct.pack_into('<128I',image,p[1],*weights)
    for h in range(48):
        x=[round32(Fraction((i*7+h*3)%31-15,16)) for i in range(128)]
        z=[round32(Fraction((i*11+h*5)%37-18,7)) for i in range(128)]
        if kind=='zero': x=[0]*128
        struct.pack_into('<128I',image,p[0]+h*512,*x); struct.pack_into('<128I',image,p[2]+h*512,*z)
        lanes=[add(mul(x[i],x[i]),mul(x[i+64],x[i+64])) for i in range(64)]
        for shift in (32,16,8,4,2,1): lanes=[add(lanes[i],lanes[i^shift]) for i in range(64)]
        inv=rsqbound(finite(add(mul(lanes[0],0x3c000000),0x358637bd)))
        for i,(a,w,gate) in enumerate(zip(x,weights,z)):
            gate=bf16(gate)<<16
            norm=ibf(imul(point(a),inv)); weighted=ibf(imul(norm,point(w)))
            output=ibf(imul(weighted,imul(point(gate),sigmoid(gate))))
            off=p[3]+h*512+4*i; intervals.append([off,*map(str,output)]); bfout.append(off)
    return result('GDN norm-before-swish '+kind,image,p,sizes,(48,1),intervals=intervals,bfout=bfout)

def logit_bits(i,kind):
    b=round32(Fraction((i*17)%251-125,16))
    if kind=='negative': b=round32(Fraction(-1-(i%127),8))
    if kind=='ties' and i in (31,33,64,248319): b=0x42c80000
    if kind=='zeros': b=0x80000000 if i%2==0 else 0
    if kind=='nan' and i in (123,248319): b=0x7fc00001
    if kind=='last' and i==248319: b=0x42c80000
    return b

def rank(bits):
    # Independent numeric order, not the kernel's sign-bit transform.
    if bits&0x7fffffff>0x7f800000: return (1,Fraction(0))
    return (0,finite(bits))

def orderkey(bits):
    if bits&0x7fffffff>0x7f800000: return 0xffffffff
    if bits&0x7fffffff==0: return 0x80000000
    return bits^ (0xffffffff if bits>>31 else 0x80000000)

@lru_cache(None)
def partials(kind):
    pairs=[]
    for start in range(0,248320,64):
        index=max(range(start,start+64),key=lambda i:rank(logit_bits(i,kind)))
        pairs.append((orderkey(logit_bits(index,kind)),index))
    return pairs

def argpart_case(kind):
    sizes=(248320*4,3880*8); image,p=alloc(sizes)
    struct.pack_into('<248320I',image,p[0],*(logit_bits(i,kind) for i in range(248320)))
    exact=[]
    for i,(key,index) in enumerate(partials(kind)): exact.extend(([p[1]+8*i,key,4],[p[1]+8*i+4,index,4]))
    return result('argmax partial '+kind,image,p,sizes,(3880,1),exact)

def argfinal_case(kind):
    sizes=(3880*8,4); image,p=alloc(sizes)
    for i,pair in enumerate(partials(kind)): struct.pack_into('<II',image,p[0]+8*i,*pair)
    index=max(range(248320),key=lambda i:rank(logit_bits(i,kind)))
    return result('argmax final '+kind,image,p,sizes,(1,1),[[p[1],index,4]])

SPECS=(('residual5120',residual_case,('cancel','varied'),('residual5120-no-round',)),
       ('embed5120',embedding_case,('bits','varied'),('embed5120-low-bits',)),
       ('gdnparams',params_case,('tails','varied'),('gdnparams-no-beta-round','gdnparams-wrong-decay')),
       ('gdnrmsgate',rmsgate_case,('zero','varied'),('gdnrmsgate-no-norm-round','gdnrmsgate-no-swish','gdnrmsgate-no-z-round')),
       ('argmax_part',argpart_case,('negative','zeros','nan','last','ties'),('argmax_part-wrong-tie',)),
       ('argmax_final',argfinal_case,('negative','zeros','nan','last','ties'),('argmax_final-wrong-tie',)))

def strip_round(text,tags):
    lines=[]; skip=False
    for line in text.splitlines():
        if line in ['# BF16_ROUND_'+tag for tag in tags]: skip=True
        if not skip: lines.append(line)
        if line in ['# END_BF16_ROUND_'+tag for tag in tags]: skip=False
    return '\n'.join(lines)+'\n'

def mutate(src,out):
    for name,factory,parameters,controls in SPECS:
        text=(src/(name+'.isa')).read_text()
        for control in controls:
            if control.endswith('no-round'): bad=strip_round(text,['OUTPUT'])
            elif control.endswith('no-beta-round'): bad=strip_round(text,['BETA'])
            elif control.endswith('no-norm-round'): bad=strip_round(text,['NORM_LOW','NORM_HIGH'])
            elif control.endswith('no-z-round'): bad=strip_round(text,['Z_LOW','Z_HIGH'])
            else:
                old,new={
                  'embed5120-low-bits':('I 8 128 1 8','I 8 129 1 8'),
                  'gdnparams-wrong-decay':('I 8 275 32 19','I 8 242 1 19'),
                  'gdnrmsgate-no-swish':('I 5 270 18 18 5','I 8 274 1 18'),
                  'argmax_part-wrong-tie':('I 6 265 11 204','I 6 267 9 204'),
                  'argmax_final-wrong-tie':('I 6 265 11 204','I 6 267 9 204'),
                }[control]
                assert old in text,control
                bad=text.replace(old,new)
            assert bad!=text,control
            (out/(control+'.isa')).write_text(bad)

def suite(exe,out):
    for name,factory,parameters,controls in SPECS:
        cases=[factory(arg) for arg in parameters]
        for case in cases:
            width=execute(exe,out/(name+'.text'),out,case)
            actual=(out/'token.memory').read_bytes(); worst=Fraction(0)
            for off,lo,hi in case['intervals']:
                y=finite(struct.unpack_from('<I',actual,off)[0]); worst=max(worst,abs(y-(Fraction(lo)+Fraction(hi))/2))
            print(f'PASS {name} {case["label"]} interval_escapes=0 worst_midpoint_error={float(worst):.12g} widest_interval={width} exact/input/guards=PASS',flush=True)
        for control in controls:
            try: execute(exe,out/(control+'.text'),out,cases[-1])
            except AssertionError as e:
                assert str(e).startswith('token mismatch'),e
                print(f'CONTROL {control} CAUGHT {str(e).split(":")[0]}',flush=True)
            else: raise AssertionError('control escaped '+control)
        export(name,cases,controls,out,factory,[(p,) for p in parameters])

if __name__=='__main__':
    if sys.argv[1]=='mutate': mutate(pathlib.Path(sys.argv[2]),pathlib.Path(sys.argv[3]))
    elif sys.argv[1]=='controls': print(*(c for spec in SPECS for c in spec[3]))
    elif sys.argv[1]=='suite': suite(sys.argv[2],pathlib.Path(sys.argv[3]))
    else: raise SystemExit('unknown decode mode')
