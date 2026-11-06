#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# doctor acceptance: cudapp-doctor (seeded Omega om/tools/fmt.om + om/tools/doctor.om) against its C oracle
# (-DCUDAPP_MIGRATION_PENDING_C_TOOLS_DOCTOR) on fake machines under CUDAPP_DOCTOR_ROOT: stdout, stderr and
# exit code byte for byte, a flipped-record negative control, the documented refusal of a build text
# holding %, and recompiled mutants of the composed unit. TOOLS_OM_COVERAGE=DIR also writes gcov branch
# data of the oracle. Run as a normal user (chmod 000 must deny access) under:
#   bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
tmp=$(mktemp -d)
trap 'chmod -R u+rwx "$tmp" 2>/dev/null; rm -rf "$tmp"' EXIT
. tests/tools-om/lib.sh
tom_pin
[ "$(id -u)" != 0 ] || { echo "NOT_RUN: root ignores chmod 000; run as a normal user"; exit 3; }
SRC=tools/doctor/cudapp_doctor.c
OFF=-DCUDAPP_MIGRATION_PENDING_C_TOOLS_DOCTOR
ABI='-DCUDAPP_NVIDIA_OPEN_ABI="580.173.02"'
CHIP=src/omega_gpu_chip.c

$CC $TOM_CFLAGS "$ABI" $OFF -o "$tmp/doc_c" $SRC $CHIP
$CC $TOM_CFLAGS "$ABI" -o "$tmp/doc_om" $SRC src/cudapp_om_doctor.c $CHIP

# ---- fake machines (partition: docs in tests/tools-om/README.md) ----
R=$tmp/m
OPEN='NVIDIA UNIX Open Kernel Module for aarch64  580.173.02  Release Build  (test)  Tue Jun 23 08:34:19 UTC 2026'
CLOSED='NVIDIA UNIX aarch64 Kernel Module  580.173.02  Release Build  (test)  Tue Jun 23 08:34:19 UTC 2026'
ver() { mkdir -p "$1/proc/driver/nvidia"; printf 'NVRM version: %s\nGCC version:  gcc version 13.3.0\n' "$2" > "$1/proc/driver/nvidia/version"; }
nodes() { d=$1; shift; mkdir -p "$d/dev"; for n in "$@"; do : > "$d/dev/$n"; chmod 666 "$d/dev/$n"; done; }
pci() { # ROOT NAME VENDOR DEVICE CLASS
  g="$1/sys/bus/pci/devices/$2"; mkdir -p "$g"
  echo "$3" > "$g/vendor"; echo "$4" > "$g/device"; echo "$5" > "$g/class"
}
mem() { mkdir -p "$1/proc"; printf 'MemTotal:       %s kB\nMemFree:        1 kB\nMemAvailable:   %s kB\n' "$2" "$3" > "$1/proc/meminfo"; }
nvcc() { mkdir -p "$1/$(dirname "$2")"; : > "$1/$2"; chmod "$3" "$1/$2"; }
std() { # ROOT: ready GB10 machine
  ver "$1" "$OPEN"; nodes "$1" nvidiactl nvidia0 nvidia-uvm
  pci "$1" 000f:01:00.0 0x10de 0x2e12 0x030000; pci "$1" 0000:00:00.0 0x8086 0x1234 0x060000
  mem "$1" 127535484 42000000
}
CASES=""
case_() { CASES="$CASES $1"; mkdir -p "$R/$1"; }

