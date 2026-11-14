cd /workspace/uxl
{ date -u +%FT%TZ; rocminfo 2>/dev/null | grep -B4 -A14 'Name: *gfx942' | head -60; rocm-smi --showproductname 2>&1 | head -30; rocm-smi --showmeminfo vram 2>&1 | head -20; hostname; } > uxl-card.txt 2>&1
E=CUDApp/evidence/amd-qwen38-native-20261113/out
cp $E/real2.plan.json uxl-real2.plan.json; cp $E/real2.plan.cmd uxl-real2.plan.cmd; cp $E/real2-controls.plan.json uxl-real2-controls.plan.json; cp $E/real2-controls.plan.cmd uxl-real2-controls.plan.cmd
sha256sum $E/real2.plan $E/real2-controls.plan $E/omega-launch $E/code/*.hsaco > uxl-plan-and-code.sha256
git -C CUDApp rev-parse HEAD > uxl-cudapp-head.txt; git -C CUDApp status --porcelain | head -20 >> uxl-cudapp-head.txt
rm -f uxl-evidence.tar.gz SHA256SUMS
sha256sum pre.log pre.sh uxl-*.sh uxl-*.log uxl-*.txt uxl-*.err uxl-*.json uxl-*.cmd uxl-*.sha256 uxl-stock.py launch-x86 vadd.hsaco > SHA256SUMS
tar czf uxl-evidence.tar.gz SHA256SUMS pre.log pre.sh uxl-*.sh uxl-*.log uxl-*.txt uxl-*.err uxl-*.json uxl-*.cmd uxl-*.sha256 uxl-stock.py
sha256sum uxl-evidence.tar.gz > uxl-evidence.tar.gz.sha256
echo FIN_DONE >> uxl-card.txt
