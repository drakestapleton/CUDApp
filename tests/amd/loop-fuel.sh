#!/bin/sh
# Loop-fuel alarm. The pinned omc (omega-mv e5b32567, sha256 2c824805...) refuses a while bound above 999999
# (error 8), so the input-driven loops in backend/amd/gfx942/hsaco.om and backend/amd/kfd/*.om stop with trap 104
# after 999999 iterations. Each such loop walks at most one byte (or line) of its input or output per iteration,
# so a file's byte count bounds its iterations. This alarm fails loudly, with the number and the file, when an
# input or output the gates use passes half that bound; the fix is the newer compiler pin
# (PROPOSAL-kfd-word-and-x86-route.md, route A) or restructuring the loop, never a lower cap.
threshold=500000
rc=0
for f in "$@"; do
  n=$(wc -c < "$f" | tr -d ' ')
  if [ "$n" -gt "$threshold" ]; then
    echo "LOOP-FUEL ALARM $f bytes=$n threshold=$threshold compiler_bound=999999"; rc=1
  else
    echo "LOOP-FUEL ok $f bytes=$n threshold=$threshold"
  fi
done
exit "$rc"
