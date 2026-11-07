#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host gate for the cloud-window receipt checker tools/access/rcheck.om (docs/access/RECEIPT.md).
# It compiles rcheck.om with the pinned seeded omc, runs it on three SYNTHETIC fixtures that must be
# accepted, on one-line mutants of them that must be refused for a named reason, and on a mutant of
# the checker itself (the pass-on-a-CPU rule removed) that must then accept the receipt the real
# checker refuses. It opens no GPU and touches no network. Every fixture says SYNTHETIC in its
# window id, and the checker prints ACCEPT-SYNTHETIC for those, so none can read as a device receipt.
#
# usage: run.sh /abs/path/to/pinned-omc
# Exit: 0 ACCESS GATE PASS; 1 FAIL; 64 usage; 69 pin refused; 74 I/O; 77 NOT_RUN (not Linux AArch64).
set -u
LC_ALL=C
export LC_ALL

if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ]; then
  echo 'NOT_RUN: the access gate needs Linux AArch64 and the pinned seeded omc' >&2
  exit 77
fi
if [ "$#" -ne 1 ]; then echo 'usage: run.sh /absolute/path/to/pinned-omc' >&2; exit 64; fi
omc=$1
case $omc in /*) ;; *) echo 'usage: omc path must be absolute' >&2; exit 64 ;; esac
# omc: omega-mv e5b32567 seed/out/omc, the same pin as tests/multitarget/run.sh.
omc_pin=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
digest() { d=$(sha256sum < "$1") || return 1; printf '%s\n' "${d%% *}"; }
[ "$(digest "$omc")" = "$omc_pin" ] || { echo "REFUSED: $omc digest differs from pin $omc_pin" >&2; exit 69; }

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
fx=$root/tests/access/fixtures
out=${ACCESS_OUT:-$(mktemp -d "${TMPDIR:-/tmp}/cudapp-access.XXXXXX")} || exit 74
mkdir -p "$out" || exit 74
echo "PIN omc $omc_pin"
echo "Outputs: $out"

npass=0; nfail=0
ok() { npass=$((npass + 1)); echo "PASS $*"; }
bad() { nfail=$((nfail + 1)); echo "FAIL $*"; }

build() { # build NAME SRC
  timeout 120 "$omc" < "$2" > "$out/$1" 2> "$out/$1.compile.err"
  rc=$?
  if [ "$rc" -ne 0 ] || [ -s "$out/$1.compile.err" ] || [ ! -s "$out/$1" ]; then
    cat "$out/$1.compile.err"; echo "FAIL compile $1 exit $rc"; exit 1
  fi
  chmod 700 "$out/$1" || exit 74
  echo "COMPILE $1 src=$(digest "$2") bin=$(digest "$out/$1")"
}
build rcheck "$root/tools/access/rcheck.om"
R=$out/rcheck

# check NAME WANT_EXIT WANT_TEXT FILE [CHECKER]: the checker must exit WANT_EXIT, and its one output
# line (stdout on accept, stderr on refuse) must contain WANT_TEXT.
check() {
  name=$1; want=$2; text=$3; file=$4; prog=${5:-$R}
  "$prog" < "$file" > "$out/$name.out" 2> "$out/$name.err"
  got=$?
  line=$(cat "$out/$name.out" "$out/$name.err")
  case $line in
    *"$text"*) hit=1 ;;
    *) hit=0 ;;
  esac
  if [ "$got" -eq "$want" ] && [ "$hit" -eq 1 ]; then ok "$name exit=$got $line"
  else bad "$name exit=$got want=$want text='$text' got='$line'"; fi
}
# mutant NAME BASE SED_EXPR: a copy of fixture BASE with one sed edit; refuses an identical copy.
mutant() {
  sed "$3" "$fx/$2" > "$out/m-$1.txt" || exit 74
  if cmp -s "$out/m-$1.txt" "$fx/$2"; then bad "mutant $1 is identical to $2"; fi
}

# 1. Fixtures that must be accepted, with exact counts.
check ok-amd 0 'RCHECK ACCEPT-SYNTHETIC provider=amd-devcloud claim=ACCESS_TESTED rows=4 pass=2 fail=0 not_run=1 missing=1 blocked=0' "$fx/amd-ok.txt"
check ok-intel 0 'RCHECK ACCEPT-SYNTHETIC provider=intel-cloud claim=ACCESS_TESTED rows=5 pass=4 fail=1 not_run=0 missing=0 blocked=0' "$fx/intel-ok.txt"
check ok-tpu-blocked 0 'RCHECK ACCEPT-SYNTHETIC provider=google-trc claim=NONE rows=2 pass=0 fail=0 not_run=0 missing=0 blocked=2' "$fx/tpu-blocked.txt"
# A real window id prints plain ACCEPT (the SYNTHETIC tag is the only difference).
mutant realwin amd-ok.txt 's/window=SYNTHETIC-amd-mi300x-fixture/window=w0-fixture-copy/'
check ok-realwin 0 'RCHECK ACCEPT provider=amd-devcloud' "$out/m-realwin.txt"

# 2. One-line mutants that must be refused, each for its own reason.
mutant pass-on-cpu amd-ok.txt 's/^ROW id=ref-sgemm \(.*\) dev=d0 /ROW id=ref-sgemm \1 dev=d7 /'
check neg-pass-on-cpu 1 'line=10 reason=pass-on-a-device-that-is-not-the-provider-accelerator' "$out/m-pass-on-cpu.txt"
mutant wrong-vendor amd-ok.txt 's/^DEVICE id=d0 vendor=amd /DEVICE id=d0 vendor=intel /'
check neg-wrong-vendor 1 'line=8 reason=pass-on-a-device-that-is-not-the-provider-accelerator' "$out/m-wrong-vendor.txt"
mutant tpu-as-gpu tpu-blocked.txt 's/^ROW id=inv-acceltype .*/DEVICE id=d0 vendor=google class=gpu model=x\nROW id=inv-acceltype kind=inventory verdict=PASS dev=d0 exit=0 log=l/'
check neg-tpu-window-gpu-device 1 'reason=pass-on-a-device-that-is-not-the-provider-accelerator' "$out/m-tpu-as-gpu.txt"
mutant undeclared-dev amd-ok.txt 's/^ROW id=ref-sgemm \(.*\) dev=d0 /ROW id=ref-sgemm \1 dev=d3 /'
check neg-undeclared-dev 1 'reason=row-dev-not-a-declared-device' "$out/m-undeclared-dev.txt"
mutant nonzero-exit amd-ok.txt 's/^\(ROW id=ref-sgemm .*\) exit=0 /\1 exit=3 /'
check neg-pass-nonzero-exit 1 'reason=pass-with-nonzero-exit' "$out/m-nonzero-exit.txt"
mutant no-log amd-ok.txt 's/ log=rows\/ref-sgemm.log$//'
check neg-pass-no-log 1 'reason=row-pass-or-fail-needs-log' "$out/m-no-log.txt"
mutant end-count amd-ok.txt 's/^END rows=4$/END rows=5/'
check neg-end-count 1 'reason=END-rows-differs-from-ROW-lines' "$out/m-end-count.txt"
mutant no-teardown amd-ok.txt '/^TEARDOWN /d'
check neg-no-teardown 1 'reason=missing-TEARDOWN' "$out/m-no-teardown.txt"
mutant not-destroyed amd-ok.txt 's/destroyed=yes/destroyed=no/'
check neg-claim-not-destroyed 1 'reason=ACCESS_TESTED-needs-inventory-and-reference-pass-on-the-accelerator-and-destroyed-yes' "$out/m-not-destroyed.txt"
mutant no-reference amd-ok.txt 's/^\(ROW id=ref-sgemm .*\) verdict=PASS /\1 verdict=FAIL /'
check neg-claim-no-reference 1 'reason=ACCESS_TESTED-needs-inventory-and-reference' "$out/m-no-reference.txt"
mutant short-rev amd-ok.txt 's/rev=0123456789abcdef0123456789abcdef01234567/rev=0123456/'
check neg-short-rev 1 'line=2 reason=src-rev-not-40-lowercase-hex' "$out/m-short-rev.txt"
mutant upper-rev amd-ok.txt 's/rev=0123456789abcdef/rev=0123456789ABCDEF/'
check neg-upper-rev 1 'reason=src-rev-not-40-lowercase-hex' "$out/m-upper-rev.txt"
mutant verdict-ok amd-ok.txt 's/^\(ROW id=inv-rocminfo .*\) verdict=PASS /\1 verdict=OK /'
check neg-verdict-word 1 'reason=row-verdict-unknown' "$out/m-verdict-ok.txt"
mutant after-end amd-ok.txt '$a\
ROW id=late kind=inventory verdict=NOT_RUN dev=none'
check neg-line-after-end 1 'reason=line-after-END' "$out/m-after-end.txt"
mutant cand-no-control intel-ok.txt 's/^\(ROW id=ctl-vadd-mutant .*\) verdict=PASS /\1 verdict=FAIL /'
check neg-candidate-without-control 1 'reason=candidate-pass-without-a-passing-control' "$out/m-cand-no-control.txt"
mutant control-want0 intel-ok.txt 's/^ROW id=ctl-vadd-mutant kind=control want=1 \(.*\) exit=1 /ROW id=ctl-vadd-mutant kind=control want=0 \1 exit=0 /'
check neg-control-want-zero 1 'reason=control-pass-needs-nonzero-want-equal-to-exit' "$out/m-control-want0.txt"
mutant control-mismatch intel-ok.txt 's/^\(ROW id=ctl-vadd-mutant .*\) exit=1 /\1 exit=2 /'
check neg-control-exit-differs 1 'reason=control-pass-needs-nonzero-want-equal-to-exit' "$out/m-control-mismatch.txt"
mutant header-twice amd-ok.txt '2i\
ACCESS1 provider=amd-devcloud window=x claim=NONE'
check neg-header-twice 1 'line=2 reason=header-repeated' "$out/m-header-twice.txt"
mutant no-header amd-ok.txt '1d'
check neg-first-line 1 'line=1 reason=first-line-must-be-ACCESS1' "$out/m-no-header.txt"
mutant bad-provider amd-ok.txt 's/provider=amd-devcloud/provider=amd-cloud/'
check neg-provider 1 'line=1 reason=unknown-provider' "$out/m-bad-provider.txt"
mutant bad-claim amd-ok.txt 's/claim=ACCESS_TESTED/claim=QUALIFIED/'
check neg-claim-word 1 'line=1 reason=unknown-claim' "$out/m-bad-claim.txt"
mutant unknown-tag amd-ok.txt 's/^TOOL /TOOLS /'
check neg-unknown-tag 1 'line=7 reason=unknown-line-tag' "$out/m-unknown-tag.txt"
mutant bad-usd amd-ok.txt 's/charged_usd=3.89/charged_usd=3.89.1/'
check neg-cost-format 1 'reason=cost-needs-decimal-charged_usd' "$out/m-bad-usd.txt"
mutant no-cost amd-ok.txt '/^COST /d'
check neg-no-cost 1 'reason=missing-COST' "$out/m-no-cost.txt"
mutant dup-device amd-ok.txt 's/^DEVICE id=d7 /DEVICE id=d0 /'
check neg-device-repeated 1 'line=6 reason=device-id-repeated' "$out/m-dup-device.txt"
mutant device-id amd-ok.txt 's/^DEVICE id=d7 /DEVICE id=d8 /'
check neg-device-id 1 'reason=device-id-not-d0-to-d7' "$out/m-device-id.txt"
mutant no-end amd-ok.txt '/^END /d'
check neg-no-end 1 'reason=missing-END' "$out/m-no-end.txt"
mutant teardown-no-by amd-ok.txt 's/ by=SYNTHETIC / /'
check neg-teardown-by 1 'reason=teardown-missing-by-or-verified_at' "$out/m-teardown-no-by.txt"
mutant no-window amd-ok.txt 's/ window=SYNTHETIC-amd-mi300x-fixture//'
check neg-no-window 1 'line=1 reason=missing-window' "$out/m-no-window.txt"
mutant no-repo amd-ok.txt 's/^SRC repo=aien-dev\/CUDApp /SRC /'
check neg-src-no-repo 1 'line=2 reason=src-missing-repo' "$out/m-no-repo.txt"
mutant no-src amd-ok.txt '/^SRC /d'
check neg-no-src 1 'reason=missing-SRC' "$out/m-no-src.txt"
mutant vendor-nvidia amd-ok.txt 's/^DEVICE id=d0 vendor=amd /DEVICE id=d0 vendor=nvidia /'
check neg-device-vendor 1 'line=5 reason=device-vendor-unknown' "$out/m-vendor-nvidia.txt"
mutant class-npu amd-ok.txt 's/^DEVICE id=d0 vendor=amd class=gpu /DEVICE id=d0 vendor=amd class=npu /'
check neg-device-class 1 'line=5 reason=device-class-unknown' "$out/m-class-npu.txt"
mutant no-model amd-ok.txt 's/^DEVICE id=d0 \(.*\) model=SYNTHETIC-fixture /DEVICE id=d0 \1 /'
check neg-device-model 1 'line=5 reason=device-missing-model' "$out/m-no-model.txt"
mutant kind-word amd-ok.txt 's/^ROW id=inv-rocminfo kind=inventory /ROW id=inv-rocminfo kind=smoke /'
check neg-row-kind 1 'line=8 reason=row-kind-unknown' "$out/m-kind-word.txt"
mutant row-no-id amd-ok.txt 's/^ROW id=inv-rocminfo /ROW /'
check neg-row-id 1 'line=8 reason=row-missing-id' "$out/m-row-no-id.txt"
mutant destroyed-maybe amd-ok.txt 's/destroyed=yes/destroyed=maybe/'
check neg-destroyed-word 1 'reason=teardown-destroyed-must-be-yes-or-no' "$out/m-destroyed-maybe.txt"
mutant teardown-twice amd-ok.txt '/^TEARDOWN /p'
check neg-teardown-twice 1 'reason=teardown-repeated' "$out/m-teardown-twice.txt"
mutant end-word amd-ok.txt 's/^END rows=4$/END rows=four/'
check neg-end-word 1 'reason=end-rows-not-decimal' "$out/m-end-word.txt"
# The key must follow a space: a value that only contains "exit=0" is not an exit field.
mutant embedded-key amd-ok.txt 's/^\(ROW id=ref-sgemm .*\) exit=0 log=rows\/ref-sgemm.log/\1 log=rows\/exit=0/'
check neg-key-inside-value 1 'reason=row-pass-or-fail-needs-decimal-exit' "$out/m-embedded-key.txt"

