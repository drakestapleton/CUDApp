# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Verification-only mutation generator. No runtime policy is implemented here."""
from pathlib import Path
import sys

root = Path(__file__).resolve().parent
out = Path(sys.argv[1])
policy = (root / "policy.om").read_text()
model = (root / "model.om").read_text()
cases = (root / "cases.om").read_text()
mutants = {
    "deadline": ("policy", "eq limit 0 ge now limit", "eq limit 0 gt now limit"),
    "overflow": ("policy", "if gt start sub UMAX budget", "if false"),
    "owner": ("policy", "if or eq pid 0 ne pid owner", "if eq pid 0"),
    "generation": ("policy", "ret and ne epoch 0 eq epoch token", "ret ne epoch 0"),
    "range": ("policy", "ret le size sub capacity off", "ret true"),
    "ack": ("policy", "if not ack", "if false"),
    "order": ("policy", "if eq state CHANNEL\nret CONTEXT", "if eq state CHANNEL\nret CLOSED"),
    "epoch": ("model", "put epochs p omega_c5_epoch get epochs p", "put epochs p get epochs p"),
    "stale": ("model", "if ne token get epochs p", "if false"),
}
for name, (unit, old, new) in mutants.items():
    source = policy if unit == "policy" else model
    if source.count(old) != 1:
        raise SystemExit(f"FAIL: {name} mutation site count {source.count(old)}")
    changed = source.replace(old, new)
    content = changed + model if unit == "policy" else policy + changed
    (out / f"{name}.om").write_text(content + cases)
