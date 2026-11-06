#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""CPU-only legacy/native-Omega differential and recompiled semantic controls."""
import argparse
import concurrent.futures
import hashlib
import gzip
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FAMILIES = ("attn256", "pattn256", "router", "gemm", "bf16", "qg")
EXPORTS = {'attn256': ['omega_dattn256_shape', 'omega_blackwell_codegen_decode_attn256_prog', 'omega_decode_attn256_prog_stats'], 'pattn256': ['omega_pattn256_shape', 'omega_pattn256_spec_ok', 'omega_blackwell_codegen_prefill_attn256_prog', 'omega_prefill_attn256_prog_stats'], 'router': ['omega_router_smem', 'omega_blackwell_codegen_router_prog', 'omega_router_prog_stats'], 'gemm': ['omega_fp8gemm_steps', 'omega_fp8gemm_shared_bytes', 'omega_blackwell_codegen_fp8gemm_prog', 'omega_fp8gemm_prog_stats'], 'bf16': ['omega_bf16_gemv_steps', 'omega_bf16_gemv_shared_bytes', 'omega_blackwell_codegen_bf16_gemv_prog', 'omega_bf16_gemv_prog_stats'], 'qg': ['omega_pq_spec_ok', 'omega_blackwell_codegen_pq_prog', 'omega_qg_spec_ok', 'omega_qg_shared_bytes', 'omega_qg_span', 'omega_blackwell_codegen_qgemm_prog']}
PIN = "2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--smoke", action="store_true")
    parser.add_argument("--mutants", action="store_true")
    parser.add_argument("--coverage", action="store_true", help="instrument retained C oracles with -O0 --coverage and report gcov arcs")
    parser.add_argument("--family", choices=FAMILIES, action="append")
    parser.add_argument("--build-dir", type=Path)
    args = parser.parse_args()
    families = args.family or FAMILIES
    omc = Path(os.environ.get("OMEGA_OMC", Path.home()/"work/cudastack/omega-mv/seed/out/omc"))
    if list(Path("/dev").glob("nvidia*")) or Path("/dev/cuse").exists() or Path("/dev/vfio").exists():
        raise SystemExit("FAIL run inside bwrap --dev-bind / / --dev /dev --proc /proc --")
    bridge = Path(os.environ.get("OMEGA_BRIDGE", Path.home()/"work/cudastack/omega-bridge"))
    if hashlib.sha256(omc.read_bytes()).hexdigest() != PIN:
        raise SystemExit("FAIL pinned seeded compiler SHA256")
    scratch = tempfile.TemporaryDirectory(prefix="cudapp-codegen-rest-")
    out = (args.build_dir or Path(scratch.name)).resolve()
    out.mkdir(parents=True, exist_ok=True)
    cc = os.environ.get("CC", "cc")
    # Keep the independent allocator/matmul prerequisites without selecting the
    # separate native NO_POSIX encoder ABI; this gate links the portable encoder.
    flags = ["-D_GNU_SOURCE", "-DCUDAPP_MIGRATION_PENDING_C_REGALLOC=1", "-DCUDAPP_MIGRATION_PENDING_C_CODEGEN_MATMUL=1", "-DCUDAPP_MIGRATION_PENDING_C_PACK=1", "-std=gnu11", "-O1", "-ffp-contract=off", "-ffunction-sections", "-fdata-sections", "-I"+str(ROOT/"src")]

    def run(cmd, expected=0, stdout=None, cwd=ROOT):
        r = subprocess.run([str(x) for x in cmd], cwd=cwd, text=True, stdout=stdout or subprocess.PIPE, stderr=subprocess.PIPE)
        text = (r.stdout or "") + (r.stderr or "")
        if r.returncode != expected:
            print(text, file=sys.stderr)
            raise RuntimeError("rc=%d command=%r" % (r.returncode, cmd))
        return text

    jobs = []
    for family in FAMILIES:
        src = ROOT/"src"/("omega_blackwell_codegen_"+family+".c")
        jobs.append((src, out/(family+".o"), []))
        defines = ["-DCUDAPP_MIGRATION_PENDING_C_CODEGEN_"+family.upper()+"=1"]
        defines += ["-D"+name+"=legacy_"+name for name in EXPORTS[family]]
        if family == "bf16":
            defines.append("-Domega_blackwell_codegen_bf16_gemv=legacy_omega_blackwell_codegen_bf16_gemv")
        jobs.append((src, out/(family+"-legacy.o"), defines))
        jobs.append((ROOT/"src"/("cudapp_om_cg_"+family+".c"), out/("cg_"+family+".o"), []))
    for name in ("omega_gpu_om_ir", "omega_blackwell_codegen", "cudapp_om_prefill_shape", "cudapp_om_prefill_order", "cudapp_om_bw_enc_sm121"):
        jobs.append((ROOT/"src"/(name+".c"), out/(name+".o"), []))
    for source in os.environ["CODEGEN_REST_BINDING"].split():
        source = ROOT / source
        jobs.append((source, out / (source.stem + ".o"), []))
    jobs.append((ROOT/"tests/gpu_codegen_rest_test.c", out/"test.o", []))

    def compile(job):
        source, obj, defines = job
        if args.coverage and obj.name.endswith("-legacy.o"):
            obj.with_suffix(".gcda").unlink(missing_ok=True)
            defines = [*defines, "-O0", "--coverage"]
        run([cc, *flags, *defines, "-c", source, "-o", obj])

    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        list(pool.map(compile, jobs))
    objects = [job[1] for job in jobs]
    binary = out/"differential"
    def link(target, override=None):
        objs = [override[1] if override and obj.name==override[0] else obj for obj in objects]
        run([cc, *objs, *(["--coverage"] if args.coverage else []), "-Wl,--gc-sections", "-o", target])
    link(binary)
    for family in families:
        print(run([binary, family, "--smoke" if args.smoke else "--full"]), end="", flush=True)
        r = subprocess.run([str(binary), family, "--smoke", "--record-flip"], text=True, capture_output=True)
        if r.returncode != 1 or "IR mismatch" not in r.stderr:
            raise RuntimeError("FAIL record flip not detected: "+family+" "+r.stdout+r.stderr)
        print("PASS record-flip caught "+family, flush=True)
    if args.mutants:
        common = (ROOT/"om/codegen/cg_rest_emit.om").read_text()
        for family in families:
            parts = [common]
            if family=="qg":
                parts += [(ROOT/"om/prefill-shape/admit.om").read_text(), (ROOT/"om/prefill-shape/order.om").read_text()]
            parts += [(ROOT/"om/codegen"/("cg_"+family+".om")).read_text()]
            source = "\n".join(parts)
            mutants = [("fixed-stall", "cc = cw 6 7 7 wt", "cc = cw 5 7 7 wt")]
            if family=="qg":
                mutants.append(("ignore-meta6", "put m 6 ulong 246", "put m 6 ulong 0"))
            for name, old, new in mutants:
                if source.count(old)!=1:
                    raise RuntimeError("ambiguous mutant "+family+" "+name)
                mutated = out/(family+"-"+name+".om")
                mutated.write_text(source.replace(old,new))
                generated = out/(family+"-"+name+".c")
                env = dict(os.environ, OMEGA_SEEDED_OMC=str(omc))
                with generated.open("w") as stream:
                    r = subprocess.run(["sh",str(bridge/"tools/bridge-seeded"),"c",str(mutated)], env=env, stdout=stream, stderr=subprocess.PIPE, text=True)
                if r.returncode:
                    raise RuntimeError("mutant did not compile: "+r.stderr)
                obj = generated.with_suffix(".o")
                compile((generated,obj,[]))
                mutantbin=out/(family+"-"+name)
                link(mutantbin,("cg_"+family+".o",obj))
                r=subprocess.run([str(mutantbin),family,"--smoke"],text=True,capture_output=True)
                if r.returncode!=1 or "mismatch" not in r.stderr:
                    raise RuntimeError("mutant survived: "+family+" "+name+" "+r.stdout+r.stderr)
                print("PASS recompiled Omega mutant killed "+family+"/"+name+": "+r.stderr.splitlines()[0],flush=True)
    if args.coverage:
        reports = []
        for family in families:
            obj = out/(family+"-legacy.gcno")
            run(["gcov", "--json-format", "--branch-counts", "--branch-probabilities", obj], cwd=out)
            packed = out/(family+"-legacy.gcov.json.gz")
            report = json.loads(gzip.decompress(packed.read_bytes()))
            source = ROOT/"src"/("omega_blackwell_codegen_"+family+".c")
            data = next(item for item in report["files"] if Path(item["file"]).resolve()==source)
            lines = data["lines"]
            arcs = [arc for line in lines for arc in line.get("branches",[])]
            entry = {"file":str(source.relative_to(ROOT)), "compiler":report["gcc_version"],
                     "lines_covered":sum(line["count"]>0 for line in lines), "lines_total":len(lines),
                     "branches_covered":sum(arc["count"]>0 for arc in arcs), "branches_total":len(arcs),
                     "uncovered":[{"line":line["line_number"], "function":line.get("function_name"),
                                    "branches":[index for index,arc in enumerate(line.get("branches",[])) if not arc["count"]],
                                    "line_covered":bool(line["count"])}
                                   for line in lines if not line["count"] or any(not arc["count"] for arc in line.get("branches",[]))],
                     "data":data}
            reports.append(entry)
            print("COVERAGE %s lines=%d/%d branches=%d/%d" % (family,entry["lines_covered"],entry["lines_total"],entry["branches_covered"],entry["branches_total"]),flush=True)
        (out/"coverage.json").write_text(json.dumps(reports,indent=2)+"\n")
    print("PASS codegen-rest CPU-only differential controls")


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, OSError) as error:
        print(str(error), file=sys.stderr)
        sys.exit(1)
