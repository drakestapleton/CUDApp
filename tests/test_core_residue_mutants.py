#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Recompile admission and packing mutants. Compilation failure is not a kill."""
import os
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OMC = os.environ.get("OMEGA_OMC", "")
BRIDGE = os.environ.get("OMEGA_BRIDGE", "")
CC = ["gcc", "-std=gnu11", "-O2", "-Wall", "-Wextra", "-Werror", "-pthread", "-Isrc"]
LINK = [
    "src/omega_cubin.c",
    "src/cudapp_om_cubin_elf.c",
    "src/cudapp_freest_core_cubinmsg.c",
    "src/cudapp_om_core_fmtnum.c",
]

MUTANTS = (
    ("sm86", "cubadmit", "ne sm 86", "ne sm 87"),
    ("sm121", "cubadmit", "ne sm 121", "ne sm 120"),
    ("target-code", "cubadmit", "ret 8", "ret 2"),
    ("null-text", "cubadmit", "put o add p 0 uchar 110", "put o add p 0 uchar 78"),
    ("target-text", "cubadmit", "put o add p 0 uchar 117", "put o add p 0 uchar 85"),
    ("sign-bit", "ampack", "if ne and v 2147483648 0", "if ne and v 1073741824 0"),
    ("control-shift", "ampack", "shl ctl 32", "shl ctl 31"),
    ("bar-shift", "ampack", "shl get src 15 8", "shl get src 15 7"),
)


def run(command, log, env=None):
    result = subprocess.run(command, cwd=ROOT, env=env, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, timeout=300)
    log.write("$ " + shlex.join(map(str, command)) + "\n" + result.stdout)
    return result


def main():
    if not OMC or not BRIDGE:
        print("NOT_RUN: set OMEGA_OMC and OMEGA_BRIDGE")
        sys.exit(3)
    killed = 0
    for name, unit, old, new in MUTANTS:
        src = (ROOT / "om/core" / f"{unit}.om").read_text()
        if src.count(old) != 1:
            print(f"MUTANT {name} BROKEN: site found {src.count(old)} times")
            sys.exit(1)
        with tempfile.TemporaryDirectory() as temp:
            temp = Path(temp)
            mutant = temp / f"{unit}.om"
            mutant.write_text(src.replace(old, new, 1))
            log_path = temp / "log"
            with log_path.open("w") as log:
                spec = f"core_{unit}={mutant}"
                if unit == "cubadmit":
                    spec = f"core_cubadmit={ROOT / 'om/core/fmtnum.om'},{mutant}"
                env = os.environ.copy()
                env.update({
                    "OMEGA_UNITS": spec,
                    "OMEGA_UNITS_DEST": str(temp),
                    "OMEGA_OMC": OMC,
                    "OMEGA_BRIDGE": BRIDGE,
                })
                gen = run(["sh", "om/units.sh"], log, env)
                if gen.returncode != 0:
                    print(f"MUTANT {name} BROKEN: units.sh {gen.returncode}")
                    sys.exit(1)
                generated = temp / (f"cudapp_freest_core_{unit}.c" if unit == "cubadmit" else f"cudapp_om_core_{unit}.c")
                other = ROOT / "src" / (
                    "cudapp_om_core_ampack.c" if unit == "cubadmit" else "cudapp_freest_core_cubadmit.c")
                binary = temp / "residue"
                built = run(CC + ["-o", str(binary), "tests/test_core_residue.c", *LINK, str(generated), str(other)], log)
                if built.returncode != 0:
                    print(f"MUTANT {name} not killed: compilation failed")
                    sys.exit(1)
                ran = run([str(binary)], log)
            text = ran.stdout or ""
            if ran.returncode == 0 or "RESIDUE FAIL" not in text:
                print(f"MUTANT {name} SURVIVED")
                sys.exit(1)
            print(f"CORE-RESIDUE mutant {name} KILLED", flush=True)
            killed += 1
    print(f"CORE-RESIDUE mutants PASS: {killed} recompiled semantic mutants killed")


if __name__ == "__main__":
    main()
