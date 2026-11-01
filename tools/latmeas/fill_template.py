#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Fill the GB10-vs-NAK markdown from a launch JSON file.

A synthetic input is refused unless --allow-synthetic is set, and then every
measured cell is the word FAKE. Missing stall_required stays blank. This
script never invents a cycle count.
"""
import argparse
import json
import os
import sys
import tempfile

CATALOG = os.path.join(os.path.dirname(__file__), "catalog.json")


def load_catalog():
    with open(CATALOG, encoding="utf-8") as f:
        return json.load(f)


def cell(text):
    return "" if text is None or text == "" else str(text)


def agree(measured, nak_stall, enc_stall):
    if measured is None or nak_stall is None:
        return ""
    if measured == nak_stall and measured == enc_stall:
        return "NAK and nvcc"
    if measured == nak_stall:
        return "NAK"
    if measured == enc_stall:
        return "nvcc"
    return "neither"


def runs_text(case, runs):
    if not case:
        return ""
    parts = ["%d runs" % runs]
    if case.get("stall_required") is None:
        parts.append("no failing stall in the sweep")
    else:
        parts.append("stall spread %s" % case.get("stall_spread"))
        if case.get("cycle_median") is not None:
            parts.append("cycles at that stall median %s spread %s" % (case["cycle_median"], case.get("cycle_spread")))
    if case.get("cycle_median_stall1") is not None:
        parts.append("stall-1 cycles median %s spread %s" % (case["cycle_median_stall1"], case.get("cycle_spread_stall1")))
    if case.get("overhead_cycle_median") is not None:
        parts.append("loop overhead median %s" % case["overhead_cycle_median"])
    if case.get("stall_required") is None and case.get("cycle_median_stall1") is None and case.get("overhead_cycle_median") is None:
        return ""
    return "; ".join(parts)


def lookup(results, measure_id):
    if not measure_id:
        return None
    cases = results.get("cases") or {}
    return cases.get(measure_id)


def build_md(cat, results, fake):
    results = results or {}
    measured_flag = bool(results.get("measured")) and not fake
    runs = results.get("runs") or 0
    driver = results.get("driver") or ""
    commit = cat["mesa_commit"]
    lines = []
    lines.append("# GB10 (sm_121) measured latencies vs NAK")
    lines.append("")
    lines.append("Chip: NVIDIA GB10 (DGX Spark), sm_121")
    if driver:
        lines.append("Driver: %s. No NVIDIA user-space libraries." % driver)
    else:
        lines.append("Driver: NVIDIA open kernel module <version>, no NVIDIA user-space libraries")
    lines.append("Method: dependent-chain timing kernel. One warp (32 threads); every lane stores, and a stall matches only when every lane matches the oracle.")
    lines.append("Each fixed-latency producer is issued at a programmed stall. The GB10 column is the smallest stall S such that S and every larger S in the sweep matched the oracle on every run. Cycles are the CS2R clock delta (NAK_SV_CLOCK_LO, 0x50) around that chain, median and max-min.")
    lines.append("Source: tests/gpu_latmeas_test.c in aien-dev/CUDApp")
    lines.append("How to rerun: sh tools/latmeas/run_spark.sh OUTPUT_DIR")
    lines.append("")
    lines.append("NAK reference: `nak/sm120_instr_latencies.rs` plus `latencies/sm100/*.csv` at Mesa commit %s" % commit)
    if fake:
        lines.append("")
        lines.append("FAKE: this table was rendered from synthetic input. It is not a measurement.")
    elif not measured_flag:
        lines.append("")
        lines.append("NOT RUN: no chip measurement is recorded in this file. Empty cells are empty on purpose.")
    lines.append("")
    lines.append("## Priority 1: values CUDA++ currently borrows from NAK (needs independent measurement)")
    lines.append("")
    lines.append("| Writer -> reader | NAK value | NAK source line | GB10 measured | Delta | Runs / spread | Kernel |")
    lines.append("|---|---|---|---|---|---|---|")
    notes = []
    for row in cat["rows"]:
        if row["priority"] != 1:
            continue
        case = lookup(results, row["measure_id"]) if measured_flag else None
        stall = None if fake or not case else case.get("stall_required")
        delta = ""
        shown = "FAKE" if fake else cell(stall)
        if not fake and stall is not None and row["nak_stall"] is not None:
            delta = str(stall - row["nak_stall"])
        spread = "FAKE" if fake else runs_text(case, runs)
        lines.append("| %s | %s | %s | %s | %s | %s | %s |" % (
            row["pair"], row["nak"], row["nak_where"], shown, delta, spread, row["kernel"]))
        if case and case.get("note"):
            notes.append("%s: %s" % (row["id"], case["note"]))
    lines.append("")
    lines.append("## Priority 2: instructions where CUDA++ (nvcc-derived) and NAK disagree")
    lines.append("")
    lines.append("| Instruction / pair | NAK value | nvcc sm_121 stall | GB10 measured | Agrees with | Kernel |")
    lines.append("|---|---|---|---|---|---|")
    for row in cat["rows"]:
        if row["priority"] != 2:
            continue
        case = lookup(results, row["measure_id"]) if measured_flag else None
        stall = None if fake or not case else case.get("stall_required")
        shown = "FAKE" if fake else cell(stall)
        who = "FAKE" if fake else agree(stall, row["nak_stall"], row["encoder_stall"])
        lines.append("| %s | %s | %s | %s | %s | %s |" % (
            row["pair"], row["nak"], row["encoder"], shown, who, row["kernel"]))
        if case and case.get("note"):
            notes.append("%s: %s" % (row["id"], case["note"]))
    lines.append("")
    lines.append("## Notes")
    lines.append("- Anything not fixed-latency (LDG/LDS, scoreboard waits) is out of scope for this table.")
    lines.append("- Decoupled ops (BSSY, BSYNC, BRA, EXIT, BAR) are listed because the encoder gives them a fixed stall and NAK does not. They have no dependent-chain kernel.")
    lines.append("- HMMA -> HMMA in NAK is 20 + 9 = 29, the same total the encoder uses. The HMMA row is the non-HMMA reader (28).")
    lines.append("- QMMA is not generated here. Its encoder stall is 30 and NAK's IMMA -> ALU total is 26 (IMMA -> IMMA is 27).")
    err = (results or {}).get("error") or ""
    if err:
        notes.append("runner: %s" % err)
    if notes:
        for n in notes:
            lines.append("- %s" % n)
    else:
        lines.append("- Confidence notes and anomalies go here.")
    lines.append("")
    return "\n".join(lines)


def write_or_refuse(results, dest, allow_synthetic):
    fake = bool(results.get("synthetic"))
    if fake and not allow_synthetic:
        print("refusing synthetic input (pass --allow-synthetic to render a FAKE table)", file=sys.stderr)
        return 2
    text = build_md(load_catalog(), results, fake)
    if fake and "FAKE" not in text:
        print("synthetic render did not stamp FAKE", file=sys.stderr)
        return 1
    if dest:
        with open(dest, "w", encoding="utf-8") as f:
            f.write(text)
    else:
        sys.stdout.write(text)
    return 0


def self_test():
    cat = load_catalog()
    synthetic = {
        "synthetic": True,
        "measured": True,
        "runs": 11,
        "driver": "synthetic",
        "cases": {"alu_alu": {"stall_required": 99, "stall_spread": 0, "cycle_median": 99, "cycle_spread": 1,
                              "overhead_cycle_median": 1, "note": ""}},
    }
    rc = write_or_refuse(synthetic, None, False)
    if rc != 2:
        print("FAIL: synthetic input was accepted", file=sys.stderr)
        return 1
    fd, path = tempfile.mkstemp(prefix="latmeas-fake-", suffix=".md")
    os.close(fd)
    try:
        rc = write_or_refuse(synthetic, path, True)
        text = open(path, encoding="utf-8").read()
    finally:
        os.unlink(path)
    if rc != 0 or "FAKE" not in text or "99" in text:
        print("FAIL: synthetic table leaked a number or missed FAKE", file=sys.stderr)
        return 1
    blank = {"synthetic": False, "measured": True, "runs": 11, "cases": {"alu_alu": {"stall_required": None}}}
    text = build_md(cat, blank, False)
    # The ALU row's measured cell is between the NAK citation and Delta. It must be empty.
    alu = [ln for ln in text.splitlines() if ln.startswith("| ALU -> ALU")]
    if len(alu) != 1 or "99" in text:
        print("FAIL: blank measured row missing", file=sys.stderr)
        return 1
    parts = [p.strip() for p in alu[0].strip("|").split("|")]
    if parts[3] != "":
        print("FAIL: null stall rendered as %r" % parts[3], file=sys.stderr)
        return 1
    filled = {"synthetic": False, "measured": False, "runs": 11,
              "cases": {"alu_alu": {"stall_required": 5, "stall_spread": 0}}}
    text = build_md(cat, filled, False)
    alu = [ln for ln in text.splitlines() if ln.startswith("| ALU -> ALU")][0]
    parts = [p.strip() for p in alu.strip("|").split("|")]
    if parts[3] != "":
        print("FAIL: measured:false still filled a cell (%r)" % parts[3], file=sys.stderr)
        return 1
    timed = {"synthetic": False, "measured": True, "runs": 11, "cases": {
        "alu_alu": {"stall_required": None, "cycle_median_stall1": 1703, "cycle_spread_stall1": 4,
                    "overhead_cycle_median": 800}}}
    text = build_md(cat, timed, False)
    alu = [ln for ln in text.splitlines() if ln.startswith("| ALU -> ALU")][0]
    parts = [p.strip() for p in alu.strip("|").split("|")]
    if parts[3] != "" or "1703" not in parts[5]:
        print("FAIL: stall-1 cycles landed in the measured cell or missed the runs cell: %r" % parts, file=sys.stderr)
        return 1
    print("fill_template self-test PASS")
    return 0


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--results")
    p.add_argument("--md")
    p.add_argument("--allow-synthetic", action="store_true")
    p.add_argument("--self-test", action="store_true")
    args = p.parse_args()
    if args.self_test:
        return self_test()
    if not args.results:
        p.error("--results is required")
    with open(args.results, encoding="utf-8") as f:
        results = json.load(f)
    return write_or_refuse(results, args.md, args.allow_synthetic)


if __name__ == "__main__":
    sys.exit(main())
