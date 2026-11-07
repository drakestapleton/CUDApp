#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# collect.sh: records one bounded cloud window as an ACCESS1 receipt (docs/access/RECEIPT.md).
# It decides nothing about what the receipt may claim: it runs the rows of a plan file, keeps every
# command's output and exit status, and maps exit status to a verdict mechanically (missing tool:
# NOT_RUN; exit equal to the row's want: PASS; any other exit: FAIL). The claim is checked on the
# Spark by tools/access/rcheck.om, which refuses a receipt whose own rows do not support it.
#
#   collect.sh run PROVIDER PLAN OUT
#       On the provider's machine. PROVIDER is amd-devcloud, intel-cloud or google-trc. PLAN is a
#       tools/access/plans/*.tsv file. OUT must not exist. Needs ACCESS_WINDOW (window id),
#       ACCESS_REPO (owner/repo of the copied bundle) and ACCESS_REV (its 40-hex commit);
#       ACCESS_BUNDLE (directory with copied artifacts) is optional. ACCESS_ROW_TIMEOUT caps each
#       row in seconds (default 1800). Rows see ACCESS_OUT, the absolute OUT, for files they keep.
#   collect.sh close OUT CLAIM CHARGED_USD DESTROYED BY VERIFIED_AT
#       On the Spark, after OUT was copied back and the instance was destroyed and the destruction
#       checked in the provider console. CLAIM is ACCESS_TESTED or NONE; DESTROYED is yes or no.
#       Verifies OUT/SHA256SUMS, writes OUT/receipt.txt, and runs $RCHECK on it when RCHECK is set.
#
# Exit: 0 done (close: and rcheck accepted, when run); 1 failure or rcheck refusal; 64 usage; 74 I/O.
# Opens no device itself except through the plan's commands; never uploads anything.
set -u
LC_ALL=C
export LC_ALL

usage() { sed -n '4,20p' "$0" >&2; exit 64; }
now() { date -u +%Y-%m-%dT%H:%M:%SZ; }
# word: squeeze a value to one receipt token (no spaces, no '=' confusion in later keys)
word() { printf '%s' "$1" | tr -c 'A-Za-z0-9._:/+@-' '_' | cut -c1-120; }

host_line() {
  os=unknown
  if [ -r /etc/os-release ]; then os=$(. /etc/os-release && printf '%s' "${PRETTY_NAME:-unknown}"); fi
  printf 'HOST os=%s kernel=%s arch=%s\n' "$(word "$os")" "$(word "$(uname -r)")" "$(word "$(uname -m)")"
}

