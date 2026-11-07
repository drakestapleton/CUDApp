#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Reverting the generic replay bound must fail the same boundary regression.

The default build replays through the seeded unit (omega_cg_replay), whose bound
is the BW_IR_LAST argument; the C replay under CUDAPP_MIGRATION_PENDING_C_CODEGEN_BASE
is the oracle and carries its own bound. Each is reverted where it is compiled,
and the unmutated source must pass in both builds first, so a kill is the bound
and not a broken build.
"""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

source = Path("src/omega_gpu_om_ir.c").read_text()
OLD_BOUND = "BW_IR_ATOMG_ADD_F32_STRONG_GPU"
cases = [
    # (name, extra cflags, site, reverted site)
    ("omega-replay", [], "                        BW_IR_LAST) != OMEGA_OK) return OMEGA_OM_IR_BRIDGE;",
     "                        " + OLD_BOUND + ") != OMEGA_OK) return OMEGA_OM_IR_BRIDGE;"),
    ("c-oracle", ["-DCUDAPP_MIGRATION_PENDING_C_CODEGEN_BASE=1"], "r[0] > (uint64_t)BW_IR_LAST",
     "r[0] > (uint64_t)" + OLD_BOUND),
]
cc = shlex.split(os.environ.get("CC", "cc"))
lib = os.environ.get("OMEGA_TEST_LIB", "build/libomega_gpu.a")


def run(root, name, flags, text):
    unit = root / (name + ".c")
    unit.write_text(text)
    binary = root / name
    subprocess.run(cc + ["-std=gnu11", "-O2", "-Isrc", "-Itests", "-I./m16"] + flags +
                   ["-o", str(binary), "tests/gpu_om_ir_boundary_test.c", str(unit), lib,
                    "-lpthread", "-lm"], check=True)
    return subprocess.run([str(binary)], text=True, capture_output=True)


with tempfile.TemporaryDirectory(prefix="om-ir-boundary-") as tmp:
    root = Path(tmp)
    for name, flags, site, reverted in cases:
        if source.count(site) != 1:
            raise SystemExit(f"om-ir-boundary-mutant: FAIL ({name} mutation site not unique)")
        control = run(root, name + "-control", flags, source)
        if control.returncode != 0:
            raise SystemExit(f"om-ir-boundary-mutant: FAIL ({name} unmutated control failed: "
                             f"{control.stderr.strip()})")
        mutant = run(root, name + "-mutant", flags, source.replace(site, reverted))
        if mutant.returncode != 1 or "om-ir-boundary: FAIL" not in mutant.stderr:
            raise SystemExit(f"om-ir-boundary-mutant: FAIL ({name} reverted bound survived)")
        print(f"om-ir-boundary-mutant: {name} control PASS, reverted-bound mutant killed")
print("om-ir-boundary-mutant: PASS (Omega replay and C oracle reverted-bound mutants killed)")
