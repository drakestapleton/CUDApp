#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Hardware capability manifest gate (DRA-400/401): builds hwman from backend/hw/*.om with the pinned
# seeded omc, checks backend/hw/manifest.tsv, runs the full target lifecycle on every admitted row
# (discover -> claim -> prepare -> submit -> complete -> quiesce -> release) and every refusal control.
# Everything that decides is seeded Omega (hwman); this script only coordinates and compares with
# independent oracles:
#   CPU   values computed in the Wolfram Language (queries recorded below), compared through EXPECT
#   TPM   tpm2_getcap properties-fixed (tpm2-tools, ESAPI) for manufacturer and firmware words
# Results are CHIP_TESTED only for the device the run actually touched; every other row is unchanged.
# The CPU runs must be pinned to one processor of the admitted row (default: processor 19, a
# Cortex-X925 on GB10); HW_CPU selects another.
#
# usage: run.sh /abs/path/to/pinned-omc
# Environment: HW_CPU (processor, default 19), HW_OUT (output dir to keep; default a new temp dir),
#   HW_NO_TPM=1 makes every TPM check NOT_RUN.
# Exit: 0 HW PASS (NOT_RUN checks listed); 1 FAIL; 64 usage; 69 pin refused; 74 I/O;
# 77 NOT_RUN (not Linux AArch64).
set -u
LC_ALL=C
export LC_ALL

if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ]; then
  echo 'NOT_RUN: the hardware manifest gate needs Linux AArch64 and the pinned seeded omc' >&2
  exit 77
