#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Apple GPU replay of the portable-backend contract kernels (ADR 0043: Apple oracle route = MoltenVK +
# SPIR-V; MSL_SOURCE stays PLANNED). The SPIR-V modules, inputs and bit-exact references are produced
# on Linux AArch64 by tests/multitarget/run.sh (seeded Omega: backend/spirv/emit.om, conform.om); this
# script never generates or edits a module. It builds the migration-pending launcher
# tools/multitarget/spirv-host.c with MT_TARGET=1 (MoltenVK linked directly, no Vulkan loader), runs every
# module on the Apple GPU and compares the output bytes with the references. The Omega comparison
# (conform check) is run on the Linux side over the returned outputs.
#
# usage: apple-gpu.sh DIR
#   DIR holds the run.sh outputs vadd.vk.spv vadd.vk.mutant.spv fsum16.vk.spv fsum16.vk.mutant.spv
#   gemm16.vk.spv gemm16.vk.mutant.spv a.bin b.bin x.bin ga.bin gb.bin vadd.ref fsum16.ref gemm16.ref
#   and SHA256SUMS over them (written by the producer).
# Environment (absolute paths): MVK_LIB (libMoltenVK.dylib), MVK_INCLUDE (MoltenVK include dir with
#   vulkan/vulkan.h), MT_CC (default cc), MT_EXPECT_DEVICE (default "Apple M2").
# Outputs stay in DIR: *.gpu.out, run logs, identity.txt.
# Exit: 0 APPLE-GPU PASS; 1 FAIL; 64 usage; 77 NOT_RUN (not macOS arm64 or MoltenVK missing).
set -u
LC_ALL=C
export LC_ALL
if [ "$(uname -s)" != Darwin ] || [ "$(uname -m)" != arm64 ]; then
  echo 'NOT_RUN: apple-gpu.sh needs macOS on Apple silicon' >&2; exit 77
fi
[ "$#" -eq 1 ] || { echo 'usage: apple-gpu.sh DIR' >&2; exit 64; }
D=$1
case $D in /*) ;; *) echo 'usage: DIR must be absolute' >&2; exit 64 ;; esac
lib=${MVK_LIB:-}; inc=${MVK_INCLUDE:-}; cc=${MT_CC:-cc}; dev=${MT_EXPECT_DEVICE:-Apple M2}
case $lib in /*libMoltenVK.dylib) ;; *) echo 'NOT_RUN: MVK_LIB must be the absolute path of libMoltenVK.dylib' >&2; exit 77 ;; esac
[ -f "$lib" ] && [ -f "$inc/vulkan/vulkan.h" ] || { echo 'NOT_RUN: MoltenVK library or headers missing' >&2; exit 77; }
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 1
npass=0; nfail=0
ok() { npass=$((npass + 1)); echo "PASS $*"; }
bad() { nfail=$((nfail + 1)); echo "FAIL $*"; }
digest() { shasum -a 256 "$1" | cut -d ' ' -f 1; }

(cd "$D" && shasum -a 256 -c SHA256SUMS) > "$D/inputs.check" 2>&1 && ok "inputs match producer SHA256SUMS ($(grep -c ': OK$' "$D/inputs.check") files)" \
  || { cat "$D/inputs.check"; bad "inputs differ from producer SHA256SUMS"; }
{
  echo "DATE $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "OS $(sw_vers -productName) $(sw_vers -productVersion) build $(sw_vers -buildVersion); $(uname -srm)"
  echo "CPU $(sysctl -n machdep.cpu.brand_string) cores=$(sysctl -n hw.ncpu) perf=$(sysctl -n hw.perflevel0.physicalcpu 2>/dev/null) eff=$(sysctl -n hw.perflevel1.physicalcpu 2>/dev/null) mem=$(sysctl -n hw.memsize)"
  echo "MODEL $(sysctl -n hw.model)"
  system_profiler SPDisplaysDataType 2>/dev/null | grep -E '^ *(Chipset Model|Type|Total Number of Cores|Vendor|Metal Support): ' | sed 's/^ */GPU /'
  echo "CC $($cc --version 2>&1 | head -n 1)"
  echo "SDK $(xcrun --show-sdk-version 2>/dev/null)"
  echo "MOLTENVK $lib sha256=$(digest "$lib")"
  echo "LAUNCHER_SRC $root/tools/multitarget/spirv-host.c sha256=$(digest "$root/tools/multitarget/spirv-host.c")"
} > "$D/identity.txt"
cat "$D/identity.txt"

