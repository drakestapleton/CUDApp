#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Mutation check for native/share/share.om. Each mutant replaces one exact span that must occur
# exactly once ("\n" in a span is a newline). The host suite must fail on every mutant; a
# surviving mutant fails this script. Called by run-share-host.sh with OMEGA_OMC set and pinned.
set -eu
: "${OMEGA_OMC:?set the pinned seeded compiler}"
OUT=${ZERO_COPY_OUT:-build/zero-copy}
mkdir -p "$OUT/mutants"
test -s "$OUT/abi.om"

# name|search|replace
cat > "$OUT/mutants/list" <<'EOF'
ro-dma-flag|fl = 33041|fl = 33040
ro-uvm-type|kind = 3|kind = 1
ro-cpu-access|r = mapbind m int fd 0 1|r = mapbind m int fd 0 3
rw-cpu-access|r = mapbind m int fd 0 3|r = mapbind m int fd 0 1
ro-map-flag|npput 44 1|npput 44 0
probe-skipped|st = nvshmap obj size 4096 ro fd|st = long 32
limit-not-erange|if eq st 46|if eq st 47
larger-accepted|if eq st 0\nst = neg long 34|if eq st 1\nst = neg long 34
dup-kept-on-refusal|if nvmrel slot\nput nverr 0 e\nend|put nverr 0 e
dup-source-swapped|npput 12 nvshget 8\nnpput 16 nvshget 12|npput 12 nvshget 12\nnpput 16 nvshget 8
share-mask-zero|npput 12 1\nnpput 16 2|npput 12 0\nnpput 16 2
share-type-wrong|npput 12 1\nnpput 16 2|npput 12 1\nnpput 16 1
want-unchecked|if ne size want|if ne size size
reexport-allowed|if eq get nvshobj slot h|if ne h h
unaligned-accepted|if ne and size 4095 0|if ne and size 0 0
bound-raised|gt size 68719476736|gt size 137438953472
magic-unchecked|if or ne nvshget 0 1129991503 ne nvshget 4 1|if ne nvshget 4 1
record-size-page|nvshp64 16 get nvmlen slot|nvshp64 16 4096
range-fault-dropped|r = uvmctl get nvfd 2 uint 73 nvpar 0 24\nif not nvos r npget 16\nput nvfault 0 1|r = uvmctl get nvfd 2 uint 73 nvpar 0 24\nif not nvos r npget 16
gpu-skipped|if not nvshgpu slot ro\nret neg long 1\nend|put nvmflag slot 15
EOF

total=0
caught=0
while IFS='|' read -r name search replace; do
    total=$((total + 1))
    python3 - native/share/share.om "$OUT/mutants/$name.om" "$search" "$replace" <<'PY'
import sys
src, dst, search, replace = sys.argv[1:5]
search = search.replace("\\n", "\n")
replace = replace.replace("\\n", "\n")
text = open(src).read()
n = text.count(search)
if n != 1:
    sys.exit("mutant site found %d times: %r" % (n, search))
open(dst, "w").write(text.replace(search, replace))
PY
    if cmp -s native/share/share.om "$OUT/mutants/$name.om"; then
        echo "zero-copy mutant $name: FAIL identical to source" >&2
        exit 1
    fi
    ZERO_COPY_OUT="$OUT/mutants/$name.d" ZERO_COPY_SHARE_SRC="$OUT/mutants/$name.om" \
        ZERO_COPY_ABI="$OUT/abi.om" sh tests/share/run-share-host.sh --suite-only \
        > "$OUT/mutants/$name.log" 2>&1 < /dev/null && rc=0 || rc=$?
    if test "$rc" = 0; then
        echo "zero-copy mutant $name: SURVIVED" >&2
        exit 1
    fi
    # A mutant that does not compile proves nothing about the suite.
    if ! grep -qx 'zero-copy: compiled' "$OUT/mutants/$name.log"; then
        echo "zero-copy mutant $name: FAIL did not compile, see $OUT/mutants/$name.log" >&2
        exit 1
    fi
    caught=$((caught + 1))
    echo "zero-copy mutant $name: caught (rc $rc) $(grep -m1 FAIL "$OUT/mutants/$name.log" || true)"
done < "$OUT/mutants/list"
echo "zero-copy mutants: $caught/$total caught"
test "$caught" = "$total"
