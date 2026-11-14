#!/bin/sh
# Pre-work on the card: public clone at the published commit, Level 1 check, weights download and hash check.
cd /workspace/gap2
exec > pre.log 2>&1
set -x
date -u +%FT%TZ
git clone https://www.aienos.com/rad/z4RxD8vXigmWg61nM8miQ8DVPAXEP.git CUDApp
cd CUDApp
git checkout dc536882c06a692991be97dc65cd2d037d82b129
git rev-parse HEAD
python3 -I evidence/amd-qwen38-native-refusal-20261113/verify.py; echo "LEVEL1_RC=$?"
cd evidence/amd-qwen38-native-20261113
export AMDSERVE_WEIGHTS=/tmp/qwen3.8-27b-fp8
date -u +%FT%TZ
python3 -c "import runpy; runpy.run_path('oracle/driver-real2.py', run_name='prepare-import')['prepare']()"; echo "PREPARE_RC=$?"
date -u +%FT%TZ
du -sh /tmp/qwen3.8-27b-fp8
echo PRE_DONE