# build NAME [extra -D flags...]
build() {
  name=$1; shift
  "$cc" -std=c11 -O2 -Wall -Wextra -Werror -DCUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 -DMT_TARGET=1 \
    -DMT_MVK_LIBRARY="\"$lib\"" -DMT_EXPECT_DEVICE="\"$dev\"" "$@" -I"$inc" \
    "$root/tools/multitarget/spirv-host.c" -o "$D/$name" "$lib" -Wl,-rpath,"$(dirname "$lib")" > "$D/$name.build.log" 2>&1
}
if ! build spirv-host-mvk; then cat "$D/spirv-host-mvk.build.log"; bad "launcher build"; echo "APPLE-GPU FAIL"; exit 1; fi
H=$D/spirv-host-mvk
echo "LAUNCHER $H sha256=$(digest "$H")"

run() { # TAG HOST KERNEL COUNT IN... -- writes $D/TAG.gpu.out, log $D/run-TAG.log, returns host exit
  tag=$1; host=$2; k=$3; n=$4; shift 4
  "$host" vk "$D/$tag.spv" "$k" "$n" "$@" "$D/$tag.gpu.out" > "$D/run-$tag.log" 2>&1
}
for k in vadd fsum16 gemm16; do
  case $k in vadd) n=1048576; set -- "$D/a.bin" "$D/b.bin" ;; fsum16) n=65536; set -- "$D/x.bin" ;; gemm16) n=65536; set -- "$D/ga.bin" "$D/gb.bin" ;; esac
  for m in "" .mutant; do
    tag=$k.vk$m
    if run "$tag" "$H" "$k" "$n" "$@"; then :; else bad "run $tag exit $?"; continue; fi
    if cmp -s "$D/$tag.gpu.out" "$D/$k.ref"; then got=match; else got=mismatch; fi
    if [ -z "$m" ]; then
      [ $got = match ] && ok "bytes $tag.gpu.out = $k.ref sha256=$(digest "$D/$k.ref")" || bad "bytes $tag.gpu.out != $k.ref"
    else
      [ $got = mismatch ] && ok "negative $tag differs from $k.ref (planted wrong kernel detected)" || bad "mutant $tag matched the reference"
    fi
  done
  # determinism: a second dispatch of the real module must give the same bytes
  cp "$D/$k.vk.gpu.out" "$D/$k.vk.gpu.first"
  if run "$k.vk" "$H" "$k" "$n" "$@" && cmp -s "$D/$k.vk.gpu.out" "$D/$k.vk.gpu.first"; then ok "repeat $k.vk identical bytes"; else bad "repeat $k.vk differs or failed"; fi
  rm -f "$D/$k.vk.gpu.first"
done
grep -h '^spirv-host: vk \(target\|device\|limits\|portability\)' "$D/run-vadd.vk.log"
grep -h '^spirv-host: isolation' "$D/run-vadd.vk.log"

# Controls: each must refuse with the named exit and write no output.
ctl() { # WANT NAME HOST ARGS...
  want=$1; name=$2; host=$3; shift 3
  rm -f "$D/ctl.out"
  "$host" "$@" "$D/ctl.out" > "$D/ctl-$name.log" 2>&1
  got=$?
  if [ "$got" -eq "$want" ] && [ ! -e "$D/ctl.out" ]; then ok "control $name exit=$got $(tail -n 1 "$D/ctl-$name.log")"
  else bad "control $name exit=$got want=$want output=$([ -e "$D/ctl.out" ] && echo written || echo none)"; fi
}
ctl 2 cl-refused "$H" cl "$D/vadd.vk.spv" vadd 1048576 "$D/a.bin" "$D/b.bin"
build spirv-host-mvk-forbid -DMT_FORBID='"libMoltenVK"' && \
  ctl 9 isolation-forbid-moltenvk "$D/spirv-host-mvk-forbid" vk "$D/vadd.vk.spv" vadd 1048576 "$D/a.bin" "$D/b.bin" || bad "forbid build"
build spirv-host-mvk-wrongdev -UMT_EXPECT_DEVICE -DMT_EXPECT_DEVICE='"Apple M9 Ultra"' && \
  ctl 5 wrong-device "$D/spirv-host-mvk-wrongdev" vk "$D/vadd.vk.spv" vadd 1048576 "$D/a.bin" "$D/b.bin" || bad "wrong-device build"
