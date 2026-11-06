#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Kills policy.om mutants: each one changes one constant, regenerates the Omega
# unit, and must make the C-oracle differential fail. HOST_ONLY, no device.
set -eu
cd "$(dirname "$0")/.."
OMC="${OMEGA_OMC:-$HOME/work/cudastack/omega-mv/seed/out/omc}"
BRIDGE="${OMEGA_BRIDGE:-$HOME/work/cudastack/omega-bridge}"
POL=native/nvrm/policy.om
cp "$POL" /tmp/policy.om.good
restore() { cp /tmp/policy.om.good "$POL"; OMEGA_OMC="$OMC" OMEGA_BRIDGE="$BRIDGE" sh om/units.sh >/dev/null; }
trap restore EXIT

kill_one() {
    name=$1; from=$2; to=$3
    cp /tmp/policy.om.good "$POL"
    sed -i "s/$from/$to/" "$POL"
    OMEGA_OMC="$OMC" OMEGA_BRIDGE="$BRIDGE" sh om/units.sh >/dev/null
    cc -std=gnu11 -O2 -D_GNU_SOURCE -Invrm -Isrc -o /tmp/nvrm-policy-mut nvrm/policy_test.c src/cudapp_om_nvrm_policy.c
    if /tmp/nvrm-policy-mut >/dev/null 2>&1; then
        echo "FAIL mutant $name: differential still passed"
        exit 1
    fi
    echo "killed $name"
}

kill_one nrstat "eq st 81" "eq st 82"
kill_one hnext "eq cls 65" "eq cls 66"
kill_one pspan "gt nwords 2097151" "gt nwords 2097150"
kill_one gpentry "shl nw 42" "shl nw 41"
kill_one mthd "shl subc 13" "shl subc 12"
kill_one pfull "sub entries 1 4294967295" "sub entries 0 4294967295"
echo "NVRM POLICY MUTANTS PASS (6 killed)"
