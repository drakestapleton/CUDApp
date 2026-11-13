# Generated from canonical host fixture/oracle functions; no embedded memory images.
import base64,struct
from decimal import Decimal,localcontext
from fractions import Fraction
from functools import lru_cache
CHANNELS=10240
LOG2E=1069066811
ONE=1065353216
def round32(value):
    if not value:
        return 0
    sign = 2147483648 if value < 0 else 0
    value = abs(value)
    e = value.numerator.bit_length() - value.denominator.bit_length()
    if value < Fraction(2) ** e:
        e -= 1
    quantum = max(-149, e - 23)
    scaled = value / Fraction(2) ** quantum
    q, r = divmod(scaled.numerator, scaled.denominator)
    if 2 * r > scaled.denominator or (2 * r == scaled.denominator and q & 1):
        q += 1
    if q == 1 << 24:
        q >>= 1
        quantum += 1
    if quantum > 104:
        return sign | 2139095040
    exponent = quantum + 150 if q >= 1 << 23 else 0
    return sign | exponent << 23 | q & 8388607
def bf16(bits):
    if bits & 2147483647 > 2139095040:
        return 32704
    return bits + 32767 + (bits >> 16 & 1) >> 16 & 65535
def finite(bits):
    exponent = bits >> 23 & 255
    significand = bits & 8388607 | (8388608 if exponent else 0)
    power = max(1, exponent) - 150
    value = Fraction(significand) * Fraction(2) ** power
    return -value if bits >> 31 else value
@lru_cache(None)
def fma(a, b, c):
    ax, bx, cx = (x & 2147483647 for x in (a, b, c))
    if max(ax, bx, cx) > 2139095040:
        return 2143289344
    product_sign = (a ^ b) & 2147483648
    if ax == 2139095040 or bx == 2139095040:
        if not ax or not bx or (cx == 2139095040 and product_sign != c & 2147483648):
            return 2143289344
        return product_sign | 2139095040
    if cx == 2139095040:
        return c
    if (not ax or not bx) and (not cx):
        return product_sign & c & 2147483648
    return round32(finite(a) * finite(b) + finite(c))
def rounded(x):
    return finite(round32(x))
@lru_cache(None)
def imul(a, b):
    vals = [x * y for x in a for y in b]
    return (rounded(min(vals)), rounded(max(vals)))
def point(bits):
    x = finite(bits)
    return (x, x)
def mul(a, b):
    return fma(a, b, (a ^ b) & 2147483648)
def ftz(b):
    return b & 2147483648 if b & 2147483647 < 8388608 else b
def nextdown(b):
    if b == 0:
        return 2147483649
    return b + 1 if b >> 31 else b - 1
def nextup(b):
    if b == 2147483648:
        return 1
    return b - 1 if b >> 31 else b + 1
def enclosure(value):
    center = round32(value)
    exponent = max(1, center >> 23 & 255) - 150
    ulp = Fraction(2) ** exponent
    uncertainty = max(abs(value), Fraction(1)) * Fraction(1, 10 ** 100)
    lower, upper = (value - ulp - uncertainty, value + ulp + uncertainty)
    lo, hi = (round32(lower), round32(upper))
    if finite(lo) > lower:
        lo = nextdown(lo)
    if finite(hi) < upper:
        hi = nextup(hi)
    return (finite(ftz(lo)), finite(ftz(hi)))
@lru_cache(None)
def expbound(bits):
    bits = ftz(bits)
    if bits == 4286578688:
        return point(0)
    if bits & 2147483647 == 0:
        return point(ONE)
    x = finite(bits)
    if x < -126:
        return point(0)
    assert x < 128, 'reference only accepts finite softmax exponent range'
    with localcontext() as ctx:
        ctx.prec = 120
        value = Decimal(2) ** (Decimal(x.numerator) / Decimal(x.denominator))
    return enclosure(Fraction(value))
@lru_cache(None)
def rcpbound(x):
    assert x > 0
    return enclosure(1 / x)
def intervalrcp(a):
    return (rcpbound(a[1])[0], rcpbound(a[0])[1])
@lru_cache(None)
def iadd(a, b):
    return (rounded(a[0] + b[0]), rounded(a[1] + b[1]))
@lru_cache(None)
def sigmoid(g):
    exponent = mul(g & 2147483647 | 2147483648, LOG2E)
    t = expbound(exponent)
    answer = intervalrcp(iadd(point(ONE), t))
    return imul(answer, t) if g >> 31 else answer
