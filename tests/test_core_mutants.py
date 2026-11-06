#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Recompile six semantic Omega mutants; compilation failure is not a kill."""
import argparse
import hashlib
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
MUTANTS = (
    ("initial-state", "putle c 0 4 1779033703", "putle c 0 4 1779033702"),
    ("round-constant", "t = add x add get w 0 1116352408", "t = add x add get w 0 1116352409"),
    ("padding-marker", "put c add 40 bi 128", "put c add 40 bi 129"),
    ("two-block-padding", "zbuf c add bi 1 64\nif ge bi 56", "zbuf c add bi 1 64\nif ge bi 57"),
    ("message-bit-count", "bits = mul cnt 8", "bits = mul cnt 7"),
    ("length-byte-order", "sh = mul sub 7 i 8", "sh = mul i 8"),
)


def run(command, log, env=None):
    result = subprocess.run(command, cwd=ROOT, env=env, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, timeout=300)
    log.write("$ " + shlex.join(map(str, command)) + "\n" + result.stdout)
    log.write(f"rc={result.returncode}\n")
    log.flush()
    return result


def require(condition, detail):
    if not condition:
        raise RuntimeError(detail)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-dir", default="build")
    parser.add_argument("--receipt", required=True)
    args = parser.parse_args()
    receipt = ROOT / args.receipt
    receipt.mkdir(parents=True, exist_ok=True)
    out_dir = ROOT / args.out_dir
    source = (ROOT / "om/core/sha256.om").read_text()
    with (receipt / "baseline.log").open("w") as log:
        require(run([str(out_dir / "test-core-util"), "--quick"], log).returncode == 0, "baseline")
    for name, before, after in MUTANTS:
        require(source.count(before) == 1, (name, "mutation site is not unique"))
        changed = source.replace(before, after)
        with tempfile.TemporaryDirectory(prefix="core-util-mutant-") as temp:
            temp = Path(temp)
            om = temp / "mutant.om"
            om.write_text(changed)
            env = dict(os.environ, OMEGA_UNITS=f"core_sha={om}", OMEGA_UNITS_DEST=str(temp))
            with (receipt / (name + ".log")).open("w") as log:
                log.write(f"source-sha256={hashlib.sha256(changed.encode()).hexdigest()}\n")
                log.write(f"replace {before!r} -> {after!r}\n")
                require(run(["sh", "om/units.sh"], log, env).returncode == 0, (name, "generation"))
                command = shlex.split(os.environ.get("CC", "cc")) + [
                    "-std=gnu11", "-D_GNU_SOURCE", "-O2", "-Wall", "-Wextra", "-Werror",
                    "-Isrc", "tests/test_core_util.c", "src/sha256.c",
                    str(out_dir / "core-util-sha-ref.o"), str(temp / "cudapp_om_core_sha.c"),
                    "-lpthread", "-o", str(temp / "test-mutant")]
                require(run(command, log).returncode == 0, (name, "compilation"))
                result = run([str(temp / "test-mutant"), "--quick"], log)
                require(result.returncode == 1 and "SHA vector length " in result.stdout, (name, "not killed by digest comparison"))
                (receipt / (name + ".ident")).write_text((temp / "cudapp_om_core_sha.ident").read_text())
                log.write("KILLED: compiled mutant disagrees with FIPS digest\n")
            print(f"CORE-UTIL mutant {name} KILLED", flush=True)
    print("CORE-UTIL mutants PASS: 6 recompiled semantic mutants killed")


if __name__ == "__main__":
    main()
