#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Gate for the DRA-202 migration-cost meter and the DRA-203 two-device receipt gate (host only, CPU).
#   1. tools/repro/cost.sh on the committed scopes must print exactly tests/repro/cost.expect.
#   2. Independent oracle: git diff --shortstat per scope equals each TOTAL line; the emitted module
#      digests at each HEAD equal the published ones (docs/repro/expected.sha256 gpu-in lines).
#   3. Negative controls on the cost records: unclassified path, malformed number, NA mixed with a
#      number, unknown effort kind, wrong END count, truncated input, an empty input; and a changed
#      kernel digest must print "differs". Two source mutants of cost.om must change the output or refuse.
#   4. tools/multidevice/mdgate.om: the committed fixture prints PASS-FIXTURE (5); a fixture=no copy made
#      here (never committed) prints PASS (0); empty input and claim=NOT_RUN print NOT_RUN (3); each of
#      16 single-rule mutants refuses (1) with its expected reason.
#   5. A compiler other than the pin is refused (69).
# usage: run.sh /absolute/path/to/pinned-omc
# Exit: 0 REPRO-GATE PASS; 1 FAIL; 64 usage; 69 pin refused; 74 I/O; 77 NOT_RUN (not Linux AArch64).
set -u
LC_ALL=C
export LC_ALL
if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ]; then
  echo 'NOT_RUN: needs Linux AArch64 and the pinned seeded omc' >&2; exit 77
fi
[ "$#" -eq 1 ] || { echo 'usage: run.sh /absolute/path/to/pinned-omc' >&2; exit 64; }
omc=$1
case $omc in /*) ;; *) echo 'usage: omc path must be absolute' >&2; exit 64 ;; esac
omc_pin=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
d=$(sha256sum < "$omc") || exit 74
[ "${d%% *}" = "$omc_pin" ] || { echo "REFUSED: $omc digest differs from pin $omc_pin" >&2; exit 69; }
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
w=$(mktemp -d "${TMPDIR:-/tmp}/cudapp-repro-gate.XXXXXX") || exit 74
trap 'rm -rf "$w"' EXIT INT TERM
npass=0; nfail=0
ok() { npass=$((npass + 1)); echo "PASS $*"; }
bad() { nfail=$((nfail + 1)); echo "FAIL $*"; }

"$omc" < "$root/tools/repro/cost.om" > "$w/cost" && chmod 700 "$w/cost" && ok "compile cost.om" || bad "compile cost.om"
"$omc" < "$root/tools/multidevice/mdgate.om" > "$w/mdgate" && chmod 700 "$w/mdgate" && ok "compile mdgate.om" || bad "compile mdgate.om"

# 1. golden
sh "$root/tools/repro/cost.sh" "$omc" > "$w/cost.out" 2> "$w/cost.err"; rc=$?
if [ "$rc" -eq 0 ] && cmp -s "$w/cost.out" "$root/tests/repro/cost.expect"; then
  ok "cost.sh output = tests/repro/cost.expect ($(wc -l < "$w/cost.out" | tr -d ' ') lines)"
else
  bad "cost.sh exit=$rc or output differs from cost.expect"; diff "$root/tests/repro/cost.expect" "$w/cost.out" | head -n 20
fi
sh "$root/tools/repro/cost.sh" "$omc" --records > "$w/rec.txt" || bad "cost.sh --records"

# 2. independent oracle (git shortstat, published digests)
grep -v '^#' "$root/tools/repro/scopes.tsv" | awk 'NF > 0' > "$w/scopes"
while read -r name base head _r; do
  ss=$(git -C "$root" diff --shortstat --no-renames "$base" "$head")
  f=$(echo "$ss" | sed -n 's/^ *\([0-9]*\) files\{0,1\} changed.*/\1/p')
  a=$(echo "$ss" | sed -n 's/.* \([0-9]*\) insertions\{0,1\}(+).*/\1/p'); a=${a:-0}
  dl=$(echo "$ss" | sed -n 's/.* \([0-9]*\) deletions\{0,1\}(-).*/\1/p'); dl=${dl:-0}
  t=$(awk -v s="COST1 scope=$name" 'index($0, s) == 1 { on = 1 } on && /^TOTAL / { print; exit }' "$w/cost.out")
  [ "$t" = "TOTAL files=$f add=$a del=$dl" ] && ok "oracle $name git shortstat = $t" || bad "oracle $name shortstat '$ss' vs '$t'"
