#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Q27 preparation reference: Fraction instruction boundaries, Decimal model functions.
Test-only Python, not a runtime dependency. Reference and manual citations: PROVENANCE17.
"""
from decimal import Decimal, localcontext
from fractions import Fraction
from functools import lru_cache
import hashlib
import json
import pathlib
import random
import runpy
import struct
import subprocess
import sys

ref = runpy.run_path(str(pathlib.Path(__file__).with_name('attention.py')))
finite, round32, fma = [ref[n] for n in ('finite', 'round32', 'fma')]
add, mul, point, imul, iadd = [ref[n] for n in ('add', 'mul', 'point', 'imul', 'iadd')]
enclosure, expbound, intervalrcp = [ref[n] for n in ('enclosure', 'expbound', 'intervalrcp')]
ONE, EPS = 0x3f800000, 0x358637bd
POSITIONS = (0, 1, 63, 64, 65)
ROPE_POSITIONS = (0, 1, 63, 64, 65, 8191, 65535, 262143, 0xffffffff)
CONTROLS = ('no-one', 'wrong-pair', 'wrong-position', 'truncate-cache', 'extra-swish', 'wrong-gate-stride')
# WolframLanguageEvaluator N[Pi,125], independently filed in PROVENANCE17.
PI = Decimal('3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582')

@lru_cache(None)
def rsqbound(x):
    assert x > 0
    with localcontext() as ctx:
        ctx.prec = 120
        result = 1 / (Decimal(x.numerator) / Decimal(x.denominator)).sqrt()
    return enclosure(Fraction(result))

def rsqprobe(exe):
    rng = random.Random(942197)
    values = [rng.randrange(0x800000, 0x7f800000) for _ in range(4096)]
    result = subprocess.run([exe], input=b''.join(struct.pack('<II',36,x) for x in values), capture_output=True, timeout=60)
    assert result.returncode == 0 and result.stderr == b'' and len(result.stdout) == 4*len(values), result
    for i, x in enumerate(values):
        inside(struct.unpack_from('<I',result.stdout,4*i)[0],rsqbound(finite(x)),f'rsq[{x:x}]')
    print(f'PASS RSQ manual-1ULP enclosing-oracle inputs={len(values)} Decimal-precision=120 seed=942197 FTZ')

def inside(bits, interval, where):
    assert bits & 0x7fffffff < 0x7f800000, f'prep mismatch {where}: nonfinite {bits:08x}'
    assert interval[0] <= finite(bits) <= interval[1], f'prep mismatch {where}: {bits:08x} not in {interval}'

@lru_cache(None)
def inverse_frequencies():
    with localcontext() as ctx:
        ctx.prec = 120
        powers = [round32(Fraction(Decimal(10000000) ** (Decimal(i)/32))) for i in range(32)]
    return tuple(round32(1/finite(p)) for p in powers)

def frequency_check(exe):
    result = subprocess.run([exe],capture_output=True,timeout=15)
    expected = struct.pack('<32I',*inverse_frequencies())
    assert result.returncode == 0 and result.stderr == b'' and result.stdout == expected, 'rope frequency mismatch'
    print('PASS rope inverse frequencies=32 bit-exact Decimal power -> RN32 -> reciprocal -> RN32')

@lru_cache(None)
def rope_reference(position):
    values = []
    with localcontext() as ctx:
        ctx.prec = 120
        pf = finite(round32(Fraction(position)))
        for freq in inverse_frequencies():
            angle = finite(round32(pf*finite(freq)))
            x = Decimal(angle.numerator)/Decimal(angle.denominator)
            # Independent high-precision argument reduction and sine/cosine power series.
            n = int((x/(PI/2)+Decimal('0.5')).to_integral_value(rounding='ROUND_FLOOR'))
            r = x-n*(PI/2)
            sin, cos, st, ct = r, Decimal(1), r, Decimal(1)
            for j in range(1,50):
                st *= -r*r/((2*j)*(2*j+1))
                ct *= -r*r/((2*j)*(2*j-1))
                sin += st
                cos += ct
            cos, sin = [(cos,sin),(-sin,cos),(-cos,-sin),(sin,-cos)][n%4]
            values.append((Fraction(cos),Fraction(sin)))
    return [x[0] for x in values]+[x[1] for x in values]

def rope_output(exe, position):
    result = subprocess.run([exe], input=struct.pack('<I',position), capture_output=True, timeout=15)
    assert result.returncode == 0 and result.stderr == b'' and len(result.stdout) == 256, result
    return struct.unpack('<64I',result.stdout)

def check_rope(exe, position):
    output = rope_output(exe,position)
    widest = Fraction(0)
    for i,(bits,exact) in enumerate(zip(output,rope_reference(position))):
        ulp = Fraction(2)**(max(1,(bits>>23)&255)-150)
        # Binary64 reduction error allowance over the full u32 input range, plus final RN32.
        # This is an absolute analytic check, not an allowance on the subsequent GPU execution.
        allowance = ulp/2 + Fraction(1,1<<39)
        error = abs(finite(bits)-exact)
        assert error <= allowance, f'rope mismatch P={position} index={i} error={error} bound={allowance}'
        widest = max(widest,error)
    print(f'PASS rope P={position} cos/sin=64 Decimal-precision=120 max_absolute_error={float(widest):.9g}')
    return output

def rope_suite(exe):
    for p in ROPE_POSITIONS: check_rope(exe,p)
    for bad in (b'',b'\0'*3,b'\0'*5):
        result = subprocess.run([exe],input=bad,capture_output=True,timeout=15)
        assert result.returncode == 64 and result.stdout == b'' and result.stderr == b'', result
    print('CONTROL rope-malformed-record CAUGHT lengths=0,3,5 exit=64 no-output')

def normrope(x, weights, cs):
    lanes = [0]*64
    for lane in range(64):
        for j in range(4): lanes[lane] = fma(x[4*lane+j],x[4*lane+j],lanes[lane])
    for shift in (32,16,8,4,2,1): lanes = [add(lanes[i],lanes[i^shift]) for i in range(64)]
    variance = add(mul(lanes[0],0x3b800000),EPS)
    inv = rsqbound(finite(variance))
    result = [imul(imul(point(a),inv),point(add(ONE,w))) for a,w in zip(x,weights)]
    before = result[:64]
    for d in range(32):
        negative = (-before[d+32][1],-before[d+32][0])
        result[d] = iadd(imul(before[d],point(cs[d])),imul(negative,point(cs[d+32])))
        result[d+32] = iadd(imul(before[d+32],point(cs[d])),imul(before[d],point(cs[d+32])))
    return result

def bf16(bits):
    if bits & 0x7fffffff > 0x7f800000: return 0x7fc0
    return ((bits + 0x7fff + ((bits>>16)&1)) >> 16) & 0xffff

def fixture(p,cs):
    cap = p+3
    sizes = (24*512*4,4*256*4,4*256*4,256*4,256*4,256,24*256*4,4*cap*256*2,4*cap*256*2)
    pointers, end = [],128
    for size in sizes:
        pointers.append(end)
        end += size+32
    data = bytearray([0xa5])*end
    struct.pack_into('<9QII',data,0,*pointers,p,cap)
    q = [[round32(Fraction((h*11+d*7)%67-33,16)) for d in range(256)] for h in range(24)]
    k = [[round32(Fraction((h*3+d*13)%71-35,16)) for d in range(256)] for h in range(4)]
    special = [0,0x80000000,1,0x00010000,0x007fffff,0x3f808000,0x3f818000,0x7f800000,0xff800000,0x7f800001,0xffc01234]
    values = [[special[(h*3+d)%len(special)] if d<32 else round32(Fraction((h+d)%19-9,8)) for d in range(256)] for h in range(4)]
    qw = [round32(Fraction(d%7-3,16)) for d in range(256)]
    kw = [round32(Fraction(d%11-5,32)) for d in range(256)]
    for h,row in enumerate(q): struct.pack_into('<256I',data,pointers[0]+h*2048,*row)
    for h,row in enumerate(k): struct.pack_into('<256I',data,pointers[1]+h*1024,*row)
    for h,row in enumerate(values): struct.pack_into('<256I',data,pointers[2]+h*1024,*row)
    struct.pack_into('<256I',data,pointers[3],*qw)
    struct.pack_into('<256I',data,pointers[4],*kw)
    struct.pack_into('<64I',data,pointers[5],*cs)
    return data,pointers,q,k,values,qw,kw,cap

def run(exe,code,path,data,x,y):
    path.write_bytes(data)
    result = subprocess.run([exe,str(code),str(path),str(x),str(y)],capture_output=True,timeout=120)
    assert result.returncode == 0 and result.stderr == b'' and result.stdout == b'', result
    return path.read_bytes()

def prep_check(exe,rope,code,out,p,invalid=False):
    cs = rope_output(rope,p)
    data,ptr,q,k,values,qw,kw,cap = fixture(p,cs)
    if invalid: struct.pack_into('<I',data,76,p)
    actual = run(exe,code,out/'prep.memory',data,25*64,2)
    if invalid:
        assert actual == data, 'prep mismatch capacity guard wrote output'
        print(f'PASS prep invalid-capacity P={p} no-output no-cache-write')
        return
    allowed = bytearray(len(data))
    widest, records, ambiguous = Fraction(0),[],0
    for h,row in enumerate(q):
        intervals = normrope(row,qw,cs)
        for d,interval in enumerate(intervals):
            offset = ptr[6]+h*1024+d*4
            inside(struct.unpack_from('<I',actual,offset)[0],interval,f'Q[{h},{d}]')
            widest = max(widest,interval[1]-interval[0])
            records.append(dict(head=h,dim=d,lower=str(interval[0]),upper=str(interval[1]),width=str(interval[1]-interval[0])))
    allowed[ptr[6]:ptr[6]+24*1024] = b'\1'*(24*1024)
    for h,row in enumerate(k):
        for d,interval in enumerate(normrope(row,kw,cs)):
            bits, = struct.unpack_from('<H',actual,ptr[7]+(h*cap+p)*512+d*2)
            lo,hi = (bf16(round32(x)) for x in interval)
            ambiguous += lo != hi
            inside(bits<<16,(min(finite(lo<<16),finite(hi<<16)),max(finite(lo<<16),finite(hi<<16))),f'K[{h},{d}]')
        got = struct.unpack_from('<256H',actual,ptr[8]+(h*cap+p)*512)
        assert got == tuple(map(bf16,values[h])),f'prep mismatch V BF16[{h}]'
        for base in (ptr[7],ptr[8]):
            offset = base+(h*cap+p)*512
            allowed[offset:offset+512] = b'\1'*512
    assert len(actual)==len(data) and all(a==b or allowed[i] for i,(a,b) in enumerate(zip(data,actual))), 'prep mismatch guard/input/history corruption'
    (out/f'prep-intervals-P{p}.json').write_text(json.dumps(records,indent=2)+'\n')
    print(f'PASS prep P={p} Q=24x256 K/V=4x256 rotary=64 all-history/guards BF16-specials widest_interval={widest} ambiguous_BF16={ambiguous}',flush=True)

def gate_check(exe,code,out):
    qp,ap,op = 64,64+24*2048+32,64+24*2048+32+24*1024+32
    data = bytearray([0xa5])*(op+24*1024+32)
    struct.pack_into('<3Q',data,0,qp,ap,op)
    inputs, gates = [],[]
    for h in range(24):
        x = [round32(Fraction((h*17+d*3)%37-18,8)) for d in range(256)]
        g = [round32(Fraction((h*5+d*7)%41-20,4)) for d in range(256)]
        g[:6] = [0,0x80000000,0x42c80000,0xc2c80000,0x7f800000,0xff800000]
        struct.pack_into('<256I',data,qp+h*2048+1024,*g)
        struct.pack_into('<256I',data,ap+h*1024,*x)
        inputs.extend(x);gates.extend(g)
    actual = run(exe,code,out/'gate.memory',data,97*64,2)
    widest = Fraction(0)
    for i,(x,g) in enumerate(zip(inputs,gates)):
        exponent = mul((g & 0x7fffffff)|0x80000000,ref['LOG2E'])
        t = expbound(exponent)
        sig = intervalrcp(iadd(point(ONE),t))
        if g>>31: sig = imul(sig,t)
        interval = imul(point(x),sig)
        inside(struct.unpack_from('<I',actual,op+i*4)[0],interval,f'gate[{i}]')
        widest = max(widest,interval[1]-interval[0])
    assert actual[:op] == data[:op] and actual[op+24*1024:] == data[op+24*1024:], 'prep mismatch gate guards/input'
    print(f'PASS attention sigmoid gate packed=24x512 outputs={len(inputs)} signed-zero/extreme/inf widest_interval={widest}',flush=True)

def mutate(src,out):
    prep = (src/'attn256_prep.isa').read_text()
    gate = (src/'attn256_gate.isa').read_text()
    changes = {'no-one':(prep,'I 5 300 12 12 1','I 8 268 1 12'),
               'wrong-pair':(prep,'I 5 160 1 28 21','I 5 144 1 28 21'),
               'wrong-position':(prep,'I 5 22 4 4 25','I 5 128 4 4 25'),
               'truncate-cache':(prep,'L 32767','L 0'),
               'extra-swish':(gate,'I 7 0 0 2 0 0 28 0 6 9 127 0 0','I 5 264 9 9 5\nI 7 0 0 2 0 0 28 0 6 9 127 0 0'),
               'wrong-gate-stride':(gate,'I 5 139 2 2 18','I 5 138 2 2 18')}
    assert tuple(changes) == CONTROLS
    for name,(text,old,new) in changes.items():
        assert old in text,name
        (out/f'prep-{name}.isa').write_text(text.replace(old,new))
    rope = (src/'rope.om').read_text()
    assert '32.0f64' in rope
    (out/'rope-wrong-dim.om').write_text(rope.replace('32.0f64','128.0f64'))

def config_check():
    data = (pathlib.Path(__file__).parent/'model/qwen38-27b-fp8-config.json').read_bytes()
    assert hashlib.sha256(data).hexdigest() == '74227dd615bf1ea975aa676bdf355a0379858c12f394b5365cd9dfa5fc2c70bc'
    c = json.loads(data)['text_config']
    assert (c['num_attention_heads'],c['num_key_value_heads'],c['head_dim']) == (24,4,256)
    assert c['rms_norm_eps'] == 1e-6 and c['attn_output_gate']
    assert EPS == struct.unpack('<I',struct.pack('<f',c['rms_norm_eps']))[0]
    assert c['rope_parameters']['rope_theta'] == 10000000
    assert c['rope_parameters']['partial_rotary_factor'] * c['head_dim'] == 64
    assert c['rope_parameters']['mrope_section'] == [11,11,10]
    print('PASS pinned Q27 config digest and compiled attention dimensions/norm/rotary contract')

def suite(exe,rope,out):
    config_check()
    frequency_check(str(out/'ropefreq-probe'))
    rope_suite(rope)
    rsqprobe(str(out/'trans-probe'))
    for p in POSITIONS: prep_check(exe,rope,out/'attn256_prep.text',out,p)
    prep_check(exe,rope,out/'attn256_prep.text',out,1,True)
    gate_check(exe,out/'attn256_gate.text',out)
    for name in CONTROLS:
        try:
            if name in ('extra-swish','wrong-gate-stride'): gate_check(exe,out/f'prep-{name}.text',out)
            else: prep_check(exe,rope,out/f'prep-{name}.text',out,1)
        except AssertionError as error:
            assert str(error).startswith('prep mismatch'),error
            print(f'CONTROL prep-{name} CAUGHT {str(error).split(":")[0]}',flush=True)
        else: raise AssertionError(f'control escaped {name}')
    try: check_rope(str(out/'rope-wrong-dim'),1)
    except AssertionError as error:
        assert str(error).startswith('rope mismatch'),error
        print('CONTROL rope-wrong-dim CAUGHT config rotary64 not256')
    else: raise AssertionError('rope dimension control escaped')
    print(f'PASS fused prep positions={",".join(map(str,POSITIONS))} prep_cases={len(POSITIONS)+1} gate_cases=1 rope_cases={len(ROPE_POSITIONS)} hardware=NOT_RUN')

if __name__=='__main__':
    if sys.argv[1]=='mutate': mutate(pathlib.Path(sys.argv[2]),pathlib.Path(sys.argv[3]))
    else: suite(sys.argv[2],sys.argv[3],pathlib.Path(sys.argv[4]))
