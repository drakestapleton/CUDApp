#!/usr/bin/env python3
"""spec-decode card measurement (instrument; runs on the MI300X notebook, Python + numpy only).
Per-dispatch time of the one-row GEMVs fp8gemv_b128f / bf16gemv2 (CUDApp dc536882 code, objects written by hsaco.om)
against their R-row variants _r2, _r3, _r8, through the unmodified native launcher's generic `run` command
(launch-x86 sha256 08a50491...). Matrices exceed the 256 MB last-level cache so each repeat reads from HBM.
Per dispatch = (time at repeat N - time at repeat 1) / (N - 1), median of TRIALS. Correctness on hardware: every row
of each R-row output must be bit-identical to the one-row kernel run on that row alone.
Writes rho-results.json after every case (verdict PARTIAL until the end) and prints RHO lines. Every launcher call has
its own timeout (SPEC_RHO_TIMEOUT seconds, default 60) after which its whole process group is killed; a timed-out case
is recorded as TIMEOUT and the run continues with the next kernel.
Usage: spec_rho.py DIR (DIR holds launch-x86 and the .hsaco files)"""
import json, os, re, signal, struct, subprocess, sys, time
import numpy as np
D = sys.argv[1]; L = os.path.join(D, 'launch-x86')
DRY = os.environ.get('SPEC_RHO_DRY') == '1'   # CPU check without a device: tiny sizes, launcher must exit 77
W = os.path.join(D, 'work'); os.makedirs(W, exist_ok=True)
N, TRIALS = 20, 5
TMO = float(os.environ.get('SPEC_RHO_TIMEOUT', '60'))
rng = np.random.default_rng(20261114)
class Timeout(Exception): pass
def manifest(kernarg_len, out_bytes, repeat, relocs, kernarg):
    h = b'OMKFD001' + struct.pack('<5Q', kernarg_len, out_bytes, repeat, 0, len(relocs))
    return h + b''.join(struct.pack('<2Q', o, i) for o, i in relocs) + kernarg
def call(cmd, tmo=None):
    """Run cmd in its own session; on timeout kill the whole process group and raise Timeout."""
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, start_new_session=True)
    try: so, se = p.communicate(timeout=tmo or TMO)
    except subprocess.TimeoutExpired:
        os.killpg(p.pid, signal.SIGKILL); p.communicate()
        print('TIMEOUT', ' '.join(cmd[:4])[-120:], f'after {tmo or TMO:.0f}s: process group killed', flush=True)
        raise Timeout(cmd)
    return p.returncode, so, se
def run(obj, kern, gx, args, inputs, out):
    cmd = [L, 'run', obj, kern, str(gx), '1', args, *inputs, out]
    if DRY: cmd = ['qemu-x86_64'] + cmd
    rc, so, se = call(cmd)
    if DRY:
        ok = rc == 77 and 'OMEGA device=absent' in so     # manifest and ELF checks precede /dev/kfd
        if not ok: raise SystemExit(f'DRY launcher exit {rc}: {so[-600:]} {se[-300:]}')
        raise StopIteration
    m = re.search(r'OMEGA dispatch_ns=(\d+) packets=(\d+)', so)
    if rc != 0 or not m: raise SystemExit(f'launcher exit {rc}: {so[-800:]} {se[-800:]}')
    return int(m.group(1)), int(m.group(2))
res = dict(schema='spec-decode-card-rho-v2', N=N, trials=TRIALS, timeout_s=TMO, kernels={}, timeouts=[], verdict='PARTIAL')
def save():
    tmp = os.path.join(D, 'rho-results.json.tmp')
    with open(tmp, 'w') as f: json.dump(res, f, indent=1)
    os.replace(tmp, os.path.join(D, 'rho-results.json'))