# 3. Input size and emptiness.
: > "$out/empty.txt"
check neg-empty 1 'reason=empty-receipt' "$out/empty.txt"
head -c 65536 /dev/zero | tr '\0' 'x' > "$out/big.txt"
check neg-over-65535-bytes 2 'reason=input-unreadable-or-over-65535-bytes' "$out/big.txt"

# 4. Checker mutant: with the accelerator rule removed, the CPU-pass receipt is accepted. This shows
#    the gate notices the rule's removal instead of passing for some other reason.
sed 's/^if ne get dcls i ulong 1$/if eq ulong 1 ulong 0/' "$root/tools/access/rcheck.om" > "$out/rcheck-mut.om" || exit 74
if [ "$(grep -c '^if eq ulong 1 ulong 0$' "$out/rcheck-mut.om")" -ne 1 ]; then bad "checker mutant did not change exactly one line"
else
  build rcheck-mut "$out/rcheck-mut.om"
  check mutant-checker-accepts-cpu-pass 0 'RCHECK ACCEPT-SYNTHETIC provider=amd-devcloud' "$out/m-pass-on-cpu.txt" "$out/rcheck-mut"
fi

# 5. The collector end to end on this host (no accelerator here for any of the three providers):
#    run, close, check. A plan whose rows cannot PASS on a CPU is accepted with claim NONE; a plan
#    whose inventory row succeeds on the host CPU is refused; an edited log fails SHA256SUMS.
C=$root/tools/access/collect.sh
export ACCESS_WINDOW=SYNTHETIC-collector-selftest ACCESS_REPO=aien-dev/CUDApp
export ACCESS_REV=0123456789abcdef0123456789abcdef01234567 ACCESS_ROW_TIMEOUT=30
coll() { # coll NAME WANT_EXIT WANT_TEXT PLAN CLAIM [tamper]
  d=$out/coll-$1
  if ! sh "$C" run amd-devcloud "$4" "$d" > "$out/coll-$1.run.log" 2>&1; then bad "collector run $1: $(tail -n 1 "$out/coll-$1.run.log")"; return; fi
  [ "${6:-}" = tamper ] && printf 'x' >> "$d/rows/ctl-false.log"
  RCHECK=$R sh "$C" close "$d" "$5" 0 yes selftest 2026-11-07T00:00:00Z > "$out/coll-$1.close.out" 2> "$out/coll-$1.close.err"
  got=$?
  line=$(tail -n 1 "$out/coll-$1.close.out"; cat "$out/coll-$1.close.err")
  case $line in *"$3"*) hit=1 ;; *) hit=0 ;; esac
  if [ "$got" -eq "$2" ] && [ "$hit" -eq 1 ]; then ok "collector-$1 exit=$got $(printf '%s' "$line" | tail -n 1)"
  else bad "collector-$1 exit=$got want=$2 text='$3' got='$line'"; fi
}
coll accept 0 'RCHECK ACCEPT-SYNTHETIC provider=amd-devcloud claim=NONE rows=3 pass=1 fail=0 not_run=1 missing=1 blocked=0' "$fx/plan-accept.tsv" NONE
coll cpu-pass 1 'reason=pass-on-a-device-that-is-not-the-provider-accelerator' "$fx/plan-cpu-pass.tsv" NONE
coll claim-without-device 1 'reason=ACCESS_TESTED-needs-inventory-and-reference' "$fx/plan-accept.tsv" ACCESS_TESTED
coll tamper 1 'SHA256SUMS check failed' "$fx/plan-accept.tsv" NONE tamper
if grep -q '^DEVICE id=d0 ' "$out/coll-accept/receipt.body"; then bad "collector found an accelerator DEVICE d0 on this host; the self-test assumes none"
else ok "collector-no-accelerator-on-host (no DEVICE id=d0 line)"; fi

echo "ACCESS GATE $( [ "$nfail" -eq 0 ] && echo PASS || echo FAIL ) pass=$npass fail=$nfail"
[ "$nfail" -eq 0 ]
