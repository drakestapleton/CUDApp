#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Each altered seeded encoder must compile, then fail the native-word regression."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

source = Path("om/encoder/bw_enc_sm121.om").read_text()
mutants = [
    ("widen-opcode", "wd o 4278219312", "wd o 4278219313"),
    ("widen-source", "or 536870912 rb o 2", "or 536870912 rb o 3"),
    ("widen-format", "16640 ctl o 17811968", "16641 ctl o 17811968"),
    ("narrow-opcode", "wd o 29444", "wd o 29445"),
    ("narrow-format", "2099200 ctl o 17704960", "2101248 ctl o 17704960"),
]
bridge = Path(os.environ["OMEGA_BRIDGE"]) / "tools/bridge-seeded"
env = dict(os.environ, OMEGA_SEEDED_OMC=os.environ["OMEGA_OMC"])
with tempfile.TemporaryDirectory(prefix="dnn-half-mutants-") as tmp:
    root = Path(tmp)
    for name, before, after in mutants:
        if source.count(before) != 1:
            raise SystemExit(f"dnn-half-mutants: FAIL ({name}: mutation site not unique)")
        path = root / f"{name}.om"
        path.write_text(source.replace(before, after))
        generated = root / f"{name}.c"
        with generated.open("w") as out:
            subprocess.run(["sh", str(bridge), "c", str(path)], env=env, stdout=out, check=True)
        binary = root / name
        command = shlex.split(os.environ.get("CC", "cc")) + [
            "-std=gnu11", "-O2", "-ffp-contract=off", "-Isrc", "-Itests", "-I./m16",
            "tests/gpu_dnn_half_test.c", "src/omega_numeric.c", str(generated),
            os.environ.get("OMEGA_TEST_LIB", "build/libomega_gpu.a"),
            "-lpthread", "-lm", "-o", str(binary)]
        subprocess.run(command, check=True)
        run = subprocess.run([str(binary)], text=True, capture_output=True)
        if run.returncode != 1 or "dnn-half: FAIL" not in run.stdout:
            raise SystemExit(f"dnn-half-mutants: FAIL ({name}: compiled mutant survived)")
        print(f"dnn-half mutant {name}: KILLED", flush=True)
print("dnn-half-mutants: PASS (5 compiled seeded encoder mutants killed)")
