cd /workspace/uxl
while ! grep -q -E 'L3_DONE|PREPARE_FAILED' uxl-l3.log; do sleep 5; done
date -u +%FT%TZ
python3 uxl-stock.py; echo "STOCK_RC=$?"
date -u +%FT%TZ
echo STOCK_DONE
