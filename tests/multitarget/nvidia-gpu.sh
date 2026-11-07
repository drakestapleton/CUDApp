#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# NVIDIA GPU replay of the portable-backend contract kernels (DRA-201: one unchanged application on an
# NVIDIA and a non-NVIDIA GPU). The SPIR-V modules, inputs and bit-exact references are the ones
# tests/multitarget/run.sh produces from seeded Omega (backend/spirv/emit.om, conform.om), the same files
# apple-gpu.sh runs on the Apple GPU; this script never generates or edits a module. It builds the
# migration-pending launcher tools/multitarget/spirv-host.c with MT_TARGET=2 (NVIDIA's Vulkan driver, the
# loader pinned to its one manifest), runs every module on the GB10 and compares the output bytes with the
# references, with Omega's own compare (conform check) and, when given, with another target's outputs.
# The route is NVIDIA's proprietary Vulkan runtime and SPIR-V compiler, not CUDA++: CUDA++'s own sm_121
# path for the same vadd is examples/hello_gpu (HELLO_GPU below, the native baseline).
#
# usage: nvidia-gpu.sh DIR
#   DIR holds the run.sh outputs vadd.vk.spv vadd.vk.mutant.spv fsum16.vk.spv fsum16.vk.mutant.spv
#   gemm16.vk.spv gemm16.vk.mutant.spv a.bin b.bin x.bin ga.bin gb.bin vadd.ref fsum16.ref gemm16.ref
#   and SHA256SUMS over them (written by the producer).
# Environment (absolute paths):
#   MT_GPU_WINDOW  id of the exclusive spark-window lease this runs in (required: no GPU outside a window)
#   MT_CFLAGS      extra -I for vulkan/vulkan.h        MT_CC (default cc)
#   MT_NV_ICD      NVIDIA's Vulkan manifest (default /usr/share/vulkan/icd.d/nvidia_icd.json)
#   MT_LVP_ICD     lavapipe manifest for the CPU-fallback refusal control (default /usr/share/vulkan/icd.d/lvp_icd.json)
#   CONFORM        run.sh's compiled conform.om (Omega bit compare of every output; unset: those rows NOT_RUN)
#   MT_COMPARE_DIR another target's outputs (<k>.vk.gpu.out and <k>.vk.mutant.gpu.out, e.g. the Apple M2's);
#                  every NVIDIA output must equal it byte for byte (unset: NOT_RUN)
#   MT_REPS        timing repetitions per kernel (default 10; observation only, never a pass condition)
#   HELLO_GPU, HELLO_GPU_CONTROL  CUDA++ build/hello_gpu and build/hello_gpu_control (unset: NOT_RUN)
# Outputs stay in DIR: *.gpu.out, run logs, identity.txt, timing.tsv.
# Exit: 0 NVIDIA-GPU PASS (NOT_RUN rows listed); 1 FAIL; 64 usage; 77 NOT_RUN (not Linux AArch64 with the
# NVIDIA driver, or no window id).
set -u
LC_ALL=C
export LC_ALL
if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ] || [ ! -r /proc/driver/nvidia/version ]; then
  echo 'NOT_RUN: nvidia-gpu.sh needs Linux AArch64 with the NVIDIA kernel driver (DGX Spark GB10)' >&2; exit 77
