#!/usr/bin/env python3
"""Byte-compare the C oracle CLI and the Omega CLI on a generated OEV corpus.

Usage: corpus_diff.py C_BIN OM_BIN [N]
Prints PASS n=... or the first mismatch and exits 1.
Deterministic: seed 20261106.
"""
import os
import random
import subprocess
import sys
import tempfile

C_BIN, OM_BIN = sys.argv[1], sys.argv[2]
N = int(sys.argv[3]) if len(sys.argv) > 3 else 1200
ASOF = "2026-10-20"


def header(slots=2, cap=1000000, bw=1e9):
    return [
        "oev version=1",
        "machine id=t soc=t cores=4 mem_bytes=1000 prov=specified src=test",
        "build repo=omega-mv sha=0000000000000000000000000000000000000000",
        "calibration id=c taken=2026-10-19 valid_days=3650 load1=0 src=synthetic",
        f"resource id=cpu kind=compute slots={slots} prov=specified src=test",
        f"resource id=mem kind=capacity cap={cap} prov=specified src=test",
        f"resource id=bus kind=bandwidth cap={bw} lo={bw} hi={bw} prov=specified src=test",
        "resource id=lch kind=launch prov=specified src=test",
        "energy instrument=none",
    ]


def emit(lines, path):
    with open(path, "w") as f:
        f.write("\n".join(lines) + "\n")


def case_chain(rng, nops):
    lines = header(slots=rng.choice([1, 2, 4]))
    for i in range(nops):
        t = rng.randint(1, 5000)
        lines.append(f"op id=a{i} kind=compute engine=cpu t_ns={t} prov=synthetic src=t")
    for i in range(nops - 1):
        if rng.random() < 0.7:
            lines.append(f"dep a{i} a{i+1}")
    return lines


def case_bw(rng, nops):
    lines = header(slots=4, bw=rng.choice([1e6, 1e8, 1e9]))
    for i in range(nops):
        t = rng.randint(10, 2000)
        b = rng.choice([0, 1000, 100000, 1000000])
        extra = f" bytes={b} bw=bus" if b else ""
        lines.append(f"op id=a{i} kind=copy engine=cpu t_ns={t}{extra} prov=synthetic src=t")
    return lines


def case_buf(rng, nops, invert=False, over=False):
    cap = 1000 if over else 1000000
    lines = header(slots=2, cap=cap)
    for i in range(nops):
        lines.append(f"op id=a{i} kind=compute engine=cpu t_ns={rng.randint(1, 100)} prov=synthetic src=t")
    for i in range(nops - 1):
        lines.append(f"dep a{i} a{i+1}")
    nb = rng.randint(1, min(4, nops))
    for b in range(nb):
        first = rng.randint(0, nops - 1)
        last = rng.randint(0, nops - 1)
        if invert:
            first, last = nops - 1, 0
        nbytes = 2000 if over else rng.choice([0, 10, 80, 500])
        lines.append(f"buffer id=b{b} bytes={nbytes} cap=mem first=a{first} last=a{last}")
    return lines


def case_cycle(rng, nops):
    lines = case_chain(rng, max(nops, 3))
    lines.append(f"dep a{nops-1} a0")
    return lines


def case_launch(rng, nops):
    lines = header(slots=1)
    for i in range(nops):
        lines.append(
            f"op id=a{i} kind=compute engine=cpu t_ns={rng.randint(1, 100)} "
            f"launch_ns={rng.randint(0, 50)} launcher=lch prov=synthetic src=t"
        )
    return lines


def case_barrier(rng, nops):
    lines = header(slots=rng.choice([1, 3]))
    for i in range(nops):
        lines.append(f"op id=a{i} kind=compute engine=cpu t_ns={rng.randint(1, 200)} prov=synthetic src=t")
    if nops >= 2:
        ids = ",".join(f"a{i}" for i in range(nops))
        lines.append(f"sync id=bar ops={ids}")
        lines.append("op id=tail kind=compute engine=cpu t_ns=5 prov=synthetic src=t")
        lines.append("dep bar tail")
    return lines


BUILDERS = [case_chain, case_bw, case_buf, case_cycle, case_launch, case_barrier]


def run(bin, path):
    p = subprocess.run([bin, "eval", path, "--asof", ASOF], capture_output=True)
    return p.returncode, p.stdout, p.stderr


def main():
    rng = random.Random(20261106)
    tmp = tempfile.mkdtemp(prefix="dagsim-corpus-")
    bad = 0
    kinds = {b.__name__: 0 for b in BUILDERS}
    for i in range(N):
        b = BUILDERS[i % len(BUILDERS)]
        nops = 1 + (i % 12)
        if b is case_buf and i % 18 == 2:
            lines = case_buf(rng, nops, invert=True)
        elif b is case_buf and i % 18 == 8:
            lines = case_buf(rng, nops, over=True)
        else:
            lines = b(rng, nops)
        path = os.path.join(tmp, f"c{i}.oev")
        emit(lines, path)
        rc1, o1, e1 = run(C_BIN, path)
        rc2, o2, e2 = run(OM_BIN, path)
        kinds[b.__name__] += 1
        if rc1 != rc2 or o1 != o2 or e1 != e2:
            bad += 1
            if bad == 1:
                sys.stderr.write(f"DIFF case {i} {b.__name__} rc {rc1} vs {rc2}\n")
                sys.stderr.write(o1.decode()[:400] + "\n---\n" + o2.decode()[:400] + "\n")
            if bad >= 5:
                break
    print(f"{'FAIL' if bad else 'PASS'} n={N} bad={bad} dir={tmp} kinds={kinds}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
