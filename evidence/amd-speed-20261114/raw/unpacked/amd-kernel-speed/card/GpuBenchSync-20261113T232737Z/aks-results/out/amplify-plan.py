#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Measuring instrument (Python, not part of Omega): per-dispatch-class timing with the unmodified native launcher.

Reads one OMPLAN01 plan (CUDApp backend/amd/kfd/launch/PLAN.md) and writes a variant in which extra copies of one
dispatch class are inserted immediately before the last dispatch of the step (step_advance). The launcher, the code
objects, the weight files, every original dispatch and their order are unchanged, so:

  per-dispatch time of the class = (median decode step of the variant - median decode step of the base) / inserted

and the variant must still print the published token ids (every inserted dispatch only rewrites a buffer that the
next step recomputes before reading it, or rewrites the same value). A variant whose ids differ is discarded.

Usage:
  amplify-plan.py list  BASE.plan                      -> class table (JSON) of the plan's step
  amplify-plan.py write BASE.plan OUT.plan CLASS N_GEN  -> variant; CLASS 'base' only changes N_GEN
Classes are named KERNEL or KERNEL:MxK (matrix-vector shapes), plus the controls
  empty:G   G workgroups of fp8gemv_b128f with K=0 (each wave leaves before reading any weight)
The copies cycle through every dispatch of the class in step order (weights of all layers, so the cache cannot hold
them) for matrix-vector and attention part/merge classes; other classes repeat their last dispatch of the step.
"""
import json, struct, sys, hashlib

GEMV = {'fp8gemv_b128f': (32, 36), 'bf16gemv2': (24, 28)}      # kernarg byte offsets of m and k
CYCLE = {'attn256_part_step', 'attn256_merge_step'}             # cycle all layers (per-layer key/value caches)
# Number of inserted dispatches per class: enough added time to stand clear of run-to-run spread (about 0.05 ms).
SWEEPS = {'fp8gemv_b128f:17408x5120': 1, 'fp8gemv_b128f:5120x17408': 2, 'fp8gemv_b128f:10240x5120': 2,
          'fp8gemv_b128f:6144x5120': 3, 'fp8gemv_b128f:5120x6144': 2, 'fp8gemv_b128f:12288x5120': 6,
          'fp8gemv_b128f:1024x5120': 4, 'bf16gemv2:48x5120': 2, 'bf16gemv2:248320x5120': 6,
          'attn256_part_step': 8, 'attn256_merge_step': 8}
REPEATS = {'rmsnorm5120': 258, 'residual5120': 256, 'mlpsilu': 256, 'gdnparams': 240, 'gdnconv4_step': 240,
           'gdnqknorm': 240, 'gdnstate_step': 240, 'gdnrmsgate': 240, 'attn256_prep_step': 128, 'attn256_gate': 128,
           'argmax_part': 64, 'argmax_final': 128, 'embed5120_step': 128}
EMPTY = {'empty:1': 256, 'empty:17408': 128}


def parse(p):
    if p[:8] != b'OMPLAN01': raise SystemExit('not an OMPLAN01 plan')
    nc, nf, nb, nd, npm, ng, tb = struct.unpack_from('<7Q', p, 8)
    sp, sl, dp, dl = struct.unpack_from('<4Q', p, len(p) - 32)
    q = 64
    names = []
    for i in range(nc):
        o, l = struct.unpack_from('<2Q', p, q + 16 * i); names.append(p[sp + o:sp + o + l].decode())
    q += 16 * nc + 48 * nb + 4 * npm; q += (-q) % 8
    start = q; recs = []
    for i in range(nd):
        ci, gx, gy, gz, bx, kb, nr, pad = struct.unpack_from('<8I', p, q)
        if not (ci < nc and gz == 1 and bx == 64 and pad == 0): raise SystemExit('dispatch %d header' % i)
        end = q + 32 + kb + 16 * nr; end += (-end) % 8
        recs.append(dict(name=names[ci], gx=gx, gy=gy, kb=kb, nr=nr, raw=bytes(p[q:end]))); q = end
    if q != sp or sp + sl != dp or dp + dl != len(p) - 32: raise SystemExit('plan layout')
    return dict(head=bytes(p[:start]), recs=recs, tail=bytes(p[sp:len(p) - 32]), sl=sl, dl=dl, n_gen=ng, n_prompt=npm)


def label(r):
    if r['name'] in GEMV:
        mo, ko = GEMV[r['name']]
        m, k = struct.unpack_from('<I', r['raw'], 32 + mo)[0], struct.unpack_from('<I', r['raw'], 32 + ko)[0]
        return '%s:%dx%d' % (r['name'], m, k)
    return r['name']


def classes(plan):
    out = {}
    for r in plan['recs']:
        c = out.setdefault(label(r), dict(count=0, gx=r['gx'], gy=r['gy'])); c['count'] += 1
    return out


def inserted(plan, cls):
    recs = plan['recs']
    if recs[-1]['name'] != 'step_advance': raise SystemExit('last dispatch is not step_advance')
    if cls == 'base': return []
    if cls in EMPTY:
        g = int(cls.split(':')[1])
        src = next(r for r in recs if label(r) == 'fp8gemv_b128f:17408x5120')
        raw = bytearray(src['raw']); struct.pack_into('<I', raw, 4, g)          # grid x = workgroups
        struct.pack_into('<I', raw, 32 + GEMV['fp8gemv_b128f'][1], 0)            # K = 0
        return [bytes(raw)] * EMPTY[cls]
    mine = [r['raw'] for r in recs if label(r) == cls]
    if not mine: raise SystemExit('class not in plan: ' + cls)
    if cls in SWEEPS: return mine * SWEEPS[cls]
    if cls in REPEATS: return [mine[-1]] * REPEATS[cls]
    raise SystemExit('no amplification rule for ' + cls)


def write(plan, cls, n_gen):
    extra = inserted(plan, cls)
    recs = [r['raw'] for r in plan['recs']]
    body = b''.join(recs[:-1]) + b''.join(extra) + recs[-1]
    head = bytearray(plan['head'])
    struct.pack_into('<Q', head, 8 + 8 * 3, len(recs) + len(extra))
    if n_gen > plan['n_gen']: raise SystemExit('N_GEN above the plan value')
    struct.pack_into('<Q', head, 8 + 8 * 5, n_gen)
    sp = len(head) + len(body)
    out = bytes(head) + body + plan['tail'] + struct.pack('<4Q', sp, plan['sl'], sp + plan['sl'], plan['dl'])
    return out, len(extra)


def main():
    mode = sys.argv[1]
    base = open(sys.argv[2], 'rb').read(); plan = parse(base)
    if mode == 'list':
        print(json.dumps(dict(dispatches=len(plan['recs']), n_prompt=plan['n_prompt'], n_gen=plan['n_gen'],
                              classes=classes(plan), rules=sorted(list(SWEEPS) + list(REPEATS) + list(EMPTY))), indent=1))
        missing = [c for c in classes(plan) if c not in SWEEPS and c not in REPEATS and c != 'step_advance']
        if missing: raise SystemExit('classes without a rule: %s' % missing)
        return
    out, cls, n_gen = sys.argv[3], sys.argv[4], int(sys.argv[5])
    data, n = write(plan, cls, n_gen)
    check = parse(data)                                       # the variant must parse with the same reader
    assert len(check['recs']) == len(plan['recs']) + n and check['n_gen'] == n_gen
    assert [r['raw'] for r in check['recs'][:len(plan['recs']) - 1]] == [r['raw'] for r in plan['recs'][:-1]]
    assert check['recs'][-1]['raw'] == plan['recs'][-1]['raw'] and check['tail'] == plan['tail']
    open(out, 'wb').write(data)
    print('VARIANT', json.dumps(dict(cls=cls, inserted=n, dispatches=len(check['recs']), n_gen=n_gen, bytes=len(data),
                                     sha256=hashlib.sha256(data).hexdigest(), base_sha256=hashlib.sha256(base).hexdigest())))


if __name__ == '__main__': main()