case_ empty
case_ ready; std "$R/ready"; nvcc "$R/ready" usr/local/cuda/bin/nvcc 755
case_ closed; std "$R/closed"; ver "$R/closed" "$CLOSED"
case_ wrong; std "$R/wrong"; ver "$R/wrong" 'NVIDIA UNIX Open Kernel Module for aarch64  575.51.03  Release Build'
case_ wrong_closed; std "$R/wrong_closed"; ver "$R/wrong_closed" 'NVIDIA UNIX Kernel Module  999.1  x'
case_ prefix_ver; std "$R/prefix_ver"; ver "$R/prefix_ver" 'v 9.9.9 Module 580.173.02 Open Kernel Module'
case_ no_module_word; std "$R/no_module_word"; ver "$R/no_module_word" 'build 580.173.02 open'
case_ short_runs; std "$R/short_runs"; ver "$R/short_runs" 'Module 12 1. .5 2.. x'
case_ dotless; std "$R/dotless"; ver "$R/dotless" 'Module 12345 then 580.173.02'
case_ run64; std "$R/run64"; ver "$R/run64" "Module $(printf '1.%.0s' $(seq 32)) 580.173.02"
case_ run63; std "$R/run63"; ver "$R/run63" "Module $(printf '1.%.0s' $(seq 31))1 Open Kernel Module"
case_ unparsable; std "$R/unparsable"; ver "$R/unparsable" 'garbage without any version at all'
case_ long_line; std "$R/long_line"; ver "$R/long_line" "Module $(printf 'x%.0s' $(seq 1100)) 580.173.02"
case_ exact120; std "$R/exact120"; ver "$R/exact120" "$(printf 'y%.0s' $(seq 105))"
case_ empty_version; std "$R/empty_version"; : > "$R/empty_version/proc/driver/nvidia/version"
case_ open_split; std "$R/open_split"; ver "$R/open_split" 'Open Kernel Modul 580.173.02 Open Kernel'
case_ trail_alpha; std "$R/trail_alpha"; ver "$R/trail_alpha" 'Module 1a 580.173.02a Open Kernel Module'
case_ node_missing; std "$R/node_missing"; rm "$R/node_missing/dev/nvidia0"
case_ node_perm; std "$R/node_perm"; chmod 000 "$R/node_perm/dev/nvidiactl"; chmod 444 "$R/node_perm/dev/nvidia-uvm"
case_ node_all; std "$R/node_all"; rm "$R/node_all/dev/nvidiactl" "$R/node_all/dev/nvidia0" "$R/node_all/dev/nvidia-uvm"
case_ no_pci_dir; std "$R/no_pci_dir"; rm -rf "$R/no_pci_dir/sys"
case_ no_gpu; std "$R/no_gpu"; rm -rf "$R/no_gpu/sys/bus/pci/devices/000f:01:00.0"
case_ audio_only; std "$R/audio_only"; rm -rf "$R/audio_only/sys/bus/pci/devices/000f:01:00.0"; pci "$R/audio_only" 000f:01:00.1 0x10de 0x22e8 0x040300
case_ unknown; std "$R/unknown"; rm -rf "$R/unknown/sys/bus/pci/devices/000f:01:00.0"; pci "$R/unknown" 0001:01:00.0 0x10de 0x2684 0x030000
case_ two_gpus; std "$R/two_gpus"; pci "$R/two_gpus" 0002:01:00.0 0x10de 0x2684 0x030200
case_ nl_bdf; std "$R/nl_bdf"; pci "$R/nl_bdf" "$(printf '00:01\nX')" 0x10de 0x1db4 0x030000
case_ long_bdf; std "$R/long_bdf"; pci "$R/long_bdf" "$(printf 'b%.0s' $(seq 70))" 0x10de 0x2e12 0x030000
case_ many; std "$R/many"; for i in $(seq 10 26); do pci "$R/many" "00$i:01:00.0" 0x10de 0x20b0 0x030000; done
case_ pct_bdf; std "$R/pct_bdf"; pci "$R/pct_bdf" '00%s:01%d.0' 0x10de 0x2e12 0x030000
case_ no_meminfo; std "$R/no_meminfo"; rm "$R/no_meminfo/proc/meminfo"
case_ mem_nounified; std "$R/mem_nounified"; rm -rf "$R/mem_nounified/sys/bus/pci/devices/000f:01:00.0"; pci "$R/mem_nounified" 0001:01:00.0 0x10de 0x2684 0x030000
case_ mem_ties; std "$R/mem_ties"; mem "$R/mem_ties" 262144 786432
case_ mem_zero; std "$R/mem_zero"; printf 'MemFree: 5 kB\n' > "$R/mem_zero/proc/meminfo"
case_ mem_huge; std "$R/mem_huge"; mem "$R/mem_huge" 18446744073709551615 9007199254740993
case_ mem_odd; std "$R/mem_odd"; mem "$R/mem_odd" 1153433 104857
case_ nvcc_usr; std "$R/nvcc_usr"; nvcc "$R/nvcc_usr" usr/local/cuda/bin/nvcc 644; nvcc "$R/nvcc_usr" usr/bin/nvcc 755
case_ nvcc_noexec; std "$R/nvcc_noexec"; nvcc "$R/nvcc_noexec" usr/bin/nvcc 644
case_ one_fail; std "$R/one_fail"; rm "$R/one_fail/dev/nvidia-uvm"

LONG=$tmp/$(printf 'L%.0s' $(seq 4300))
run_all() { # BIN PREFIX
  for c in $CASES; do
    rc=0; CUDAPP_DOCTOR_ROOT="$R/$c" "$1" > "$2.$c.out" 2> "$2.$c.err" || rc=$?; echo "$rc" > "$2.$c.rc"
  done
  rc=0; CUDAPP_DOCTOR_ROOT="$LONG" "$1" > "$2.long.out" 2> "$2.long.err" || rc=$?; echo "$rc" > "$2.long.rc"
  # The real machine (CUDAPP_DOCTOR_ROOT unset): its MemAvailable changes between two runs, so the
  # memory line is dropped before the comparison.
  { rc=0; env -u CUDAPP_DOCTOR_ROOT "$1" 2> "$2.real.err" || rc=$?; echo "$rc" > "$2.real.rc"; } \
    | { grep -v '^  [a-z]* *memory: ' || true; } > "$2.real.out"
}
ALL="$CASES long real"
same() { # PREFIX_A PREFIX_B CASE
  cmp -s "$1.$3.out" "$2.$3.out" && cmp -s "$1.$3.err" "$2.$3.err" && cmp -s "$1.$3.rc" "$2.$3.rc"
}
run_all "$tmp/doc_c" "$tmp/c"
run_all "$tmp/doc_om" "$tmp/om"
machines=0
for c in $ALL; do
  same "$tmp/c" "$tmp/om" "$c" || { echo "FAIL differential $c"; diff "$tmp/c.$c.out" "$tmp/om.$c.out" | head -20; exit 1; }
  machines=$((machines + 1))
