#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compile real seeded mutations; compilation failures are NOT mutant kills."""
import hashlib
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "om/encoder/bw_enc_sm121.om"
PIN = "2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f"
MUTANTS = (
    ("wrong-dadd-opcode", "wda o 29225 0 rb o 3", "wda o 29224 0 rb o 3"),
    ("swapped-dfma-operands", "wda o 29227 rb o 3 rb o 4", "wda o 29227 rb o 4 rb o 3"),
    ("dropped-pair-alignment", "if or ne and r 1 0 ge r 254", "if or ne and r 0 0 ge r 254"),
    ("wrong-dsetp-mode", "get o 6 66084864 ctl o 926720", "get o 6 66060288 ctl o 926720"),
    ("bound-off-by-one", "if gt op 79", "if gt op 80"),
    ("wrong-mufu-function", "pick eq op 73 6144 7168", "pick eq op 73 7168 6144"),
)


def main():
    omc = Path(os.environ["OMEGA_OMC"])
    bridge = Path(os.environ["OMEGA_BRIDGE"]) / "tools/bridge-seeded"
    if hashlib.sha256(omc.read_bytes()).hexdigest() != PIN:
        raise RuntimeError("NOT_RUN: seeded compiler pin differs")
    # Defense in depth: this gate is a host verifier, never a device consumer. Refuse when a node that
    # reaches a GPU is visible (NVIDIA, aien-gpu, a VFIO-bound group); /dev/cuse and /dev/vfio/vfio reach none.
    if (any(Path("/dev").glob("nvidia*")) or any(Path("/dev").glob("aien-gpu*"))
            or [p for p in Path("/dev/vfio").glob("*") if p.name != "vfio"]):
        raise RuntimeError("NOT_RUN: run inside the required device-free bwrap namespace")
    env = dict(os.environ, OMEGA_SEEDED_OMC=str(omc))
    text = SOURCE.read_text()
    cc = shlex.split(os.environ.get("CC", "cc"))
    lib = str(Path(os.environ["OMEGA_TEST_LIB"]).resolve())
    with tempfile.TemporaryDirectory(prefix="omega-f64-mutants-") as tmp:
        tmp = Path(tmp)
        for name, before, after in MUTANTS:
            if text.count(before) != 1:
                raise RuntimeError(f"mutation site drift: {name}")
            source = tmp / (name + ".om")
            source.write_text(text.replace(before, after))
            generated = tmp / (name + ".c")
            with generated.open("w") as out:
                subprocess.run(["sh", str(bridge), "c", str(source)], cwd=ROOT, env=env, stdout=out, check=True)
            binary = tmp / name
            subprocess.run(cc + ["-std=gnu11", "-O2", "-ffp-contract=off", "-Isrc", "-Itests", "-I./m16",
                                "tests/codegen_f64_test.c", str(generated), lib, "-lpthread", "-lm", "-o", str(binary)],
                           cwd=ROOT, check=True)
            result = subprocess.run([str(binary)], cwd=ROOT, text=True, capture_output=True)
            if result.returncode != 1 or "codegen-f64: FAIL" not in result.stdout or "f64 check line" not in result.stderr:
                raise RuntimeError(f"mutant not killed by an assertion: {name}: rc={result.returncode}\n{result.stdout}\n{result.stderr}")
            print(f"codegen-f64 mutant {name}: KILLED", flush=True)
    print("codegen-f64 mutants: PASS (6 compiled seeded mutants killed)")


if __name__ == "__main__":
    main()
