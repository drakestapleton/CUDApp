#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compile changed Omega, then require the C differential to reject each variant.

Compiler/converter failures are errors, never killed mutants. The real checked
portable artifact is linked normally for the unmodified negative control.
"""
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
OUT = Path(os.environ.get("OUT_DIR", "build"))
OMC = os.environ["OMEGA_OMC"]
BRIDGE = Path(os.environ["OMEGA_BRIDGE"])
ENV = dict(os.environ, OMEGA_SEEDED_OMC=OMC)
EMIT = (ROOT / "om/codegen-tc/cg_tc_emit.om").read_text()
SOURCE = (ROOT / "om/codegen-tc/cg_tc.om").read_text()


def run(args, **kw):
    return subprocess.run(args, cwd=ROOT, env=ENV, text=True, **kw)


def replace_phase(source, kind, before, after):
    # Match the owning builder's phase, not helper text in a different builder.
    pattern = re.compile(r"fn tc_" + str(kind) + r"_part\d+ .*?\nend\n(?=fn|\n|$)", re.S)
    for match in pattern.finditer(source):
        text = match.group()
        if before in text:
            return source[:match.start()] + text.replace(before, after, 1) + source[match.end():]
    raise RuntimeError(f"missing Omega mutation site: {kind}: {before}")


mutants = [
    ("probe-hmma-spacing", 0, 0, "put x 7 ulong 15", "put x 7 ulong 14"),
    ("peak-hmma-spacing", 1, 1, "put x 7 get r 18", "put x 7 add get r 18 ulong 1"),
    ("bandwidth-fold", 2, 2, "tc_i x ulong 29", "tc_i x ulong 18"),
    ("tcgemm-fp8-decode", 3, 3, "put x 6 ulong 8323199", "put x 6 ulong 8323198"),
    ("tc2-f32-rescale", 4, 4, "tc_i x 32", "tc_i x 30"),
    ("tc2-swiglu-rescale", 5, 4, "tc_i x 32", "tc_i x 30"),
]

with tempfile.TemporaryDirectory(prefix="codegen-tc-mutants-") as td:
    tmp = Path(td)
    for name, variant, kind, before, after in mutants:
        control = run([str(OUT / "codegen_tc_diff"), "--variant", str(variant)])
        if control.returncode:
            raise SystemExit(f"unmodified {name} differential failed")
        om = tmp / (name + ".om")
        c = tmp / (name + ".c")
        exe = tmp / name
        om.write_text(EMIT + "\n" + replace_phase(SOURCE, kind, before, after))
        with c.open("w") as f:
            compiled = run(["sh", str(BRIDGE / "tools/bridge-seeded"), "c", str(om)], stdout=f)
        if compiled.returncode:
            raise SystemExit(f"{name}: converter failed (not a kill)")
        linked = run([os.environ.get("CC", "cc"), "-std=gnu11", "-O2", "-Isrc", "-o", str(exe),
                      "tests/codegen_tc_diff.c", str(c), str(OUT / "codegen_tc_oracle.o"),
                      str(OUT / "codegen_tc2_oracle.o"), str(OUT / "libomega_gpu.a"), "-lpthread", "-lm"])
        if linked.returncode:
            raise SystemExit(f"{name}: link failed (not a kill)")
        result = run([str(exe), "--variant", str(variant)], capture_output=True)
        print(result.stderr, end="")
        if result.returncode != 1 or "IR differs" not in result.stderr:
            raise SystemExit(f"{name}: differential did not kill emitted semantic mutant")
        print(f"CODEGEN TC OMEGA MUTANT {name}: KILLED", flush=True)
print("CODEGEN TC OMEGA MUTANTS PASS (6 mutants)")
