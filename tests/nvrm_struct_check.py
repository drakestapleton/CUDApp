#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Verification oracle only: vendor layout probe and real Omega source mutants."""
import hashlib
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
VENDOR = Path(os.environ.get("NVRM_VENDOR", str(Path.home() / "workspace/_ext/open-gpu-kernel-modules-580.173.02")))
BRIDGE = Path(os.environ["OMEGA_BRIDGE"])
OMC = Path(os.environ["OMEGA_OMC"])
PINS = [(OMC, "2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f"),
        (BRIDGE / "src/toc.om", "d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538"),
        (BRIDGE / "tools/bridge-seeded", "a3a455bd77381e132215924295763e8728889d7d578278ae376de78324be8629")]
for path, sha in PINS:
    if hashlib.sha256(path.read_bytes()).hexdigest() != sha:
        raise SystemExit(f"NOT_RUN unpinned verification input {path}")
if Path("/dev/nvidiactl").exists():
    raise SystemExit("REFUSED: tests require isolated /dev")
INCS = [ROOT / "src", ROOT / "nvrm"] + [VENDOR / p for p in (
    "src/common/sdk/nvidia/inc", "kernel-open/common/inc", "kernel-open/nvidia-uvm", "src/nvidia/arch/nvalloc/unix/include")]
CC = shlex.split(os.environ.get("CC", "cc")) + ["-std=gnu11", "-O2", "-Wall", "-Wextra", "-Werror"] + ["-I" + str(p) for p in INCS]
ENV = dict(os.environ, OMEGA_SEEDED_OMC=str(OMC))

def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], check=True, timeout=120, **kw)

# Each mutant changes one real semantic site; compilation errors do not count.
MUTANTS = [
    ("gpu-block-size", "if eq k 5\nret 564\n", "if eq k 5\nret 8\n"),
    ("card-table-size", "ret 4608\n", "ret 4607\n"),
    ("device-mode-offset", "ret 48\n", "ret 44\n"),
    ("vas-base-offset", "ret 40\nend\nif eq k 9", "ret 32\nend\nif eq k 9"),
    ("gid-data-offset", "ret add 12 sub f 3", "ret add 8 sub f 3"),
    ("memory-limit-offset", "ret 88\n", "ret 80\n"),
    ("channel-engine-offset", "ret 128\nend\nif eq k 12", "ret 124\nend\nif eq k 12"),
    ("card-index-reversed", "if ge idx 64", "if lt idx 64"),
    ("gid-data-width", "if eq k 9\nif ge f 3\nret 1", "if eq k 9\nif ge f 3\nret 2"),
    ("zero-fill", "for i ulong sz\nput b i 0\n", "for i ulong sz\nput b i 1\n"),
    ("uvm-status-width", "ret ns_load b ns_off k sub ns_count k 1 4", "ret ns_load b ns_off k sub ns_count k 1 1"),
    ("heap-free-offset", "ret ns_load b 32 8", "ret ns_load b 24 8"),
]
with tempfile.TemporaryDirectory(prefix="nvrm-struct-check-") as work:
    work = Path(work)
    probe = work / "probe"
    run(CC + [ROOT / "tests/nvrm_struct_layout.c", "-o", probe])
    measured = run([probe], stdout=subprocess.PIPE).stdout
    if measured != (ROOT / "tests/nvrm_struct_layout.txt").read_bytes():
        raise SystemExit("FAIL vendor layout probe differs from committed measurement")
    print("NVRM STRUCT VENDOR PROBE SNAPSHOT PASS", flush=True)
    normal = work / "test"
    run(CC + [ROOT / "tests/nvrm_struct_test.c", ROOT / "src/cudapp_om_nvrm_struct.c", "-o", normal])
    run([normal])
    red = subprocess.run([str(normal), "--red"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
    if red.returncode == 0 or b"FAIL byte differential" not in red.stderr:
        raise SystemExit("FAIL red control was not caught by byte oracle")
    print("NVRM STRUCT RED CONTROL KILLED", flush=True)
    src = (ROOT / "native/nvrm/uvm_fields.om").read_text() + "\n" + (ROOT / "native/nvrm/struct.om").read_text() + "\n"
    for name, before, after in MUTANTS:
        if src.count(before) != 1:
            raise SystemExit(f"FAIL mutant {name} site count {src.count(before)}")
        mutated = work / (name + ".om")
        mutated.write_text(src.replace(before, after))
        generated = work / (name + ".c")
        with generated.open("wb") as out:
            run(["sh", BRIDGE / "tools/bridge-seeded", "--runtime-views", "c", mutated], env=ENV, stdout=out)
        exe = work / name
        run(CC + [ROOT / "tests/nvrm_struct_test.c", generated, "-o", exe])
        result = subprocess.run([str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
        if result.returncode != 1 or b"FAIL " not in result.stderr:
            raise SystemExit(f"FAIL mutant {name} did not trigger oracle assertion (rc={result.returncode})")
        print(f"NVRM STRUCT MUTANT KILLED {name}: {result.stderr.decode().strip()}", flush=True)
print("NVRM STRUCT MUTANTS PASS (12 semantic mutants; HOST_ONLY)")
