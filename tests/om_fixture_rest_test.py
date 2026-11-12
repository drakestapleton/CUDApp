#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host-only acceptance. Invoke inside the mandatory no-device bwrap wrapper."""
import os
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent.parent
os.chdir(root)
out = root / "build/fixture-rest"
out.mkdir(parents=True, exist_ok=True)
cc = ["gcc", "-std=gnu11", "-O2", "-Wall", "-Wextra", "-Werror", "-ffp-contract=off", "-Isrc"]
unit = "src/cudapp_om_eng.c"
text = "src/cudapp_om_fixture_text.c"
test = "tests/om_fixture_rest_test.c"

def run(args, name, expected=0, cwd=None):
    p = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd)
    (out / (name + ".log")).write_bytes(p.stdout)
    print(f"{name}: rc={p.returncode}", flush=True)
    if p.returncode != expected:
        print(p.stdout.decode(errors="replace"))
        raise SystemExit(1)
    return p.stdout

# Strong references: omission must fail to link, not substitute a C answer.
p = subprocess.run(cc + [test, "-o", str(out / "missing")], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
(out / "missing.log").write_bytes(p.stdout)
assert p.returncode != 0 and b"undefined reference" in p.stdout
print("PASS link-failure negative", flush=True)
run(cc + [test, unit, text, "-o", str(out / "omega")], "build-omega")
run(cc + ["-DCUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST=1", "-DCUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS=1", test, "-o", str(out / "oracle")], "build-oracle")
expected = run([str(out / "oracle")], "oracle")
actual = run([str(out / "omega")], "omega")
assert actual == expected, "host differential mismatch"
print("PASS host differential: scalars, overflow edges, flush words, f64 scaling, bounded text", flush=True)
for flag in ["CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST", "CUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS"]:
    run(cc + ["-D"+flag+"=1", test, unit, text, "-o", str(out/flag)], "build-"+flag)
    assert run([str(out/flag)], flag) == expected, flag+" independent oracle mismatch"
run(["gcc", "-std=gnu11", "-O0", "--coverage", "-Isrc",
     "-DCUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST=1",
     "-DCUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS=1", test, "-o", str(out/"coverage")], "build-coverage")
run([str(out/"coverage")], "coverage")
# gcov writes its .gcov files into the current directory: run it in the build directory, not the checkout
run(["gcov", "-b", "-c", "-o", str(out), str(out/"coverage-om_fixture_rest_test.gcno")], "gcov", cwd=out)

# Fresh mutant copies only; never modify production source or generated artifacts.
source = (root / "om/engine/fixture_rest.om").read_text()
eng = (root / "om/engine/eng.om").read_text()
mutants = [
    ("ns_scale", "mul a 1000000000", "mul a 1000000001"),
    ("ms_divisor", "div b 1000000", "div b 1000001"),
    ("size_cycle", "ix = rem a 8", "ix = rem a 7"),
    ("count_wrap", "ret and add mul b 2 3 4294967295", "ret add mul b 2 3"),
    ("flush_offset", "ret and add va 16 4294967295", "ret and add va 17 4294967295"),
    ("float_scale", "1073741824.0", "1073741825.0"),
    ("text_equal", "if same", "if not same"),
]
bridge = Path(os.environ.get("OMEGA_BRIDGE", str(Path.home()/"work/cudastack/omega-bridge")))
env = dict(os.environ, OMEGA_SEEDED_OMC=os.environ.get("OMEGA_OMC", str(Path.home()/"work/cudastack/omega-mv/seed/out/omc")))
for name, old, new in mutants:
    assert source.count(old) == 1, (name, "site count")
    changed = source.replace(old,new,1)
    mdir = out/name
    mdir.mkdir(exist_ok=True)
    inputs = [("eng", changed+"\n"+eng), ("text", changed)]
    cs = []
    for kind, body in inputs:
        om = mdir/(kind+".om")
        om.write_text(body)
        p = subprocess.run(["sh", str(bridge/"tools/bridge-seeded"), "c", str(om)], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (mdir/(kind+"-gen.log")).write_bytes(p.stderr)
        assert p.returncode == 0, (name,p.stderr.decode())
        c = mdir/(kind+".c")
        c.write_bytes(p.stdout)
        cs.append(str(c))
    run(cc + [test] + cs + ["-o", str(mdir/"test")], name+"-build")
    p = subprocess.run([str(mdir/"test")], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (mdir/"actual.txt").write_bytes(p.stdout)
    assert p.returncode != 0 or p.stdout != expected, name+" SURVIVED"
    print("KILLED " + name, flush=True)
print("PASS red control: ns_scale changed a real unit answer", flush=True)

# Hardware fixtures are linked only; never executed, even in the device-free namespace.
tp = "third_party/nvidia-open-580.173.02"
inc = ["-Invrm", "-Im16", "-I"+tp+"/src/common/sdk/nvidia/inc", "-I"+tp+"/kernel-open/common/inc", "-I"+tp+"/kernel-open/nvidia-uvm", "-I"+tp+"/src/nvidia/arch/nvalloc/unix/include"]
common = ["nvrm/nvrm.c", "m16/m16_native.c", "src/omega_ampere_qmd.c", "src/cudapp_om_amp_runtime.c", "src/cudapp_om_amp_qmd.c", unit, text]
# Match the integrated native NVRM link: serializers, transport and chip admission.
common += [str(p.relative_to(root)) for p in sorted((root / "src").glob("cudapp_om_nvrm_*.c"))]
common += ["src/omega_gpu_chip.c", "src/cudapp_om_chip_admit.c", "src/cudapp_om_chip_enum.c"]
for path in ["m16/m16_requalify.c", "m16/m16_concurrent.c", "nvrm/lifecycle_gates.c", "nvrm/submission_visibility_gates.c"]:
    extra = ["m16/m16_gpu_wait.c"] if path.endswith("m16_concurrent.c") else []
    name = Path(path).stem
    run(cc + inc + common + extra + [path,"-o",str(out/name)], "build-only-"+name)
print("PASS fixtures BUILD_ONLY; HW NOT_RUN; mutants 7/7 KILLED", flush=True)
