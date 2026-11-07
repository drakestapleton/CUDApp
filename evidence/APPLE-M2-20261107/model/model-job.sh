#!/bin/sh
# apple195: the shared own-model workload (aien-train ollm, AArch64 Linux build) as one job, run on a
# given host: the UEFI acceptance job's first eight commands (tests/omegallm/efi-job.txt: mathck,
# adamck, prep, base, init, gcheck, train 20 steps, eval) through efi-ref.sh, plus a checkpoint
# save/load round trip (0->10 then resume 10->20, compared with the single 0->20 run), an argmax sample
# from the trained checkpoint, a corrupted-checkpoint refusal and the mutant-b build.
# Usage: sh model-job.sh DIR CORE   (DIR holds ollm ollm-mut-b efi-ref.sh mathref.txt STORIES.TXT)
set -u
D=$1; CORE=${2:-0}; export CORE
cd "$D" || exit 1
J=$D/job8.txt
grep -v -E '^(eval|sample) \\OLLM\\CK.BIN' "$D/efi-job.txt" | sed 's/^mathck .*/mathck \\OLLM\\MATHREF.TXT/' > "$J"
cp mathref.txt MATHREF.TXT
rm -f TR.BIN HE.BIN S.CK S2.CK
sh ./efi-ref.sh "$D/ollm" "$J" "$D" > job8.out 2>&1; echo "JOB8 rc=$?" >> job8.out
sha256sum TR.BIN HE.BIN S.CK > files.txt
# round trip: same init, save at 10, a new process resumes from the file to 20
B="taskset -c $CORE nice -n 19 $D/ollm"
{
  $B init S2.CK 256 8 16 2 32 2 2 52 200 1000
  $B train S2.CK TR.BIN 10 10
  cp S2.CK S2-10.CK
  $B train S2.CK TR.BIN 20 10
  if cmp -s S2.CK S.CK; then echo "ROUNDTRIP resume 0->10->20 == 0->20 cmp rc 0"; else echo "ROUNDTRIP DIFFERENT"; fi
  sha256sum S2-10.CK S2.CK
  $B eval S2.CK HE.BIN 16
  $B sample S.CK 7 64 0 "Once upon a time"; rc=$?; echo
  echo "sample rc=$rc"
  cp S.CK BAD.CK; printf '\377' | dd of=BAD.CK bs=1 seek=600 conv=notrunc status=none
  $B eval BAD.CK HE.BIN 16; echo "refuse-flip-payload rc=$?"
  taskset -c "$CORE" nice -n 19 "$D/ollm-mut-b" eval S.CK HE.BIN 16; echo "mut-b eval rc=$?"
} > extra.out 2>&1
cat job8.out files.txt extra.out
