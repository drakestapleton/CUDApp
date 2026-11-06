# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host verification oracle: mutate the seeded Omega GDN sources, never the checked C output.

Each mutant is one textual swap at a site that occurs exactly once across the unit sources. Every unit that contains the
mutated file is regenerated with tools/bridge-seeded and linked into codegen_gdn_diff_test, which must then fail with a
GDN DIFF (IR/regalloc/bytes or refusal) or GDN SHAPE DIFF against the retained C oracle."""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
OMC = os.environ["OMEGA_OMC"]
BRIDGE = Path(os.environ["OMEGA_BRIDGE"])
UNITS = (
    ("CG_GDN_SRC", ("cg_gdn_shape.om", "cg_gdn_common.om", "cg_gdn.om")),
    ("CG_GDN_POLICY_SRC", ("cg_gdn_shape.om", "cg_gdn_policy.om")),
)
MUTANTS = (
    # name, source file, before, after
    ("shape-value-heads", "cg_gdn_shape.om", "vh = mul 16 r", "vh = mul 17 r"),
    ("control-word-wait", "cg_gdn_common.om", "shl wt 20", "shl wt 21"),
    ("branch-patch-offset", "cg_gdn_common.om", "and sub tg at 4294967295", "and sub add tg 1 at 4294967295"),
    ("butterfly-step", "cg_gdn_common.om", "o = mul o 2", "o = mul o 4"),
    ("mutant-selector-bound", "cg_gdn.om", "if or ne c 0 gt a 4", "if or ne c 0 gt a 5"),
    ("one-minus-qs", "cg_gdn.om", "get s 33 1063870306", "get s 33 1063870307"),
    ("prep-prefetch-offset", "cg_gdn.om", "get s 187 32768 0", "get s 187 32769 0"),
)


def run(argv, **kwargs):
    return subprocess.run(argv, cwd=ROOT, text=True, capture_output=True, **kwargs)


def main():
    baseline = ROOT / "build/codegen_gdn_diff_test"
    r = run([str(baseline)])
    if r.returncode or "CODEGEN GDN DIFF PASS" not in r.stdout:
        raise RuntimeError(f"baseline: rc={r.returncode}\n{r.stdout}{r.stderr}")
    src = ROOT / "om/codegen"
    killed = 0
    for name, source, before, after in MUTANTS:
        sites = sum((src / part).read_text().count(before) for part in {p for _, parts in UNITS for p in parts})
        if sites != 1 or (src / source).read_text().count(before) != 1:
            raise RuntimeError(f"{name}: mutation site found {sites} times")
        with tempfile.TemporaryDirectory(prefix="omega-gdn-mutant-") as tmp:
            tmp = Path(tmp)
            overrides = []
            for variable, parts in UNITS:
                if source not in parts:
                    continue
                data = "\n".join((src / part).read_text() for part in parts)
                om = tmp / f"{variable}.om"
                om.write_text(data.replace(before, after))
                env = dict(os.environ, OMEGA_SEEDED_OMC=OMC)
                r = run(["sh", str(BRIDGE / "tools/bridge-seeded"), "c", str(om)], env=env)
                if r.returncode:
                    raise RuntimeError(f"{name}: mutant must compile: rc={r.returncode}\n{r.stderr}")
                c = tmp / f"{variable}.c"
                c.write_text(r.stdout)
                overrides.append(f"{variable}={c}")
            out = tmp / "build"
            r = run(["make", "-j3", f"OUT_DIR={out}", *overrides,
                     "CFLAGS=-std=gnu11 -Wall -Wextra -Werror -O0 -Isrc", str(out / "codegen_gdn_diff_test")])
            if r.returncode:
                raise RuntimeError(f"{name}: mutant C must build: rc={r.returncode}\n{r.stdout}{r.stderr}")
            r = run([str(out / "codegen_gdn_diff_test")])
            if r.returncode != 1 or "GDN DIFF" not in r.stderr and "GDN SHAPE DIFF" not in r.stderr:
                raise RuntimeError(f"{name}: mutant survived or crashed: rc={r.returncode}\n{r.stdout}{r.stderr}")
            print(f"KILLED Omega {name} rc=1: {r.stderr.splitlines()[0]}", flush=True)
            killed += 1
    print(f"CODEGEN GDN OMEGA MUTANTS PASS ({killed} source mutants killed)")


if __name__ == "__main__":
    main()