cp "$D/vadd.vk.spv" "$D/vadd.vk.trunc.spv" && printf '\000\000' >> "$D/vadd.vk.trunc.spv"
ctl 4 module-missing "$H" vk "$D/no-such-module.spv" vadd 1048576 "$D/a.bin" "$D/b.bin"
ctl 4 module-not-words "$H" vk "$D/vadd.vk.trunc.spv" vadd 1048576 "$D/a.bin" "$D/b.bin"
# A missing entry point is refused by the Omega loader before any run (targets load SPIRV_VK FILE ENTRY,
# exit 23, run.sh load-neg-entry). Given one anyway, MoltenVK 1.4.2 aborts the process inside
# vkCreateComputePipelines (SIGABRT, exit 134) instead of returning an error: fail-closed, no output,
# but not a clean refusal. Recorded as a driver finding; the loader check is the contract's gate.
ctl 134 wrong-entry-driver-abort "$H" vk "$D/vadd.vk.spv" nosuch 1048576 "$D/a.bin" "$D/b.bin"
ctl 4 short-input "$H" vk "$D/vadd.vk.spv" vadd 1048577 "$D/a.bin" "$D/b.bin"

# Observation only (not a pass condition): the same modules with Metal fast math forced on.
if build spirv-host-mvk-fast -DMT_MVK_FAST_MATH='"1"'; then
  for k in vadd fsum16 gemm16; do
    case $k in vadd) n=1048576; set -- "$D/a.bin" "$D/b.bin" ;; fsum16) n=65536; set -- "$D/x.bin" ;; gemm16) n=65536; set -- "$D/ga.bin" "$D/gb.bin" ;; esac
    if run "$k.vk" "$D/spirv-host-mvk-fast" "$k" "$n" "$@"; then
      cp "$D/$k.vk.gpu.out" "$D/$k.vk.fastmath.out"
      if cmp -s "$D/$k.vk.fastmath.out" "$D/$k.ref"; then o=match; else o=mismatch; fi
      echo "OBSERVED fastmath=1 $k bytes $o reference (fast math is not the conformance mode)"
    else echo "OBSERVED fastmath=1 $k run failed"; fi
  done
  # restore the conformance outputs (fast-math runs reused the tag)
  run vadd.vk "$H" vadd 1048576 "$D/a.bin" "$D/b.bin"; run fsum16.vk "$H" fsum16 65536 "$D/x.bin"
  run gemm16.vk "$H" gemm16 65536 "$D/ga.bin" "$D/gb.bin"
  cmp -s "$D/vadd.vk.gpu.out" "$D/vadd.ref" && cmp -s "$D/fsum16.vk.gpu.out" "$D/fsum16.ref" \
    && cmp -s "$D/gemm16.vk.gpu.out" "$D/gemm16.ref" || bad "conformance outputs not restored"
fi
# Timing (observation only, DRA-201): MT_REPS fresh processes per kernel with the conformance build. Every
# repetition's output must still equal the reference. Same columns as nvidia-gpu.sh's timing.tsv.
reps=${MT_REPS:-10}
T=$D/timing.tsv
printf 'target\tkernel\trep\tsetup_us\tpipeline_us\trun_us\tdevice_ns\ttotal_us\tmaxrss_kib\tbuffers_bytes\n' > "$T"
field() { sed -n "s/^spirv-host: time .*$1=\([0-9A-Z_]*\).*/\1/p" "$2"; }
for k in vadd fsum16 gemm16; do
  case $k in vadd) n=1048576; set -- "$D/a.bin" "$D/b.bin" ;; fsum16) n=65536; set -- "$D/x.bin" ;; gemm16) n=65536; set -- "$D/ga.bin" "$D/gb.bin" ;; esac
  same=0; r=1
  while [ "$r" -le "$reps" ]; do
    if run "$k.vk" "$H" "$k" "$n" "$@" && cmp -s "$D/$k.vk.gpu.out" "$D/$k.ref"; then same=$((same + 1)); fi
    l=$D/run-$k.vk.log
    printf 'apple-moltenvk\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$k" "$r" "$(field setup_us "$l")" "$(field pipeline_us "$l")" \
      "$(field run_us "$l")" "$(field device_ns "$l")" "$(field total_us "$l")" "$(field maxrss_kib "$l")" "$(field buffers_bytes "$l")" >> "$T"
    r=$((r + 1))
  done
  [ "$same" -eq "$reps" ] && ok "timing reps $k: $same/$reps outputs = $k.ref" || bad "timing reps $k: only $same/$reps outputs = $k.ref"
done
for k in vadd fsum16 gemm16; do echo "OUTPUT $k.vk.gpu.out sha256=$(digest "$D/$k.vk.gpu.out")"; done
echo "route: SPIRV_VK -> MoltenVK -> Metal on the Apple GPU (vendor runtime); MSL_SOURCE, ANE and an independent Apple GPU driver are NOT_RUN"
if [ "$nfail" -ne 0 ]; then echo "APPLE-GPU FAIL pass=$npass fail=$nfail"; exit 1; fi
echo "APPLE-GPU PASS pass=$npass fail=0"
exit 0
