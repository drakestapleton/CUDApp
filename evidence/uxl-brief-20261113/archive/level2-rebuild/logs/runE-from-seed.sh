# Proof run for the corrected "rebuild the compiler from its seed" command, as an outside engineer would type it.
# Fresh local copy of the public omega-mv clone (SCRATCH/B/omega-mv, cloned over https), nothing moved aside.
D=/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild
cd $D && rm -rf S2 && mkdir S2 && cd S2
git clone -q --no-hardlinks ../B/omega-mv omega-mv; echo "### rc=$? local clone"
git -C omega-mv checkout -q 2ee0a70d1830581b706f288e722f3329d51f3583; echo "### rc=$? checkout"
cd omega-mv
git rev-parse HEAD
echo "--- git status --short --untracked-files=all BEFORE ---"; git status --short --untracked-files=all; echo "### before: $(git status --short --untracked-files=all | wc -l) lines"
sha256sum seed/out/omc seed/out/omc.sha256 seed/omc/size-history.tsv
stat -c '%y %n' seed/out/omc
t0=$(date +%s.%N)
sh seed/omega-from-seed.sh > ../from-seed.log 2>&1; rc=$?
t1=$(date +%s.%N)
echo "### rc=$rc sh seed/omega-from-seed.sh"; echo "wall seconds $(echo "$t1 - $t0" | bc)"
cat ../from-seed.log
echo "--- git status --short --untracked-files=all AFTER ---"; git status --short --untracked-files=all; echo "### after: $(git status --short --untracked-files=all | wc -l) lines"
echo "--- git status --short --ignored AFTER (ignored build output) ---"; git status --short --ignored | head -5
git diff --stat | tail -1; echo "### git diff rc=$(git diff --quiet; echo $?) (0 = no tracked content changed)"
sha256sum seed/out/omc seed/out/omc.sha256 seed/omc/size-history.tsv
stat -c '%y %n' seed/out/omc
du -sh build 2>/dev/null