fi
[ "$#" -eq 1 ] || { echo 'usage: nvidia-gpu.sh DIR' >&2; exit 64; }
D=$1
case $D in /*) ;; *) echo 'usage: DIR must be absolute' >&2; exit 64 ;; esac
[ -n "${MT_GPU_WINDOW:-}" ] || { echo 'NOT_RUN: MT_GPU_WINDOW is not set (run only inside an exclusive spark-window lease)' >&2; exit 77; }
cc=${MT_CC:-cc}; icd=${MT_NV_ICD:-/usr/share/vulkan/icd.d/nvidia_icd.json}; lvp=${MT_LVP_ICD:-/usr/share/vulkan/icd.d/lvp_icd.json}
reps=${MT_REPS:-10}
[ -f "$icd" ] || { echo "NOT_RUN: NVIDIA Vulkan manifest $icd missing" >&2; exit 77; }
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 1
npass=0; nfail=0; nnr=0
ok() { npass=$((npass + 1)); echo "PASS $*"; }
bad() { nfail=$((nfail + 1)); echo "FAIL $*"; }
notrun() { nnr=$((nnr + 1)); echo "NOT_RUN $*"; }
digest() { d=$(sha256sum < "$1") || return 1; printf '%s\n' "${d%% *}"; }
lib_digest() { p=$(readlink -f "$1" 2>/dev/null) && [ -f "$p" ] && echo "$p sha256=$(digest "$p")" || echo "$1 missing"; }

(cd "$D" && sha256sum -c SHA256SUMS) > "$D/inputs.check" 2>&1 && ok "inputs match producer SHA256SUMS ($(grep -c ': OK$' "$D/inputs.check") files)" \
  || { cat "$D/inputs.check"; bad "inputs differ from producer SHA256SUMS"; }
L=/usr/lib/aarch64-linux-gnu
{
  echo "DATE $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "WINDOW $MT_GPU_WINDOW"
  echo "HOST $(hostname) $(uname -srm)"
  echo "OS $(. /etc/os-release && echo "$PRETTY_NAME")"
  echo "CPU $(grep -m1 -i 'model name\|^CPU part' /proc/cpuinfo | sed 's/.*: //') cores=$(nproc --all) mem_kib=$(sed -n 's/^MemTotal: *//p' /proc/meminfo)"
  echo "NVIDIA_KERNEL_DRIVER $(head -n 1 /proc/driver/nvidia/version) module=$(cat /sys/module/nvidia/version 2>/dev/null)"
  echo "ICD $icd sha256=$(digest "$icd") library=$(sed -n 's/.*"library_path" *: *"\([^"]*\)".*/\1/p' "$icd")"
  for f in libGLX_nvidia.so.0 libnvidia-glcore.so.580.173.02 libnvidia-glvkspirv.so.580.173.02 libnvidia-gpucomp.so.580.173.02 libvulkan.so.1; do
    echo "LIB $(lib_digest "$L/$f")"
  done
  for f in /lib/firmware/nvidia/580.173.02/gsp_ga10x.bin /lib/firmware/nvidia/580.173.02/gsp_tu10x.bin; do
    [ -f "$f" ] && echo "FIRMWARE $f sha256=$(digest "$f")"
  done
  echo "CC $($cc --version 2>&1 | head -n 1)"
  echo "LAUNCHER_SRC $root/tools/multitarget/spirv-host.c sha256=$(digest "$root/tools/multitarget/spirv-host.c")"
} > "$D/identity.txt"
cat "$D/identity.txt"
# The driver's default shader cache (used only if the launcher's pin failed): its listing must not change
# during this script.
defcache=${XDG_CACHE_HOME:-$HOME/.cache}/nvidia
deflist() { find "$defcache" -type f -printf '%p %s %T@\n' 2>/dev/null | sort; }
deflist > "$D/default-cache.before"

# build NAME [extra -D flags...]
build() {
  name=$1; shift
  # shellcheck disable=SC2086
  $cc -std=c11 -O2 -Wall -Wextra -Werror -DCUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 -DMT_TARGET=2 \
    -DMT_NV_ICD="\"$icd\"" "$@" ${MT_CFLAGS:-} "$root/tools/multitarget/spirv-host.c" -o "$D/$name" -l:libvulkan.so.1 \
    > "$D/$name.build.log" 2>&1
}
if ! build spirv-host-nv; then cat "$D/spirv-host-nv.build.log"; bad "launcher build"; echo "NVIDIA-GPU FAIL"; exit 1; fi
H=$D/spirv-host-nv
echo "LAUNCHER $H sha256=$(digest "$H")"

run() { # TAG HOST KERNEL COUNT IN... -- writes $D/TAG.gpu.out, log $D/run-TAG.log, returns host exit
  tag=$1; host=$2; k=$3; n=$4; shift 4
  "$host" vk "$D/$tag.spv" "$k" "$n" "$@" "$D/$tag.gpu.out" > "$D/run-$tag.log" 2>&1
}
args() { case $1 in vadd) n=1048576; set -- "$D/a.bin" "$D/b.bin" ;; fsum16) n=65536; set -- "$D/x.bin" ;; gemm16) n=65536; set -- "$D/ga.bin" "$D/gb.bin" ;; esac; echo "$n $*"; }
conform() { # WANT KERNEL FILE
  if [ -n "${CONFORM:-}" ] && [ -x "$CONFORM" ]; then
    "$CONFORM" check "$2" "$3" > "$D/conform-$(basename "$3").log" 2>&1
    got=$?
    if [ "$got" -eq "$1" ]; then ok "conform check $2 $(basename "$3") exit=$got $(tail -n 1 "$D/conform-$(basename "$3").log")"
    else bad "conform check $2 $(basename "$3") exit=$got want=$1 $(tail -n 1 "$D/conform-$(basename "$3").log")"; fi
  fi
}
[ -n "${CONFORM:-}" ] && [ -x "$CONFORM" ] || notrun "Omega conform check of the GPU outputs: CONFORM not set"
for k in vadd fsum16 gemm16; do
  # shellcheck disable=SC2046
  set -- $(args $k); n=$1; shift
  for m in "" .mutant; do
    tag=$k.vk$m
    if run "$tag" "$H" "$k" "$n" "$@"; then :; else bad "run $tag exit $?"; continue; fi
    if cmp -s "$D/$tag.gpu.out" "$D/$k.ref"; then got=match; else got=mismatch; fi
    if [ -z "$m" ]; then
      [ $got = match ] && ok "bytes $tag.gpu.out = $k.ref sha256=$(digest "$D/$k.ref")" || bad "bytes $tag.gpu.out != $k.ref"
      conform 0 "$k" "$D/$tag.gpu.out"
    else
      [ $got = mismatch ] && ok "negative $tag differs from $k.ref (planted wrong kernel detected)" || bad "mutant $tag matched the reference"
      conform 1 "$k" "$D/$tag.gpu.out"
    fi
  done
  cp "$D/$k.vk.gpu.out" "$D/$k.vk.gpu.first"
  if run "$k.vk" "$H" "$k" "$n" "$@" && cmp -s "$D/$k.vk.gpu.out" "$D/$k.vk.gpu.first"; then ok "repeat $k.vk identical bytes"; else bad "repeat $k.vk differs or failed"; fi
  rm -f "$D/$k.vk.gpu.first"
