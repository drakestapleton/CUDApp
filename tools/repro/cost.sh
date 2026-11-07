#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Migration-cost meter for the portable application (DRA-202, docs/repro/MIGRATION-COST.md).
# Collects, for every scope in tools/repro/scopes.tsv, the commits, the numstat of the whole-tree diff
# BASE..HEAD, the SPIR-V module bytes emitted by backend/spirv/emit.om at BASE and at HEAD, and the
# scope's effort rows from docs/repro/effort.tsv, as COSTIN1 records; tools/repro/cost.om (seeded Omega)
# makes every classification, sum and comparison and prints the COST1 table.
# Anyone with a clone of aien-dev/CUDApp (full history; a blobless clone fetches blobs on demand) and
# the pinned omc gets the same bytes: everything here is a function of public commits and of the
# committed effort file.
#
# usage: cost.sh /absolute/path/to/pinned-omc [--records]
#   --records  print the COSTIN1 records instead of running cost.om (for review and the tests)
# Environment: COST_SCOPES (default tools/repro/scopes.tsv), COST_EFFORT (default docs/repro/effort.tsv)
# Exit: cost.om's exit (0 COST1 OK, 1 refused, 2 input too large); 64 usage; 69 compiler pin refused;
# 74 git or I/O failure; 77 NOT_RUN (not Linux AArch64: omc writes AArch64 Linux executables).
set -u
LC_ALL=C
export LC_ALL
if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ]; then
  echo 'NOT_RUN: cost.sh runs the seeded omc, which needs Linux AArch64' >&2; exit 77
fi
[ "$#" -ge 1 ] && [ "$#" -le 2 ] || { echo 'usage: cost.sh /absolute/path/to/pinned-omc [--records]' >&2; exit 64; }
omc=$1; mode=${2:-run}
case $omc in /*) ;; *) echo 'usage: omc path must be absolute' >&2; exit 64 ;; esac
case $mode in run|--records) ;; *) echo 'usage: second argument must be --records' >&2; exit 64 ;; esac
omc_pin=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
d=$(sha256sum < "$omc") || exit 74
[ "${d%% *}" = "$omc_pin" ] || { echo "REFUSED: $omc digest differs from pin $omc_pin" >&2; exit 69; }
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
scopes=${COST_SCOPES:-$root/tools/repro/scopes.tsv}
effort=${COST_EFFORT:-$root/docs/repro/effort.tsv}
[ -f "$scopes" ] && [ -f "$effort" ] || { echo "cost.sh: missing $scopes or $effort" >&2; exit 74; }
w=$(mktemp -d "${TMPDIR:-/tmp}/cudapp-cost.XXXXXX") || exit 74
trap 'rm -rf "$w"' EXIT INT TERM
KERNELS='vadd fsum16 gemm16'
ENVS='cl vk'

# modsha REV KERNEL ENV: sha256 of the module emit.om at REV writes, or "absent"
modsha() {
  if [ ! -x "$w/emit.$1" ]; then
    if git -C "$root" cat-file -e "$1:backend/spirv/emit.om" 2>/dev/null; then
      git -C "$root" show "$1:backend/spirv/emit.om" > "$w/emit.$1.om" || return 1
      "$omc" < "$w/emit.$1.om" > "$w/emit.$1" || { echo "cost.sh: emit.om at $1 did not compile" >&2; return 1; }
      chmod 700 "$w/emit.$1"
    else
      : > "$w/emit.$1.none"
    fi
  fi
  if [ -f "$w/emit.$1.none" ]; then echo absent; return 0; fi
  if "$w/emit.$1" "$2" "$3" > "$w/m.spv" 2>/dev/null && [ -s "$w/m.spv" ]; then
    s=$(sha256sum < "$w/m.spv"); echo "${s%% *}"
  else
    echo absent
  fi
}

records() {
  echo COSTIN1
  grep -v '^#' "$scopes" | awk 'NF > 0' > "$w/scopes" || return 1
  exec 3< "$w/scopes"
  while read -r name base head _rest <&3; do
    echo "S $name $base $head"
    git -C "$root" log --reverse --format='C %H %at %ct' "$base..$head" > "$w/log" || return 1
    cat "$w/log"
    git -C "$root" diff --numstat --no-renames "$base" "$head" > "$w/numstat" || return 1
    tab=$(printf '\t')
    while IFS="$tab" read -r a dl p; do
      if [ "$a" = - ] && [ "$dl" = - ]; then echo "B $p"; else echo "N $a $dl $p"; fi
    done < "$w/numstat"
    for k in $KERNELS; do
      for e in $ENVS; do
        sb=$(modsha "$base" "$k" "$e") || return 1
        sh_=$(modsha "$head" "$k" "$e") || return 1
        [ "$sb" = absent ] && [ "$sh_" = absent ] && continue
        echo "K $k.$e $sb $sh_"
      done
    done
    grep -v '^#' "$effort" | awk -v s="$name" '$1 == s { print "E", $2, $3 }'
    echo X
  done
  exec 3<&-
  echo "END $(wc -l < "$w/scopes" | tr -d ' ')"
}

records > "$w/in.txt" || exit 74
if [ "$mode" = --records ]; then cat "$w/in.txt"; exit 0; fi
"$omc" < "$root/tools/repro/cost.om" > "$w/cost" || { echo 'cost.sh: cost.om did not compile' >&2; exit 74; }
chmod 700 "$w/cost"
"$w/cost" < "$w/in.txt"
