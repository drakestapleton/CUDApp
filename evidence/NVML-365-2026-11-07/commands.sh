#!/bin/sh
# Scratch runner (not shipped): fresh-clone verification of CUDApp#365 at $1 with
# omega-bridge bc11b30 (OMEGA_BRIDGE) and omega-bridge#83 at $2 (OMEGA_CABI_BRIDGE).
set -u
H=$1
B83=$2
R=/tmp/bab/fc
E=$R/ev-cudapp
rm -rf $R/cudapp $R/b30 $R/b83 $R/b83-wrongtoc $E
mkdir -p $E
export OMEGA_SEED=$HOME/work/finish/nvml365-bridge/build/bootstrap/omc
OMC=$HOME/work/finish/nvml365-tools/omc
git clone -q ~/work/finish/CUDApp-365 $R/cudapp && git -C $R/cudapp checkout -q "$H" || exit 9
git clone -q ~/work/finish/nvml365-bridge $R/b30 && git -C $R/b30 checkout -q bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4 || exit 9
git clone -q ~/work/bridgeabi $R/b83 && git -C $R/b83 checkout -q "$B83" || exit 9
(cd $R/b30 && OMEGA_SEEDED_OMC=$OMC sh tools/build.sh) > $E/build-b30.log 2>&1; echo "rc=$?" >> $E/build-b30.log
(cd $R/b83 && OMEGA_SEEDED_OMC=$OMC sh tools/build.sh) > $E/build-b83.log 2>&1; echo "rc=$?" >> $E/build-b83.log
cd $R/cudapp
{
echo "CUDApp clone: $(git rev-parse HEAD)"
echo "OMEGA_BRIDGE clone: $(git -C $R/b30 rev-parse HEAD)"
echo "  src/toc.om sha256: $(sha256sum < $R/b30/src/toc.om | cut -c1-64) (om/units.sh $(grep '^CONVERTER_PIN=' om/units.sh))"
echo "  tools/bridge-seeded sha256: $(sha256sum < $R/b30/tools/bridge-seeded | cut -c1-64) (om/units.sh $(grep '^FRONTEND_PIN=' om/units.sh))"
echo "OMEGA_CABI_BRIDGE clone: $(git -C $R/b83 rev-parse HEAD)"
echo "  src/toc.om sha256: $(sha256sum < $R/b83/src/toc.om | cut -c1-64) (lock $(grep '^CABI_CONVERTER_PIN=' om/omega-bridge-cabi.lock))"
echo "  tools/bridge-seeded sha256: $(sha256sum < $R/b83/tools/bridge-seeded | cut -c1-64) (lock $(grep '^CABI_FRONTEND_PIN=' om/omega-bridge-cabi.lock))"
echo "OMEGA_OMC sha256: $(sha256sum < $OMC | cut -c1-64)"
echo "OMEGA_SEED (bridge bootstrap) sha256: $(sha256sum < $OMEGA_SEED | cut -c1-64)"
echo "cc (ORACLE): $(cc --version | sed -n 1p)"
echo "nm (ORACLE): $(nm --version | sed -n 1p)"
echo "readelf (ORACLE): $(readelf --version | sed -n 1p)"
echo "make: $(make --version | sed -n 1p)"
echo "CUDA nvml.h (ORACLE header): $(sha256sum < /usr/local/cuda/include/nvml.h | cut -c1-64)"
} > $E/env.txt
run() { name=$1; shift; "$@" > $E/$name.log 2>&1; echo "rc=$?" >> $E/$name.log; echo "$name $(tail -1 $E/$name.log)"; }
MK="make test-nvml CUDAPP_OMC_HOST=1 OMEGA_OMC=$OMC OMEGA_BRIDGE=$R/b30"
run test-nvml $MK OMEGA_CABI_BRIDGE=$R/b83
run test-nvml-unconfigured make test-nvml CUDAPP_OMC_HOST=1 OMEGA_OMC=$OMC OMEGA_BRIDGE=$R/b30
run gate-no-cabi-env env -u OMEGA_CABI_BRIDGE OMEGA_OMC=$OMC OMEGA_BRIDGE=$R/b30 build/nvml_gate
# The 53 portable units regenerate byte for byte with bc11b30 (unchanged since 1fae7beb).
run units-check-b30 env OMEGA_OMC=$OMC OMEGA_BRIDGE=$R/b30 sh om/units.sh --check
grep -c '^generating portable unit' $E/units-check-b30.log > $E/units-count.txt
git diff --stat 1fae7beb HEAD -- src om/units.sh om/omega-bridge.lock > $E/units-diff-since-1fae7beb.txt; echo "rc=$? (empty stat = no unit, pin or generated file changed)" >> $E/units-diff-since-1fae7beb.txt
# The #83 bridge cannot regenerate the other units: om/units.sh refuses it.
run units-check-b83-refused env OMEGA_OMC=$OMC OMEGA_BRIDGE=$R/b83 sh om/units.sh --check
# Wrong CABI converter: one note line appended to a copy's src/toc.om.
cp -a $R/b83 $R/b83-wrongtoc && printf 'note wrong-toc negative control\n' >> $R/b83-wrongtoc/src/toc.om
run neg-wrong-toc env OMEGA_OMC=$OMC OMEGA_BRIDGE=$R/b30 OMEGA_CABI_BRIDGE=$R/b83-wrongtoc build/nvml_gate
G="env OMEGA_OMC=$OMC OMEGA_BRIDGE=$R/b30 OMEGA_CABI_BRIDGE=$R/b83 build/nvml_gate"
# Lock with one hex digit changed in CABI_CONVERTER_PIN.
sed -i 's/^CABI_CONVERTER_PIN=2/CABI_CONVERTER_PIN=3/' om/omega-bridge-cabi.lock && git diff --stat -- om/omega-bridge-cabi.lock > $E/neg-lock-pin.diffstat
run neg-lock-pin $G
git checkout -q -- om/omega-bridge-cabi.lock
# Lock without its CABI_FRONTEND_PIN row.
sed -i '/^CABI_FRONTEND_PIN=/d' om/omega-bridge-cabi.lock && git diff --stat -- om/omega-bridge-cabi.lock > $E/neg-lock-row.diffstat
run neg-lock-row $G
git checkout -q -- om/omega-bridge-cabi.lock
# Lock with a second CABI_CONVERTER_PIN row (all zeros) after the pinned one.
printf 'CABI_CONVERTER_PIN=%064d\n' 0 >> om/omega-bridge-cabi.lock && git diff -- om/omega-bridge-cabi.lock > $E/neg-lock-dup.diff
run neg-lock-dup $G
git checkout -q -- om/omega-bridge-cabi.lock
# Committed generated library with one byte changed.
sed -i '0,/nvmlReturn_t/s//nvmlReturn_t /' compat/nvml/libnvidia-ml.c && git diff --stat -- compat/nvml/libnvidia-ml.c > $E/neg-stale-lib.diffstat
run neg-stale-lib $G
git checkout -q -- compat/nvml/libnvidia-ml.c
# Committed export map with one export dropped.
sed -i '/nvmlShutdown;/d' compat/nvml/libnvidia-ml.map && git diff --stat -- compat/nvml/libnvidia-ml.map > $E/neg-stale-map.diffstat
run neg-stale-map $G
git checkout -q -- compat/nvml/libnvidia-ml.map
git status --short > $E/clone-status-after.txt
echo done
