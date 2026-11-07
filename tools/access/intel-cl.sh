#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# intel-cl.sh: the Intel GPU (OpenCL) candidate and control rows of tools/access/plans/intel-gpu.tsv
# (DRA-194), plus the two Spark-side steps around them. It decides nothing about a kernel's result
# itself: bytes are compared with cmp against backend/spirv/conform.om's reference on the VM, and with
# conform.om itself on the Spark after the copy back. The launcher is the migration-pending C
# tools/multitarget/spirv-host.c built with MT_TARGET=3 (OpenCL GPU through one vendor ICD library).
#
#   intel-cl.sh bundle MT_OUT BUNDLE                       (Spark)
#       Copies the OpenCL modules (vadd, fsum16, gemm16 and their planted-wrong mutants), the inputs and
#       the conform.om references from a tests/multitarget/run.sh MT_OUT directory, plus spirv-host.c and
#       this script, into BUNDLE (must not exist) with a SHA256SUMS file. BUNDLE goes to the VM.
#   intel-cl.sh run KERNEL [mutant]                        (VM, one plan row)
#       Builds the MT_TARGET=3 launcher once into $ACCESS_OUT/bin, runs KERNEL's module on the GPU with
#       MT_CL_GPU=1, writes $ACCESS_OUT/outputs/KERNEL.cl[.mutant].gpu.out and compares it byte for byte
#       with BUNDLE/KERNEL.ref. Exit 0 equal; 1 differ; 2..9 the launcher's own refusal; 3 build or ICD
#       problem; 4 bundle changed; 64 usage.
#   intel-cl.sh reps N                                     (VM, one plan row)
#       Runs every kernel N times (fresh processes; timing lines kept in the log). Exit 0 only if every
#       output equals its reference; 1 otherwise.
#   intel-cl.sh ctl flag|isolation|require|vendor          (VM, one plan row each)
#       Planted refusals; the row's want is the exit the launcher must give, with no output written:
#       flag      the GPU build without MT_CL_GPU=1                        want 2
#       isolation a copy forbidding "/dev/dri/" (the GPU's DRM node)       want 9
#       require   a copy requiring a library that is never mapped          want 9
#       vendor    a copy expecting vendor id 0x1002 (AMD) on Intel's GPU   want 5
#       An output file written by a control exits 70; the right exit without the launcher's named
#       refusal text (exit 5 also means "no GPU device") exits 71.
#   intel-cl.sh compare OUT CONFORM RESULT                 (Spark, after the copy back, before close)
#       Checks OUT/SHA256SUMS, then runs CONFORM (conform.om built at the receipt's SRC rev with the
#       pinned omc) on every candidate and control output: want 0 for cand-K-cl, 1 for ctl-K-mutant-cl.
#       A line passes only when conform.om gives its want and the VM row says PASS. Writes RESULT (must
#       not exist). Exit 0 every compared output passes (at least one); 1 any fails or SHA256SUMS
#       differs; 3 NOT_RUN (no output to compare); 64 usage.
#
# Environment on the VM: ACCESS_BUNDLE and ACCESS_OUT (exported by collect.sh run); MT_CL_GPU_ICD (the
# vendor ICD library; default: the path in /etc/OpenCL/vendors/intel.icd); MT_CC (cc); MT_CFLAGS (extra
# -I for CL/cl.h); MT_CL_GPU_INDEX (only if the platform has more than one GPU).
set -u
LC_ALL=C
export LC_ALL

usage() { sed -n '3,/^set -u$/{/^#/p;}' "$0" >&2; exit 64; }
digest() { d=$(sha256sum < "$1") || return 1; printf '%s\n' "${d%% *}"; }
KERNELS="vadd fsum16 gemm16"
args() { # KERNEL DIR: prints "COUNT IN..."
  case $1 in
    vadd) echo "1048576 $2/a.bin $2/b.bin" ;;
    fsum16) echo "65536 $2/x.bin" ;;
    gemm16) echo "65536 $2/ga.bin $2/gb.bin" ;;
    *) return 1 ;;
  esac
}

