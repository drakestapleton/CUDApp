#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Recompiled Omega mutants for the cg_dec and gdn_admit units.

Each mutant is a literal text swap on a site that occurs exactly once in its .om source. The mutated source is
compiled with the pinned seeded compiler through bridge-seeded, and the resulting unit object is linked ahead of
libomega_gpu.a into the unit's differential, so the mutant replaces the checked-in unit. A mutant is killed when
the differential exits nonzero. Usage: OMEGA_OMC=... OMEGA_BRIDGE=... python3 tests/dec_mutants.py [build-dir]
"""
import os, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "build")
DEC = ["om/codegen/cg_dec_emit.om", "om/codegen/cg_dec.om"]
ADMIT = ["om/gdn/admit.om"]
MUTANTS = [
    ("dec-inv-h", DEC, "ret 973078528", "ret 973078529"),
    ("dec-width-bound", DEC, "if or ne rem h 1024 0 gt n 32", "if or ne rem h 1024 0 gt n 36"),  # 33..35 are equivalent: no multiple of 1024 lies in 8193..9215
    ("dec-dispatch-argmax", DEC, "if eq kind 4", "if eq kind 9"),
    ("dec-group-slots", DEC, "if or lt sl 1 gt sl 16", "if or lt sl 1 gt sl 17"),
    ("dec-axpy-stride", DEC, "if or eq st 0 ne rem st 256 0", "if or eq st 0 ne rem st 128 0"),
    ("dec-warp-reduce", DEC, "while gt o 0 bound 5", "while gt o 1 bound 5"),
    ("admit-max-layers", ADMIT, "if or eq a 0 gt a 64", "if or eq a 0 gt a 65"),
    ("admit-move-grid", ADMIT, "if gt div by 4096 2147483647", "if gt div by 4096 2147483648"),
    ("admit-align16", ADMIT, "if ne and m16 15 0", "if ne and m16 7 0"),
    ("admit-tail-bytes", ADMIT, "put o 7 mul vh 6144", "put o 7 mul vh 6145"),
    ("admit-finite", ADMIT, "if eq and shr x 23 255 255", "if eq and shr x 24 255 255"),
]

def run(cmd, **kw):
    return subprocess.run(cmd, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, **kw)

def main():
    env = os.environ
    omc, bridge = env["OMEGA_OMC"], env.get("OMEGA_BRIDGE") or env["OMEGA_BRIDGE_DIR"]
    cc = env.get("CC", "cc")
    flags = ["-std=gnu11", "-O2", "-D_GNU_SOURCE", "-Isrc", "-Itests"]
    killed = 0
    with tempfile.TemporaryDirectory() as tmp:
        for name, files, old, new in MUTANTS:
            text = "\n".join(open(os.path.join(ROOT, f)).read() for f in files)
            if text.count(old) != 1:
                print(f"FAIL mutant {name}: site found {text.count(old)} times"); return 1
            src = os.path.join(tmp, name + ".om")
            open(src, "w").write(text.replace(old, new))
            gen = run(["sh", os.path.join(bridge, "tools/bridge-seeded"), "c", src],
                      env=dict(env, OMEGA_SEEDED_OMC=omc))
            if gen.returncode:
                print(f"FAIL mutant {name}: did not compile rc={gen.returncode} {gen.stdout[-200:]}"); return 1
            unit_c = os.path.join(tmp, name + ".c")
            open(unit_c, "w").write(gen.stdout)
            if files is DEC:
                test, extra = "tests/codegen_dec_diff.c", os.path.join(OUT, "codegen_dec_oracle.o")
            else:
                test, extra = "tests/gdn_admit_diff.c", os.path.join(OUT, "gdn_admit_oracle.o")
            exe = os.path.join(tmp, name)
            link = run([cc, *flags, "-o", exe, test, unit_c, extra, os.path.join(OUT, "libomega_gpu.a"), "-lpthread", "-lm"])
            if link.returncode:
                print(f"FAIL mutant {name}: link {link.stdout[-400:]}"); return 1
            res = run([exe])
            last = (res.stdout.strip().splitlines() or [""])[-1]
            if res.returncode == 0:
                print(f"FAIL mutant {name} survived: {last}"); return 1
            killed += 1
            print(f"killed {name} rc={res.returncode}: {last[:160]}")
    dec = sum(1 for m in MUTANTS if m[1] is DEC)
    print(f"PASS dec mutants killed={killed}/{len(MUTANTS)} (cg_dec {dec}, gdn_admit {len(MUTANTS) - dec})")
    return 0

if __name__ == "__main__":
    sys.exit(main())
