#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Card run for the Q4D8 kernels on the MI300X notebook (outside instrument; Omega kernels through the Omega
KFD launcher, no HIP in the measured path).

usage: python3 card_q4.py [--skip-download] [--only CLASS,...]   (cwd = the unpacked bundle directory)

Phases, each logged as one line in card_q4.log and kept resumable through card_q4.state.json:
  0 env      uname, rocminfo marketing name and gfx target, python/numpy, bundle sha256 check
  1 weights  download the source shards named in expected.json from Hugging Face at the pinned revision into
             $Q4_WEIGHTS (default /tmp/qwen3.8-27b-fp8); sha256 must equal the pin
  2 convert  q4convert.py --only those shards; every converted shard's sha256 must equal the Spark's
  3 kernels  for each tensor class: the 4-bit kernel and the FP8 (or BF16) kernel on the same real tensor,
             repeat 1 and repeat R; output bits of the 4-bit kernel against oracle_vec (exact), FP8 output
             against a float64 reference (relative error reported)
  4 final    results.json, SHA256SUMS, q4card.tar.gz
"""
import hashlib
import json
import os
import pathlib
import platform
import re
import struct
import subprocess
import sys
import time
import urllib.request

import numpy as np

HERE = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import q4format as F  # noqa: E402
import oracle_vec as OV  # noqa: E402
from q4convert import read_header, tensor_view  # noqa: E402

EXP = json.loads((HERE / "expected.json").read_text())
REV = EXP["source_rev"]
WD = pathlib.Path(os.environ.get("Q4_WEIGHTS", "/tmp/qwen3.8-27b-fp8"))
CD = pathlib.Path(os.environ.get("Q4_CONTAINER", "/tmp/qwen3.8-27b-q4d8"))
OUT = HERE / "out"
LOG = HERE / "card_q4.log"
STATE = HERE / "card_q4.state.json"
REPEAT = int(os.environ.get("Q4_REPEAT", "50"))
PFX = "model.language_model."
# (class, tensor stem, shard, kind)  kind: fp8 (baseline fp8gemv_b128f) or bf16 (baseline bf16gemv2)
CLASSES = [
    ("gdn in_proj_qkv 10240x5120 seg4", PFX + "layers.0.linear_attn.in_proj_qkv", "layers-0.safetensors", "fp8"),
    ("gdn in_proj_z 6144x5120 seg4", PFX + "layers.0.linear_attn.in_proj_z", "layers-0.safetensors", "fp8"),
    ("gdn out_proj 5120x6144", PFX + "layers.0.linear_attn.out_proj", "layers-0.safetensors", "fp8"),
    ("mlp gate_proj 17408x5120", PFX + "layers.0.mlp.gate_proj", "layers-0.safetensors", "fp8"),
    ("mlp up_proj 17408x5120", PFX + "layers.0.mlp.up_proj", "layers-0.safetensors", "fp8"),
    ("mlp down_proj 5120x17408", PFX + "layers.0.mlp.down_proj", "layers-0.safetensors", "fp8"),
    ("attn q_proj 12288x5120", PFX + "layers.3.self_attn.q_proj", "layers-3.safetensors", "fp8"),
    ("attn k_proj 1024x5120", PFX + "layers.3.self_attn.k_proj", "layers-3.safetensors", "fp8"),
    ("attn o_proj 5120x6144", PFX + "layers.3.self_attn.o_proj", "layers-3.safetensors", "fp8"),
    ("lm_head 248320x5120", "lm_head", "outside.safetensors", "bf16"),
]


def log(*a):
    line = time.strftime("%Y-%m-%dT%H:%M:%SZ ", time.gmtime()) + " ".join(str(x) for x in a)
    print(line, flush=True)
    with open(LOG, "a") as f:
        f.write(line + "\n")


def sha_file(p):
    h = hashlib.sha256()
    with open(p, "rb") as f:
        for b in iter(lambda: f.read(1 << 24), b""):
            h.update(b)
    return h.hexdigest()


def state():
    return json.loads(STATE.read_text()) if STATE.exists() else {"done": [], "results": {}}


def save(st):
    tmp = STATE.with_suffix(".partial")
    tmp.write_text(json.dumps(st, indent=1))
    os.replace(tmp, STATE)


def phase_env(st):
    info = {"uname": platform.uname()._asdict(), "python": sys.version, "numpy": np.__version__}
    try:
        r = subprocess.run(["rocminfo"], capture_output=True, text=True, timeout=60).stdout
        info["gfx"] = sorted(set(re.findall(r"gfx9\d\d", r)))
        info["marketing"] = sorted(set(m.strip() for m in re.findall(r"Marketing Name:\s*(.*)", r)))
        info["compute_units"] = sorted(set(re.findall(r"Compute Unit:\s*(\d+)", r)))
    except Exception as e:  # noqa: BLE001
        info["rocminfo"] = "unavailable: %r" % e
    bad = [n for n, h in EXP["bundle"].items() if sha_file(HERE / n) != h]
    info["bundle_sha256_bad"] = bad
    log("ENV", json.dumps(info))
    if bad:
        raise SystemExit("bundle sha256 mismatch: %s" % bad)
    st["results"]["env"] = info


def phase_weights(st):
    WD.mkdir(parents=True, exist_ok=True)
    need = sorted(set(EXP["source_shards"]) | {"config.json", "model.safetensors.index.json"})
    for n in need:
        p = WD / n
        want = EXP["source_sha256"].get(n)
        if p.exists() and (want is None or sha_file(p) == want):
            log("WEIGHTS have", n)
            continue
        url = "https://huggingface.co/Qwen/Qwen3.8-27B-FP8/resolve/%s/%s" % (REV, n)
        tmp = p.with_suffix(p.suffix + ".partial")
        t0 = time.time()
        with urllib.request.urlopen(url) as r, open(tmp, "wb") as f:
            while True:
                b = r.read(1 << 23)
                if not b:
                    break
                f.write(b)
        if want is not None and sha_file(tmp) != want:
            raise SystemExit("download sha256 mismatch " + n)
        os.replace(tmp, p)
        log("WEIGHTS downloaded", n, "bytes=%d s=%.0f" % (p.stat().st_size, time.time() - t0))
    log("WEIGHTS sha256 verified %d files at %s" % (len(need), REV))


def phase_convert(st):
    shards = sorted(set(EXP["source_shards"]))
    t0 = time.time()
    r = subprocess.run([sys.executable, "-I", str(HERE / "q4convert.py"), str(WD), str(CD), "--workers",
                        str(min(16, os.cpu_count() or 1)), "--only", ",".join(shards)], capture_output=True, text=True)
    (OUT / "convert.log").write_text(r.stdout + r.stderr)
    if r.returncode:
        raise SystemExit("convert failed rc=%d" % r.returncode)
    rows = {}
    for s in shards:
        got = sha_file(CD / s)
        rows[s] = {"card": got, "spark": EXP["container_sha256"][s], "equal": got == EXP["container_sha256"][s]}
    log("CONVERT seconds=%.0f" % (time.time() - t0), json.dumps(rows))
    st["results"]["convert"] = rows
    if not all(v["equal"] for v in rows.values()):
        raise SystemExit("container sha256 differs from the Spark's")


def manifest(raw, outbytes, repeats, relocs, timeout_ns=0):
    hdr = b"OMKFD001" + struct.pack("<QQQQQ", len(raw), outbytes, repeats, timeout_ns, len(relocs))
    for off, idx in relocs:
        hdr += struct.pack("<QQ", off, idx)
    return hdr + raw


def launch(code, kernel, gx, args_path, inputs, out_path):
    cmd = [os.environ.get("Q4_LAUNCH", str(HERE / "launch-x86")), "run", str(code), kernel, str(gx), "1", str(args_path)] + \
          [str(p) for p in inputs] + [str(out_path)]
    t0 = time.time()
    r = subprocess.run(cmd, capture_output=True, timeout=900)
    m = re.search(rb"OMEGA dispatch_ns=(\d+) packets=(\d+)", r.stdout)
    return r.returncode, (int(m.group(1)) if m else None), r.stdout, r.stderr, time.time() - t0


def timed(code, kernel, gx, raw, outbytes, relocs, inputs, tag):
    res = {}
    for rep in (1, REPEAT):
        ap = OUT / ("%s.r%d.args" % (tag, rep))
        ap.write_bytes(manifest(raw, outbytes, rep, relocs))
        op = OUT / ("%s.r%d.y" % (tag, rep))
        ns = []
        for k in range(3):
            rc, d, so, se, wall = launch(code, kernel, gx, ap, inputs, op)
            (OUT / ("%s.r%d.%d.stdout" % (tag, rep, k))).write_bytes(so)
            (OUT / ("%s.r%d.%d.stderr" % (tag, rep, k))).write_bytes(se)
            if rc != 0 or d is None:
                return {"error": "rc=%d" % rc, "stderr": se[-300:].decode("utf-8", "replace")}
            ns.append(d)
        res["r%d_ns" % rep] = ns
    m1 = float(np.median(res["r1_ns"]))
    mR = float(np.median(res["r%d_ns" % REPEAT]))
    res["per_dispatch_ms"] = (mR - m1) / (REPEAT - 1) / 1e6
    res["repeat"] = REPEAT
    return res


FLUSH_BYTES = int(os.environ.get("Q4_TILE_BYTES", str(600 * 1000 * 1000)))


def phase_kernels(st, only):
    """Each tensor is tiled C times along rows so that one dispatch streams at least FLUSH_BYTES of 4-bit weights
    (more than the 256 MB Infinity Cache), so repeat timings are not cache-resident. The FP8 baseline is tiled
    by the same C. Outputs of the tiled rows must equal the oracle bit for bit."""
    OUT.mkdir(exist_ok=True)
    import mmap
    import zlib
    for cls, stem, shard, kind in CLASSES:
        if only and not any(o in cls for o in only):
            continue
        if cls in st["results"].get("kernels", {}):
            continue
        rng = np.random.default_rng([20261113, zlib.crc32(cls.encode())])
        hq, mq, bq = read_header(CD / shard)
        hs, _, bs = read_header(WD / shard)
        if mq.get("q4_format") != F.FORMAT:
            raise SystemExit("container shard %s is not %s" % (shard, F.FORMAT))
        seg = json.loads(mq["q4_seg"])[stem + ".weight_q4"]
        with open(CD / shard, "rb") as fq, open(WD / shard, "rb") as fs:
            mmq = mmap.mmap(fq.fileno(), 0, access=mmap.ACCESS_READ)
            mms = mmap.mmap(fs.fileno(), 0, access=mmap.ACCESS_READ)
            nib = np.ascontiguousarray(tensor_view(mmq, bq, hq[stem + ".weight_q4"]))
            sb = np.ascontiguousarray(tensor_view(mmq, bq, hq[stem + ".weight_q4_scale"]))
            M, half = nib.shape
            T = half // 256
            nseg = sb.shape[1] // 64
            K = hs[stem + ".weight"]["shape"][1]
            C = max(1, -(-FLUSH_BYTES // (nib.nbytes + sb.nbytes)))
            x = (rng.standard_normal(K) * 0.5).astype(np.float32)
            xq, xs = F.quantize_x(x)
            xbuf = F.pack_x(xq, xs)
            tag = stem.split(".")[-1] + "-" + shard.split(".")[0]
            wq, xqp, sq = OUT / (tag + ".q4w"), OUT / (tag + ".q4x"), OUT / (tag + ".q4s")
            with open(wq, "wb") as f:
                for _ in range(C):
                    f.write(nib.tobytes())
            xqp.write_bytes(xbuf)
            with open(sq, "wb") as f:
                for _ in range(C):
                    f.write(sb.astype("<u2").tobytes())
            MB = M * C
            raw = bytes(32) + struct.pack("<IIII", MB, T, seg, nseg * 128)
            exp = np.tile(OV.expected_bits(nib, sb, xbuf, seg), C)
            qbytes = int(C * (nib.nbytes + sb.nbytes))
            res = {}
            for name, code, kern, gx in (("q4", "q4gemv_d8.hsaco", "q4gemv_d8", MB),
                                          ("q4r2", "q4gemv_d8r2.hsaco", "q4gemv_d8r2", MB // 2)):
                if name == "q4r2" and MB % 2:
                    res[name] = {"error": "odd M"}
                    continue
                r = timed(HERE / code, kern, gx, raw, 4 * MB, [(0, 0), (8, 1), (16, 2), (24, 3)],
                          [wq, xqp, sq], tag + "." + name)
                if "error" not in r:
                    y = np.frombuffer((OUT / (tag + ".%s.r1.y" % name)).read_bytes(), dtype="<u4")
                    yR = np.frombuffer((OUT / (tag + ".%s.r%d.y" % (name, REPEAT))).read_bytes(), dtype="<u4")
                    r["output_bits_equal_oracle"] = int((y == exp).sum())
                    r["rows"] = int(MB)
                    r["repeat_output_equal"] = bool((yR == y).all())
                    r["verdict"] = "PASS" if r["output_bits_equal_oracle"] == MB and r["repeat_output_equal"] else "FAIL"
                    r["weight_bytes_per_dispatch"] = qbytes
                    r["per_tensor_ms"] = r["per_dispatch_ms"] / C
                    r["weight_GBps"] = qbytes / (r["per_dispatch_ms"] * 1e-3) / 1e9
                res[name] = r
            # baseline kernel on the same tensor in its source format, tiled by the same C
            if kind == "fp8":
                w8 = np.ascontiguousarray(tensor_view(mms, bs, hs[stem + ".weight"]))
                s8 = np.ascontiguousarray(tensor_view(mms, bs, hs[stem + ".weight_scale_inv"]))
                if M % 128:
                    raise SystemExit("FP8 tiling needs M % 128 == 0")
                bw, bx, bsx = OUT / (tag + ".f8w"), OUT / (tag + ".f8x"), OUT / (tag + ".f8s")
                with open(bw, "wb") as f:
                    for _ in range(C):
                        f.write(w8.tobytes())
                bx.write_bytes(x.tobytes())
                with open(bsx, "wb") as f:
                    for _ in range(C):
                        f.write(s8.astype("<u2").tobytes())
                rawb = bytes(32) + struct.pack("<II", MB, K)
                rb = timed(HERE / "fp8gemv_b128f.hsaco", "fp8gemv_b128f", MB, rawb, 4 * MB,
                           [(0, 0), (8, 1), (16, 2), (24, 3)], [bw, bx, bsx], tag + ".f8")
                wbytes = int(C * (w8.nbytes + s8.nbytes))
                real = F.e4m3_table()[w8].astype(np.float64)
                sc = F.bf16_value(s8).astype(np.float64)
                real *= sc[np.arange(M)[:, None] // 128, np.arange(K)[None, :] // 128]
                ref = real @ x.astype(np.float64)
                bsuf = ".f8"
            else:
                wb = np.ascontiguousarray(tensor_view(mms, bs, hs[stem + ".weight"]))
                bw, bx = OUT / (tag + ".bfw"), OUT / (tag + ".bfx")
                with open(bw, "wb") as f:
                    for _ in range(C):
                        f.write(wb.tobytes())
                bx.write_bytes(x.tobytes())
                rawb = bytes(24) + struct.pack("<II", MB, K)
                rb = timed(HERE / "bf16gemv2.hsaco", "bf16gemv2", MB, rawb, 4 * MB, [(0, 0), (8, 1), (16, 2)],
                           [bw, bx], tag + ".bf")
                wbytes = int(C * wb.nbytes)
                ref = np.concatenate([F.bf16_value(wb[r:r + 8192]).astype(np.float64) @ x.astype(np.float64)
                                      for r in range(0, M, 8192)])
                bsuf = ".bf"
            if "error" not in rb:
                yb = np.frombuffer((OUT / (tag + bsuf + ".r1.y")).read_bytes(), dtype="<f4").astype(np.float64)
                rb["rel_err_vs_float64"] = float(np.linalg.norm(yb[:M] - ref) / np.linalg.norm(ref))
                rb["weight_bytes_per_dispatch"] = wbytes
                rb["per_tensor_ms"] = rb["per_dispatch_ms"] / C
                rb["weight_GBps"] = wbytes / (rb["per_dispatch_ms"] * 1e-3) / 1e9
            for name in ("q4", "q4r2"):
                r = res[name]
                if "error" not in r:
                    y4 = np.frombuffer((OUT / (tag + ".%s.r1.y" % name)).read_bytes(), dtype="<f4").astype(np.float64)
                    r["rel_err_vs_source_float64"] = float(np.linalg.norm(y4[:M] - ref) / np.linalg.norm(ref))
        row = {"q4": res["q4"], "q4r2": res["q4r2"], "baseline_kind": kind, "baseline": rb, "M": int(M), "K": int(K),
               "seg": int(seg), "tile_copies": int(C)}
        st["results"].setdefault("kernels", {})[cls] = row
        save(st)
        log("KERNEL", cls, json.dumps(row))
        for q in OUT.glob(tag + ".*w"):
            q.unlink()
        for q in OUT.glob(tag + ".*s"):
            if q.suffix in (".q4s", ".f8s"):
                q.unlink()


def phase_final(st):
    res = OUT / "results.json"
    res.write_text(json.dumps(st["results"], indent=1, sort_keys=True) + "\n")
    files = sorted(p for p in OUT.iterdir() if p.is_file() and p.suffix not in (".q4w", ".q4x", ".q4s", ".f8w",
                                                                                 ".f8x", ".f8s", ".bfw", ".bfx"))
    files += [LOG, STATE]
    lines = "".join("%s  %s\n" % (sha_file(p), p.name) for p in files)
    (OUT / "SHA256SUMS").write_text(lines)
    subprocess.run(["tar", "-czf", str(HERE / "q4card.tar.gz"), "-C", str(HERE), "out", LOG.name, STATE.name],
                   check=True)
    log("FINAL tarball sha256=%s" % sha_file(HERE / "q4card.tar.gz"))
    bad = [c for c, r in st["results"].get("kernels", {}).items()
           for n in ("q4", "q4r2") if r[n].get("verdict") == "FAIL" or ("error" in r[n] and r[n]["error"] != "odd M")]
    if bad:
        log("CARD-Q4 FAIL output or launch failure in " + ", ".join(bad))
        raise SystemExit(1)
    log("CARD-Q4 COMPLETE all 4-bit outputs bit-exact")


def main(argv):
    st = state()
    skip = "--skip-download" in argv
    only = None
    if "--only" in argv:
        only = argv[argv.index("--only") + 1].split(",")
    OUT.mkdir(exist_ok=True)
    for name, fn in (("env", phase_env), ("weights", phase_weights), ("convert", phase_convert)):
        if name in st["done"] or (name == "weights" and skip):
            continue
        fn(st)
        st["done"].append(name)
        save(st)
    phase_kernels(st, only)
    phase_final(st)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
