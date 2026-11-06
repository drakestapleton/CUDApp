#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# cudapp-smi host tier: fixture parsing, refusals, the --rm path against a host fake, and mutants.
# Device-free: the shipped binary only ever reads a fixture root; --rm runs only in the fake build,
# whose OS effects are rewritten out of a private copy of native/nvrm/context.om.
# Usage: tests/smi/run.sh [OMC]   (default $OMEGA_OMC)
set -u
if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ]; then
  echo 'NOT_RUN: cudapp-smi host tier needs Linux AArch64 (the pinned omc emits AArch64 Linux code)' >&2
  exit 77
fi
omc=${1:-${OMEGA_OMC:-}}
if [ -z "$omc" ] || [ ! -x "$omc" ]; then
  echo 'NOT_RUN: set OMEGA_OMC to the pinned seeded omc' >&2
  exit 77
fi
pin=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
got=$(sha256sum "$omc") || exit 74
if [ "${got%% *}" != "$pin" ]; then
  echo "REFUSED: omc digest ${got%% *} is not the omega-mv e5b32567 pin" >&2
  exit 69
fi
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
t=$root/tests/smi
out=$(mktemp -d "${TMPDIR:-/tmp}/cudapp-smi.XXXXXX") || exit 74
trap 'rm -rf "$out"' EXIT
fails=0
fail() { echo "FAIL: $*" >&2; fails=$((fails + 1)); }

# Rewrites OS effects in context.om into the rmfake.om words; the request number is kept.
fakectx() {
  awk '
    $1 == "note" { print; next }
    $2 == "=" && ($3 == "devcall" || $3 == "devctl" || $3 == "uvmctl") {
      q = ""; for (i = 4; i < NF; i++) if ($i == "uint") { q = $(i + 1); break }
      if (q == "") { print "REWRITE_FAILED"; next }
      print $1 " = smdrv " q; next }
    $2 == "=" && $3 == "fopen" { print $1 " = smfopen"; next }
    $2 == "=" && ($3 == "mapdrop" || $3 == "mapnew" || $3 == "mapset" || $3 == "mapread" || $3 == "mapwrite" || $3 == "maphold" || $3 == "mapbind") { print $1 " = smbad"; next }
    { s = ""
      for (i = 1; i <= NF; i++) {
        w = $i
        if (w == "fclose") w = "smclose"
        if (w == "mapget") w = "smget"
        if (w == "mapaddr") w = "smaddr"
        s = (s == "" ? w : s " " w) }
      print s }
  ' "$1"
}

# Device words: none may survive in the fake context; rm.om and smi.om may never hold any.
dev='devcall|devctl|uvmctl|mapnew|mapdrop|mapget|mapset|mapread|mapwrite|maphold|mapbind|mapaddr'
guard() {
  awk -v re="^($dev$2)\$" '$1 != "note" { for (i = 1; i <= NF; i++) if ($i ~ re || $i == "REWRITE_FAILED") { print FILENAME ":" NR ": " $i; bad = 1 } } END { exit bad }' "$1"
}

build() { # smi.om rm.om out [fake]
  if [ "${4:-}" = fake ]; then
    fakectx "$root/native/nvrm/context.om" > "$out/ctx-fake.om" || return 1
    guard "$out/ctx-fake.om" '|fopen|fclose' || { echo 'REFUSED: unbound OS effect in fake context' >&2; exit 69; }
    cat "$1" "$out/ctx-fake.om" "$t/rmfake.om" "$2" > "$3.om"
  else
    cat "$1" "$root/native/nvrm/context.om" "$2" > "$3.om"
  fi
  "$omc" < "$3.om" > "$3" 2> "$3.log" && [ -s "$3" ] && chmod 700 "$3"
}

