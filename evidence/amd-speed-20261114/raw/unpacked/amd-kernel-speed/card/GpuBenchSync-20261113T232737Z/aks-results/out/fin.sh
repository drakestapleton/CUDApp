#!/bin/sh
# Pack results with per-file SHA256SUMS once card-measure.sh printed MEASURE_DONE.
cd /workspace/aks
while ! grep -q MEASURE_DONE measure.log 2>/dev/null; do sleep 5; done
rm -rf out && mkdir out && cp -r runs measure.log pre.log plan/real2.plan.json plan/real2.plan.cmd plan/cap286.plan.json amplify-plan.py card-measure.sh pre.sh fin.sh upload.json out/ 2>/dev/null
(cd out && find . -type f ! -name SHA256SUMS | sort | xargs sha256sum > SHA256SUMS && cd runs && sha256sum * > SHA256SUMS)
tar czf aks-results.tar.gz out && sha256sum aks-results.tar.gz > aks-results.sha256
echo FIN_DONE >> measure.log