cases = [('fp8gemv_b128f', 81920, 5120), ('bf16gemv2', 248320, 5120)]
if DRY:
    cases = [('fp8gemv_b128f', 256, 1024), ('bf16gemv2', 256, 1024)]
    for kern, M, K in cases:
        for R in (1, 2, 3, 8):
            name = f'{kern}_r{R}'; obj = os.path.join(D, name + '.hsaco')
            karg = bytes(32 if kern == 'fp8gemv_b128f' else 24) + struct.pack('<II', M, K)
            rel = [(0, 0), (8, 1), (16, 2), (24, 3)] if kern == 'fp8gemv_b128f' else [(0, 0), (8, 1), (16, 2)]
            ins = []
            for i, n in enumerate([M * K, R * K * 4] + ([((M + 127) // 128) * ((K + 127) // 128) * 2] if kern == 'fp8gemv_b128f' else [])):
                f = os.path.join(W, f'dry{i}'); open(f, 'wb').write(bytes(n)); ins.append(f)
            if kern == 'bf16gemv2': open(ins[0], 'wb').write(bytes(2 * M * K))
            a = os.path.join(W, 'adry'); open(a, 'wb').write(manifest(len(karg), R * M * 4, 3, rel, karg))
            try: run(obj, name, M, a, ins, os.path.join(W, 'ydry'))
            except StopIteration: print('DRY OK', name, flush=True)
            bad = bytearray(open(a, 'rb').read()); bad[0] ^= 1; open(a, 'wb').write(bad)       # control: bad magic
            rc, _, _ = call(['qemu-x86_64', L, 'run', obj, name, str(M), '1', a, *ins, os.path.join(W, 'ydry')])
            assert rc == 64, ('bad-magic control not refused', rc)
            print('DRY CONTROL bad-magic refused 64', name, flush=True)
    t = time.time()                                  # control: a hung child (and its own child) is killed on time
    try: call(['sh', '-c', 'sleep 30 & sleep 30'], 2); raise SystemExit('DRY timeout control did not fire')
    except Timeout: pass
    assert time.time() - t < 10, 'timeout control took too long'
    print('DRY CONTROL timeout killed process group', flush=True)
    sys.exit(0)
for kern, M, K in cases:
    t0 = time.time()
    if kern == 'fp8gemv_b128f':
        w = rng.integers(0, 256, M * K, dtype=np.uint8); w[(w & 0x7f) == 0x7f] ^= 1             # no E4M3 NaN
        sc = (0x3f00 + rng.integers(0, 0x100, ((M + 127) // 128) * ((K + 127) // 128))).astype(np.uint16)
        files = {'W': w.tobytes(), 'S': sc.tobytes()}; karg_len = 40
    else:
        w = (0x3a00 + rng.integers(0, 0x700, M * K)).astype(np.uint16) | (rng.integers(0, 2, M * K).astype(np.uint16) << 15)
        files = {'W': w.tobytes()}; karg_len = 32
    for n, b in files.items(): open(os.path.join(W, f'{kern}.{n}'), 'wb').write(b)
    del w, files
    X = rng.uniform(-2, 2, 8 * K).astype(np.float32)
    rows = {}
    try:
        for R in (1, 2, 3, 8):
            obj = os.path.join(D, f'{kern}_r{R}.hsaco'); name = f'{kern}_r{R}'
            xf = os.path.join(W, f'x{R}'); open(xf, 'wb').write(X[:R * K].tobytes())
            if kern == 'fp8gemv_b128f':
                karg = bytes(32) + struct.pack('<II', M, K); rel = [(0, 0), (8, 1), (16, 2), (24, 3)]
                inputs = [os.path.join(W, f'{kern}.W'), xf, os.path.join(W, f'{kern}.S')]
            else:
                karg = bytes(24) + struct.pack('<II', M, K); rel = [(0, 0), (8, 1), (16, 2)]
                inputs = [os.path.join(W, f'{kern}.W'), xf]
            out = os.path.join(W, f'y{R}')
            a1 = os.path.join(W, 'a1'); open(a1, 'wb').write(manifest(karg_len, R * M * 4, 1, rel, karg))
            aN = os.path.join(W, 'aN'); open(aN, 'wb').write(manifest(karg_len, R * M * 4, N, rel, karg))
            per = []
            for t in range(TRIALS):
                t1, _ = run(obj, name, M, a1, inputs, out); tn, pk = run(obj, name, M, aN, inputs, out)
                per.append((tn - t1) / (N - 1))
            y = np.fromfile(out, dtype=np.uint32).reshape(R, M)
            ident = None
            if R == 1:
                rows[0] = y[0].copy()
                for r in range(1, 8):                       # one-row reference for every activation row
                    open(xf, 'wb').write(X[r * K:(r + 1) * K].tobytes()); run(obj, name, M, a1, inputs, out)
                    rows[r] = np.fromfile(out, dtype=np.uint32).copy()
            else:
                ident = all(np.array_equal(y[r], rows[r]) for r in range(R))
            finite = bool(np.all(((y >> 23) & 0xff) != 0xff))
            med = float(np.median(per))
            res['kernels'][name] = dict(M=M, K=K, R=R, per_dispatch_ns=per, median_ns=med, rows_bit_identical_to_one_row=ident,
                                        finite=finite, weight_bytes=M * K * (1 if kern == 'fp8gemv_b128f' else 2))
            if R > 1:
                one = res['kernels'][f'{kern}_r1']['median_ns']; res['kernels'][name]['rho'] = (med / one - 1) / (R - 1)
            save()
            print('RHO', name, f'M={M} K={K} median_us={med / 1000:.3f}', 'identical' if ident in (None, True) else 'DIFFERENT',
                  'finite' if finite else 'NONFINITE', f"rho={res['kernels'][name].get('rho', 0):.4f}" if R > 1 else '', flush=True)
    except Timeout:
        res['timeouts'].append(dict(kernel=kern, R=R, timeout_s=TMO)); save()
    print('TIME', kern, f'{time.time() - t0:.0f}s', flush=True)
ok = not res['timeouts'] and all(v['rows_bit_identical_to_one_row'] in (None, True) and v['finite'] for v in res['kernels'].values())
res['verdict'] = 'PASS' if ok else 'FAIL'
save()
print('SPEC-RHO', res['verdict'], 'hardware=MI300X', flush=True)