# pci_first VENDOR DRIVER...: first PCI function with that vendor id bound to one of the drivers;
# prints "VENDOR:DEVICE COUNT DRIVER".
pci_first() {
  ven=$1; shift
  first=; n=0; drv=
  for d in /sys/bus/pci/devices/*; do
    [ -r "$d/vendor" ] || continue
    [ "$(cat "$d/vendor")" = "$ven" ] || continue
    [ -e "$d/driver" ] || continue
    b=$(basename "$(readlink "$d/driver")")
    for want in "$@"; do
      if [ "$b" = "$want" ]; then
        n=$((n + 1))
        if [ -z "$first" ]; then first=$(sed 's/^0x//' "$d/device"); drv=$b; fi
      fi
    done
  done
  [ -n "$first" ] || return 1
  printf '%s:%s %s %s\n' "$(printf '%s' "$ven" | sed 's/^0x//')" "$first" "$n" "$drv"
}

cpu_line() {
  v=$(awk -F': *' '/^vendor_id/{print $2; exit}' /proc/cpuinfo 2>/dev/null)
  case $v in GenuineIntel) cv=intel ;; AuthenticAMD) cv=amd ;; *) cv=other ;; esac
  m=$(awk -F': *' '/^model name/{print $2; exit}' /proc/cpuinfo 2>/dev/null)
  printf 'DEVICE id=d7 vendor=%s class=cpu model=%s\n' "$cv" "$(word "${m:-unknown}")"
}

# The provider's accelerator as DEVICE id=d0, only when the vendor's own tool and the PCI/accel
# nodes both show it. Nothing found: no d0 line, so no row can PASS on an accelerator.
dev_amd() {
  command -v rocminfo > /dev/null 2>&1 || return 0
  rocminfo > "$out/rows/detect-rocminfo.log" 2>&1 || return 0
  set -- $(awk '/^ *Name: *gfx/ && !g {g=$2; f=1; next} f && /Marketing Name:/ {sub(/^.*Marketing Name: */,""); gsub(/ /,"_"); print g, $0; exit}' "$out/rows/detect-rocminfo.log")
  [ "$#" -ge 2 ] || return 0
  p=$(pci_first 0x1002 amdgpu) || return 0
  set -- "$1" "$2" $p
  kv=$(cat /sys/module/amdgpu/version 2>/dev/null || echo none)
  printf 'DEVICE id=d0 vendor=amd class=gpu model=%s arch=%s pci=%s count=%s driver=%s kmod=%s\n' \
    "$(word "$2")" "$(word "$1")" "$3" "$4" "$5" "$(word "$kv")"
}
dev_intel() {
  p=$(pci_first 0x8086 i915 xe) || return 0
  set -- $p
  m=none
  if command -v clinfo > /dev/null 2>&1; then
    clinfo -l > "$out/rows/detect-clinfo.log" 2>&1
    m=$(awk -F'Device #[0-9]*: ' '/Device #/{print $2; exit}' "$out/rows/detect-clinfo.log")
  fi
  printf 'DEVICE id=d0 vendor=intel class=gpu model=%s pci=%s count=%s driver=%s\n' \
    "$(word "${m:-none}")" "$1" "$2" "$3"
}
dev_tpu() {
  command -v curl > /dev/null 2>&1 || return 0
  t=$(curl -sf -m 10 -H 'Metadata-Flavor: Google' \
    http://metadata.google.internal/computeMetadata/v1/instance/attributes/accelerator-type) || return 0
  [ -n "$t" ] || return 0
  n=$(ls -d /dev/accel* /dev/vfio/[0-9]* 2>/dev/null | wc -l)
  [ "$n" -gt 0 ] || return 0
  printf 'DEVICE id=d0 vendor=google class=tpu model=%s count=%s\n' "$(word "$t")" "$n"
}

tool_line() { # tool_line NAME
  p=$(command -v "$1" 2>/dev/null) || return 0
  r=$(readlink -f "$p" 2>/dev/null || printf '%s' "$p")
  s=$(sha256sum < "$r" 2>/dev/null | cut -c1-64)
  printf 'TOOL name=%s path=%s sha256=%s\n' "$(word "$1")" "$(word "$r")" "${s:-unreadable}"
}

do_run() {
  [ "$#" -eq 3 ] || usage
  prov=$1; plan=$2; out=$3
  case $prov in amd-devcloud|intel-cloud|google-trc) ;; *) echo "unknown provider $prov" >&2; exit 64 ;; esac
  [ -r "$plan" ] || { echo "plan $plan unreadable" >&2; exit 64; }
  [ -e "$out" ] && { echo "$out exists; use a new directory per window" >&2; exit 64; }
  : "${ACCESS_WINDOW:?set ACCESS_WINDOW}" "${ACCESS_REPO:?set ACCESS_REPO}" "${ACCESS_REV:?set ACCESS_REV}"
  ACCESS_BUNDLE=${ACCESS_BUNDLE:-}
  export ACCESS_BUNDLE
  tmo=${ACCESS_ROW_TIMEOUT:-1800}
  mkdir -p "$out/rows" || exit 74
  # Rows may keep files (for example GPU outputs) under ACCESS_OUT; they are hashed into SHA256SUMS.
  ACCESS_OUT=$(CDPATH= cd -- "$out" && pwd) || exit 74
  export ACCESS_OUT
  cp "$plan" "$out/plan.tsv" || exit 74
  printf 'provider=%s\nwindow=%s\n' "$prov" "$(word "$ACCESS_WINDOW")" > "$out/meta" || exit 74
  body=$out/receipt.body
  {
    printf 'SRC repo=%s rev=%s\n' "$(word "$ACCESS_REPO")" "$(word "$ACCESS_REV")"
    printf 'WHEN start=%s\n' "$(now)"
    host_line
    case $prov in
      amd-devcloud) dev_amd ;;
      intel-cloud) dev_intel ;;
      google-trc) dev_tpu ;;
    esac
    cpu_line
  } > "$body" || exit 74
  dev=d7
  grep -q '^DEVICE id=d0 ' "$body" && dev=d0
  tools=$(awk -F'\t' '!/^#/ && NF >= 5 && $4 != "-" {print $4}' "$plan" | sort -u)
  for t in $tools; do tool_line "$t" >> "$body"; done
  # rows: id<TAB>kind<TAB>want<TAB>needs<TAB>command
  tab=$(printf '\t')
  while IFS="$tab" read -r id kind want needs cmd; do
    case $id in ''|'#'*) continue ;; esac
    id=$(word "$id")
    case $cmd in
      MISSING_IMPLEMENTATION*)
        printf 'ROW id=%s kind=%s want=%s verdict=MISSING_IMPLEMENTATION dev=none reason=%s\n' \
          "$id" "$kind" "$want" "$(word "${cmd#MISSING_IMPLEMENTATION }")" >> "$body"
        continue ;;
    esac
    if [ "$needs" != - ] && ! command -v "$needs" > /dev/null 2>&1; then
      printf 'ROW id=%s kind=%s want=%s verdict=NOT_RUN dev=none reason=missing-tool-%s\n' \
        "$id" "$kind" "$want" "$(word "$needs")" >> "$body"
      continue
    fi
    log=rows/$id.log
    { printf '# command: %s\n# start: %s\n' "$cmd" "$(now)"; } > "$out/$log"
    timeout "$tmo" sh -c "$cmd" >> "$out/$log" 2>&1 < /dev/null
    rc=$?
    printf '# exit: %s\n# end: %s\n' "$rc" "$(now)" >> "$out/$log"
    v=FAIL
    [ "$rc" -eq "$want" ] && v=PASS
    printf 'ROW id=%s kind=%s want=%s verdict=%s dev=%s exit=%s log=%s\n' \
      "$id" "$kind" "$want" "$v" "$dev" "$rc" "$log" >> "$body"
    echo "ROW $id $v exit=$rc"
  done < "$plan"
  printf 'WHEN end=%s\n' "$(now)" >> "$body"
  (cd "$out" && find . -type f ! -name SHA256SUMS | LC_ALL=C sort | sed 's|^\./||' | xargs sha256sum > SHA256SUMS) || exit 74
  echo "RECORDED $out ($(grep -c '^ROW ' "$body") rows). Copy $out back, destroy the instance, then run close."
}

