# Any-shape attention chip procedure

**NOT_RUN. No GPU window is authorized by this PR.** Do not run the chip command
until Drake authorizes an exclusive window on the exact device and commit.
Do not acquire the chip lock, stop a service or change a gate on this lane's
authority. In particular, leave :18091, :18173–18178, PID 68451, omega-tool
services, NVIDIA modules and vfio untouched.

1. Have the operator apply the current exclusive-window/preflight procedure.
   A free lock alone is not authority. Record UTC date, device identity,
   driver/firmware state, branch/commit, compiler/bridge pins and authorization.
2. Check that the host suite and source mutants passed on the exact commit.
   Build `build/gpu_attn_any_test` with the documented pinned tools, on cores
   10–14 at nice 10 and at most -j3. Preserve build stdout/stderr and rc.
3. Run `make doctor` under the authorization procedure. A refusal stops the
   experiment; never change its gate. This runbook does not claim direct APIs
   have the wrapper's driver/PCI admission checks.
4. Only then run:

   ```sh
   taskset -c 10-14 nice -n 10 build/gpu_attn_any_test --chip
   ```

   The command executes f32 and bf16 attention against the independent double
   host reference, including rectangular sequences, tails, causal modes and
   GQA. The same wrong-mask negative control must disagree. Unsupported-shape
   checks must return REFUSED before device open. Record complete output and
   rc; expected final line:
   `attn-any chip: PASS (reference, shape sweep, refusals, wrong-mask control)`.
5. Collect telemetry only through the authorized procedure. Any launch failure,
   timeout, mismatch, unwritten output or refusal is FAIL/INCONCLUSIVE, not
   PASS. Do not lift the session's uncertain-completion latch. Preserve new
   evidence under a fresh dated directory; never edit an old receipt.
6. Return the device according to the window procedure. A chip result applies
   only to the exact commit/driver/shape envelope exercised. It is not a
   performance claim or exhaustive all-input numerical qualification.

The CPU simulator's EX2/RCP are host oracles. The provisional error allowance
in README is not a measured hardware approximation bound. If chip error exceeds
the documented tolerance, fix/characterize the kernel rather than silently
loosening the gate. ROADMAP remains PARTIAL until a reviewed chip receipt
justifies a separate status decision.
