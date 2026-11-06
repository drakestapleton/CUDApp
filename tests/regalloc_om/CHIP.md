# Core allocator and matmul chip qualification

**NOT_RUN. No GPU window is authorized.** Host, mutation, and simulator results
are not hardware qualification. Do not access live RM, VFIO, NVIDIA devices,
service ports, modules, or the GB10 lock under the present assignment.

1. Integrator first adopts all eleven units listed in
   `om/codegen/REGALLOC.md`, the exact-liveness replay change, and BridgeHost's
   separately reviewed freestanding OM2 array support. Freeze source/compiler/
   bridge revisions and generated aggregate identities in a new receipt.
2. Run the lane's host gate, ten semantic mutants, and signed-GE negative
   control in the required device-free namespace:

   ```sh
   bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh tests/regalloc_om/run.sh
   bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 python3 tests/regalloc_om/mutants.py
   bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh tests/regalloc_om/sim_ge.sh
   ```
3. Obtain current explicit exclusive-window authorization through the current
   Spark window procedure. Have the operator specify permitted device bindings
   and service disposition. The existing minimal `/dev` wrapper intentionally
   cannot run a chip test; do not remove it or add devices without that new
   authorization. A free lock is not authorization.
4. After that gate only, use the repository's `gpu_matmul_api_test` chip sweep,
   documented at the top of `tests/gpu_matmul_api_test.c`, with a new `--out`
   receipt. The `--host-only` mode is not a chip run. Under the newly authorized
   device namespace, compare all five builder families to their existing CPU
   reference, including K=16/32/64, scalar pressure refusals, fragment/block
   layouts and partial tiles. Require retained C and Omega encoded digests to
   match before launching either artifact.
5. Run the existing `--mutant` negative control under the same authorization;
   require a real detected output mismatch, not a missing-device refusal.
   Stop on a driver fault or unexpected session state; never weaken the gate.
6. Record exact commands, rc, revisions, shape corpus, expected/observed output,
   digest equality, device state and negative-control outcome in a *new*
   evidence directory. Report PASS/FAIL/INCONCLUSIVE/NOT_RUN separately for
   hosted, freestanding and chip tiers. Integrator owns the integrated full
   test gate. No merge or deployment follows from a chip result alone.