do_bundle() {
  [ "$#" -eq 2 ] || usage
  src=$1; dst=$2
  [ -e "$dst" ] && { echo "$dst exists; use a new directory" >&2; exit 64; }
  root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
  files=
  for k in $KERNELS; do files="$files $k.cl.spv $k.cl.mutant.spv $k.ref"; done
  files="$files a.bin b.bin x.bin ga.bin gb.bin"
  mkdir -p "$dst" || exit 74
  for f in $files; do cp "$src/$f" "$dst/$f" || { echo "bundle: $src/$f missing (run tests/multitarget/run.sh with MT_OUT)" >&2; exit 74; }; done
  cp "$root/tools/multitarget/spirv-host.c" "$dst/spirv-host.c" && cp "$0" "$dst/intel-cl.sh" || exit 74
  # shellcheck disable=SC2086
  (cd "$dst" && sha256sum $files spirv-host.c intel-cl.sh > SHA256SUMS) || exit 74
  echo "BUNDLE $dst files=$(wc -l < "$dst/SHA256SUMS") SHA256SUMS=$(digest "$dst/SHA256SUMS")"
}

# VM side ---------------------------------------------------------------------------------------------
need_env() {
  [ -n "${ACCESS_BUNDLE:-}" ] && [ -d "$ACCESS_BUNDLE" ] || { echo 'intel-cl: ACCESS_BUNDLE is not a directory' >&2; exit 64; }
  [ -n "${ACCESS_OUT:-}" ] && [ -d "$ACCESS_OUT" ] || { echo 'intel-cl: ACCESS_OUT is not a directory (collect.sh run exports it)' >&2; exit 64; }
  B=$ACCESS_BUNDLE
  (cd "$B" && sha256sum -c --quiet SHA256SUMS) || { echo 'intel-cl: bundle differs from its SHA256SUMS' >&2; exit 4; }
  echo "BUNDLE SHA256SUMS=$(digest "$B/SHA256SUMS")"
  mkdir -p "$ACCESS_OUT/bin" "$ACCESS_OUT/outputs" || exit 74
}
icd_path() {
  icd=${MT_CL_GPU_ICD:-}
  if [ -z "$icd" ] && [ -r /etc/OpenCL/vendors/intel.icd ]; then icd=$(sed -n '/./{p;q;}' /etc/OpenCL/vendors/intel.icd); fi
  case $icd in
    /*) ;;
    '') echo 'intel-cl: no MT_CL_GPU_ICD and no /etc/OpenCL/vendors/intel.icd (Intel compute runtime not installed?)' >&2; return 3 ;;
    *) echo "intel-cl: ICD library \"$icd\" is not an absolute path; set MT_CL_GPU_ICD" >&2; return 3 ;;
  esac
  [ -f "$icd" ] || { echo "intel-cl: ICD library $icd missing" >&2; return 3; }
  printf '%s\n' "$icd"
}
# build NAME [-D...]: the MT_TARGET=3 launcher from the bundle's spirv-host.c, once per name.
build() {
  name=$1; shift
  out=$ACCESS_OUT/bin/$name
  [ -x "$out" ] && { echo "LAUNCHER $out sha256=$(digest "$out") (built earlier in this window)"; return 0; }
  icd=$(icd_path) || return 3
  cc=${MT_CC:-cc}
  echo "CC $($cc --version 2>&1 | head -n 1)"
  echo "ICD $icd realpath=$(readlink -f "$icd") sha256=$(digest "$(readlink -f "$icd")")"
  # shellcheck disable=SC2086
  if ! $cc -std=c11 -O2 -Wall -Wextra -Werror -DCUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 -DMT_TARGET=3 \
      -DMT_CL_GPU_ICD="\"$icd\"" "$@" ${MT_CFLAGS:-} "$B/spirv-host.c" -o "$out" -l:libOpenCL.so.1; then
    echo "intel-cl: launcher build $name failed" >&2; return 3
  fi
  echo "LAUNCHER $out sha256=$(digest "$out") src=$(digest "$B/spirv-host.c") flags=$*"
}
# launch HOST KERNEL VARIANT OUTFILE: one run on the GPU; returns the launcher's exit.
launch() {
  h=$1; k=$2; v=$3; o=$4
  # shellcheck disable=SC2046
  set -- $(args "$k" "$B") || { echo "intel-cl: unknown kernel $k" >&2; return 64; }
  n=$1; shift
  rm -f "$o"
  MT_CL_GPU=1 "$h" cl "$B/$k.cl$v.spv" "$k" "$n" "$@" "$o"
}

do_run() {
  [ "$#" -eq 1 ] || [ "$#" -eq 2 ] || usage
  k=$1; v=
  case ${2:-} in '') ;; mutant) v=.mutant ;; *) usage ;; esac
  args "$k" . > /dev/null || usage
  need_env
  build spirv-host-cl3 || exit 3
  o=$ACCESS_OUT/outputs/$k.cl$v.gpu.out
  launch "$ACCESS_OUT/bin/spirv-host-cl3" "$k" "$v" "$o"
  rc=$?
  [ "$rc" -eq 0 ] || { echo "LAUNCH $k.cl$v exit=$rc (no compare)"; exit "$rc"; }
  echo "OUTPUT outputs/$k.cl$v.gpu.out sha256=$(digest "$o") reference=$k.ref sha256=$(digest "$B/$k.ref")"
  if cmp "$o" "$B/$k.ref"; then echo "BYTES EQUAL $k.cl$v.gpu.out = $k.ref"; exit 0; fi
  echo "BYTES DIFFER $k.cl$v.gpu.out != $k.ref"
  exit 1
}

do_reps() {
  [ "$#" -eq 1 ] || usage
  case $1 in ''|*[!0-9]*) usage ;; esac
  need_env
  build spirv-host-cl3 || exit 3
  bad=0
  for k in $KERNELS; do
    r=1; same=0
    while [ "$r" -le "$1" ]; do
      o=$ACCESS_OUT/bin/rep.out
      if launch "$ACCESS_OUT/bin/spirv-host-cl3" "$k" "" "$o" 2> "$ACCESS_OUT/bin/rep.log" && cmp -s "$o" "$B/$k.ref"; then same=$((same + 1)); fi
      echo "REP $k $r $(grep '^spirv-host: time ' "$ACCESS_OUT/bin/rep.log" | sed 's/^spirv-host: time //')"
      r=$((r + 1))
    done
    rm -f "$ACCESS_OUT/bin/rep.out" "$ACCESS_OUT/bin/rep.log"
    echo "REPS $k $same/$1 outputs equal $k.ref"
    [ "$same" -eq "$1" ] || bad=1
  done
  exit "$bad"
}

do_ctl() {
  [ "$#" -eq 1 ] || usage
  need_env
  case $1 in
    flag) build spirv-host-cl3 || exit 3; h=$ACCESS_OUT/bin/spirv-host-cl3; want='set MT_CL_GPU=1 to opt in' ;;
    isolation) build spirv-host-cl3-forbid-dri -DMT_FORBID='"/dev/dri/"' || exit 3; h=$ACCESS_OUT/bin/spirv-host-cl3-forbid-dri
      want='isolation FAILED: forbidden path "/dev/dri/" present' ;;
    require) build spirv-host-cl3-require -DMT_CL_GPU_REQUIRE='"libnot-a-loaded-driver"' || exit 3; h=$ACCESS_OUT/bin/spirv-host-cl3-require
      want='isolation FAILED: required driver "libnot-a-loaded-driver" is not mapped' ;;
    vendor) build spirv-host-cl3-vendor -DMT_CL_GPU_VENDOR=0x1002u || exit 3; h=$ACCESS_OUT/bin/spirv-host-cl3-vendor
      want='device vendor id differs from MT_CL_GPU_VENDOR' ;;
    *) usage ;;
  esac
  o=$ACCESS_OUT/outputs/ctl-$1.out
  e=$ACCESS_OUT/bin/ctl-$1.err
  rm -f "$o"
  if [ "$1" = flag ]; then
    # shellcheck disable=SC2046
    set -- $(args vadd "$B"); n=$1; shift
    env -u MT_CL_GPU "$h" cl "$B/vadd.cl.spv" vadd "$n" "$@" "$o" 2> "$e"
  else
    launch "$h" vadd "" "$o" 2> "$e"
  fi
  rc=$?
  cat "$e"
  [ -e "$o" ] && { echo "CONTROL wrote an output (exit $rc): refused"; exit 70; }
  # The exit alone is not enough (exit 5 also means "no GPU"): the launcher must name this refusal.
  grep -qF "$want" "$e" || { echo "CONTROL exit=$rc without the expected refusal text: $want"; exit 71; }
  echo "CONTROL exit=$rc, no output, refusal: $want"
  exit "$rc"
}

# Spark side ------------------------------------------------------------------------------------------
do_compare() {
  [ "$#" -eq 3 ] || usage
  O=$1; C=$2; R=$3
  [ -r "$O/receipt.body" ] && [ -r "$O/SHA256SUMS" ] || { echo "$O is not a collect.sh run directory" >&2; exit 64; }
  [ -x "$C" ] || { echo "CONFORM $C is not executable (build backend/spirv/conform.om with the pinned omc)" >&2; exit 64; }
  [ -e "$R" ] && { echo "$R exists; compare results are never rewritten" >&2; exit 64; }
  (cd "$O" && sha256sum -c --quiet SHA256SUMS) || { echo "SHA256SUMS check failed: the copied run differs from what was recorded" >&2; exit 1; }
  pass=0; fail=0; nr=0; acc=0
  {
    echo "COMPARE1 run=$(basename "$O") window=$(sed -n 's/^window=//p' "$O/meta") $(sed -n 's/^SRC //p' "$O/receipt.body" | head -n 1) conform_sha256=$(digest "$C") sums_sha256=$(digest "$O/SHA256SUMS")"
    for k in $KERNELS; do for v in "" .mutant; do
      if [ -z "$v" ]; then id=cand-$k-cl; want=0; else id=ctl-$k-mutant-cl; want=1; fi
      row=$(grep "^ROW id=$id " "$O/receipt.body" | head -n 1)
      vmv=$(printf '%s\n' "$row" | sed -n 's/.* verdict=\([A-Z_]*\).*/\1/p'); dev=$(printf '%s\n' "$row" | sed -n 's/.* dev=\([a-z0-9]*\).*/\1/p')
      f=outputs/$k.cl$v.gpu.out
      if [ -z "$row" ]; then echo "COMPARE row=$id verdict=NOT_RUN reason=no-row-in-receipt"; nr=$((nr + 1)); continue; fi
      if [ ! -f "$O/$f" ]; then
        case $vmv in
          PASS|FAIL) echo "COMPARE row=$id file=$f vm_verdict=$vmv dev=$dev verdict=FAIL reason=output-missing"; fail=$((fail + 1)) ;;
          *) echo "COMPARE row=$id vm_verdict=$vmv verdict=NOT_RUN reason=no-output"; nr=$((nr + 1)) ;;
        esac
        continue
      fi
      line=$("$C" check "$k" "$O/$f" 2>&1); ce=$?
      line=$(printf '%s' "$line" | tr ' \n' '_ ')
      if [ "$ce" -eq "$want" ] && [ "$vmv" = PASS ]; then v2=PASS; why=agree; pass=$((pass + 1)); [ "$dev" = d0 ] && acc=$((acc + 1))
      elif [ "$ce" -ne "$want" ]; then v2=FAIL; why=conform-exit-differs-from-want; fail=$((fail + 1))
      else v2=FAIL; why=vm-row-$vmv-but-conform-agrees-with-want; fail=$((fail + 1)); fi
      echo "COMPARE row=$id file=$f sha256=$(digest "$O/$f") want=$want conform_exit=$ce vm_verdict=$vmv dev=$dev verdict=$v2 reason=$why conform=$line"
    done; done
    echo "COMPARE-SUMMARY pass=$pass fail=$fail not_run=$nr pass_on_d0=$acc"
  } > "$R" || exit 74
  cat "$R"
  [ "$fail" -eq 0 ] || exit 1
  [ "$pass" -gt 0 ] || exit 3
  exit 0
}

[ "$#" -ge 1 ] || usage
cmd=$1; shift
case $cmd in
  bundle) do_bundle "$@" ;;
  run) do_run "$@" ;;
  reps) do_reps "$@" ;;
  ctl) do_ctl "$@" ;;
  compare) do_compare "$@" ;;
  *) usage ;;
esac
