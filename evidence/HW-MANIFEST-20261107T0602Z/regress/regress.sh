#!/bin/sh
# Runs the new review controls against hwman built from CUDApp 06237840 (H) and prints, per control,
# the exit code and last output line. Usage: before.sh HWMAN MANIFEST_DIR OUTDIR
H=$1; R=$2; o=$3
M=$R/backend/hw/manifest.tsv
mkdir -p "$o" || exit 74
run() { _n=$1; shift; "$@" > "$o/$_n.log" 2>&1; _rc=$?; echo "$_n exit=$_rc last=$(tail -n 1 "$o/$_n.log")"; }
mut() { awk -F '\t' -v OFS='\t' "$2" "$M" > "$o/$1.tsv"; }
# 1. persistence: old release is pwrite64 #1 / fsync #1 (no in-flight mark); also the new indices.
for c in failed-write:pwrite64:error=ENOSPC short-write:pwrite64:retval=3 failed-sync:fsync:error=EIO; do
  n=${c%%:*}; inj=${c#*:}
  for w in 1 2; do
    st=$o/persist-$n-w$w.state; rm -f "$st"
    run persist-$n-w$w.seed "$H" run "$M" cpu-gb10-x925 sumsq64 1 0 "$st" 0
    run persist-$n-w$w strace -qq -o /dev/null -P "$st" -e trace=pwrite64,fsync,ftruncate -e "inject=$inj:when=$w" \
      "$H" run "$M" cpu-gb10-x925 sumsq64 1000 1 "$st" 332833500
    echo "persist-$n-w$w state=[$(cat "$st")]"
    for g in 0 1 2; do run persist-$n-w$w-replay-$g "$H" run "$M" cpu-gb10-x925 sumsq64 1 "$g" "$st" 0; done
  done
done
# 2. binding
mut m-cpuop '$1=="cpu-gb10-x925"{$12="sumsq64,fnv1a64,matmul"}1'
mut m-tpmop '$1=="tpm-ntc-npct75x"{$12="props,getrandom,seal"}1'
mut m-runner '$1=="cpu-gb10-x925"{$20="hwman-gpu"}1'
mut m-famrun '$1=="tpm-ntc-npct75x"{$20="hwman-cpu"}1'
mut m-auth '$1=="cpu-gb10-x925"{$6="DIRECT_DEVICE"}1'
for m in m-cpuop m-tpmop m-runner m-famrun m-auth; do run check-$m "$H" check "$o/$m.tsv"; done
S=$o/cpu.state; rm -f "$S"
run cpu-seed "$H" run "$M" cpu-gb10-x925 sumsq64 1 0 "$S" 0
run run-cpu-invented-op "$H" run "$o/m-cpuop.tsv" cpu-gb10-x925 matmul 1 1 "$S"
echo "cpu state after invented op=[$(cat "$S")]"
run run-cpu-invented-runner "$H" run "$o/m-runner.tsv" cpu-gb10-x925 sumsq64 1 1 "$S" 0
echo "cpu state after invented runner=[$(cat "$S")]"
T=$o/tpm.state; rm -f "$T"
run tpm-seed "$H" run "$M" tpm-ntc-npct75x props 0 0 "$T"
run run-tpm-invented-op "$H" run "$o/m-tpmop.tsv" tpm-ntc-npct75x seal 1 1 "$T"
echo "tpm state after invented op=[$(cat "$T")]"
g=$(cut -d ' ' -f 1 "$T")
run run-tpm-runner-family "$H" run "$o/m-famrun.tsv" tpm-ntc-npct75x props 0 "$g" "$T"
echo "tpm state after runner family=[$(cat "$T")]"
# 3. fault oracle
g=$(cut -d ' ' -f 1 "$T")
for c in badsize-eio:badsize:write:error=EIO badcc-kernel-einval:badcc:write:error=EINVAL badcc-short-response:badcc:read:retval=4; do
  n=${c%%:*}; r=${c#*:}; k=${r%%:*}; inj=${r#*:}
  run fault-$n strace -qq -o /dev/null -P /dev/tpmrm0 -e trace=read,write -e "inject=$inj:when=2" \
    "$H" fault "$M" tpm-ntc-npct75x "$k" "$g" "$T"
  g=$((g + 1))
done