done
grep -h '^spirv-host: vk \(target\|device\|limits\)' "$D/run-vadd.vk.log"
grep -h '^spirv-host: isolation' "$D/run-vadd.vk.log"

# Cross-target: the same module bytes on another vendor's GPU must give the same output bytes.
if [ -n "${MT_COMPARE_DIR:-}" ] && [ -d "$MT_COMPARE_DIR" ]; then
  for k in vadd fsum16 gemm16; do for m in "" .mutant; do
    o=$MT_COMPARE_DIR/$k.vk$m.gpu.out
    if [ ! -f "$o" ]; then bad "cross-target $k.vk$m: $o missing"
    elif cmp -s "$D/$k.vk$m.gpu.out" "$o"; then ok "cross-target bytes $k.vk$m.gpu.out = $o sha256=$(digest "$o")"
    else bad "cross-target bytes $k.vk$m.gpu.out != $o"; fi
  done; done
else
  notrun "cross-target byte compare: MT_COMPARE_DIR not set"
fi

# Controls: each must refuse with the named exit and write no output.
ctl() { # WANT NAME HOST ARGS...  (WANT=any: any nonzero exit, fail-closed)
  want=$1; name=$2; host=$3; shift 3
  rm -f "$D/ctl.out"
  "$host" "$@" "$D/ctl.out" > "$D/ctl-$name.log" 2>&1
  got=$?
  if [ ! -e "$D/ctl.out" ] && { [ "$got" -eq "$want" ] 2>/dev/null || { [ "$want" = any ] && [ "$got" -ne 0 ]; }; }; then
    ok "control $name exit=$got $(tail -n 1 "$D/ctl-$name.log")"
  else bad "control $name exit=$got want=$want output=$([ -e "$D/ctl.out" ] && echo written || echo none)"; fi
}
V="vk $D/vadd.vk.spv vadd 1048576 $D/a.bin $D/b.bin"
# shellcheck disable=SC2086
ctl 2 cl-refused "$H" cl "$D/vadd.vk.spv" vadd 1048576 "$D/a.bin" "$D/b.bin"
# shellcheck disable=SC2086
build spirv-host-nv-forbid -DMT_FORBID='"libnvidia-glcore"' && ctl 9 isolation-forbid-driver "$D/spirv-host-nv-forbid" $V || bad "forbid build"
# shellcheck disable=SC2086
build spirv-host-nv-require -DMT_REQUIRE='"libnot-a-loaded-driver"' && ctl 9 isolation-require-missing "$D/spirv-host-nv-require" $V || bad "require build"
# shellcheck disable=SC2086
build spirv-host-nv-wrongdev -DMT_EXPECT_DEVICE='"NVIDIA H100"' && ctl 5 wrong-device "$D/spirv-host-nv-wrongdev" $V || bad "wrong-device build"
# A manifest that selects the CPU (lavapipe) must be refused before any run: no hidden CPU fallback.
# shellcheck disable=SC2086
build spirv-host-nv-cpu -UMT_NV_ICD -DMT_NV_ICD="\"$lvp\"" && ctl 5 cpu-fallback-refused "$D/spirv-host-nv-cpu" $V || bad "cpu-fallback build"
# shellcheck disable=SC2086
build spirv-host-nv-cachenopath -DMT_NV_DISK_CACHE='"1"' && ctl 3 cache-needs-private-path "$D/spirv-host-nv-cachenopath" $V || bad "cache-no-path build"
cp "$D/vadd.vk.spv" "$D/vadd.vk.trunc.spv" && printf '\000\000' >> "$D/vadd.vk.trunc.spv"
ctl 4 module-missing "$H" vk "$D/no-such-module.spv" vadd 1048576 "$D/a.bin" "$D/b.bin"
ctl 4 module-not-words "$H" vk "$D/vadd.vk.trunc.spv" vadd 1048576 "$D/a.bin" "$D/b.bin"
# A missing entry point is refused by the Omega loader before any run (run.sh load-neg-entry, exit 23).
# Given one anyway, the driver must fail closed (any nonzero exit, no output); the exit is recorded.
ctl any wrong-entry-driver "$H" vk "$D/vadd.vk.spv" nosuch 1048576 "$D/a.bin" "$D/b.bin"
ctl 4 short-input "$H" vk "$D/vadd.vk.spv" vadd 1048577 "$D/a.bin" "$D/b.bin"