fi
if [ "$#" -ne 1 ]; then echo 'usage: run.sh /absolute/path/to/pinned-omc' >&2; exit 64; fi
omc=$1
case $omc in /*) ;; *) echo 'usage: omc path must be absolute' >&2; exit 64 ;; esac
# omc: omega-mv e5b32567 seed/out/omc, equal to its committed seed/out/omc.sha256.
omc_pin=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
digest() { d=$(sha256sum < "$1") || return 1; printf '%s\n' "${d%% *}"; }
[ "$(digest "$omc")" = "$omc_pin" ] || { echo "REFUSED: $omc digest differs from pin $omc_pin" >&2; exit 69; }

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
out=${HW_OUT:-$(mktemp -d "${TMPDIR:-/tmp}/cudapp-hw.XXXXXX")} || exit 74
mkdir -p "$out" || exit 74
cpu=${HW_CPU:-19}
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
# mutate NAME AWKPROGRAM: write $out/NAME.tsv from the manifest through awk (tab-separated)
mutate() { awk -F '\t' -v OFS='\t' "$2" "$M" > "$out/$1.tsv" || exit 74; }
# says NAME PATTERN: the last line of $out/NAME.log must match the grep pattern (the refusal class)
says() { tail -n 1 "$out/$1.log" | grep -q "$2" && ok "$1-class $2" || bad "$1-class want $2 got $(tail -n 1 "$out/$1.log")"; }

# 1. Build hwman from the four units with the pinned omc.
M=$root/backend/hw/manifest.tsv
H=$out/hwman
srcs="$root/backend/hw/core.om $root/backend/hw/cpu.om $root/backend/hw/tpm.om $root/backend/hw/main.om"
# shellcheck disable=SC2086
cat $srcs | timeout 120 "$omc" > "$H" 2> "$out/hwman.compile.err"
rc=$?
if [ "$rc" -ne 0 ] || [ -s "$out/hwman.compile.err" ] || [ ! -s "$H" ]; then
  cat "$out/hwman.compile.err"; echo "FAIL compile hwman exit $rc"; exit 1
fi
chmod 700 "$H" || exit 74
for s in $srcs "$M"; do echo "SRC ${s#"$root"/} $(digest "$s")"; done
echo "COMPILE hwman bin=$(digest "$H")"
P="taskset -c $cpu"

# 2. Manifest: the checked-in register passes; every mutated copy is refused (exit 3).
expect 0 manifest-check "$H" check "$M"
expect 0 manifest-doctor "$H" doctor "$M"
grep -q '^CHECK rows=[0-9]* families=21 admitted=2$' "$out/manifest-check.log" \
  && ok "manifest-families $(cat "$out/manifest-check.log")" || bad "manifest-families $(cat "$out/manifest-check.log")"
mutate m-schema 'NR==1{$0="hwman\t2"}1';                     expect 3 neg-manifest-schema "$H" check "$out/m-schema.tsv"
mutate m-header 'NR==2{$3="maker"}1';                        expect 3 neg-manifest-header "$H" check "$out/m-header.tsv"
mutate m-cols '$1=="tpm-ntc-npct75x"{NF=24}1';               expect 3 neg-manifest-fieldcount "$H" check "$out/m-cols.tsv"
mutate m-empty '$1=="qpu-ibm"{$3=""}1';                      expect 3 neg-manifest-emptyfield "$H" check "$out/m-empty.tsv"
mutate m-idch '$1=="qpu-ibm"{$1="QPU_ibm"}1';                expect 3 neg-manifest-idcharset "$H" check "$out/m-idch.tsv"
mutate m-dup '$1=="qpu-ibm"{$1="lpu-groq"}1';                expect 3 neg-manifest-duplicate "$H" check "$out/m-dup.tsv"
mutate m-fam '$1=="qpu-ibm"{$2="QUANTUM"}1';                 expect 3 neg-manifest-family "$H" check "$out/m-fam.tsv"
mutate m-admit '$1=="cpu-gb10-a725"{$23="YES";$25="-"}1';    expect 3 neg-manifest-admit-without-evidence "$H" check "$out/m-admit.tsv"
mutate m-blk '$1=="lpu-groq"{$25="-"}1';                     expect 3 neg-manifest-blocked-without-blocker "$H" check "$out/m-blk.tsv"
mutate m-unres '$1=="ppu-unresolved"{$22="BLOCKED_ACCESS"}1'; expect 3 neg-manifest-unresolved-route "$H" check "$out/m-unres.tsv"
mutate m-tmo '$1=="cpu-gb10-x925"{$21="2s"}1';               expect 3 neg-manifest-timeout "$H" check "$out/m-tmo.tsv"
mutate m-miss '$2!="BPU"';                                   expect 3 neg-manifest-missing-family "$H" check "$out/m-miss.tsv"
expect 74 neg-manifest-absent "$H" check "$out/absent.tsv"
expect 2 neg-usage "$H" run "$M"

# 3. Admission.
expect 0 admit-cpu "$H" admit "$M" sumsq64 cpu-gb10-x925
expect 0 admit-any-getrandom "$H" admit "$M" getrandom
expect 4 neg-admit-unknown-target "$H" admit "$M" sumsq64 cpu-gb10-x926
expect 5 neg-admit-unsupported-op "$H" admit "$M" getrandom cpu-gb10-x925
expect 5 neg-admit-no-target "$H" admit "$M" matmul
expect 6 neg-admit-not-admitted "$H" admit "$M" sumsq64 cpu-gb10-a725
expect 6 neg-admit-gpu-lease "$H" admit "$M" cudapp-kernels gpu-gb10-sm121
mutate m-auth '$1=="cpu-gb10-x925"{$6="DIRECT_DEVICE"}1'
expect 6 neg-admit-wrong-authority "$H" admit "$out/m-auth.tsv" sumsq64 cpu-gb10-x925
mutate m-off '$1=="cpu-gb10-x925"{$9="/sys/devices/system/nocpu"}1'
expect 10 neg-admit-offline "$H" admit "$out/m-off.tsv" sumsq64 cpu-gb10-x925

# 4. CPU lifecycle. Oracles, Wolfram Language (WolframLanguageEvaluator):
#   ss[n_]:=Mod[Sum[i^2,{i,0,n-1}],2^64]
#   fnv[n_]:=Fold[Mod[BitXor[#1,#2]*1099511628211,2^64]&,14695981039346656037,Mod[Range[0,n-1],251]]
#   {ss[1],ss[1000],ss[16777216],fnv[1],fnv[1000],fnv[1000000]} ->
#   {0,332833500,6148773953750958080,12638153115695167455,10432030352173600877,4529795169249947525}
S=$out/cpu.state
rm -f "$S"
g=0
for c in sumsq64:1:0 sumsq64:1000:332833500 sumsq64:16777216:6148773953750958080 \
         fnv1a64:1:12638153115695167455 fnv1a64:1000:10432030352173600877 fnv1a64:1000000:4529795169249947525; do
  op=${c%%:*}; r=${c#*:}; n=${r%%:*}; v=${r#*:}
  expect 0 "cpu-$op-$n" $P "$H" run "$M" cpu-gb10-x925 "$op" "$n" "$g" "$S" "$v"
  g=$((g + 1))
done
[ "$(cat "$S")" = "$g cpu-gb10-x925" ] && ok "cpu-handle generation=$g" || bad "cpu-handle $(cat "$S")"
grep -q "midr_el1=0x410fd851" "$out/cpu-sumsq64-1.log" && ok "cpu-identity $(head -n 1 "$out/cpu-sumsq64-1.log")" \
  || bad "cpu-identity $(head -n 1 "$out/cpu-sumsq64-1.log")"
expect 7 neg-cpu-stale-replay $P "$H" run "$M" cpu-gb10-x925 sumsq64 1000 0 "$S" 332833500
expect 12 neg-cpu-oracle-mismatch $P "$H" run "$M" cpu-gb10-x925 sumsq64 1000 "$g" "$S" 332833501
g=$((g + 1))
expect 14 neg-cpu-arg-zero $P "$H" run "$M" cpu-gb10-x925 sumsq64 0 "$g" "$S"
expect 14 neg-cpu-arg-over $P "$H" run "$M" cpu-gb10-x925 fnv1a64 16777217 "$g" "$S"
expect 14 neg-cpu-arg-malformed $P "$H" run "$M" cpu-gb10-x925 fnv1a64 12x "$g" "$S"
expect 14 neg-cpu-gen-malformed $P "$H" run "$M" cpu-gb10-x925 fnv1a64 1 -1 "$S"
expect 5 neg-cpu-unsupported-op $P "$H" run "$M" cpu-gb10-x925 getrandom 1 "$g" "$S"
expect 4 neg-cpu-unknown-target $P "$H" run "$M" cpu-gb10-x9250 sumsq64 1 "$g" "$S"
mutate m-midr '$1=="cpu-gb10-x925"{$8="midr_el1=0x410fd871"}1'
expect 8 neg-cpu-wrong-device $P "$H" run "$out/m-midr.tsv" cpu-gb10-x925 sumsq64 1 "$g" "$S" 0
expect 6 neg-cpu-wrong-authority $P "$H" run "$out/m-auth.tsv" cpu-gb10-x925 sumsq64 1 "$g" "$S" 0
expect 10 neg-cpu-offline $P "$H" run "$out/m-off.tsv" cpu-gb10-x925 sumsq64 1 "$g" "$S" 0
expect 6 neg-gpu-wrong-authority $P "$H" run "$M" gpu-gb10-sm121 cudapp-kernels 1 0 "$out/gpu.state"
[ ! -e "$out/gpu.state" ] && ok "gpu-no-claim state file never created" || bad "gpu-no-claim state file created"
mutate m-t0 '$1=="cpu-gb10-x925"{$21="0"}1'
expect 11 neg-cpu-timeout $P "$H" run "$out/m-t0.tsv" cpu-gb10-x925 sumsq64 16777216 "$g" "$S" 6148773953750958080
says neg-cpu-timeout '^REFUSE timeout elapsed_ns=[0-9]* budget_ns=0$'
g=$((g + 1))
# busy: another process holds the handle lock
flock -x "$S" sleep 3 &
lp=$!
sleep 1
expect 9 neg-cpu-busy $P "$H" run "$M" cpu-gb10-x925 sumsq64 1 "$g" "$S" 0
wait "$lp"
expect 0 cpu-after-busy $P "$H" run "$M" cpu-gb10-x925 sumsq64 1 "$g" "$S" 0
g=$((g + 1))
printf '0 tpm-ntc-npct75x\n' > "$out/wrong.state"
expect 7 neg-cpu-wrong-handle $P "$H" run "$M" cpu-gb10-x925 sumsq64 1 0 "$out/wrong.state" 0
printf '7x cpu-gb10-x925\n' > "$out/corrupt.state"
expect 7 neg-cpu-corrupt-handle $P "$H" run "$M" cpu-gb10-x925 sumsq64 1 7 "$out/corrupt.state" 0
notrun "neg-cpu-unpinned: needs a CPU set wider than the one processor this run is allotted"
notrun "cpu-gb10-a725: no Cortex-A725 processor allotted to this run"

# 5. TPM lifecycle against tpm2_getcap (independent ESAPI client).
T=$out/tpm.state
rm -f "$T"
if [ "${HW_NO_TPM:-0}" = 1 ] || [ ! -e /dev/tpmrm0 ]; then
  notrun "tpm: HW_NO_TPM=1 or /dev/tpmrm0 absent"
else
  expect 0 tpm-props $P "$H" run "$M" tpm-ntc-npct75x props 0 0 "$T"
  if command -v tpm2_getcap > /dev/null 2>&1; then
    tpm2_getcap -T device:/dev/tpmrm0 properties-fixed > "$out/tpm2_getcap.txt" 2>&1
    raw() { awk -v k="$1:" '$1==k{getline; print tolower($2); exit}' "$out/tpm2_getcap.txt"; }
    pad() { printf '0x%08x' "$1"; }
    want="tpm_manufacturer=$(pad "$(raw TPM2_PT_MANUFACTURER)") firmware=$(pad "$(raw TPM2_PT_FIRMWARE_VERSION_1)").$(pad "$(raw TPM2_PT_FIRMWARE_VERSION_2)")"
    grep -q "^RECEIPT .* $want " "$out/tpm-props.log" && ok "tpm-props-oracle $want" || bad "tpm-props-oracle want $want got $(tail -n 1 "$out/tpm-props.log")"
  else
    notrun "tpm-props-oracle: tpm2_getcap absent"
  fi
  expect 0 tpm-getrandom-a $P "$H" run "$M" tpm-ntc-npct75x getrandom 32 1 "$T"
  expect 0 tpm-getrandom-b $P "$H" run "$M" tpm-ntc-npct75x getrandom 32 2 "$T"
  ra=$(sed -n 's/^RECEIPT .* result=\([0-9a-f]*\) .*/\1/p' "$out/tpm-getrandom-a.log")
  rb=$(sed -n 's/^RECEIPT .* result=\([0-9a-f]*\) .*/\1/p' "$out/tpm-getrandom-b.log")
  [ "${#ra}" -eq 64 ] && [ "${#rb}" -eq 64 ] && [ "$ra" != "$rb" ] && ok "tpm-getrandom-distinct 32+32 bytes" \
    || bad "tpm-getrandom-distinct a=$ra b=$rb"
  expect 7 neg-tpm-stale-replay $P "$H" run "$M" tpm-ntc-npct75x getrandom 32 1 "$T"
  expect 14 neg-tpm-arg-over $P "$H" run "$M" tpm-ntc-npct75x getrandom 33 3 "$T"
  expect 14 neg-tpm-props-arg $P "$H" run "$M" tpm-ntc-npct75x props 1 3 "$T"
  expect 5 neg-tpm-unsupported-op $P "$H" run "$M" tpm-ntc-npct75x sumsq64 1 3 "$T"
  expect 0 tpm-fault-badsize $P "$H" fault "$M" tpm-ntc-npct75x badsize 3 "$T"
  expect 0 tpm-fault-badcc $P "$H" fault "$M" tpm-ntc-npct75x badcc 4 "$T"
  expect 5 neg-tpm-fault-kind $P "$H" fault "$M" tpm-ntc-npct75x badlen 5 "$T"
  expect 5 neg-cpu-fault $P "$H" fault "$M" cpu-gb10-x925 badsize 0 "$out/cpu-fault.state"
  [ "$(cat "$T")" = "5 tpm-ntc-npct75x" ] && ok "tpm-handle generation=5" || bad "tpm-handle $(cat "$T")"
  mutate m-tid '$1=="tpm-ntc-npct75x"{$8="manufacturer=0x49465800"}1'
  expect 8 neg-tpm-wrong-device $P "$H" run "$out/m-tid.tsv" tpm-ntc-npct75x props 0 5 "$T"
  mutate m-toff '$1=="tpm-ntc-npct75x"{$9="/dev/tpmrm9"}1'
  expect 10 neg-tpm-offline $P "$H" run "$out/m-toff.tsv" tpm-ntc-npct75x props 0 5 "$T"
  mutate m-tt0 '$1=="tpm-ntc-npct75x"{$21="0"}1'
  expect 11 neg-tpm-timeout $P "$H" run "$out/m-tt0.tsv" tpm-ntc-npct75x props 0 5 "$T"
  says neg-tpm-timeout '^REFUSE timeout elapsed_ns=[0-9]* budget_ns=0$'
fi

echo "HW pass=$npass fail=$nfail not_run=$nnr"
[ "$nfail" -eq 0 ] || exit 1
exit 0
