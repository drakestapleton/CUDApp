#!/bin/sh
# Kill five semantic mutants of om/tools/dag_eval.om.
# Each mutant is recompiled through the pinned bridge and linked as the
# default scheduler. A kill is a changed eval of a fixture the unmutated
# scheduler still answers correctly. A record flip of that answer must fail cmp.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
OMC=${OMEGA_OMC:-$HOME/work/cudastack/omega-mv/seed/out/omc}
BRIDGE=${OMEGA_BRIDGE:-$HOME/work/cudastack/omega-bridge}
PIN=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
[ "$(sha256sum < "$OMC" | cut -c1-64)" = "$PIN" ] || { echo "NOT_RUN: omc pin"; exit 3; }
ORACLE=${1:-}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
CC=${CC:-cc}
cflags="-std=gnu11 -Wall -Wextra -D_GNU_SOURCE -O2 -ffp-contract=off -Isrc -Itools/dagsim"
$CC $cflags -c -o "$tmp/dagsim.o" tools/dagsim/dagsim.c
$CC $cflags -c -o "$tmp/rx.o" src/runtime/rx_costmodel.c
if [ -z "$ORACLE" ]; then
  OMEGA_SEEDED_OMC="$OMC" sh "$BRIDGE/tools/bridge-seeded" c om/tools/dag_eval.om > "$tmp/base.c"
  $CC $cflags -c -o "$tmp/base.o" "$tmp/base.c"
  $CC $cflags -o "$tmp/oracle" tools/dagsim/dagsim_cli.c "$tmp/dagsim.o" "$tmp/rx.o" "$tmp/base.o" src/sha256.c src/cudapp_om_dag_parse.c src/cudapp_om_dag_fmt.c src/cudapp_om_dag_res.c src/cudapp_om_dag_diff.c -lm
  ORACLE=$tmp/oracle
fi

cat > "$tmp/cycle.oev" <<'EOF'
oev version=1
machine id=t soc=t cores=1 mem_bytes=100 prov=specified src=test
build repo=omega-mv sha=0000000000000000000000000000000000000000
calibration id=c taken=2026-10-19 valid_days=3650 load1=0 src=synthetic
resource id=cpu kind=compute slots=1 prov=specified src=test
energy instrument=none
op id=A kind=compute engine=cpu t_ns=10 prov=synthetic src=test
op id=B kind=compute engine=cpu t_ns=10 prov=synthetic src=test
dep A B
dep B A
EOF
cat > "$tmp/alone.oev" <<'EOF'
oev version=1
machine id=t soc=t cores=1 mem_bytes=100 prov=specified src=test
build repo=omega-mv sha=0000000000000000000000000000000000000000
calibration id=c taken=2026-10-19 valid_days=3650 load1=0 src=synthetic
resource id=cpu kind=compute slots=1 prov=specified src=test
resource id=mem kind=capacity cap=100 prov=specified src=test
op id=A kind=compute engine=cpu t_ns=10 prov=synthetic src=test
buffer id=big bytes=200 cap=mem first=A last=A
energy instrument=none
EOF

kill_one() {
  name=$1
  src_sed=$2
  fixture=$3
  expect=$4
  sed "$src_sed" om/tools/dag_eval.om > "$tmp/$name.om"
  cmp -s om/tools/dag_eval.om "$tmp/$name.om" && { echo "mutant $name: sed changed nothing"; exit 1; }
  OMEGA_SEEDED_OMC="$OMC" sh "$BRIDGE/tools/bridge-seeded" c "$tmp/$name.om" > "$tmp/$name.c"
  $CC $cflags -c -o "$tmp/$name.o" "$tmp/$name.c"
  $CC $cflags -o "$tmp/$name.bin" tools/dagsim/dagsim_cli.c "$tmp/dagsim.o" "$tmp/rx.o" "$tmp/$name.o" src/sha256.c src/cudapp_om_dag_parse.c src/cudapp_om_dag_fmt.c src/cudapp_om_dag_res.c src/cudapp_om_dag_diff.c -lm
  "$ORACLE" eval "$fixture" --asof 2026-10-20 > "$tmp/$name.good" || true
  grep -q "$expect" "$tmp/$name.good" || { echo "mutant $name: oracle missing $expect"; cat "$tmp/$name.good"; exit 1; }
  "$tmp/$name.bin" eval "$fixture" --asof 2026-10-20 > "$tmp/$name.bad" || true
  if cmp -s "$tmp/$name.good" "$tmp/$name.bad"; then
    echo "mutant $name survived"
    exit 1
  fi
  echo "mutant $name: killed"
}

kill_one cycle 's/if eq dag_topo m s 0/if eq dag_topo m s 9/' "$tmp/cycle.oev" 'dependency cycle'
kill_one cap 's/rc = dag_cap m s/rc = ulong 0/' "$tmp/alone.oev" 'alone exceeds'
kill_one life 's/rc = dag_life m s/rc = ulong 0/' tests/dagsim/fixtures/inverted_lone.oev 'inverted lifetime'
kill_one ledg 's/rc = dag_ledg m s/rc = ulong 0/' tests/dagsim/fixtures/capacity_two_80.oev 'REFUSED_CAPACITY'
kill_one span 's/if gt t ms/if gt ms t/' tests/dagsim/fixtures/mw_baseline.oev '"nominal": 280'

# Negative control: one flipped byte of a passing record must fail the compare.
"$ORACLE" eval tests/dagsim/fixtures/mw_baseline.oev --asof 2026-10-20 > "$tmp/flip.in"
python3 - "$tmp/flip.in" "$tmp/flip.out" <<'PY'
import sys
b = open(sys.argv[1], "rb").read()
i = b.index(b"280")
open(sys.argv[2], "wb").write(b[:i] + b"281" + b[i+3:])
PY
if cmp -s "$tmp/flip.in" "$tmp/flip.out"; then
  echo "negative control: flip was not caught"
  exit 1
fi
echo "negative control: record flip caught"
echo "omega mutants: 5 killed"
