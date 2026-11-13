#!/bin/sh
# usage: pool.sh CMD...   runs CMD under hive CPU pool locks 10-11,10,11 on cores 10,11 at nice 10
# with a clean environment (no lane variables): HOME, PATH (system only), LANG, TERM.
Q=/home/drakestapleton/handoffs/hive/queen
exec flock "$Q/pool-10-11.lock" flock "$Q/pool-10.lock" flock "$Q/pool-11.lock" \
  taskset -c 10,11 nice -n 10 \
  env -i HOME="$HOME" PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin LANG=C.UTF-8 TERM=dumb "$@"
