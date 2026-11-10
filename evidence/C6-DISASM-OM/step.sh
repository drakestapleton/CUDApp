#!/bin/sh
# t_81083ab1: one step of the C6 host tier under one pool-10-11 hold (capped at 19 minutes). usage: step.sh NAME CMD...
set -u
cd "$HOME/work/hive/t_81083ab1/CUDApp" || exit 1
N=$1; shift
E=$HOME/work/hive/t_81083ab1/ev
mkdir -p "$E"
ulimit -c 0
{
echo "step $N start $(date -u +%FT%TZ) head $(git rev-parse HEAD) dirty $(git status --porcelain | wc -l)"
echo "cmd: $*"
} > "$E/$N.log"
flock -o "$HOME/handoffs/hive/queen/pool-10-11.lock" timeout 1140 taskset -c 10-11 nice -n 10 "$@" >> "$E/$N.log" 2>&1
rc=$?
echo "step $N end $(date -u +%FT%TZ) exit $rc" >> "$E/$N.log"
exit $rc
