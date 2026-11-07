#!/bin/sh
# cuda202 (DRA-202): remove the Mac working directory of macrun.sh after its receipts were copied back.
B=$HOME/work/hive/cuda202
[ -d "$B" ] && rm -rf "$B"
ls -d "$B" 2>/dev/null || echo "removed $B"
