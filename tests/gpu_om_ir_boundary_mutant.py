#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Reverting the generic replay bound must fail the same boundary regression."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

source = Path("src/omega_gpu_om_ir.c").read_text()
site = "r[0] > (uint64_t)BW_IR_LAST"
if source.count(site) != 1:
    raise SystemExit("om-ir-boundary-mutant: FAIL (mutation site not unique)")
with tempfile.TemporaryDirectory(prefix="om-ir-boundary-") as tmp:
    root = Path(tmp)
    mutant = root / "omega_gpu_om_ir.c"
    mutant.write_text(source.replace(site, "r[0] > (uint64_t)BW_IR_ATOMG_ADD_F32_STRONG_GPU"))
    binary = root / "boundary"
    command = shlex.split(os.environ.get("CC", "cc")) + [
        "-std=gnu11", "-O2", "-Isrc", "-Itests", "-I./m16", "-o", str(binary),
        "tests/gpu_om_ir_boundary_test.c", str(mutant),
        os.environ.get("OMEGA_TEST_LIB", "build/libomega_gpu.a"), "-lpthread", "-lm"]
    subprocess.run(command, check=True)
    run = subprocess.run([str(binary)], text=True, capture_output=True)
    if run.returncode != 1 or "om-ir-boundary: FAIL" not in run.stderr:
        raise SystemExit("om-ir-boundary-mutant: FAIL (reverted bound survived)")
print("om-ir-boundary-mutant: PASS (compiled reverted-bound mutant killed)")