done < "$w/scopes"
for k in vadd fsum16 gemm16; do
  want=$(sed -n "s#^\([0-9a-f]*\)  gpu-in/$k.vk.spv\$#\1#p" "$root/docs/repro/expected.sha256")
  got=$(awk -v m="$k.vk" '$1 == "K" && $2 == m { h = $4 } END { print h }' "$w/rec.txt")
  [ -n "$want" ] && [ "$got" = "$want" ] && ok "oracle $k.vk at S1 head = published module $want" || bad "oracle $k.vk got=$got want=$want"
done

# 3. negative controls on the records; expect REASON: the command must exit 1 with that reason
refuse() {
  name=$1; reason=$2
  "$w/cost" < "$w/neg.txt" > "$w/neg.out" 2> "$w/neg.err"; r=$?
  if [ "$r" -eq 1 ] && grep -q "reason=$reason\$" "$w/neg.err"; then ok "neg $name refused: $(cat "$w/neg.err")"; else bad "neg $name exit=$r $(cat "$w/neg.err")"; fi
}
sed '/^X$/{x;s/.*/N 3 1 src\/omega_gpu_new.c/;p;x;}' "$w/rec.txt" > "$w/neg.txt"; refuse unclassified-path unclassified-path
sed '0,/^N /s/^N \([0-9]*\) /N 1x /' "$w/rec.txt" > "$w/neg.txt"; refuse bad-number number-not-1-to-18-decimal-digits
sed 's/^E review_s NA$/E review_s NA\nE review_s 60/' "$w/rec.txt" > "$w/neg.txt"; refuse na-mixed effort-kind-has-NA-and-a-number
sed 's/^E gpu_exclusive_s 48$/E gpu_minutes 1/' "$w/rec.txt" > "$w/neg.txt"; refuse unknown-kind unknown-effort-kind
sed 's/^END 2$/END 3/' "$w/rec.txt" > "$w/neg.txt"; refuse end-count END-count-differs-from-scopes
sed '$d' "$w/rec.txt" > "$w/neg.txt"; refuse truncated missing-END
sed '$d' "$w/rec.txt" | sed '$d' > "$w/neg.txt"; refuse ends-in-scope missing-END
: > "$w/neg.txt"; refuse empty first-line-must-be-COSTIN1
sed '1s/.*/COSTIN0/' "$w/rec.txt" > "$w/neg.txt"; refuse header first-line-must-be-COSTIN1
awk '$1 == "K" && $2 == "vadd.vk" { $4 = "0000000000000000000000000000000000000000000000000000000000000000" } { print }' "$w/rec.txt" > "$w/neg.txt"
"$w/cost" < "$w/neg.txt" > "$w/neg.out" 2>&1
grep -q '^KERNEL vadd.vk differs$' "$w/neg.out" && ok "neg changed kernel digest prints KERNEL vadd.vk differs" || bad "neg changed kernel not detected"
# source mutants: docs/ no longer a category (must refuse); added lines summed into deletions (output must change)
sed 's#^text pdc "docs/"$#text pdc "dox/"#' "$root/tools/repro/cost.om" > "$w/mut1.om"
sed 's#^put cat add i 1 add get cat add i 1 ad$#put cat add i 1 add get cat add i 1 de#' "$root/tools/repro/cost.om" > "$w/mut2.om"
for m in mut1 mut2; do
  if cmp -s "$w/$m.om" "$root/tools/repro/cost.om"; then bad "source mutant $m did not apply"; continue; fi
  "$omc" < "$w/$m.om" > "$w/$m" && chmod 700 "$w/$m" || { bad "source mutant $m did not compile"; continue; }
  "$w/$m" < "$w/rec.txt" > "$w/$m.out" 2> "$w/$m.err"; r=$?
  if [ "$r" -ne 0 ] || ! cmp -s "$w/$m.out" "$root/tests/repro/cost.expect"; then ok "source mutant $m caught (exit $r$(head -c 80 "$w/$m.err" | tr '\n' ' '))"; else bad "source mutant $m not caught"; fi