# Cache lifecycle: the conformance build keeps the driver's on-disk shader cache off (a private path given
# to a cache-off build must stay empty); a cache-on build fills its private path, a warm run gives the same
# bytes, and corrupted cache entries never give wrong bytes (same bytes, or a fail-closed refusal).
C0=$D/nvcache-off; C1=$D/nvcache-on
rm -rf "$C0" "$C1"; mkdir -p "$C0" "$C1"
nfiles() { find "$1" -type f | wc -l | tr -d ' '; }
pipe_us() { sed -n 's/^spirv-host: time .*pipeline_us=\([0-9]*\).*/\1/p' "$1"; }
if build spirv-host-nv-offpath -DMT_NV_CACHE_PATH="\"$C0\"" && build spirv-host-nv-cache -DMT_NV_DISK_CACHE='"1"' -DMT_NV_CACHE_PATH="\"$C1\""; then
  for k in vadd fsum16 gemm16; do
    # shellcheck disable=SC2046
    set -- $(args $k); n=$1; shift
    run "$k.vk" "$D/spirv-host-nv-offpath" "$k" "$n" "$@" && cmp -s "$D/$k.vk.gpu.out" "$D/$k.ref" || bad "cache-off run $k"
    cp "$D/run-$k.vk.log" "$D/cache-off-$k.log"
    run "$k.vk" "$D/spirv-host-nv-cache" "$k" "$n" "$@" && cmp -s "$D/$k.vk.gpu.out" "$D/$k.ref" || bad "cache-on cold run $k"
    cp "$D/run-$k.vk.log" "$D/cache-cold-$k.log"
    run "$k.vk" "$D/spirv-host-nv-cache" "$k" "$n" "$@" && cmp -s "$D/$k.vk.gpu.out" "$D/$k.ref" && ok "cache-on warm run $k bytes = $k.ref" || bad "cache-on warm run $k"
    cp "$D/run-$k.vk.log" "$D/cache-warm-$k.log"
    echo "OBSERVED cache $k pipeline_us off=$(pipe_us "$D/cache-off-$k.log") cold=$(pipe_us "$D/cache-cold-$k.log") warm=$(pipe_us "$D/cache-warm-$k.log")"
  done
  [ "$(nfiles "$C0")" -eq 0 ] && ok "cache-off build wrote nothing to its private path" || bad "cache-off build wrote $(nfiles "$C0") files"
  c1=$(nfiles "$C1")
  [ "$c1" -gt 0 ] && ok "cache-on build wrote $c1 files to its private path" || bad "cache-on build wrote no cache files"
  find "$C1" -type f | while read -r f; do s=$(wc -c < "$f"); [ "$s" -gt 64 ] && printf '\377\377\377\377\377\377\377\377' | dd of="$f" bs=1 seek=$((s / 2)) conv=notrunc status=none; done
  for k in vadd fsum16 gemm16; do
    # shellcheck disable=SC2046
    set -- $(args $k); n=$1; shift
    rm -f "$D/$k.vk.gpu.out"
    if run "$k.vk" "$D/spirv-host-nv-cache" "$k" "$n" "$@"; then
      cmp -s "$D/$k.vk.gpu.out" "$D/$k.ref" && ok "corrupted cache $k: same bytes = $k.ref" || bad "corrupted cache $k gave wrong bytes"
    else
      rc=$?; [ ! -e "$D/$k.vk.gpu.out" ] && ok "corrupted cache $k: fail-closed exit=$rc, no output" || bad "corrupted cache $k: exit $rc with output"
    fi
    cp "$D/run-$k.vk.log" "$D/cache-corrupt-$k.log"
  done
