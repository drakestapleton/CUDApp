#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Diff CUDA++ fixed-latency stalls against Mesa NAK at one pinned commit.

The numbers in catalog.json are citations (file, line, cell), not a copy of
Mesa. --check reads the CUDA++ lines and checks the SM120 +1 arithmetic.
--verify-mesa re-downloads the pinned files and checks the cited cells.
It does not write a chip result.
"""
import argparse
import json
import os
import sys
import tempfile
import urllib.request

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
CATALOG = os.path.join(os.path.dirname(__file__), "catalog.json")


def load():
    with open(CATALOG, encoding="utf-8") as f:
        return json.load(f)


def sm120_gpr(base, adjust):
    """sm120_instr_latencies.rs GPR raw: table value + max(adjust, 1)."""
    return base + max(adjust, 1)


def check_sources(cat):
    errors = []
    for row in cat["rows"]:
        path = os.path.join(ROOT, row["encoder_file"])
        with open(path, encoding="utf-8") as f:
            lines = f.read().splitlines()
        n = row["encoder_line"]
        omega = row["encoder_file"].endswith(".om")
        actual = lines[n - 1].strip() if 1 <= n <= len(lines) else None
        expected = row["encoder_substr"]
        if actual is None or (actual != expected if omega else expected not in actual):
            errors.append("%s: %s:%d does not match %r" % (row["id"], row["encoder_file"], n, expected))
        # The Omega stall assignment is a separate statement from NOP creation.
        # Also retain the preceding tensor instruction's stall in the total.
        contexts = {
            "qmma_reader": ((-4, "put x 7 15"), (-1, "omega_cg_qmma_i x 0"),
                            (1, "omega_cg_qmma_emit b m x 11")),
            "hmma_reader": ((-5, "put x 7 ulong 15"), (-1, "tc_i x 0"),
                            (1, "tc_emit b m x 11")),
        }
        if omega:
            for offset, expected in contexts.get(row["id"], ()):
                index = n - 1 + offset
                if not 0 <= index < len(lines) or lines[index].strip() != expected:
                    errors.append("%s: %s:%d does not match %r" %
                                  (row["id"], row["encoder_file"], n + offset, expected))
    # The pair tightener's default class excludes IMAD.WIDE and FMNMX.
    sched = os.path.join(ROOT, "src/omega_gpu_elementwise_api.c")
    with open(sched, encoding="utf-8") as f:
        text = f.read().splitlines()
    window = "\n".join(text[791:801])
    if "IMAD_WIDE" in window or "FMNMX" in window:
        errors.append("fixed_pair_class now names IMAD_WIDE or FMNMX; the catalog's blanket-6 claim changed")
    if "case K_FIXED: in.control = CW(6," not in text[76]:
        errors.append("K_FIXED blanket stall is not 6 at src/omega_gpu_elementwise_api.c:77")
    return errors


def disagreements(cat):
    out = []
    for row in cat["rows"]:
        nak = row["nak_stall"]
        enc = row["encoder_stall"]
        if nak is None or enc != nak:
            out.append(row)
    return out


def render(cat, rows):
    lines = []
    lines.append("# CUDA++ fixed latencies vs NAK")
    lines.append("")
    lines.append("Mesa commit `%s`." % cat["mesa_commit"])
    lines.append("Clock: %s." % cat["mesa_clock"])
    lines.append("Priority 1 pairs match NAK on paper. They are measured because that match is circular.")
    lines.append("Rows below are the ones that differ. A blank NAK stall means NAK calls the op decoupled.")
    lines.append("")
    lines.append("| id | pair | CUDA++ | NAK | kernel |")
    lines.append("|---|---|---|---|---|")
    for row in rows:
        nak = row["nak"]
        lines.append("| %s | %s | %s | %s | %s |" % (row["id"], row["pair"], row["encoder"], nak, row["kernel"]))
    lines.append("")
    return "\n".join(lines) + "\n"


def self_check(cat):
    """Negative control: an equal pair is not a disagreement; 1 vs 2 is."""
    fake = {"rows": [
        {"encoder_stall": 5, "nak_stall": 5, "id": "equal"},
        {"encoder_stall": 1, "nak_stall": 2, "id": "differ"},
        {"encoder_stall": 5, "nak_stall": None, "id": "class"},
    ]}
    got = [r["id"] for r in disagreements(fake)]
    if got != ["differ", "class"]:
        return "negative control listed %s" % got
    if sm120_gpr(4, 0) != 5 or sm120_gpr(19, 9) != 28 or sm120_gpr(3, 0) != 4:
        return "SM120 +1 arithmetic drifted"
    if sm120_gpr(20, 9) != 29 or sm120_gpr(19, 7) != 26 or (13 + 1) != 14:
        return "cited NAK totals drifted"
    # Priority 1 must not show up as a disagreement.
    p1 = [r["id"] for r in disagreements(cat) if r["priority"] == 1]
    if p1:
        return "priority 1 rows differ from NAK: %s" % p1
    # Exercise the real source checker, not a parallel string-match oracle.
    # A neighboring line and both changed/prefix-preserving stall values fail.
    for row in cat["rows"]:
        if not row["encoder_file"].endswith(".om"):
            continue
        bad = dict(row, encoder_line=row["encoder_line"] - 1)
        if not any(e.startswith(row["id"] + ":") for e in check_sources({"rows": [bad]})):
            return row["id"] + ": wrong-line negative control escaped"
        with open(os.path.join(ROOT, row["encoder_file"]), encoding="utf-8") as f:
            source = f.read().splitlines()
        n = row["encoder_line"] - 1
        with tempfile.TemporaryDirectory(prefix="latmeas-source-") as directory:
            path = os.path.join(directory, "mutant.om")
            bad = dict(row, encoder_file=path)
            for replacement in (source[n].rsplit(" ", 1)[0] + " 0", source[n] + "0"):
                changed = source[:]
                changed[n] = replacement
                with open(path, "w", encoding="utf-8") as f:
                    f.write("\n".join(changed) + "\n")
                if not any(e.startswith(row["id"] + ":") for e in check_sources({"rows": [bad]})):
                    return row["id"] + ": changed-stall negative control escaped"
    return None


def verify_mesa(commit):
    base = "https://gitlab.freedesktop.org/mesa/mesa/-/raw/%s/" % commit
    reg = urllib.request.urlopen(base + "src/nouveau/compiler/latencies/sm100/reg_raw.csv", timeout=60).read().decode()
    pred = urllib.request.urlopen(base + "src/nouveau/compiler/latencies/sm100/pred_raw.csv", timeout=60).read().decode()
    rs = urllib.request.urlopen(base + "src/nouveau/compiler/nak/sm120_instr_latencies.rs", timeout=60).read().decode()
    lines = reg.splitlines()
    hdr = lines[0].split(",")
    alu = lines[1].split(",")
    fma = lines[4].split(",")

    def cell(parts, name):
        return parts[hdr.index(name)]

    checks = [
        (cell(alu, "alu"), "4"),
        (cell(alu, "fma"), "5"),
        (cell(alu, "dualalu"), "4"),
        (cell(alu, "hmma"), "19"),
        (cell(alu, "imma"), "19"),
        (cell(alu, "imad_wide_write_dl"), "3"),
        (cell(alu, "imad_wide_write_dh"), "5"),
        (cell(fma, "fma"), "4"),
        (cell(fma, "alu"), "5"),
    ]
    for got, want in checks:
        if got != want:
            raise SystemExit("reg_raw cell %s != %s" % (got, want))
    pred_lines = pred.splitlines()
    pred_hdr = pred_lines[0].split(",")
    redir = pred_lines[10].split(",")
    if redir[0] != "redirected_fp64" or redir[pred_hdr.index("dualalu")] != "13":
        raise SystemExit("pred_raw redirected_fp64/dualalu is not 13")
    for needle in ("RegLatencySM100::Hmma => 9", "RegLatencySM100::Imma => 7", "NAK_SV_CLOCK" ):
        # CLOCK is not in this file. Check the adjust arms and the +1 in raw().
        pass
    if "RegLatencySM100::Hmma => 9" not in rs or "RegLatencySM100::Imma => 7" not in rs:
        raise SystemExit("sm120 latency_adjust does not match the catalog")
    if "+ 1" not in rs:
        raise SystemExit("sm120 raw no longer adds padding")
    print("mesa %s cells match the catalog" % commit)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--check", action="store_true")
    p.add_argument("--md", help="write the disagreement table")
    p.add_argument("--verify-mesa", action="store_true")
    args = p.parse_args()
    cat = load()
    errors = check_sources(cat)
    bad = self_check(cat)
    if bad:
        errors.append(bad)
    if errors:
        for e in errors:
            print("FAIL:", e, file=sys.stderr)
        return 1
    text = render(cat, disagreements(cat))
    sys.stdout.write(text)
    if args.md:
        with open(args.md, "w", encoding="utf-8") as f:
            f.write(text)
    if args.verify_mesa:
        verify_mesa(cat["mesa_commit"])
    if not args.check and not args.md and not args.verify_mesa:
        return 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