done

# 4. mdgate
F=$root/tests/repro/md2/good.md2
gate() {
  want=$1; name=$2; reason=$3
  "$w/mdgate" < "$w/md.txt" > "$w/md.out" 2> "$w/md.err"; r=$?
  if [ "$r" -eq "$want" ] && { [ -z "$reason" ] || grep -q "reason=$reason\$" "$w/md.err" "$w/md.out"; }; then
    ok "mdgate $name exit=$r $(cat "$w/md.out" "$w/md.err" | head -n 1)"
  else
    bad "mdgate $name exit=$r want=$want $(cat "$w/md.out" "$w/md.err" | head -n 1)"
  fi
}
cp "$F" "$w/md.txt"; gate 5 fixture-pass ''
grep -q '^MDGATE PASS-FIXTURE ' "$w/md.out" && ok "fixture verdict is labelled PASS-FIXTURE" || bad "fixture verdict label"
sed 's/^fixture=yes$/fixture=no/' "$F" > "$w/md.txt"; gate 0 real-copy-not-committed ''
: > "$w/md.txt"; gate 3 empty no-receipt
sed 's/^claim=.*/claim=NOT_RUN/' "$F" > "$w/md.txt"; gate 3 claim-not-run receipt-claims-NOT_RUN
mut() { want_reason=$1; shift; sed "$@" "$F" > "$w/md.txt"; gate 1 "$want_reason" "$want_reason"; }
mut unknown-claim 's/^claim=.*/claim=DATACENTER_SCALE/'
mut devices-must-be-2 's/^devices=2$/devices=3/'
mut dev0-equals-dev1 's/^dev1=.*/dev1=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/'
mut two-device-checkpoint-differs-from-single-device-reference 's/^md_ck=1/md_ck=3/'
mut rank-1-checkpoint-differs-from-rank-0 's/^md_ck1=1/md_ck1=3/'
mut rank-swap-control-equals-reference 's/^ctl_rank_swap=.*/ctl_rank_swap=1111111111111111111111111111111111111111111111111111111111111111/'
mut a-negative-control-was-not-refused 's/^ctl_kill_rank=REFUSED$/ctl_kill_rank=NOT_REFUSED/'
mut bytes_step-below-ordered-reduction-minimum 's/^bytes_step=.*/bytes_step=239963135/'
mut times-inconsistent-overlap-comm-t2 's/^overlap_us=.*/overlap_us=200001/'
mut eff_ppm-differs-from-t1-over-2-t2 's/^eff_ppm=.*/eff_ppm=833334/'
mut qualified-claim-missing-a-key '/^deps=/d'
mut key-repeated 's/^steps=20$/steps=20\nsteps=21/'
mut unknown-key 's/^deps=/scale_nodes=1024\ndeps=/'
mut value-malformed-for-key 's/^topology=T1$/topology=T9/'
mut first-line-must-be-MD2-v1 '1s/.*/MD2 v0/'
mut missing-END '/^END$/d'

# 5. pin refusal
cp "$omc" "$w/omc-other" && printf 'x' >> "$w/omc-other" && chmod 700 "$w/omc-other"
sh "$root/tools/repro/cost.sh" "$w/omc-other" > /dev/null 2>&1; r=$?
[ "$r" -eq 69 ] && ok "cost.sh refuses a compiler other than the pin (69)" || bad "pin refusal exit=$r"

if [ "$nfail" -eq 0 ]; then echo "REPRO-GATE PASS pass=$npass fail=0"; exit 0; fi
echo "REPRO-GATE FAIL pass=$npass fail=$nfail"; exit 1
