#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Recompile one semantic mutant at a time. Each must make the differential fail."""
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MUTANTS = [
    ("om/api/apish_shp.om",
     "if eq rem k 512 0\nret ulong 16\n",
     "if eq rem k 512 0\nret ulong 8\n",
     "lane-16-to-8"),
    ("om/api/apish_shp.om",
     "if eq and lo mask 0\nret ulong 1\n",
     "if eq and lo 0 0\nret ulong 1\n",
     "alignok-always"),
    ("om/api/apish_pack.om",
     "ret 2143289344\n",
     "ret 0\n",
     "refb-nan-zero"),
    ("om/api/apish_slot.om",
     "bump rec 10\nret -6\nend\nput rec 0 1\n",
     "bump rec 10\nret 0\nend\nput rec 0 1\n",
     "slot-busy-ok"),
    ("om/api/apish_shp.om",
     "if eq ww 16\nret ulong 128\nend\nret ulong 64\n",
     "if eq ww 16\nret ulong 128\nend\nret ulong 32\n",
     "tgbm-64-to-32"),
    ("om/api/apish_shp.om",
     "ret ulong mul div add n 7 8 8\n",
     "ret ulong n\n",
     "pad8-no-round"),
    ("om/api/apish_shp.om",
     "if ne rem p 256 0\nret ulong 1\n",
     "if ne rem p 256 0\nret ulong 0\n",
     "dsplit-bad-align-ok"),
    ("om/api/apish_shp.om",
     "if gt m 32768\nret ulong 1\n",
     "if gt m 32769\nret ulong 1\n",
     "pa256-bound-off-by-one"),
]


def main() -> int:
    script = os.path.join(ROOT, "tests", "api_shells_run.sh")
    killed = 0
    for path, old, new, name in MUTANTS:
        full = os.path.join(ROOT, path)
        with open(full) as fh:
            src = fh.read()
        count = src.count(old)
        if count != 1:
            print(f"FAIL mutant {name}: site count {count}")
            return 1
        with open(full, "w") as fh:
            fh.write(src.replace(old, new, 1))
        try:
            r = subprocess.run(
                ["bash", script, "regen"],
                cwd=ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
            )
        finally:
            with open(full, "w") as fh:
                fh.write(src)
        if r.returncode == 0:
            tail = r.stdout[-400:]
            print(f"FAIL mutant {name} survived\n{tail}")
            return 1
        print(f"KILLED {name} rc={r.returncode}")
        killed += 1
    if killed < 6:
        print(f"FAIL only {killed} mutants")
        return 1
    print(f"PASS mutants killed={killed}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