# Fixture root: recorded GB10 files plus a synthetic /proc with one holder, a duplicate fd,
# a control-node-only fd and a non-GPU process.
mkroot() {
  rm -rf "$1"
  cp -R "$t/fixtures/gb10" "$1" && chmod -R u+w "$1" || exit 74
  mkdir -p "$1/proc/4242/fd" "$1/proc/77/fd" "$1/proc/self" || exit 74
  printf '/opt/x/server\000--port\0008\000' > "$1/proc/4242/cmdline"
  ln -s /dev/nvidia0 "$1/proc/4242/fd/3"
  ln -s /dev/nvidiactl "$1/proc/4242/fd/4"
  ln -s /dev/nvidia0 "$1/proc/4242/fd/5"
  ln -s /dev/null "$1/proc/77/fd/1"
  ln -s /dev/nvidia01 "$1/proc/77/fd/2"
  printf 'idle\n' > "$1/proc/77/comm"
}

gi=proc/driver/nvidia/gpus/000f:01:00.0/information
sy=sys/bus/pci/devices/000f:01:00.0
# name|expected rc|edit command run inside the variant root
variants='
noversion|4|rm proc/driver/nvidia/version
nonvrm|10|printf "Driver: 580.173.02\n" > proc/driver/nvidia/version
nover|10|printf "NVRM version: NVIDIA module  580.  build\n" > proc/driver/nvidia/version
nobus|11|grep -v "^Bus Location" GI > x && mv x GI
nouuid|11|grep -v "^GPU UUID" GI > x && mv x GI
busdiff|12|sed "s/000f:01:00.0/000f:02:00.0/" GI > x && mv x GI
baduuid|12|sed "s/GPU-0123abcd/GPU-0123abcZ/" GI > x && mv x GI
badminor|12|sed "s/^Device Minor:.*/Device Minor: \t 256/" GI > x && mv x GI
wrapminor|12|sed "s/^Device Minor:.*/Device Minor: \t 18446744073709551616/" GI > x && mv x GI
uuidpfx|12|sed "s/GPU-0123abcd/GPX-0123abcd/" GI > x && mv x GI
baddir|12|mkdir proc/driver/nvidia/gpus/bogus
vendor|13|printf "0x1002\n" > SY/vendor
uphex|14|printf "0x2E12\n" > SY/device
nohex|14|printf "2e12\n" > SY/device
widthx|14|printf "x16\n" > SY/max_link_width
speed|15|printf "3.0 GT/s PCIe\n" > SY/current_link_speed
'

check_fixture() { # bin -> 0 when every fixture expectation holds
  b=$1; ok=0
  mkroot "$out/r"
  "$b" --root "$out/r" > "$out/fx.out" 2> "$out/fx.err"; rc=$?
  if [ "$rc" -ne 0 ] || ! cmp -s "$out/fx.out" "$t/expected-gb10.txt" || [ -s "$out/fx.err" ]; then
    echo "  fixture: rc=$rc" >&2; ok=1
  fi
  # Absent optional files become [N/A]; "Unknown" link speed too.
  mkroot "$out/r"
  rm "$out/r/$sy/class" "$out/r/$sy/subsystem_device"
  printf 'Unknown\n' > "$out/r/$sy/max_link_speed"
  "$b" --root "$out/r" > "$out/na.out" 2>&1; rc=$?
  if [ "$rc" -ne 0 ] || ! grep -qx 'gpu.0.pci.base_classcode=\[N/A\]' "$out/na.out" \
     || ! grep -qx 'gpu.0.pci.sub_device_id=\[N/A\]' "$out/na.out" \
     || ! grep -qx 'gpu.0.pcie.link.gen.max=\[N/A\]' "$out/na.out" \
     || ! grep -qx 'gpu.0.pcie.link.gen.current=1' "$out/na.out"; then
    echo "  optional-absent: rc=$rc" >&2; ok=1
  fi
  # No GPU directory at all: count=0, no gpu keys.
  mkroot "$out/r"
  rm -rf "$out/r/proc/driver/nvidia/gpus"
  "$b" --root "$out/r" > "$out/ng.out" 2>&1; rc=$?
  if [ "$rc" -ne 0 ] || ! grep -qx 'count=0' "$out/ng.out" || grep -q '^gpu\.' "$out/ng.out"; then
    echo "  no-gpus: rc=$rc" >&2; ok=1
  fi
  printf '%s\n' "$variants" | while IFS='|' read -r name want cmd; do
    [ -n "$name" ] || continue
    mkroot "$out/v"
    c=$(printf '%s' "$cmd" | sed "s#GI#$gi#g; s#SY#$sy#g")
    (cd "$out/v" && sh -c "$c") || { echo "  variant $name: setup failed" >&2; exit 1; }
    "$b" --root "$out/v" > "$out/v.out" 2> "$out/v.err"; rc=$?
    if [ "$rc" -ne "$want" ] || [ -s "$out/v.out" ] || ! grep -q '^SMI REFUSE ' "$out/v.err"; then
      echo "  variant $name: rc=$rc want $want" >&2; exit 1
    fi
  done || ok=1
  for a in '--bogus' '--root' '--rm --rm' "--root $out/r --root $out/r"; do
    # shellcheck disable=SC2086
    "$b" $a > "$out/u.out" 2> "$out/u.err"; rc=$?
    if [ "$rc" -ne 2 ] || [ -s "$out/u.out" ]; then echo "  usage '$a': rc=$rc" >&2; ok=1; fi
  done
  return $ok
}