do_close() {
  [ "$#" -eq 6 ] || usage
  out=$1; claim=$2; usd=$3; des=$4; by=$5; vat=$6
  [ -r "$out/meta" ] && [ -r "$out/receipt.body" ] && [ -r "$out/SHA256SUMS" ] || { echo "$out is not a run directory" >&2; exit 64; }
  [ -e "$out/receipt.txt" ] && { echo "$out/receipt.txt exists; receipts are never rewritten" >&2; exit 64; }
  (cd "$out" && sha256sum -c --quiet SHA256SUMS) || { echo "SHA256SUMS check failed: the copied run differs from what was recorded" >&2; exit 1; }
  prov=$(sed -n 's/^provider=//p' "$out/meta"); win=$(sed -n 's/^window=//p' "$out/meta")
  n=$(grep -c '^ROW ' "$out/receipt.body")
  {
    printf 'ACCESS1 provider=%s window=%s claim=%s\n' "$prov" "$win" "$(word "$claim")"
    cat "$out/receipt.body"
    printf 'COST charged_usd=%s\n' "$(word "$usd")"
    printf 'TEARDOWN destroyed=%s by=%s verified_at=%s\n' "$(word "$des")" "$(word "$by")" "$(word "$vat")"
    printf 'END rows=%s\n' "$n"
  } > "$out/receipt.txt" || exit 74
  sha256sum "$out/receipt.txt"
  if [ -n "${RCHECK:-}" ]; then "$RCHECK" < "$out/receipt.txt"; exit $?; fi
  echo "NOT_RUN rcheck: set RCHECK to the binary built from tools/access/rcheck.om"
}

[ "$#" -ge 1 ] || usage
cmd=$1; shift
case $cmd in
  run) do_run "$@" ;;
  close) do_close "$@" ;;
  *) usage ;;
esac
