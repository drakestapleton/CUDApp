# Level 2 block of BRIEF.md section 6, lines after the omc hash, verbatim; cwd = directory holding CUDApp and omega-mv
echo "START $(date -u +%FT%T.%NZ)"
cd CUDApp; echo "### rc=$? cd CUDApp"
python3 -I tests/amd-kfd-launch/check.py "$(realpath ../omega-mv/seed/out/omc)" > ../../gate-stdout.log 2> ../../gate-stderr.log; echo "### rc=$? check.py"
sha256sum build/amd-kfd-launch/gate/launch-x86; echo "### rc=$? sha256sum"
echo "END $(date -u +%FT%T.%NZ)"
