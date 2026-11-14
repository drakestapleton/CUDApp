#!/bin/sh
# Card pre-work (same commands as the published brief's pre.sh): public clone at dc536882, checkpoint download + SHA256.
cd /workspace/aks
exec > pre.log 2>&1
set -x
date -u +%FT%TZ
git clone https://www.aienos.com/rad/z4RxD8vXigmWg61nM8miQ8DVPAXEP.git CUDApp
cd CUDApp && git checkout dc536882c06a692991be97dc65cd2d037d82b129 && git rev-parse HEAD
cd evidence/amd-qwen38-native-20261113
export AMDSERVE_WEIGHTS=/tmp/qwen3.8-27b-fp8
python3 -c "import runpy; runpy.run_path('oracle/driver-real2.py', run_name='prepare-import')['prepare']()"; echo "PREPARE_RC=$?"
date -u +%FT%TZ
echo PRE_DONE
