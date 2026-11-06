# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host verification oracle: mutate seeded Omega sources, never the checked C output."""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
OMC = os.environ["OMEGA_OMC"]
BRIDGE = Path(os.environ["OMEGA_BRIDGE"])
# Layout, output gathering, epilogue, staging depth and chunk axes are all represented.
POINTS = (
    (256, 0, 1, 1, 1, 0, 0), (256, 1, 2, 1, 1, 0, 0),
    (256, 0, 1, 32, 1, 0, 0), (256, 1, 2, 32, 1, 0, 0),
    (256, 0, 1, 1, 1, 1, 0), (256, 1, 2, 1, 1, 1, 0),
    (256, 0, 1, 32, 1, 1, 0), (256, 1, 2, 32, 1, 1, 0),
    (4096, 0, 1, 1, 16, 0, 0), (4096, 1, 2, 1, 8, 1, 0),
    (16384, 0, 1, 1, 16, 0, 0), (16384, 1, 2, 1, 8, 0, 0),
)


def run(argv, **kwargs):
    return subprocess.run(argv, cwd=ROOT, text=True, capture_output=True, **kwargs)


def main():
    baseline = ROOT / "build/codegen_tern_diff_test"
    for point in POINTS:
        r = run([str(baseline), *map(str, point)])
        if r.returncode:
            raise RuntimeError(f"baseline {point}: rc={r.returncode}\n{r.stdout}{r.stderr}")
    killed = 0
    for name, source, before, after, variable, parts in (
        ("builder-bias", "cg_tern.om", "put x 6 2147516416", "put x 6 2147516417", "CG_TERN_SRC", ("cg_tern_common.om", "cg_tern.om")),
        ("fixed-latency", "cg_tern_sched.om", "put s 4 add get s 0 6", "put s 4 add get s 0 5", "CG_TERN_SCHED_SRC", ("cg_tern_sched.om",)),
    ):
        with tempfile.TemporaryDirectory(prefix="omega-tern-mutant-") as tmp:
            tmp = Path(tmp)
            data = "\n".join((ROOT / "om/codegen" / part).read_text() for part in parts)
            if data.count(before) != 1:
                raise RuntimeError(f"{name}: mutation site is not unique")
            om = tmp / "mutant.om"
            om.write_text(data.replace(before, after))
            env = dict(os.environ, OMEGA_SEEDED_OMC=OMC)
            r = run(["sh", str(BRIDGE / "tools/bridge-seeded"), "c", str(om)], env=env)
            if r.returncode:
                raise RuntimeError(f"{name}: mutant must compile: rc={r.returncode}\n{r.stderr}")
            c = tmp / "mutant.c"
            c.write_text(r.stdout)
            out = tmp / "build"
            r = run(["make", "-j3", f"OUT_DIR={out}", f"{variable}={c}", "CFLAGS=-std=gnu11 -Wall -Wextra -Werror -O0 -Isrc", str(out / "codegen_tern_diff_test")])
            if r.returncode:
                raise RuntimeError(f"{name}: mutant C must build: rc={r.returncode}\n{r.stdout}{r.stderr}")
            for point in POINTS:
                r = run([str(out / "codegen_tern_diff_test"), *map(str, point)])
                if r.returncode != 1 or "TERN DIFF" not in r.stderr:
                    raise RuntimeError(f"{name}: mutant survived or crashed at {point}: rc={r.returncode}\n{r.stdout}{r.stderr}")
                print(f"KILLED Omega {name} config={point} rc=1", flush=True)
                killed += 1
    print(f"CODEGEN TERN OMEGA MUTANTS PASS ({killed} source-mutant/config controls)")


if __name__ == "__main__":
    main()
