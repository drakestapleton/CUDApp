# Generated from canonical host fixture/oracle functions; no embedded memory images.
import base64,struct
from decimal import Decimal,localcontext
from fractions import Fraction
from functools import lru_cache
EPS=897988541
MEAN=961334477
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
def mul(a, b):
    return fma(a, b, (a ^ b) & 2147483648)
def add(a, b):
    return fma(a, ONE, b)
def nextdown(b):
    if b == 0:
        return 2147483649
    return b + 1 if b >> 31 else b - 1
def nextup(b):
    if b == 2147483648:
        return 1
    return b - 1 if b >> 31 else b + 1
def ftz(b):
    return b & 2147483648 if b & 2147483647 < 8388608 else b
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
def rsqbound(x):
    assert x > 0
    with localcontext() as ctx:
        ctx.prec = 120
        result = 1 / (Decimal(x.numerator) / Decimal(x.denominator)).sqrt()
    return enclosure(Fraction(result))
def rounded(x):
    return finite(round32(x))
@lru_cache(None)
def imul(a, b):
    vals = [x * y for x in a for y in b]
    return (rounded(min(vals)), rounded(max(vals)))
def point(bits):
    x = finite(bits)
    return (x, x)
def fixture(rows, kind):
    xp, wp, op = (64, 64 + rows * 5120 * 4 + 64, 64 + rows * 5120 * 4 + 64 + 5120 * 4 + 64)
    image = bytearray([165]) * (op + rows * 5120 * 4 + 64)
    struct.pack_into('<3QI', image, 0, xp, wp, op, rows)
    weights = [round32(Fraction(i * 19 % 67 - 33, 32)) for i in range(5120)]
    struct.pack_into('<5120I', image, wp, *weights)
    intervals = []
    for row in range(rows):
        x = [round32(Fraction((i * 17 + row * 5) % 97 - 48, 16)) for i in range(5120)]
        if kind == 'zero':
            x = [0] * 5120
        elif kind == 'large':
            x = [mul(v, 1124073472) for v in x]
        else:
            x[:4] = [0, 2147483648, 1, 2147483649]
        struct.pack_into('<5120I', image, xp + row * 5120 * 4, *x)
        lanes = [0] * 64
        for lane in range(64):
            for chunk in range(20):
                for j in range(4):
                    a = x[chunk * 256 + lane * 4 + j]
                    lanes[lane] = fma(a, a, lanes[lane])
        for shift in (32, 16, 8, 4, 2, 1):
            lanes = [add(lanes[i], lanes[i ^ shift]) for i in range(64)]
        variance = add(mul(lanes[0], MEAN), EPS)
        inv = rsqbound(finite(variance))
        for i, (a, w) in enumerate(zip(x, weights)):
            lo, hi = imul(imul(point(a), inv), point(add(ONE, w)))
            intervals.append([op + (row * 5120 + i) * 4, str(lo), str(hi)])
    return dict(label=f'rows={rows} hidden=5120 kind={kind}', memory=base64.b64encode(image).decode(), intervals=intervals, args=[['ptr', xp], ['ptr', wp], ['ptr', op], ['u32', rows]], gx=rows + 1, gy=2, input_bytes=(rows + 1) * 5120 * 4 + 4, timing_grid=[max(1, rows), 1])
def _make_payload():
    cases=[fixture(*args) for args in ((0, 'varied'), (1, 'zero'), (2, 'varied'), (3, 'large'), (1, 'varied'))]
    return dict(name='rmsnorm5120',cases=cases,controls=['rmsnorm5120-no-one', 'rmsnorm5120-wrong-mean', 'rmsnorm5120-dropped-chunk'],timing=len(cases)-1)
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
        n = 5120
        x, w = (tensor(case['args'][j][1], n) for j in (0, 1))
        operation = lambda: x * torch.rsqrt(x.square().mean() + 1e-06) * (1 + w)
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
        dtype = 'FP32'
        say(f'torch {name} (AMD library, not Omega) warm_wall_ms={elapsed:.6f} dtype={dtype}')
    except ImportError:
        say(f'torch {name} (AMD library, not Omega) NOT_RUN unavailable')
    return True
