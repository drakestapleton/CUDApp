# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test-only notebook instrument. token.py exports this with generated fixtures.
No production dependency; the numerical intervals come from the host oracle.
"""
def run_section(ctx):
    import base64
    import ctypes as C
    import json
    import math
    import struct
    import time
    import zlib
    from fractions import Fraction

    p = json.loads(zlib.decompress(base64.b64decode(PAYLOAD)))
    name = p['name']
    say, put, get = (ctx[k] for k in ('say', 'put', 'get'))
    load, Params, launch = (ctx[k] for k in ('load', 'Params', 'launch'))
    objects, controls = ctx['objs'], ctx['controls']
    fn = load(objects[name], name)
    owned = []

    def alloc(data):
        d = put(data)
        owned.append(d)
        return d

    def raw(case):
        return base64.b64decode(case['memory'])

    def step(case, base, function=fn):
        vals = [C.c_void_p(base.value+v) if kind == 'ptr' else C.c_uint32(v)
                for kind, v in case['args']]
        return function, case['gx'], case['gy'], Params(vals)

    def check(actual, case):
        before = raw(case)
        assert len(actual) == len(before), 'notebook memory size mismatch'
        unchanged = bytearray(actual)
        widest = Fraction(0)
        for off, lo, hi in case['intervals']:
            bits = struct.unpack_from('<I', actual, off)[0]
            x = struct.unpack('<f', struct.pack('<I', bits))[0]
            lo, hi = Fraction(lo), Fraction(hi)
            assert math.isfinite(x) and lo <= Fraction(x) <= hi, f'{name} interval mismatch offset={off} bits={bits:08x}'
            widest = max(widest, hi-lo)
            unchanged[off:off+4] = before[off:off+4]
        for off, bits, width in case.get('exact', []):
            assert int.from_bytes(actual[off:off+width], 'little') == bits, f'{name} exact mismatch offset={off}'
            unchanged[off:off+width] = before[off:off+width]
        assert unchanged == before, f'{name} input/guard corruption'
        return widest

    try:
        for case in p['cases']:
            d = alloc(raw(case))
            launch([step(case,d)])
            width = check(get(d,len(raw(case))),case)
            say(f'PASS {name} {case["label"]} every-output interval/exact inputs/guards widest_interval={width}')
        case = dict(p['cases'][p['timing']])
        case['gx'],case['gy'] = case['timing_grid']
        image = raw(case)
        d = alloc(image)
        steps = [step(case,d)]
        ev = ctx['event_ms'](steps)
        warm = ctx['wall_ms']([steps])
        expected = get(d,len(image))
        width = check(expected,case)
        # Distinct input allocations/addresses exceed the requested cold working set.
        # The output is a separate region; all inputs are immutable across launches.
        # Each case is idempotent, including stateful operators with old/new buffers.
        copies = int(ctx['COLD_BYTES']) // case['input_bytes'] + 1
        cold = alloc(image*copies)
        rotation = [[step(case,C.c_void_p(cold.value+i*len(image)))] for i in range(copies)]
        cold_ms = ctx['wall_ms'](rotation)
        observed = memoryview(get(cold,len(image)*copies))
        for i in range(copies):
            # Exact equality with the interval-checked warm run checks EVERY output,
            # every guard and every input of EVERY timed cold copy, not a sample.
            assert observed[i*len(image):(i+1)*len(image)] == expected, f'{name} cold output/guard mismatch copy={i}'
        say(f'{name} {case["label"]} event_ms={ev:.6f} warm_wall_ms={warm:.6f} cold_wall_ms={cold_ms:.6f} copies={copies} input_bytes={copies*case["input_bytes"]} every-timed-output=PASS guards=PASS widest_interval={width}')
        for control in p['controls']:
            bad = load(objects[control],name)
            d = alloc(image)
            launch([step(case,d,bad)])
            try:
                check(get(d,len(image)),case)
            except AssertionError:
                controls[control] = True
                say(f'CONTROL {control} CAUGHT')
            else:
                controls[control] = False
                raise AssertionError(f'{control} escaped')
    finally:
        for d in reversed(owned):
            ctx['chk'](ctx['hip'].hipFree(d),f'free {name} section')
    # Reference implementation is a measurement instrument, never an Omega kernel.
    try:
        import torch
        import torch.nn.functional as F
        case = p['cases'][p['timing']]
        image = raw(case)
        def tensor(off,n):
            vals = struct.unpack_from('<'+str(n)+'f',image,off)
            return torch.tensor(vals,device='cuda',dtype=torch.float32)
        if name == 'mlpsilu':
            n = case['args'][3][1]
            g,u = (tensor(case['args'][j][1],n) for j in (0,1))
            operation = lambda: F.silu(g)*u
        elif name == 'rmsnorm5120':
            n = 5120
            x,w = (tensor(case['args'][j][1],n) for j in (0,1))
            operation = lambda: x*torch.rsqrt(x.square().mean()+1e-6)*(1+w)
        else:
            raise ValueError('missing torch reference for '+name)
        operation(); torch.cuda.synchronize()
        repetitions = ctx.get('NWALL',200)
        start = time.perf_counter()
        for _ in range(repetitions): reference = operation()
        torch.cuda.synchronize()
        elapsed = (time.perf_counter()-start)*1000/repetitions
        # Different reduction/exp implementations are deliberately not the Omega
        # interval oracle. Print the reference clock without claiming bit parity.
        assert torch.isfinite(reference).all().item()
        say(f'torch {name} (AMD library, not Omega) warm_wall_ms={elapsed:.6f} dtype=FP32')
    except ImportError:
        say(f'torch {name} (AMD library, not Omega) NOT_RUN unavailable')
    return True