done
rcs=$(for c in $ALL; do cat "$tmp/c.$c.rc"; done | sort | uniq -c | tr -s ' \n' ' ')
echo "differential: $machines fake machines identical (stdout, stderr, exit; exit histogram:$rcs)"
tom_flip "$tmp/c.mem_odd.out" "$tmp/om.mem_odd.out"

# Documented deviation: the C printed the ABI and compiler texts as part of a printf format; the unit
# refuses a % or newline in them with exit 2 instead of printing what printf would make of it.
$CC $TOM_CFLAGS '-DCUDAPP_NVIDIA_OPEN_ABI="580%d"' -o "$tmp/doc_pct" $SRC src/cudapp_om_doctor.c $CHIP
rc=0; CUDAPP_DOCTOR_ROOT="$R/ready" "$tmp/doc_pct" > "$tmp/pct.out" 2> "$tmp/pct.err" || rc=$?
[ "$rc" = 2 ] && [ ! -s "$tmp/pct.out" ] && grep -qF 'omega_doctor refused (status 0, code 1)' "$tmp/pct.err" \
  || { echo "FAIL build-text refusal: exit $rc"; exit 1; }
echo "build-text refusal: ABI holding % exits 2 with no stdout"

# ---- mutants of the composed unit (fmt.om then doctor.om, as om/units.sh composes it) ----
UNIT=$tmp/doctor_unit.om
{ cat om/tools/fmt.om; echo; cat om/tools/doctor.om; } > "$UNIT"
killed=0
mut() { # NAME SITE REPLACEMENT
  tom_mutant "$1" "$UNIT" "$2" "$3"
  $CC $TOM_CFLAGS "$ABI" -o "$tmp/$1.bin" $SRC "$tmp/$1.c" $CHIP
  run_all "$tmp/$1.bin" "$tmp/$1"
  for c in $ALL; do
    if ! same "$tmp/c" "$tmp/$1" "$c"; then echo "mutant $1: killed ($c)"; killed=$((killed + 1)); return 0; fi
  done
  echo "FAIL mutant $1 survived"; exit 1
}
mut trunc120 'k = ulong 120' 'k = ulong 119'
mut runmin 'if gt k 2' 'if gt k 3'
mut runmax 'if lt k 64' 'if lt k 63'
mut unified 'put st 1 1' 'put st 1 0'
mut clamp16 'n = ulong 16' 'n = ulong 15'
mut prefix 'fi = add add at 6 get f add g 1' 'fi = add add at 5 get f add g 1'
mut gib_prec 'n = fm_f div f64 v DCMB 1 t' 'n = fm_f div f64 v DCMB 2 t'
mut plural 'if ne nf 1' 'if gt nf 2'
mut closed 'if eq dc_open b at add at n 0' 'if ne dc_open b at add at n 0'
mut moreskip 's = add k 1' 's = add k 2'
echo "doctor mutants: $killed killed"

if [ -n "${TOOLS_OM_COVERAGE:-}" ]; then
  mkdir -p "$TOOLS_OM_COVERAGE" "$tmp/cov"
  cp $SRC "$tmp/cov/cudapp_doctor.c"
  (cd "$tmp/cov" && $CC -std=gnu11 -D_GNU_SOURCE -O0 --coverage -I"$ROOT/src" "$ABI" $OFF -o doc_cov cudapp_doctor.c "$ROOT/$CHIP")
  run_all "$tmp/cov/doc_cov" "$tmp/cov/r"
  (cd "$tmp/cov" && gcov -b -c doc_cov-cudapp_doctor.c > "$TOOLS_OM_COVERAGE/doctor.gcov.txt" 2>&1)
  cp "$tmp/cov/cudapp_doctor.c.gcov" "$TOOLS_OM_COVERAGE/cudapp_doctor.c.gcov"
  echo "coverage: $TOOLS_OM_COVERAGE/doctor.gcov.txt"
fi
echo "tools-om doctor: PASS (differential $machines machines, negative control, build-text refusal, $killed mutants killed)"
