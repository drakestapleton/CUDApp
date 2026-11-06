# Prefill host residue migration

Base #333 feat/api-views-rest-om (5dad87a5). Existing views and the #277 kernel generator are retained. New seeded units own the register-file admission comparison and post-copy poison count. C effect sites continue to allocate, launch and copy. Default-OFF C oracle: CUDAPP_MIGRATION_PENDING_C_PREFILL_HOST_REST. Public API unchanged; strong binding references, no fallback.

## Reproduction and evidence

Compiler: omega-mv e5b32567, omc sha256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.
Bridge: /tmp/bridge-73-apiviews matching base registration pin (toc sha256 d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538). Generation uses OMEGA_SEED=~/work/cudastack/omega-bridge/build/bootstrap/omc, OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc and OMEGA_BRIDGE=/tmp/bridge-73-apiviews with sh om/units.sh. Unit registration is committed before consumer cutover.

All checks were run inside `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.

- `env OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/tmp/bridge-73-apiviews python3 om/apirest/pfhost-mutants.py`: rc 0, `PASS prefill host red control and six mutants`. The missing strong binding fails linking (red). Six compiled seeded mutations killed: limit65535, warp31, compare>=, poison-998, count+2, skip0.
- `make -j3 test-gpu-prefill-host`: rc 0, `test-gpu-prefill-host: PASS (8 prefill tests, host model + warp simulator, no device)`. Artifact artifact://9242.

The register policy differential crosses 65536 and source-derived warps/GPR boundary values, includes zero and UINT32_MAX. Poison-count equality is checked against C over ±0, subnormal, normal, infinity and NaN bit patterns, poison and immediate bit-neighbors, and varying buffer lengths including empty. Count is integer-exact; floating equality has identical class behavior to the old C. No floating arithmetic is introduced in the register comparison.

## Tier verdicts

IMPLEMENTED and EXERCISED: differential PASS, negative link control PASS, six semantic mutants killed, prefill host + warp simulation PASS. Chip NOT_RUN; no current GPU authorization. Not MERGED or DEPLOYED.

## Chip runbook

In a separately authorized exclusive window, record the pinned build/model inputs; run gpu_prefill_attn_test on hardware with the default Omega policy, then the default-OFF C oracle build, compare output vector bits and poisoned counts for the same shapes. Run a refused register-overflow shape and verify no launch. Record results in a new chip receipt. Do not interpret the host simulator as hardware qualification. No live model or device service was disturbed by this lane.