def fixture(active, kind):
    pointers = []
    end = 64
    for size in (CHANNELS * 4, CHANNELS * 8, CHANNELS * 8, CHANNELS * 8, CHANNELS * 4):
        pointers.append(end)
        end += size + 64
    xp, wp, sp, np, op = pointers
    image = bytearray([165]) * end
    struct.pack_into('<5Q', image, 0, *pointers)
    inputs = [round32(Fraction(i * 19 % 127 - 63, 32)) for i in range(CHANNELS)]
    inputs[:8] = [1065385984, 1065451520, 3212869632, 3212935168, 1, 2147483649, 1120403456, 3267887104]
    if kind == 'range':
        inputs[8:12] = [2139095040, 4286578688, 2139099700, 4290777652]
    struct.pack_into('<' + str(CHANNELS) + 'I', image, xp, *inputs)
    intervals = []
    exact = []
    for c in range(CHANNELS):
        weights = [bf16(round32(Fraction((c * 7 + j * 11) % 31 - 15, 16))) for j in range(4)]
        old = [0 if kind == 'zero' else bf16(round32(Fraction((c * 17 + j * 5) % 61 - 30, 16))) for j in range(4)]
        struct.pack_into('<4H', image, wp + c * 8, *weights)
        struct.pack_into('<4H', image, sp + c * 8, *old)
        if c >= active:
            continue
        new = old[1:] + [bf16(inputs[c])]
        exact.extend(([np + c * 8 + j * 2, x, 2] for j, x in enumerate(new)))
        total = 0
        for a, w in zip(new, weights):
            total = fma(a << 16, w << 16, total)
        conv = bf16(total) << 16
        if conv & 2147483647 >= 2139095040:
            answer = 2139095040 if conv == 2139095040 else 2143289344
            exact.append([op + c * 4, answer, 4])
        else:
            endpoints = imul(point(conv), sigmoid(conv))
            ends = [finite(bf16(round32(x)) << 16) for x in endpoints]
            intervals.append([op + c * 4, str(min(ends)), str(max(ends))])
    return dict(label=f'channels={active}/{CHANNELS} taps=4 kind={kind}', memory=base64.b64encode(image).decode(), intervals=intervals, exact=exact, bf16_outputs=[v[0] for v in intervals], args=[['ptr', p] for p in pointers], gx=CHANNELS // 64 + 1 if active == CHANNELS else active // 64, gy=2, timing_grid=[CHANNELS // 64, 1], input_bytes=CHANNELS * (4 + 8 + 8))
def _make_payload():
    cases=[fixture(*args) for args in ((64, 'zero'), (128, 'range'), (10240, 'history'))]
    return dict(name='gdnconv4',cases=cases,controls=['gdnconv4-no-input-round', 'gdnconv4-no-conv-round', 'gdnconv4-no-silu-round', 'gdnconv4-wrong-cache-shift'],timing=len(cases)-1)
"""Test-only notebook instrument. token.py exports this with generated fixtures.
No production dependency; the numerical intervals come from the host oracle.
"""

def run_section(ctx):
    import base64
    import ctypes as C
    import struct
    import time
    from fractions import Fraction
    p = _make_payload()
    name = p['name']
    say, put, get = (ctx[k] for k in ('say', 'put', 'get'))
    load, Params, launch = (ctx[k] for k in ('load', 'Params', 'launch'))
    objects, controls = (ctx['objs'], ctx['controls'])
    fn = load(objects[name], name)
    owned = []

    def alloc(data):
        d = put(data)
        owned.append(d)
        return d

    def raw(case):
        return base64.b64decode(case['memory'])

    def step(case, base, function=fn):
        vals = [C.c_void_p(base.value + v) if kind == 'ptr' else C.c_uint32(v) for kind, v in case['args']]
        return (function, case['gx'], case['gy'], Params(vals))

    def check(actual, case):
        before = raw(case)
        assert len(actual) == len(before), 'notebook memory size mismatch'
        unchanged = bytearray(actual)
        widest = Fraction(0)
        for off in case.get('bf16_outputs', []):
            assert struct.unpack_from('<I', actual, off)[0] & 65535 == 0, f'{name} BF16 rounding mismatch offset={off}'
        for off, lo, hi in case['intervals']:
            bits = struct.unpack_from('<I', actual, off)[0]
            x = struct.unpack('<f', struct.pack('<I', bits))[0]
            lo, hi = (Fraction(lo), Fraction(hi))
            assert math.isfinite(x) and lo <= Fraction(x) <= hi, f'{name} interval mismatch offset={off} bits={bits:08x}'
            widest = max(widest, hi - lo)
            unchanged[off:off + 4] = before[off:off + 4]
        for off, bits, width in case.get('exact', []):
            assert int.from_bytes(actual[off:off + width], 'little') == bits, f'{name} exact mismatch offset={off}'
            unchanged[off:off + width] = before[off:off + width]
        assert unchanged == before, f'{name} input/guard corruption'
        return widest
    try:
        for case in p['cases']:
            d = alloc(raw(case))
            launch([step(case, d)])
            width = check(get(d, len(raw(case))), case)
            say(f'PASS {name} {case['label']} every-output interval/exact inputs/guards widest_interval={width}')
        case = dict(p['cases'][p['timing']])
        case['gx'], case['gy'] = case['timing_grid']
        image = raw(case)
        d = alloc(image)
        steps = [step(case, d)]
        ev = ctx['event_ms'](steps)
        warm = ctx['wall_ms']([steps])
        expected = get(d, len(image))
        width = check(expected, case)
        copies = int(ctx['COLD_BYTES']) // case['input_bytes'] + 1
        cold = alloc(image * copies)
        rotation = [[step(case, C.c_void_p(cold.value + i * len(image)))] for i in range(copies)]
        cold_ms = ctx['wall_ms'](rotation)
        observed = memoryview(get(cold, len(image) * copies))
        for i in range(copies):
            assert observed[i * len(image):(i + 1) * len(image)] == expected, f'{name} cold output/guard mismatch copy={i}'
        say(f'{name} {case['label']} event_ms={ev:.6f} warm_wall_ms={warm:.6f} cold_wall_ms={cold_ms:.6f} copies={copies} input_bytes={copies * case['input_bytes']} every-timed-output=PASS guards=PASS widest_interval={width}')
        for control in p['controls']:
            bad = load(objects[control], name)
            d = alloc(image)
            launch([step(case, d, bad)])
            try:
                check(get(d, len(image)), case)
            except AssertionError:
                controls[control] = True
                say(f'CONTROL {control} CAUGHT')
            else:
                controls[control] = False
                raise AssertionError(f'{control} escaped')
    finally:
        for d in reversed(owned):
            ctx['chk'](ctx['hip'].hipFree(d), f'free {name} section')
    try:
        import torch
        import torch.nn.functional as F
        case = p['cases'][p['timing']]
        image = raw(case)

        def tensor(off, n):
            vals = struct.unpack_from('<' + str(n) + 'f', image, off)
            return torch.tensor(vals, device='cuda', dtype=torch.float32)
        n = 16 * 128 * 2 + 48 * 128
        x = tensor(case['args'][0][1], n).view(1, n, 1)

        def bfloat(off):
            values = struct.unpack_from('<' + str(n * 4) + 'H', image, off)
            words = struct.pack('<' + str(n * 4) + 'I', *(v << 16 for v in values))
            return torch.tensor(struct.unpack('<' + str(n * 4) + 'f', words), device='cuda', dtype=torch.bfloat16)
        weight = bfloat(case['args'][1][1]).view(n, 1, 4)
        old = bfloat(case['args'][2][1]).view(1, n, 4)

        def operation():
            state = torch.cat((old, x), dim=-1).to(weight.dtype)
            new = state[:, :, -4:].clone()
            out = F.silu(F.conv1d(state, weight, groups=n)[:, :, -1:]).float()
            return (out, new)
        operation()
        torch.cuda.synchronize()
        repetitions = ctx.get('NWALL', 200)
        start = time.perf_counter()
        for _ in range(repetitions):
            reference = operation()
        torch.cuda.synchronize()
        elapsed = (time.perf_counter() - start) * 1000 / repetitions
        for value in reference if isinstance(reference, tuple) else (reference,):
            assert torch.isfinite(value).all().item()
        dtype = 'BF16-conv/cache FP32-accum/output'
        say(f'torch {name} (AMD library, not Omega) warm_wall_ms={elapsed:.6f} dtype={dtype}')
    except ImportError:
        say(f'torch {name} (AMD library, not Omega) NOT_RUN unavailable')
    return True
