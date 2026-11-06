# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host verification oracle: mutate seeded .om input, never generated C or the C oracle."""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
OMC = os.environ["OMEGA_OMC"]
BRIDGE = Path(os.environ["OMEGA_BRIDGE"])
PARTS = ("cg_moe_emit.om", "cg_moe_core.om")
P1 = (1, 512, 8, 1, 3, 2, 0, 0, 1, 1)
P2 = (2, 512, 8, 1, 3, 0, 0, 0, 0, 0)
MUTANTS = (
    ("descriptor-immediate", "CG_MOE_SRC", PARTS + ("cg_moe.om",),
     "put x ulong 6 ulong 856", "put x ulong 6 ulong 857", P1),
    ("e4m3-bit-mask", "CG_MOE2_SRC", PARTS + ("cg_moe2.om",),
     "put x ulong 6 ulong 133169152", "put x ulong 6 ulong 133169153", P2),
    ("branch-relative-origin", "CG_MOE_SRC", PARTS + ("cg_moe.om",),
     "imm = and sub tgt at ulong 4294967295", "imm = and sub tgt add at ulong 1 ulong 4294967295", P1),
    ("allocator-bundle-kind", "CG_MOE2_SRC", PARTS + ("cg_moe2.om",),
     "put m add ulong 64 n kind", "put m add ulong 64 n ulong 1", P2),
    ("fixed-producer-latency", "CG_MOE_SCHED_SRC", ("cg_moe_sched.om",),
     "put s 4 add get s 0 6", "put s 4 add get s 0 5", (1, 512, 8, 1, 3, 2, 0, 1, 1, 1)),
    ("ignored-high-scalar-leak", "CG_MOE2_SRC", PARTS + ("cg_moe2.om",),
     "put x ulong 6 ulong 856", "put x ulong 6 add ulong 856 get z ulong 4", (2, 512, 8, 1, 3, 2147483648, 0, 0, 0, 0)),
    ("accepted-k-boundary", "CG_MOE_SRC", PARTS + ("cg_moe.om",),
     "gt k ulong 32768", "gt k ulong 32767", (1, 32768, 8, 1, 1, 0, 0, 0, 0, 0)),
)


def run(argv, **kwargs):
    return subprocess.run(argv, cwd=ROOT, text=True, capture_output=True, **kwargs)


def main():
    baseline = ROOT / "build/codegen_moe_diff_test"
    for _, _, _, _, _, point in MUTANTS:
        result = run([str(baseline), *map(str, point)])
        if result.returncode:
            raise RuntimeError(f"baseline {point}: rc={result.returncode}\n{result.stdout}{result.stderr}")
    killed = 0
    for name, variable, parts, before, after, point in MUTANTS:
        with tempfile.TemporaryDirectory(prefix="omega-moe-mutant-") as directory:
            tmp = Path(directory)
            source = "\n".join((ROOT / "om/codegen" / part).read_text() for part in parts)
            if source.count(before) != 1:
                raise RuntimeError(f"{name}: mutation site is not unique")
            om = tmp / "mutant.om"
            om.write_text(source.replace(before, after))
            env = dict(os.environ, OMEGA_SEEDED_OMC=OMC)
            result = run(["sh", str(BRIDGE / "tools/bridge-seeded"), "c", str(om)], env=env)
            if result.returncode:
                raise RuntimeError(f"{name}: mutant must compile: rc={result.returncode}\n{result.stderr}")
            c = tmp / "mutant.c"
            c.write_text(result.stdout)
            out = tmp / "build"
            result = run(["make", "-j3", f"OUT_DIR={out}", f"{variable}={c}",
                          "CFLAGS=-std=gnu11 -Wall -Wextra -Werror -O0 -Isrc",
                          str(out / "codegen_moe_diff_test")])
            if result.returncode:
                raise RuntimeError(f"{name}: mutant C must build: rc={result.returncode}\n{result.stdout}{result.stderr}")
            result = run([str(out / "codegen_moe_diff_test"), *map(str, point)])
            if result.returncode != 1 or "DIFF" not in result.stderr:
                raise RuntimeError(f"{name}: mutant survived or crashed: rc={result.returncode}\n{result.stdout}{result.stderr}")
            print(f"KILLED Omega {name} config={point} rc=1", flush=True)
            killed += 1
    print(f"CODEGEN MOE OMEGA MUTANTS PASS ({killed} source-mutant controls)")


if __name__ == "__main__":
    main()
