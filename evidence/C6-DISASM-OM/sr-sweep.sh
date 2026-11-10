#!/bin/sh
# t_81083ab1: sweep the S2R special-register field 0..255 through the nvdisasm ORACLE (offline, nothing shipped)
set -u
cd "$HOME/work/hive/t_81083ab1/scratch" || exit 1
out=sr-sweep.bin
: > "$out"
i=0
while [ $i -lt 256 ]; do
    printf '\031\171\002\000\000\000\000\000' >> "$out"
    printf "\\000\\$(printf '%03o' $i)\\000\\000\\000\\000\\000\\000" >> "$out"
    i=$((i + 1))
done
wc -c < "$out"
/usr/local/cuda/bin/nvdisasm -b SM121 -hex "$out" > sr-sweep.nv 2> sr-sweep.err; echo "nvdisasm exit $?"
sh ../CUDApp/tools/sm121dis/oracle_pairs.sh sr-sweep.nv > sr-sweep.want
../CUDApp/build/sm121dis --pairs "$out" | sed 's/[[:space:]]\{1,\}/ /g' > sr-sweep.got
diff sr-sweep.want sr-sweep.got | grep -c '^<'
