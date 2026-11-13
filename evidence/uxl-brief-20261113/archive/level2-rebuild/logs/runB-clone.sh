set -x
date -u +%FT%TZ
git clone https://www.aienos.com/rad/z4RxD8vXigmWg61nM8miQ8DVPAXEP.git CUDApp; echo "### rc=$?"
cd CUDApp; echo "### rc=$?"
git checkout dc536882c06a692991be97dc65cd2d037d82b129; echo "### rc=$?"
cd ..
git clone https://www.aienos.com/rad/z6do9WDBhuSnvopDkgFdFr1yWA62.git omega-mv; echo "### rc=$?"
git -C omega-mv checkout 2ee0a70d1830581b706f288e722f3329d51f3583; echo "### rc=$?"
sha256sum omega-mv/seed/out/omc; echo "### rc=$?"
date -u +%FT%TZ
