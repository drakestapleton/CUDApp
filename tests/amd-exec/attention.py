#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Independent Fraction RN-even + Decimal transcendental enclosure; test instrument only.
CDNA3 PDF pp195-196 permit 1ULP for EXP/RCP, not device-identical transcendental bits.
Each arithmetic interval is rounded at the SAME instruction boundary as the kernel.
"""
from decimal import Decimal, localcontext
from fractions import Fraction
from functools import lru_cache
import json
import pathlib
import runpy
import struct
import subprocess
import sys

ref = runpy.run_path(str(pathlib.Path(__file__).with_name('sgemm.py')))
finite, round32 = ref['finite'], ref['round32']
fma = lru_cache(None)(ref['fma'])
SHAPES = (0, 1, 2, 63, 64, 65, 129)
ONE, SCALE, LOG2E = 0x3f800000, 0x3d800000, 0x3fb8aa3b

def add(a, b): return fma(a, ONE, b)
def mul(a, b): return fma(a, b, (a ^ b) & 0x80000000)
def sub(a, b): return add(a, b ^ 0x80000000)
def point(bits):
    x = finite(bits)
    return x, x

def rounded(x): return finite(round32(x))
@lru_cache(None)
def imul(a, b):
    vals = [x*y for x in a for y in b]
    return rounded(min(vals)), rounded(max(vals))
@lru_cache(None)
def iadd(a, b): return rounded(a[0]+b[0]), rounded(a[1]+b[1])
@lru_cache(None)
def ifma(a, b, c):
    vals = [x*y for x in a for y in b]
    return rounded(min(vals)+c[0]), rounded(max(vals)+c[1])

def nextup(b):
    if b == 0x80000000: return 1
    return b-1 if b >> 31 else b+1

def nextdown(b):
    if b == 0: return 0x80000001
    return b+1 if b >> 31 else b-1

def ftz(b): return b & 0x80000000 if b & 0x7fffffff < 0x800000 else b

def enclosure(value):
    # Decimal uncertainty is far below binary32. Enclose its final rounding and
    # the ln/pow working precision, then enclose the ISA's absolute 1ULP allowance.
    center = round32(value)
    exponent = max(1, (center >> 23) & 255) - 150
    ulp = Fraction(2) ** exponent
    uncertainty = max(abs(value), Fraction(1)) * Fraction(1, 10**100)
    lower, upper = value-ulp-uncertainty, value+ulp+uncertainty
    lo, hi = round32(lower), round32(upper)
    if finite(lo) > lower: lo = nextdown(lo)
    if finite(hi) < upper: hi = nextup(hi)
    # Outward rounding is conservative: this is an enclosing interval, not a
    # claim that every representable value inside is a legal hardware answer.
    return finite(ftz(lo)), finite(ftz(hi))

@lru_cache(None)
def expbound(bits):
    bits = ftz(bits)
    if bits == 0xff800000: return point(0)
    if bits & 0x7fffffff == 0: return point(ONE)  # manual functional example
    x = finite(bits)
    if x < -126: return point(0)
    assert x < 128, 'reference only accepts finite softmax exponent range'
    with localcontext() as ctx:
        ctx.prec = 120
        value = Decimal(2) ** (Decimal(x.numerator)/Decimal(x.denominator))
    return enclosure(Fraction(value))

@lru_cache(None)
def rcpbound(x):
    assert x > 0
    return enclosure(1/x)

def intervalrcp(a):
    return rcpbound(a[1])[0], rcpbound(a[0])[1]

def fixture(t, sensitive=False):
    cap, tiles = t+3, (t+63)//64
    qptr = 64
    kptr = qptr + 24*256*4 + 32
    vptr = kptr + 4*cap*256*2 + 32
    pptr = vptr + 4*cap*256*2 + 32
    optr = pptr + 24*tiles*1040 + 32
    memory = bytearray([0xa5])*(optr+24*256*4+64)
    struct.pack_into('<5QII', memory, 0, qptr, kptr, vptr, pptr, optr, t, cap)
    q = [[round32(Fraction(((h*3+d*7)%17)-8, 8)) for d in range(256)] for h in range(24)]
    k = [[[round32(Fraction(((g*13+n*3+d*5)%19)-9, 8)) for d in range(256)] for n in range(cap)] for g in range(4)]
    v = [[[round32(Fraction(((g*11+n*7+d*3)%23)-11, 16)) for d in range(256)] for n in range(cap)] for g in range(4)]
    if sensitive:
        q = [[0x41800000]*256 for _ in range(24)]
        k = [[[round32(Fraction(16+(n%2))) for _ in range(256)] for n in range(cap)] for _ in range(4)]
    for h in range(24): struct.pack_into('<256I', memory, qptr+h*1024, *q[h])
    for g in range(4):
        for n in range(cap):
            for ptr, values in [(kptr,k), (vptr,v)]:
                assert all(x & 65535 == 0 for x in values[g][n])
                struct.pack_into('<256H', memory, ptr+(g*cap+n)*512, *(x>>16 for x in values[g][n]))
    return memory, q, k, v, pptr, optr, tiles

@lru_cache(None)
def dot(q, k):
    lanes = [0]*64
    for lane in range(64):
        for j in range(4): lanes[lane] = fma(q[4*lane+j], k[4*lane+j], lanes[lane])
    for shift in (32,16,8,4,2,1): lanes = [add(lanes[i], lanes[i^shift]) for i in range(64)]
    return mul(lanes[0], SCALE)

def maxima(a,b):
    if a == 0xff800000: return b
    return a if finite(a) >= finite(b) else b

def weights(old, new, score):
    return expbound(mul(sub(old,new),LOG2E)), expbound(mul(sub(score,new),LOG2E))

def oracle(t,q,k,v):
    partials, outputs = [], []
    for h in range(24):
        rows = []
        for start in range(0,t,64):
            m, den, acc = 0xff800000, point(0), [point(0)]*256
            for token in range(start,min(t,start+64)):
                score = dot(tuple(q[h]),tuple(k[h//6][token]))
                nxt = maxima(m,score)
                alpha,beta = weights(m,nxt,score)
                den = iadd(imul(den,alpha),beta)
                acc = [ifma(beta,point(value),imul(a,alpha)) for a,value in zip(acc,v[h//6][token])]
                m = nxt
            rows.append((m,den,acc))
        partials.append(rows)
        m,den,acc = 0xff800000,point(0),[point(0)]*256
        for pm,pden,pacc in rows:
            nxt = maxima(m,pm)
            alpha,beta = weights(m,nxt,pm)
            den = ifma(beta,pden,imul(den,alpha))
            acc = [ifma(beta,p,imul(a,alpha)) for a,p in zip(acc,pacc)]
            m = nxt
        outputs.append([imul(a,intervalrcp(den)) for a in acc] if t else acc)
    return partials,outputs

def inside(actual, interval, label):
    assert actual & 0x7fffffff < 0x7f800000, f'attention mismatch {label}: nonfinite {actual:08x}'
    value = finite(actual)
    assert interval[0] <= value <= interval[1], f'attention mismatch {label}: {actual:08x} outside {interval}'

def check(exe, part, merge, out, t, sensitive=False):
    data,q,k,v,pp,op,tiles = fixture(t,sensitive)
    partials,expected = oracle(t,q,k,v)
    path = out/'attention.memory'
    path.write_bytes(data)
    for text,x,y in [(part,(tiles+1)*64,25),(merge,64,25)]:
        p = subprocess.run([exe,str(text),str(path),str(x),str(y)],capture_output=True,timeout=600)
        assert p.returncode == 0 and not p.stdout and not p.stderr, p
    actual = path.read_bytes()
    allowed = bytearray(len(data))
    for h,rows in enumerate(partials):
        for tile,(m,den,acc) in enumerate(rows):
            base = pp+(h*tiles+tile)*1040
            assert struct.unpack_from('<I',actual,base)[0] == m, 'attention mismatch maximum'
            inside(struct.unpack_from('<I',actual,base+4)[0],den,'partial denominator')
            allowed[base:base+8] = b'\1'*8
            for d,interval in enumerate(acc):
                inside(struct.unpack_from('<I',actual,base+16+d*4)[0],interval,'partial accumulator')
            allowed[base+16:base+1040] = b'\1'*1024
    intervals = []
    widest = Fraction(0)
    for h in range(24):
        for d,interval in enumerate(expected[h]):
            offset = op+(h*256+d)*4
            inside(struct.unpack_from('<I',actual,offset)[0],interval,f'output[{h},{d}]')
            width = interval[1]-interval[0]
            widest = max(widest,width)
            intervals.append(dict(head=h,dim=d,lower=str(interval[0]),upper=str(interval[1]),width=str(width)))
    allowed[op:op+24*1024] = b'\1'*(24*1024)
    assert len(data)==len(actual) and all(a==b or allowed[i] for i,(a,b) in enumerate(zip(data,actual))), 'attention guard/input corruption'
    (out/f'attention-intervals-T{t}-sensitive{int(sensitive)}.json').write_text(json.dumps(intervals,indent=2)+'\n')
    print(f'PASS attention T={t} capacity={t+3} heads=24 KV=4 dim=256 sensitive={int(sensitive)} every-output/manual-interval all-guards widest_interval={widest} widest_float={float(widest):.9g}',flush=True)

def mutate(source,out):
    text = source.read_text()
    changes = {'no-max':('I 5 272 18 19 11','I 8 128 1 19'),
               'wrong-gqa':('I 2 20 134 28 1','I 2 20 133 28 1'),
               'v-from-k':('I 7 0 0 2 0 0 21 0 4 0 127 0 24','I 7 0 0 2 0 0 21 0 2 0 127 0 24')}
    for name,(old,new) in changes.items():
        assert text.count(old)==1,(name,text.count(old))
        (out/f'attention-{name}.isa').write_text(text.replace(old,new))

def suite(exe,out):
    part,merge = out/'attn256_part.text',out/'attn256_merge.text'
    for t in SHAPES: check(exe,part,merge,out,t)
    check(exe,part,merge,out,2,True)
    for name in ('no-max','wrong-gqa','v-from-k'):
        try: check(exe,out/f'attention-{name}.text',merge,out,2,name=='no-max')
        except AssertionError as error:
            assert str(error).startswith('attention mismatch'),error
            print(f'CONTROL attention-{name} CAUGHT {str(error).split(":")[0]}',flush=True)
        else: raise AssertionError(f'control escaped {name}')
    print(f'PASS attention cases={len(SHAPES)+1} T={",".join(map(str,SHAPES))} scale=1/16 tile=64 order=lane4,XOR32..1,online,tile-ascending hardware=NOT_RUN')

def transprobe(exe):
    import random
    rng = random.Random(942195)
    exp_inputs = [round32(Fraction(i, 16)) for i in range(-2016, 2017)]
    exp_inputs += [round32(Fraction(rng.randrange(-1250000, 1260001), 10000)) for _ in range(4096)]
    rcp_inputs = [rng.randrange(0x800000, 0x7f800000) for _ in range(4096)]
    records = [(32,b) for b in exp_inputs] + [(34,b) for b in rcp_inputs]
    result = subprocess.run([exe], input=b''.join(struct.pack('<II',op,b) for op,b in records),
                            capture_output=True, timeout=60)
    assert result.returncode==0 and result.stderr==b'' and len(result.stdout)==4*len(records), result
    for i,(op,b) in enumerate(records):
        actual, = struct.unpack_from('<I',result.stdout,i*4)
        interval = expbound(b) if op==32 else rcpbound(finite(b))
        inside(actual,interval,f'trans opcode={op} input={b:08x}')
    print(f'PASS EXP/RCP manual-1ULP enclosing-oracle inputs={len(records)} Decimal-precision=120 seed=942195 FTZ',flush=True)

if __name__=='__main__':
    mode = sys.argv[1]
    if mode=='mutate': mutate(pathlib.Path(sys.argv[2]),pathlib.Path(sys.argv[3]))
    elif mode=='one': check(sys.argv[2],pathlib.Path(sys.argv[3]),pathlib.Path(sys.argv[4]),pathlib.Path(sys.argv[5]),int(sys.argv[6]))
    elif mode=='transprobe': transprobe(sys.argv[2])
    else: suite(sys.argv[2],pathlib.Path(sys.argv[3]))
