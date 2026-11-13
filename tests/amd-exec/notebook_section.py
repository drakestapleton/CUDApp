# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test-only notebook instrument. token.py exports this with generated fixtures.
No production dependency; the numerical intervals come from the host oracle.
"""
def run_section(ctx):
    import base64
    import ctypes as C
    import math
    import struct
    import time
    from fractions import Fraction

    p = _make_payload()
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
        for off in case.get('bf16_outputs',[]):
            assert struct.unpack_from('<I',actual,off)[0]&0xffff==0, f'{name} BF16 rounding mismatch offset={off}'
        for off, lo, hi in case['intervals']:
            bits = struct.unpack_from('<I', actual, off)[0]
            x = struct.unpack('<f', struct.pack('<I', bits))[0]
            lo, hi = Fraction(lo), Fraction(hi)
            assert math.isfinite(x) and lo <= Fraction(x) <= hi, f'{name} interval mismatch offset={off} bits={bits:08x}'
            widest = max(widest, hi-lo)
            unchanged[off:off+4] = before[off:off+4]
        for off,lo,hi in case.get('half_intervals',[]):
            bits=struct.unpack_from('<H',actual,off)[0]<<16
            x=struct.unpack('<f',struct.pack('<I',bits))[0]
            lo,hi=Fraction(lo),Fraction(hi)
            assert math.isfinite(x) and lo<=Fraction(x)<=hi, f'{name} BF16 interval mismatch offset={off}'
            widest=max(widest,hi-lo)
            unchanged[off:off+2]=before[off:off+2]
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
        elif name == 'gdnconv4':
            n = 16*128*2+48*128
            x = tensor(case['args'][0][1],n).view(1,n,1)
            def bfloat(off):
                values=struct.unpack_from('<'+str(n*4)+'H',image,off)
                words=struct.pack('<'+str(n*4)+'I',*(v<<16 for v in values))
                return torch.tensor(struct.unpack('<'+str(n*4)+'f',words),device='cuda',dtype=torch.bfloat16)
            weight=bfloat(case['args'][1][1]).view(n,1,4)
            old=bfloat(case['args'][2][1]).view(1,n,4)
            def operation():
                state=torch.cat((old,x),dim=-1).to(weight.dtype)
                new=state[:,:,-4:].clone()
                out=F.silu(F.conv1d(state,weight,groups=n)[:,:,-1:]).float()
                return out,new
        elif name == 'gdnqknorm':
            x=tensor(case['args'][0][1],4096).view(32,128).bfloat16()
            def operation():
                value=(x*torch.rsqrt((x*x).sum(dim=-1,keepdim=True)+1e-6)).float()
                return torch.cat((value[:16]*(128**-0.5),value[16:]))
        elif name == 'gdnstate':
            qk=tensor(case['args'][0][1],4096).view(2,16,128).repeat_interleave(3,dim=1)
            values=tensor(case['args'][1][1],48*128).view(48,128)
            old=tensor(case['args'][2][1],48*128*128).view(48,128,128)
            decay=tensor(case['args'][5][1],48).view(48,1,1)
            beta=tensor(case['args'][6][1],48).view(48,1)
            def operation():
                state=old*decay
                memory=(state*qk[1,:,:,None]).sum(dim=1)
                delta=(values-memory)*beta
                state=state+qk[1,:,:,None]*delta[:,None,:]
                return (state*qk[0,:,:,None]).sum(dim=1).bfloat16().float(),state
        elif name == 'residual5120':
            a,b=(tensor(case['args'][j][1],5120).bfloat16() for j in (0,1))
            operation=lambda: (a+b).float()
        elif name == 'embed5120':
            words=struct.unpack_from('<5120H',image,case['args'][0][1])
            row=torch.tensor(words,device='cuda',dtype=torch.uint16).view(torch.bfloat16)
            operation=lambda: row.float()
        elif name == 'gdnparams':
            a,b,A,dt=(tensor(case['args'][j][1],48) for j in range(4))
            a,b=a.bfloat16(),b.bfloat16()
            operation=lambda: ((-A.exp()*F.softplus(a.float()+dt)).exp(),b.sigmoid().float())
        elif name == 'gdnrmsgate':
            x=tensor(case['args'][0][1],6144).view(48,128).bfloat16()
            w=tensor(case['args'][1][1],128).bfloat16()
            z=tensor(case['args'][2][1],6144).view(48,128).bfloat16().float()
            def operation():
                norm=(x.float()*torch.rsqrt(x.float().square().mean(-1,keepdim=True)+1e-6)).bfloat16()
                return ((norm*w).float()*F.silu(z)).bfloat16().float()
        elif name == 'argmax_part':
            logits=tensor(case['args'][0][1],248320).view(-1,64)
            operation=lambda: logits.argmax(dim=-1)
        elif name == 'argmax_final':
            pairs=struct.unpack_from('<7760I',image,case['args'][0][1])
            keys=torch.tensor(pairs[::2],device='cuda',dtype=torch.int64)
            indices=torch.tensor(pairs[1::2],device='cuda',dtype=torch.int64)
            operation=lambda: torch.where(keys==keys.max(),indices,248320).min()
        elif name == 'attn256_prep':
            q=tensor(case['args'][0][1],12288).view(24,512)[:,:256]
            k=tensor(case['args'][1][1],1024).view(4,256)
            v=tensor(case['args'][2][1],1024).view(4,256)
            qw,kw=(tensor(case['args'][j][1],256) for j in (3,4))
            cs=tensor(case['args'][5][1],64)
            def rotate(x,w):
                x=x*torch.rsqrt(x.square().mean(-1,keepdim=True)+1e-6)*(1+w)
                a,b=x[:,:32].clone(),x[:,32:64].clone()
                x[:,:32]=a*cs[:32]-b*cs[32:]
                x[:,32:64]=b*cs[:32]+a*cs[32:]
                return x
            operation=lambda: (rotate(q,qw),rotate(k,kw).bfloat16(),v.bfloat16())
        elif name == 'attn256_gate':
            gate=tensor(case['args'][0][1],12288).view(24,512)[:,256:]
            attention=tensor(case['args'][1][1],6144).view(24,256)
            operation=lambda: attention*gate.sigmoid()
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
        for index,value in enumerate(reference if isinstance(reference,tuple) else (reference,)):
            if name == 'attn256_prep':
                if index==2:
                    assert torch.equal(torch.isnan(value),torch.isnan(v))
                    assert torch.equal(torch.isinf(value),torch.isinf(v))
                    continue
            assert torch.isfinite(value).all().item()
        dtype = 'BF16-conv/cache FP32-accum/output' if name == 'gdnconv4' else 'FP32'
        say(f'torch {name} (AMD library, not Omega) warm_wall_ms={elapsed:.6f} dtype={dtype}')
    except ImportError:
        say(f'torch {name} (AMD library, not Omega) NOT_RUN unavailable')
    return True
