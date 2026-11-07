#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Portable-backend host gate (docs/ARCHITECTURE.md "Portable backend"). Everything under test that
# makes a decision is seeded Omega: backend/targets.om (target table, code-object loader checks),
# backend/spirv/emit.om (SPIR-V kernel builder), backend/spirv/conform.om (host reference and bit
# compare). This script only coordinates and compares them with independent oracles:
#   spirv-val (SPIRV-Tools)            every emitted module, plus corrupted copies that must fail
#   llvm-mc + ld.lld + llvm-readelf     ORACLE gfx1151 code object (backend/amd/vadd-gfx1151.s)
#   hello_gpu --sim                     CUDA++'s own warp-simulator output digest for the same vadd
#   PoCL / Mesa lavapipe                CPU implementations that execute the SPIR-V (via the
#                                       migration-pending C launcher tools/multitarget/spirv-host.c)
# It opens no GPU: the launcher pins CPU-only drivers and fails if any nvidia path is mapped or open.
# Results are host EXERCISED at most; every GPU row stays hardware NOT_RUN.
#
# usage: run.sh /abs/path/to/pinned-omc
# Environment (absolute paths; a missing tool makes its checks NOT_RUN, never PASS):
#   SPIRV_VAL     spirv-val                       LLVM_BIN  dir with llvm-mc, llvm-readelf
#   LD_LLD        ld.lld                          HELLO_GPU build/hello_gpu (for --sim digest)
#   CUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 builds and runs the C launcher (default 0: run NOT_RUN)
#   MT_CC (cc), MT_CFLAGS (extra -I for CL/ and vulkan/ headers), POCL_LIBRARY (absolute libpocl.so),
#   LVP_ICD (lavapipe manifest), MT_OUT (output dir to keep; default a new temp dir)
# Exit: 0 MULTITARGET PASS (NOT_RUN checks listed); 1 FAIL; 64 usage; 69 pin refused; 74 I/O;
# 77 NOT_RUN (not Linux AArch64).
set -u
LC_ALL=C
export LC_ALL

if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ]; then
  echo 'NOT_RUN: the multitarget host gate needs Linux AArch64 and the pinned seeded omc' >&2
  exit 77