else
  bad "cache lifecycle builds"
fi

# Timing (observation only): MT_REPS fresh processes per kernel, conformance build (cache off). Every
# repetition's output must still equal the reference.
T=$D/timing.tsv
printf 'target\tkernel\trep\tsetup_us\tpipeline_us\trun_us\tdevice_ns\ttotal_us\tmaxrss_kib\tbuffers_bytes\n' > "$T"
field() { sed -n "s/^spirv-host: time .*$1=\([0-9A-Z_]*\).*/\1/p" "$2"; }
for k in vadd fsum16 gemm16; do
  # shellcheck disable=SC2046
  set -- $(args $k); n=$1; shift
  same=0; r=1
  while [ "$r" -le "$reps" ]; do
    if run "$k.vk" "$H" "$k" "$n" "$@" && cmp -s "$D/$k.vk.gpu.out" "$D/$k.ref"; then same=$((same + 1)); fi
    l=$D/run-$k.vk.log
    printf 'nvidia-vk\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$k" "$r" "$(field setup_us "$l")" "$(field pipeline_us "$l")" \
      "$(field run_us "$l")" "$(field device_ns "$l")" "$(field total_us "$l")" "$(field maxrss_kib "$l")" "$(field buffers_bytes "$l")" >> "$T"
    r=$((r + 1))
  done
  [ "$same" -eq "$reps" ] && ok "timing reps $k: $same/$reps outputs = $k.ref" || bad "timing reps $k: only $same/$reps outputs = $k.ref"