cnt() { grep -cx "FAKE $1" "$2"; }

check_rm() { # fake-built bin -> 0 when the --rm expectations hold
  b=$1; ok=0
  mkroot "$out/r"
  env -u SMIFAULT "$b" --root "$out/r" --rm > "$out/rm.out" 2> "$out/rm.err"; rc=$?
  if [ "$rc" -ne 0 ] || ! cmp -s "$out/rm.out" "$t/expected-gb10-rm.txt" \
     || [ "$(cnt free "$out/rm.err")" != 3 ] || [ "$(cnt close "$out/rm.err")" != 2 ] \
     || [ "$(cnt open "$out/rm.err")" != 2 ] || grep -q 'UNKNOWN\|UNEXPECTED' "$out/rm.err"; then
    echo "  rm: rc=$rc" >&2; ok=1
  fi
  # letter|rc|frees|closes|opens
  for f in 'o|30|0|0|1' 's|30|3|2|2' 'c|31|3|2|2' 'n|32|3|2|2' 't|32|3|2|2' 'p|32|3|2|2' 'f|33|3|2|2'; do
    IFS='|' read -r l want fr cl op <<EOF
$f
EOF
    SMIFAULT=$l "$b" --root "$out/r" --rm > "$out/f.out" 2> "$out/f.err"; rc=$?
    if [ "$rc" -ne "$want" ] || [ -s "$out/f.out" ] || ! grep -q '^SMI REFUSE ' "$out/f.err" \
       || [ "$(cnt free "$out/f.err")" != "$fr" ] || [ "$(cnt close "$out/f.err")" != "$cl" ] \
       || [ "$(cnt open "$out/f.err")" != "$op" ] || grep -q 'UNEXPECTED' "$out/f.err"; then
      echo "  rm fault $l: rc=$rc want $want" >&2; ok=1
    fi
  done
  # --rm needs exactly one GPU; refused before any device effect.
  mkroot "$out/r"
  rm -rf "$out/r/proc/driver/nvidia/gpus"
  "$b" --root "$out/r" --rm > "$out/f.out" 2> "$out/f.err"; rc=$?
  if [ "$rc" -ne 34 ] || [ -s "$out/f.out" ] || grep -q '^FAKE' "$out/f.err"; then
    echo "  rm no-gpu: rc=$rc" >&2; ok=1
  fi
  return $ok
}

