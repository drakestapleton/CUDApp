#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Verification oracle only. Run through the documented device-masked wrapper."""
import glob
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "build/mathlibs-host-om"
OUT.mkdir(parents=True, exist_ok=True)
# Refuse when a node that reaches a GPU is visible: NVIDIA, aien-gpu, or a VFIO-bound group. The
# /dev/vfio/vfio control node of a hosted runner reaches no device.
if (glob.glob("/dev/nvidia*") or glob.glob("/dev/aien-gpu*")
        or [p for p in Path("/dev/vfio").glob("*") if p.name != "vfio"]):
    sys.exit("REFUSED: run under bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10")


def run(name, command, expect=0, env=None):
    result = subprocess.run(command, cwd=ROOT, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    text = result.stdout.decode(errors="replace")
    (OUT / (name + ".log")).write_text("$ " + " ".join(map(str, command)) + "\n" + text + "\nrc=" + str(result.returncode) + "\n")
    print(name + ": rc=" + str(result.returncode) + " " + (text.strip().splitlines()[-1] if text.strip() else "(no output)"), flush=True)
    if expect is not None and result.returncode != expect:
        sys.exit("FAIL " + name + "; see " + str(OUT / (name + ".log")))
    return result.returncode, text


CC = os.environ.get("CC", "cc")
CFLAGS = ["-std=gnu11", "-Wall", "-Wextra", "-Werror", "-D_GNU_SOURCE", "-Isrc", "-Im16", "-Invrm", "-ffp-contract=off"]
TEST = "tests/mathlibs_host_om_test.c"
LIB = "build/libomega_gpu.a"
run("build", ["make", "-j3", "build/gpu_mathlibs_test"])
base = OUT / "differential"
for path in OUT.glob("*.gcda"):
    path.unlink()
run("compile", [CC, *CFLAGS, "-O0", "--coverage", "-o", str(base), TEST, LIB, "-lpthread", "-lm"])
run("differential", [str(base)])
rc, _ = run("red", [str(base), "--red"], expect=None)
if rc != 1:
    sys.exit("FAIL red control did not reject the changed expected address")
run("coverage", ["gcov", "-b", "-c", "-f", "-o", str(OUT / "differential-mathlibs_host_om_test.gcno"), TEST])
# gcov emits source-relative names in cwd; retain its text without adding build artifacts to git.
for path in ROOT.glob("*.gcov"):
    shutil.move(str(path), OUT / path.name)

with tempfile.TemporaryDirectory(prefix="mathlibs-host-om-") as tmpname:
    tmp = Path(tmpname)
    archive = tmp / "without-host.a"
    shutil.copyfile(ROOT / LIB, archive)
    members = ["host", "walk", "step", "many", "index", "fft"]
    run("strip-host", ["ar", "d", str(archive), *["cudapp_om_mathlibs_" + n + ".o" for n in members]])
    rc, text = run("missing-unit", [CC, *CFLAGS, "-O2", "-o", str(tmp / "missing"), TEST, str(archive), "-lpthread", "-lm"], expect=None)
    if not rc or "undefined reference" not in text or "omega_native_mh" not in text:
        sys.exit("FAIL missing Omega unit did not fail at link")

    original = (ROOT / "om/mathlibs/host.om").read_text()
    mutants = [
        ("rng-stream-limit", "gt ns 131072", "gt ns 131073"),
        ("seed-increment", "mul add i 1 11400714819323198485", "mul add i 2 11400714819323198485"),
        ("poison64", "eq a 18427282365706066861", "eq a 18427282365706066860"),
        ("chunk-bound", "grid = mh_min sub ctas first 1024", "grid = mh_min sub ctas first 1023"),
        ("zero-stride", "if eq get s 243 0", "if eq get s 243 1"),
        ("real-half", "put s 22 add div get s 21 2 1", "put s 22 add div get s 21 2 0"),
        ("axis-rotation", "v = get s add 41 get s 20", "v = get s 41"),
        ("overlap-bit", "bit = shl ulong 1 and idx 7", "bit = shl ulong 1 and idx 6"),
    ]
    rows = ["mutant\tcompile_rc\trun_rc\tverdict"]
    for name, before, after in mutants:
        if original.count(before) != 1:
            sys.exit("FAIL mutant site is not unique: " + name)
        source = tmp / "host.om"
        source.write_text(original.replace(before, after))
        dest = tmp / name
        dest.mkdir()
        env = os.environ.copy()
        env["OMEGA_UNITS_DEST"] = str(dest)
        env["OMEGA_UNITS"] = " ".join("mathlibs_" + n + "=" + str(source) + ",om/mathlibs/host_" + ("scalar" if n == "host" else n) + ".om" for n in members)
        run(name + "-generate", ["sh", "om/units.sh"], env=env)
        binary = tmp / (name + "-test")
        run(name + "-compile", [CC, *CFLAGS, "-O2", "-o", str(binary), TEST, *[str(dest / ("cudapp_om_mathlibs_" + n + ".c")) for n in members], LIB, "-lpthread", "-lm"])
        rc, text = run(name, [str(binary)], expect=None)
        if not rc or "MATHLIBS-HOST-OM PASS" in text:
            sys.exit("FAIL survived .om mutant " + name)
        rows.append(name + "\t0\t" + str(rc) + "\tKILLED")
    (OUT / "mutants.tsv").write_text("\n".join(rows) + "\n")
print("MATHLIBS-HOST-OM GATE PASS: differential, red, strong-link negative, gcov, 8 killed .om mutants")