fi
if [ "$#" -ne 1 ]; then echo 'usage: run.sh /absolute/path/to/pinned-omc' >&2; exit 64; fi
omc=$1
case $omc in /*) ;; *) echo 'usage: omc path must be absolute' >&2; exit 64 ;; esac
# omc: omega-mv 369dbc96 seed/out/omc, equal to its committed seed/out/omc.sha256 (AGENTS.md pin).
omc_pin=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
digest() { d=$(sha256sum < "$1") || return 1; printf '%s\n' "${d%% *}"; }
[ "$(digest "$omc")" = "$omc_pin" ] || { echo "REFUSED: $omc digest differs from pin $omc_pin" >&2; exit 69; }

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
out=${MT_OUT:-$(mktemp -d "${TMPDIR:-/tmp}/cudapp-multitarget.XXXXXX")} || exit 74
mkdir -p "$out" || exit 74
echo "PIN omc $omc_pin"
echo "Outputs: $out"

npass=0; nfail=0; nnr=0
ok() { npass=$((npass + 1)); echo "PASS $*"; }
bad() { nfail=$((nfail + 1)); echo "FAIL $*"; }
notrun() { nnr=$((nnr + 1)); echo "NOT_RUN $*"; }
# expect RC NAME cmd...: the command must exit RC (stdout+stderr kept in $out/NAME.log)
expect() {
  want=$1; name=$2; shift 2
  "$@" > "$out/$name.log" 2>&1
  got=$?
  if [ "$got" -eq "$want" ]; then ok "$name exit=$got $(tail -n 1 "$out/$name.log")"; else bad "$name exit=$got want=$want $(tail -n 1 "$out/$name.log")"; fi
}
# poke FILE OFFSET OCTALBYTE: overwrite one byte in place
poke() { printf "\\$3" | dd of="$1" bs=1 seek="$2" conv=notrunc status=none; }

# 1. Compile the three Omega units with the pinned omc.
for u in targets:backend/targets.om emit:backend/spirv/emit.om conform:backend/spirv/conform.om; do
  name=${u%%:*}; src=$root/${u#*:}
  timeout 120 "$omc" < "$src" > "$out/$name" 2> "$out/$name.compile.err"
  rc=$?
  if [ "$rc" -ne 0 ] || [ -s "$out/$name.compile.err" ] || [ ! -s "$out/$name" ]; then
    cat "$out/$name.compile.err"; echo "FAIL compile $name exit $rc"; exit 1
  fi
  chmod 700 "$out/$name" || exit 74
  echo "COMPILE $name src=$(digest "$src") bin=$(digest "$out/$name")"
done
T=$out/targets; E=$out/emit; C=$out/conform

# 2. Target table: 22 rows; only nvidia-sm_121 SUPPORTED; every PLANNED row hardware NOT_RUN;
#    host rows say not-a-GPU. The same checker must reject a table with one NOT_RUN turned into PASS.
check_table() {
  rows=$(grep -v '^TARGET id vendor ' "$1" | grep -c '^TARGET [a-z]')
  [ "$rows" -eq 22 ] || return 1
  [ "$(grep -c ' SUPPORTED ' "$1")" -eq 1 ] || return 1
  grep -q '^TARGET nvidia-sm_121 .* SUPPORTED ' "$1" || return 1
  [ "$(grep -c ' PLANNED NOT_RUN ' "$1")" -eq 19 ] || return 1
  [ "$(grep -c ' HOST-ONLY(not-a-GPU) ' "$1")" -eq 2 ] || return 1
  return 0
}
"$T" doctor > "$out/doctor.txt" 2>&1 || bad "targets doctor exit $?"
if check_table "$out/doctor.txt"; then ok "table rows=22 supported=nvidia-sm_121 planned_not_run=19 host=2"; else bad table; fi
sed '0,/ PLANNED NOT_RUN /s// PLANNED PASS /' "$out/doctor.txt" > "$out/doctor-mutant.txt"
if check_table "$out/doctor-mutant.txt"; then bad "table checker accepted a row claiming PASS"; else ok "table-negative row claiming PASS rejected"; fi

# 3. Emit the SPIR-V modules (and the planted-wrong mutants) from Omega.
for k in vadd fsum16 gemm16; do for a in cl vk; do
  "$E" $k $a > "$out/$k.$a.spv" || bad "emit $k $a"
  "$E" $k $a mutant > "$out/$k.$a.mutant.spv" || bad "emit $k $a mutant"
  echo "MODULE $k.$a.spv sha256=$(digest "$out/$k.$a.spv") bytes=$(wc -c < "$out/$k.$a.spv")"
done; done

# 4. spirv-val on every module; corrupted copies must fail it.
if [ -n "${SPIRV_VAL:-}" ] && [ -x "$SPIRV_VAL" ]; then
  echo "TOOL spirv-val $("$SPIRV_VAL" --version 2>&1 | head -n 1)"
  for k in vadd fsum16 gemm16; do for m in "" .mutant; do
    expect 0 "spirv-val-$k.cl$m" "$SPIRV_VAL" --target-env opencl2.1 "$out/$k.cl$m.spv"
    expect 0 "spirv-val-$k.vk$m" "$SPIRV_VAL" --target-env vulkan1.1 "$out/$k.vk$m.spv"
  done; done
else
  notrun "spirv-val: SPIRV_VAL not set to an executable"
fi
head -c $(( $(wc -c < "$out/vadd.vk.spv") - 4 )) "$out/vadd.vk.spv" > "$out/vadd.vk.trunc.spv"
cp "$out/vadd.vk.spv" "$out/vadd.vk.badmagic.spv" && poke "$out/vadd.vk.badmagic.spv" 0 000
# first instruction's word count (high byte of word 5) set to 0x7f..: runs past the end
cp "$out/vadd.vk.spv" "$out/vadd.vk.overrun.spv" && poke "$out/vadd.vk.overrun.spv" 23 177
if [ -n "${SPIRV_VAL:-}" ] && [ -x "$SPIRV_VAL" ]; then
  for c in trunc badmagic overrun; do
    if "$SPIRV_VAL" --target-env vulkan1.1 "$out/vadd.vk.$c.spv" > "$out/spirv-val-$c.log" 2>&1; then
      bad "spirv-val accepted corrupted module ($c)"; else ok "spirv-val-negative $c rejected: $(head -n 1 "$out/spirv-val-$c.log")"; fi
  done
fi

# 5. Omega loader: accepts, and refuses every wrong format, corruption and entry.
for k in vadd fsum16 gemm16; do
  expect 0 "load-cl-$k" "$T" load SPIRV_CL "$out/$k.cl.spv" $k
  expect 0 "load-vk-$k" "$T" load SPIRV_VK "$out/$k.vk.spv" $k
done
expect 21 load-neg-truncated "$T" load SPIRV_VK "$out/vadd.vk.trunc.spv"
expect 21 load-neg-overrun "$T" load SPIRV_VK "$out/vadd.vk.overrun.spv"
expect 20 load-neg-badmagic "$T" load SPIRV_VK "$out/vadd.vk.badmagic.spv"
expect 11 load-neg-aarch64 "$T" load AARCH64 "$out/vadd.vk.spv"
expect 22 load-neg-cl-as-vk "$T" load SPIRV_VK "$out/vadd.cl.spv"
expect 22 load-neg-vk-as-cl "$T" load SPIRV_CL "$out/vadd.vk.spv"
expect 23 load-neg-entry "$T" load SPIRV_VK "$out/vadd.vk.spv" nosuch
expect 23 load-neg-entry-prefix "$T" load SPIRV_CL "$out/fsum16.cl.spv" fsum
expect 11 load-neg-msl "$T" load MSL_SOURCE "$out/vadd.vk.spv"
expect 12 load-neg-cubin "$T" load CUBIN "$out/vadd.vk.spv"
expect 10 load-neg-unknown "$T" load PTX "$out/vadd.vk.spv"
expect 40 load-neg-spv-as-sm121 "$T" load SM121_RAW "$out/vadd.vk.spv"
head -c 32 /dev/zero > "$out/sm121-two.bin"
expect 0 load-sm121-shape "$T" load SM121_RAW "$out/sm121-two.bin"
head -c 20 /dev/zero > "$out/sm121-odd.bin"
expect 40 load-neg-sm121-odd "$T" load SM121_RAW "$out/sm121-odd.bin"

# 6. Host reference (conform.om) and its cross-check against hello_gpu --sim.
"$C" in vadd "$out/a.bin" "$out/b.bin" && "$C" in fsum16 "$out/x.bin" && "$C" in gemm16 "$out/ga.bin" "$out/gb.bin" \
  && "$C" ref vadd "$out/vadd.ref" && "$C" ref fsum16 "$out/fsum16.ref" && "$C" ref gemm16 "$out/gemm16.ref" \
  || bad "conform inputs/reference"
echo "REFERENCE vadd sha256=$(digest "$out/vadd.ref") fsum16 sha256=$(digest "$out/fsum16.ref")"
echo "REFERENCE gemm16 sha256=$(digest "$out/gemm16.ref")"
if [ -n "${HELLO_GPU:-}" ] && [ -x "$HELLO_GPU" ]; then
  hs=$("$HELLO_GPU" --sim | sed -n 's/^output sha256 //p')
  if [ "$hs" = "$(digest "$out/vadd.ref")" ]; then ok "reference = hello_gpu --sim output sha256 $hs"; else bad "reference $(digest "$out/vadd.ref") != hello_gpu --sim $hs"; fi
else
  notrun "hello_gpu --sim cross-check: HELLO_GPU not set"
fi
cp "$out/vadd.ref" "$out/vadd.planted.out" && poke "$out/vadd.planted.out" 4096 377
expect 1 conform-neg-planted "$C" check vadd "$out/vadd.planted.out"
expect 0 conform-self "$C" check vadd "$out/vadd.ref"
cp "$out/gemm16.ref" "$out/gemm16.planted.out" && poke "$out/gemm16.planted.out" 131072 377
expect 1 conform-neg-planted-gemm16 "$C" check gemm16 "$out/gemm16.planted.out"
expect 0 conform-self-gemm16 "$C" check gemm16 "$out/gemm16.ref"

# 7. Execute on CPU implementations (migration-pending C launcher, default OFF).
if [ "${CUDAPP_MIGRATION_PENDING_SPIRV_HOST:-0}" != 1 ]; then
  notrun "spirv-host runs on PoCL and lavapipe: CUDAPP_MIGRATION_PENDING_SPIRV_HOST is not 1 (migration-pending C, default OFF)"
else
  cc=${MT_CC:-cc}
  pocl=${POCL_LIBRARY:-}; lvp=${LVP_ICD:-/usr/share/vulkan/icd.d/lvp_icd.json}
  build() { # NAME FORBID
    $cc -std=c11 -O2 -Wall -Wextra -Werror -DCUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 \
      -DMT_POCL_LIBRARY="\"$pocl\"" -DMT_LVP_ICD="\"$lvp\"" -DMT_FORBID="\"$2\"" ${MT_CFLAGS:-} \
      "$root/tools/multitarget/spirv-host.c" -o "$out/$1" -l:libOpenCL.so.1 -l:libvulkan.so.1
  }
  if ! build spirv-host nvidia > "$out/spirv-host.build.log" 2>&1; then
    cat "$out/spirv-host.build.log"; bad "spirv-host build"
  else
    H=$out/spirv-host
    for a in cl vk; do
      case $a in cl) [ -n "$pocl" ] || { notrun "cl runs: POCL_LIBRARY not set"; continue; } ;; esac
      for m in "" .mutant; do
        "$H" $a "$out/vadd.$a$m.spv" vadd 1048576 "$out/a.bin" "$out/b.bin" "$out/vadd.$a$m.out" > "$out/run-vadd.$a$m.log" 2>&1 || bad "run vadd $a$m exit $?"
        "$H" $a "$out/fsum16.$a$m.spv" fsum16 65536 "$out/x.bin" "$out/fsum16.$a$m.out" > "$out/run-fsum16.$a$m.log" 2>&1 || bad "run fsum16 $a$m exit $?"
        "$H" $a "$out/gemm16.$a$m.spv" gemm16 65536 "$out/ga.bin" "$out/gb.bin" "$out/gemm16.$a$m.out" > "$out/run-gemm16.$a$m.log" 2>&1 || bad "run gemm16 $a$m exit $?"
      done
      grep -h 'device=\|platform=' "$out/run-vadd.$a.log"
      expect 0 "conform-vadd-$a" "$C" check vadd "$out/vadd.$a.out"
      expect 0 "conform-fsum16-$a" "$C" check fsum16 "$out/fsum16.$a.out"
      expect 0 "conform-gemm16-$a" "$C" check gemm16 "$out/gemm16.$a.out"
      expect 1 "conform-neg-vadd-$a-mutant" "$C" check vadd "$out/vadd.$a.mutant.out"
      expect 1 "conform-neg-fsum16-$a-mutant" "$C" check fsum16 "$out/fsum16.$a.mutant.out"
      expect 1 "conform-neg-gemm16-$a-mutant" "$C" check gemm16 "$out/gemm16.$a.mutant.out"
      for k in vadd fsum16 gemm16; do
        if cmp -s "$out/$k.$a.out" "$out/$k.ref"; then ok "bytes $k.$a.out = $k.ref sha256=$(digest "$out/$k.ref")"; else bad "bytes $k.$a.out != $k.ref"; fi
      done
    done
    # Isolation guard control: a copy forbidding the CPU driver it must load has to exit 9.
    if build spirv-host-forbid-pocl libpocl > "$out/forbid.build.log" 2>&1 && build spirv-host-forbid-lvp vulkan_lvp >> "$out/forbid.build.log" 2>&1; then
      [ -n "$pocl" ] && expect 9 isolation-neg-cl "$out/spirv-host-forbid-pocl" cl "$out/vadd.cl.spv" vadd 1048576 "$out/a.bin" "$out/b.bin" "$out/iso.out"
      expect 9 isolation-neg-vk "$out/spirv-host-forbid-lvp" vk "$out/vadd.vk.spv" vadd 1048576 "$out/a.bin" "$out/b.bin" "$out/iso.out"
    else
      bad "isolation control build"
    fi
    # MT_TARGET=3 (OpenCL GPU through one vendor ICD library, DRA-194). This host has no such GPU, so only
    # the refusals run here; the GPU rows are tools/access/plans/intel-gpu.tsv (hardware NOT_RUN). No
    # refusal may write an output. The ICDs given are PoCL (a CPU implementation) or paths that do not
    # exist, so no NVIDIA library can load.
    build3() { # NAME ICD [extra -D...]
      n3=$1; icd3=$2; shift 2
      # shellcheck disable=SC2086
      $cc -std=c11 -O2 -Wall -Wextra -Werror -DCUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 -DMT_TARGET=3 \
        -DMT_CL_GPU_ICD="\"$icd3\"" "$@" ${MT_CFLAGS:-} "$root/tools/multitarget/spirv-host.c" -o "$out/$n3" -l:libOpenCL.so.1
    }
    # refuse WANT NAME TEXT cmd...: the command must exit WANT, print TEXT and write no output
    refuse() {
      rwant=$1; rname=$2; rtext=$3; shift 3
      rm -f "$out/cl3.out"
      "$@" > "$out/$rname.log" 2>&1
      rgot=$?
      if [ "$rgot" -eq "$rwant" ] && [ ! -e "$out/cl3.out" ] && grep -qF -- "$rtext" "$out/$rname.log"; then ok "$rname exit=$rgot $(tail -n 1 "$out/$rname.log")"
      else bad "$rname exit=$rgot want=$rwant output=$([ -e "$out/cl3.out" ] && echo written || echo none) $(tail -n 1 "$out/$rname.log")"; fi
    }
    V3="cl $out/vadd.cl.spv vadd 1048576 $out/a.bin $out/b.bin $out/cl3.out"
    if build3 spirv-host-cl3 "${pocl:-$out/no-such-icd/libpocl.so}" > "$out/cl3.build.log" 2>&1 \
       && build3 spirv-host-cl3-noicd "" >> "$out/cl3.build.log" 2>&1 \
       && build3 spirv-host-cl3-relicd libigdrcl.so >> "$out/cl3.build.log" 2>&1 \
       && build3 spirv-host-cl3-nvpath "$out/no-such-icd/libnvidia-opencl.so.1" >> "$out/cl3.build.log" 2>&1; then
      if nm -D "$out/spirv-host-cl3" 2>/dev/null | grep -q ' vk'; then bad "MT_TARGET=3 build references Vulkan symbols"
      else ok "MT_TARGET=3 builds without Vulkan (OpenCL only)"; fi
      # shellcheck disable=SC2086
      refuse 2 cl3-neg-no-optin 'set MT_CL_GPU=1 to opt in' env -u MT_CL_GPU "$out/spirv-host-cl3" $V3
      # shellcheck disable=SC2086
      refuse 2 cl3-neg-optin-0 'set MT_CL_GPU=1 to opt in' env MT_CL_GPU=0 "$out/spirv-host-cl3" $V3
      # shellcheck disable=SC2086
      refuse 2 cl3-neg-optin-yes 'set MT_CL_GPU=1 to opt in' env MT_CL_GPU=yes "$out/spirv-host-cl3" $V3
      refuse 2 cl3-neg-vk 'only cl' env MT_CL_GPU=1 "$out/spirv-host-cl3" vk "$out/vadd.vk.spv" vadd 1048576 "$out/a.bin" "$out/b.bin" "$out/cl3.out"
      # shellcheck disable=SC2086
      refuse 3 cl3-neg-no-icd 'build needs an absolute MT_CL_GPU_ICD' env MT_CL_GPU=1 "$out/spirv-host-cl3-noicd" $V3
      # shellcheck disable=SC2086
      refuse 3 cl3-neg-relative-icd 'build needs an absolute MT_CL_GPU_ICD' env MT_CL_GPU=1 "$out/spirv-host-cl3-relicd" $V3
      # shellcheck disable=SC2086
      refuse 9 cl3-neg-icd-names-nvidia 'contains forbidden "nvidia" (not loaded)' env MT_CL_GPU=1 "$out/spirv-host-cl3-nvpath" $V3
      # ocl-icd reads a directory as a vendors search path and an .icd file as a descriptor naming another
      # library: both, and a file that is not ELF, must be refused before any OpenCL call. The descriptors
      # name a mock library that does not exist; it must never be looked for.
      mkdir -p "$out/icd-dir" || exit 74
      printf '%s\n' "$out/mock-driver/libmockcl.so" > "$out/icd-dir/mock.icd" && cp "$out/icd-dir/mock.icd" "$out/clean.icd" \
        && printf 'not an ELF library\n' > "$out/notelf.so" || exit 74
      if build3 spirv-host-cl3-icddir "$out/icd-dir" >> "$out/cl3.build.log" 2>&1 \
         && build3 spirv-host-cl3-icdfile "$out/clean.icd" >> "$out/cl3.build.log" 2>&1 \
         && build3 spirv-host-cl3-notelf "$out/notelf.so" >> "$out/cl3.build.log" 2>&1; then
        # shellcheck disable=SC2086
        refuse 3 cl3-neg-icd-directory 'MT_CL_GPU_ICD is not a regular file' env MT_CL_GPU=1 "$out/spirv-host-cl3-icddir" $V3
        # shellcheck disable=SC2086
        refuse 3 cl3-neg-icd-descriptor 'MT_CL_GPU_ICD is an .icd descriptor' env MT_CL_GPU=1 "$out/spirv-host-cl3-icdfile" $V3
        # shellcheck disable=SC2086
        refuse 3 cl3-neg-icd-not-elf 'MT_CL_GPU_ICD is not an ELF library' env MT_CL_GPU=1 "$out/spirv-host-cl3-notelf" $V3
        if command -v strace > /dev/null 2>&1; then
          : > "$out/cl3-indirect.strace"
          for n3 in icddir icdfile; do
            # shellcheck disable=SC2086
            strace -f -qq -e trace=open,openat,stat,newfstatat,statx -o "$out/cl3-$n3.strace" env MT_CL_GPU=1 "$out/spirv-host-cl3-$n3" $V3 > /dev/null 2>&1
            cat "$out/cl3-$n3.strace" >> "$out/cl3-indirect.strace"
          done
          if ! grep -q 'libmockcl\|mock.icd' "$out/cl3-indirect.strace" && ! grep -q 'cl target=gpu' "$out/cl3-neg-icd-directory.log" "$out/cl3-neg-icd-descriptor.log"; then
            ok "cl3 directory and .icd descriptor refused before the loader: the mock driver they name was never looked for (strace)"
          else bad "cl3 ICD indirection reached the loader (strace shows the mock driver or the pin line was printed)"; fi
        else
          notrun "cl3 ICD indirection strace check: strace not installed"
        fi
      else
        bad "MT_TARGET=3 ICD indirection control builds"
      fi
      if [ -n "$pocl" ]; then
        # shellcheck disable=SC2086
        refuse 9 cl3-neg-icd-is-pocl 'contains forbidden "libpocl" (not loaded)' env MT_CL_GPU=1 "$out/spirv-host-cl3" $V3
        # A copy that allows PoCL: the loader runs on the named library, which has only a CPU device.
        if build3 spirv-host-cl3-cpuicd "$pocl" -DMT_FORBID='"nvidia"' >> "$out/cl3.build.log" 2>&1; then
          # shellcheck disable=SC2086
          refuse 5 cl3-neg-cpu-only-platform 'no CL_DEVICE_TYPE_GPU device on the pinned platform (no CPU fallback)' \
            env MT_CL_GPU=1 POCL_DEVICES=cpu OCL_ICD_FILENAMES=/nonexistent "$out/spirv-host-cl3-cpuicd" $V3
          if grep -q '^spirv-host: cl library=.* platform="Portable Computing Language"' "$out/cl3-neg-cpu-only-platform.log"; then
            ok "cl3 loader pinned to the named ICD (PoCL platform listed, then refused for having no GPU)"
          else bad "cl3-neg-cpu-only-platform did not reach the pinned platform"; fi
          if command -v strace > /dev/null 2>&1; then
            # shellcheck disable=SC2086
            strace -f -qq -e trace=open,openat -o "$out/cl3-cpu.strace" env MT_CL_GPU=1 "$out/spirv-host-cl3-cpuicd" $V3 > /dev/null 2>&1
            nv=$(grep -c -i 'libnvidia\|/dev/nvidia\|libcuda' "$out/cl3-cpu.strace")
            if grep -q 'libpocl' "$out/cl3-cpu.strace" && [ "$nv" -eq 0 ]; then ok "cl3 GPU mode opened PoCL and no libnvidia, libcuda or /dev/nvidia path (strace)"
            else bad "cl3 GPU mode strace: $nv NVIDIA/CUDA opens or PoCL not opened"; fi
          else
            notrun "cl3 strace open check: strace not installed"
          fi
        else
          bad "MT_TARGET=3 PoCL-allowing control build"
        fi
        # The default build stays CPU-only whatever MT_CL_GPU says.
        MT_CL_GPU=1 "$H" cl "$out/vadd.cl.spv" vadd 1048576 "$out/a.bin" "$out/b.bin" "$out/vadd.cl.optin.out" > "$out/run-vadd.cl.optin.log" 2>&1
        rc3=$?
        if [ "$rc3" -eq 0 ] && grep -q '^spirv-host: cl device=.* type=CPU$' "$out/run-vadd.cl.optin.log" && cmp -s "$out/vadd.cl.optin.out" "$out/vadd.ref"; then
          ok "default build ignores MT_CL_GPU=1: PoCL CPU device, bytes = vadd.ref"
        else bad "default build with MT_CL_GPU=1 exit=$rc3 (want 0, CPU device, bytes = vadd.ref)"; fi
      else
        notrun "cl3 PoCL-as-ICD refusals and default-build MT_CL_GPU check: POCL_LIBRARY not set"
      fi
    else
      cat "$out/cl3.build.log"; bad "MT_TARGET=3 build"
    fi
  fi
fi

# 8. AMDGPU_HSACO: ORACLE gfx1151 code object (llvm-mc + ld.lld), Omega loader validation and refusals.
mc=${LLVM_BIN:-/usr/lib/llvm-18/bin}/llvm-mc; re=${LLVM_BIN:-/usr/lib/llvm-18/bin}/llvm-readelf; lld=${LD_LLD:-}
if [ -x "$mc" ] && [ -x "$re" ] && [ -n "$lld" ] && [ -x "$lld" ]; then
  echo "TOOL llvm-mc $("$mc" --version | sed -n 's/.*LLVM version //p' | head -n 1); ld.lld $("$lld" --version)"
  hs=$out/vadd-gfx1151.hsaco
  if "$mc" -triple amdgcn-amd-amdhsa -mcpu=gfx1151 -filetype=obj "$root/backend/amd/vadd-gfx1151.s" -o "$out/vadd-gfx1151.o" \
     && "$lld" -shared "$out/vadd-gfx1151.o" -o "$hs"; then
    echo "HSACO ORACLE sha256=$(digest "$hs") bytes=$(wc -c < "$hs")"
    "$re" -h --notes --dyn-syms "$hs" > "$out/hsaco.readelf.txt" 2>&1
    if grep -q 'Flags: *0x4A, gfx1151' "$out/hsaco.readelf.txt" && grep -q 'amdhsa.target: *amdgcn-amd-amdhsa--gfx1151' "$out/hsaco.readelf.txt" \
       && grep -q 'OS/ABI: *AMDGPU - HSA' "$out/hsaco.readelf.txt" && grep -q ' vadd.kd$' "$out/hsaco.readelf.txt"; then
      ok "llvm-readelf: gfx1151 flags 0x4A, HSA OSABI, NT_AMDGPU_METADATA target gfx1151, vadd.kd"
    else bad "llvm-readelf did not show the expected gfx1151 code object"; fi
    expect 0 load-hsaco "$T" load AMDGPU_HSACO "$hs" gfx1151
    expect 33 load-neg-hsaco-wrong-gfx "$T" load AMDGPU_HSACO "$hs" gfx1100
    expect 37 load-neg-hsaco-unknown-gfx "$T" load AMDGPU_HSACO "$hs" gfx9999
    expect 2 load-neg-hsaco-no-gfx "$T" load AMDGPU_HSACO "$hs"
    expect 30 load-neg-spv-as-hsaco "$T" load AMDGPU_HSACO "$out/vadd.cl.spv" gfx1151
    expect 20 load-neg-hsaco-as-spv "$T" load SPIRV_VK "$hs"
    expect 40 load-neg-hsaco-as-sm121 "$T" load SM121_RAW "$hs"
    # Wrong note: the metadata target says gfx1150 while e_flags still says gfx1151.
    t=$(grep -obUa 'amdgcn-amd-amdhsa--gfx1151' "$hs" | head -n 1 | cut -d: -f1)
    cp "$hs" "$out/neg-note-target.hsaco" && poke "$out/neg-note-target.hsaco" $((t + 25)) 060
    if "$re" --notes "$out/neg-note-target.hsaco" 2>/dev/null | grep -q 'amdgcn-amd-amdhsa--gfx1150'; then
      ok "oracle sees the planted note target gfx1150"; else bad "planted note target not visible to llvm-readelf"; fi
    expect 36 load-neg-note-target "$T" load AMDGPU_HSACO "$out/neg-note-target.hsaco" gfx1151
    # Wrong note type: 32 (NT_AMDGPU_METADATA) becomes 33.
    n=$(grep -obUa 'AMDGPU' "$hs" | head -n 1 | cut -d: -f1)
    cp "$hs" "$out/neg-note-type.hsaco" && poke "$out/neg-note-type.hsaco" $((n - 4)) 041
    expect 34 load-neg-note-type "$T" load AMDGPU_HSACO "$out/neg-note-type.hsaco" gfx1151
    cp "$hs" "$out/neg-eflags.hsaco" && poke "$out/neg-eflags.hsaco" 48 101
    expect 33 load-neg-eflags "$T" load AMDGPU_HSACO "$out/neg-eflags.hsaco" gfx1151
    cp "$hs" "$out/neg-osabi.hsaco" && poke "$out/neg-osabi.hsaco" 7 000
    expect 31 load-neg-osabi "$T" load AMDGPU_HSACO "$out/neg-osabi.hsaco" gfx1151
    cp "$hs" "$out/neg-abi6.hsaco" && poke "$out/neg-abi6.hsaco" 8 004
    expect 31 load-neg-abi-v6 "$T" load AMDGPU_HSACO "$out/neg-abi6.hsaco" gfx1151
    head -c 600 "$hs" > "$out/neg-trunc.hsaco"
    expect 34 load-neg-hsaco-truncated "$T" load AMDGPU_HSACO "$out/neg-trunc.hsaco" gfx1151
  else
    bad "ORACLE hsaco assemble/link"
  fi
else
  notrun "AMDGPU_HSACO checks: llvm-mc/llvm-readelf under LLVM_BIN or LD_LLD missing"
fi

echo "hardware NOT_RUN: every GPU target except the GB10 (no AMD, Intel, Apple, Qualcomm, Arm, Imagination, Broadcom or Vortex device on this host; PoCL and lavapipe are CPUs; MT_TARGET=3 ran its refusals only)"
if [ "$nfail" -ne 0 ]; then echo "MULTITARGET FAIL pass=$npass fail=$nfail not_run=$nnr"; exit 1; fi
echo "MULTITARGET PASS pass=$npass fail=0 not_run=$nnr"
exit 0