smi=$root/native/smi/smi.om
rmu=$root/native/smi/rm.om
build "$smi" "$rmu" "$out/cudapp-smi" || { cat "$out/cudapp-smi.log" >&2; fail 'cudapp-smi did not compile'; exit 1; }
build "$smi" "$rmu" "$out/smi-fake" fake || { cat "$out/smi-fake.log" >&2; fail 'fake build did not compile'; exit 1; }
guard "$rmu" '|fopen|fclose' || fail 'rm.om holds an OS effect word'
guard "$smi" '' || fail 'smi.om holds a device word'
if guard "$root/native/nvrm/context.om" '|fopen|fclose' > /dev/null; then
  fail 'guard negative control: unrewritten context.om was not refused'
else
  echo 'PASS guard negative control (unrewritten context.om refused)'
fi

if check_fixture "$out/cudapp-smi"; then echo 'PASS fixture tier (gb10, optional-absent, no-gpus, 16 refusals, usage)'; else fail 'fixture tier'; fi
if check_rm "$out/smi-fake"; then echo 'PASS rm fake tier (report, 7 faults, teardown counts, one-GPU rule)'; else fail 'rm fake tier'; fi

# Checker negative control: one altered expected byte must be rejected.
mkroot "$out/r"
"$out/cudapp-smi" --root "$out/r" > "$out/nc.out" 2>/dev/null
sed 's/^count=1$/count=2/' "$out/nc.out" > "$out/nc.alt"
if cmp -s "$out/nc.alt" "$t/expected-gb10.txt" || ! cmp -s "$out/nc.out" "$t/expected-gb10.txt"; then
  fail 'checker negative control: altered report was not distinguished'
else
  echo 'PASS checker negative control (altered count rejected)'
fi

# Mutants: each edits one line that must occur exactly once, and must be caught.
# unit|search|replacement
mutants='
smi|if and ge ven 0 ne ven 0x10de|if false
smi|if lnpre 0 sg1|if lnpre 0 sg2
smi|chkbus g|note chkbus g
smi|put gseen g 1|note gseen
smi|if not lnpre o tgpup|if false
smi|ofileu get gst 4 get gst 5|ofile get gst 4 get gst 5
smi|if or eq n 0 gt n 3|if false
rm|if or eq n 0 eq n 64|if eq n 0
rm|if or ne npget add o 4 1 ne npget o 0|if false
rm|if gt n 200|if false
rm|odec shr m 20|odec m
rm|refuse code msg|exit 0
'
mk=0; caught=0
printf '%s\n' "$mutants" | { while IFS='|' read -r unit from to; do
  [ -n "$unit" ] || continue
  mk=$((mk + 1))
  if [ "$unit" = smi ]; then src=$smi; else src=$rmu; fi
  n=$(grep -cxF "$from" "$src")
  if [ "$n" != 1 ]; then echo "FAIL: mutant site '$from' found $n times" >&2; exit 1; fi
  awk -v f="$from" -v r="$to" '$0 == f { print r; next } { print }' "$src" > "$out/mut.om"
  if [ "$unit" = smi ]; then s=$out/mut.om; r=$rmu; else s=$smi; r=$out/mut.om; fi
  if ! build "$s" "$r" "$out/m-real" || ! build "$s" "$r" "$out/m-fake" fake; then
    echo "FAIL: mutant '$from' did not compile; it proves nothing" >&2; continue
  fi
  if check_fixture "$out/m-real" 2>/dev/null && check_rm "$out/m-fake" 2>/dev/null; then
    echo "FAIL: mutant survived: $from -> $to" >&2
  else
    caught=$((caught + 1)); echo "  mutant '$from': caught"
  fi
done
echo "mutants caught $caught/$mk"
[ "$caught" -eq "$mk" ]; } || fail 'mutants'

if [ "$fails" -ne 0 ]; then
  echo "CUDAPP-SMI HOST FAIL ($fails)"
  exit 1
fi
echo 'CUDAPP-SMI HOST PASS'