done

# Native baseline on the same device: CUDA++'s own sm_121 vadd (no CUDA, no NVIDIA user-space library).
if [ -n "${HELLO_GPU:-}" ] && [ -x "$HELLO_GPU" ]; then
  want=$(digest "$D/vadd.ref"); same=0; r=1
  while [ "$r" -le "$reps" ]; do
    "$HELLO_GPU" > "$D/hello_gpu-$r.log" 2>&1; hrc=$?
    hs=$(sed -n 's/^output sha256 //p' "$D/hello_gpu-$r.log")
    [ "$hrc" -eq 0 ] && [ "$hs" = "$want" ] && same=$((same + 1))
    us=$(sed -n 's/.*the GPU took \([0-9.]*\) us.*/\1/p' "$D/hello_gpu-$r.log")
    printf 'cudapp-native\tvadd\t%s\t\t\t\t%s\t\t\t\n' "$r" "$( [ -n "$us" ] && echo "$us" | awk '{printf "%.0f", $1 * 1000}')" >> "$T"
    r=$((r + 1))
  done
  [ "$same" -eq "$reps" ] && ok "CUDA++ native hello_gpu vadd on the GB10: $same/$reps PASS, output sha256 = vadd.ref $want" \
    || bad "CUDA++ native hello_gpu: only $same/$reps runs PASS with output sha256 = vadd.ref"
  grep -h '^kernel\|^work\|at the GB10' "$D/hello_gpu-1.log"
else
  notrun "CUDA++ native baseline (hello_gpu on the GB10): HELLO_GPU not set"
fi
if [ -n "${HELLO_GPU_CONTROL:-}" ] && [ -x "$HELLO_GPU_CONTROL" ]; then
  "$HELLO_GPU_CONTROL" --bug=short > "$D/hello_gpu_control.log" 2>&1; hrc=$?
  if [ "$hrc" -eq 1 ] && grep -q '1 wrong, 1 never written$' "$D/hello_gpu_control.log" && grep -qx FAIL "$D/hello_gpu_control.log"; then
    ok "negative hello_gpu_control --bug=short is red (exit 1, the unwritten element caught)"
  else bad "hello_gpu_control --bug=short exit=$hrc did not go red as required"; fi
else
  notrun "hello_gpu_control negative: HELLO_GPU_CONTROL not set"
fi

for k in vadd fsum16 gemm16; do
  # shellcheck disable=SC2046
  set -- $(args $k); n=$1; shift
  run "$k.vk" "$H" "$k" "$n" "$@" || bad "final run $k"
  echo "OUTPUT $k.vk.gpu.out sha256=$(digest "$D/$k.vk.gpu.out")"
  echo "OUTPUT $k.vk.mutant.gpu.out sha256=$(digest "$D/$k.vk.mutant.gpu.out")"
done
deflist > "$D/default-cache.after"
if cmp -s "$D/default-cache.before" "$D/default-cache.after"; then ok "driver default shader cache $defcache unchanged ($(wc -l < "$D/default-cache.after") files)"
else bad "driver default shader cache $defcache changed during the run"; fi
echo "route: SPIRV_VK -> NVIDIA Vulkan driver 580.173.02 (proprietary user space, nvidia.ko) on the GB10 (vendor runtime, not CUDA++); CUDA++ own sm_121 path = hello_gpu vadd only"
if [ "$nfail" -ne 0 ]; then echo "NVIDIA-GPU FAIL pass=$npass fail=$nfail not_run=$nnr"; exit 1; fi
echo "NVIDIA-GPU PASS pass=$npass fail=0 not_run=$nnr"
exit 0
