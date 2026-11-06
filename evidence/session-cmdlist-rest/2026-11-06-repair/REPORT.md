# PR #334 repair: session and command-list decisions

Stack: `feat/session-cmdlist-rest-om`, repairing `fe565343a4950bc76bb0f56e4943558de22fb201`, based on PR #304 `feat/cmdlist-program-om` at `581c048799f13b5b29a5172031c15889d6067672`.

## Corrections to the original report

The earlier `2026-11-06/REPORT.md` is retained as historical evidence, not the current verdict. Its attribution of the command-list shell failure to unchanged #304 code was incorrect. The stale decision did not preserve its argument order, and operation 1 implemented an unrelated wait-argument predicate instead of the latch predicate. Both have been repaired in `om/cmdlist/book.om`; unused, uncalled wait helpers were removed. Latch generations retain the original uint32 wrap at `UINT32_MAX`.

Its description of the C oracle as default-OFF without qualification was also incomplete: `OMEGA_GPU_NO_POSIX` still selects C session/cmdlist decisions in this branch. The hosted path calls the new Omega units. This repair does not claim the freestanding session/cmdlist cutover.

## Clean-head comparison

Created a separate detached worktree `/tmp/session-cmdlist-base304` at the exact #304 head above, after querying `gh pr view 304` and fetching its branch. Both checks ran before the repair:

- `make -j3 test-om-cmdlist`: **PASS, rc 0**. Planner differential, simulator, real command-list shell and cost checks completed. See `base304-cmdlist.log`.
- `sh -c 'make -j3 build/test_omega_gpu_platform_linux && ./build/test_omega_gpu_platform_linux'`: **PASS, rc 0**, 17 passed, 0 failed. See `base304-platform-linux.log`.

Therefore neither reported failure is a #304 baseline failure. PR #329's `5643e397cc6549d97a5c527c704510f5758d09e6` changes `/memfd` recognition and alias-bound rounding in its platform-rest unit; it is not present here and was not needed to repair these failures. Both affected command-list reference cases now pass: `CMDLIST_NEAR_CAP_SUBMITS_WITH_L2_INVAL_PENDING` and `CMDLIST_TIMEOUT_LATCHES_WITHOUT_LAUNCH_BEHIND_IT`.

## Repairs

- Replaced the split-shell `smut` recipe with `tests/check_session_mutants.py`. The broken continuation and stale C-only sed sites no longer exist.
- The new harness first requires a passing unmodified hosted session against the existing explicit fake platform. `tests/session_no_default.c` is a verification-only provider that cannot open a device. The session itself is compiled without `OMEGA_GPU_NO_POSIX`, so its decisions and packets use the default Omega bindings.
- Sixteen compiled mutants are killed by assertions: seven regenerate `session_admit`, `session_words`, `session_park` or `cmdlist_book` from altered `.om` sources using the pinned compiler/bridge; nine alter remaining platform/atomic C glue. Each source site must have the exact expected occurrence count. Conversion/compilation failures, absent/ambiguous sites, signals and timeouts abort the harness and never count as kills. A kill requires exit status 1 plus assertion `FAIL` output, not arbitrary execution failure.
- Fixed the parked-buffer binding call to pass the declared 512-element view lengths, with initialized unused slots. The old runtime-count lengths were rejected by the checked ABI and hid parked handles.
- Fixed session bookkeeping defects reached during repair: preserve 64-bit page sizes and scratch capacity, preserve scratch high-water, classify frees after device close as stale rather than parked, and preserve the original already-open admission precedence. The C fallback mirrors these corrections, including copying setup words individually into its uint64 packet buffer and using the real Ampere compute-class constant.
- Added direct policy boundary regressions in `tests/session_policy_test.c`. Its first stale-order assertion failed with rc 1 against the original generated binding before repair, and passes now. It also covers stale epoch rejection, latch generation and wrap, a page size beyond uint32, stale free disposition, and retained scratch high-water.
- Regenerated checked C and identity manifests from the `.om` source. No handwritten C computation was added to the production path.

The shared makefile recipe accepts both `GPU_FS_OBJS` and the separate `GPU_NO_POSIX_OBJS` interface used by the concurrent NO_POSIX lane; the latter is empty on this branch.

## Execution and safety

Every build/test/mutant and generated-unit check used this wrapper, with at most three build jobs:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" \
      OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge" \
      make -j3 test-om-cmdlist test-gpu-session-mutants test-gpu-session-platform
```

The same wrapper surrounds `sh om/units.sh` and `sh om/units.sh --check`. Compiler: omega-mv `e5b32567`; omc SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. Outside dependency used only for checked conversion: the existing pinned Omega Bridge converter/frontend recorded in `src/cudapp_om_units.ident`. C test compilers and fake-driver simulators remain verification oracles.

The page-mask constant was checked with WolframLanguageEvaluator: `{Integrate[x^2,{x,0,1}],2^64-4096,FromDigits["7c0",16]} -> {1/3,18446744073709547520,1984}`. The evaluator and Alpha mount verification completed; these are modeled integer values, not hardware measurements.

Full `make test`, GPU, live RM, VFIO and hardware qualification: **NOT_RUN**. No service, driver state, gate, watchdog or historical receipt was changed. The draft is not merged or deployed.

## Final verdicts

**IMPLEMENTED and EXERCISED** on the repaired lane:

| Acceptance | Result |
|---|---|
| `make -j3 test-om-cmdlist test-gpu-session-mutants test-gpu-session-platform` | PASS, overall rc 0; all three named targets completed |
| Omega session mutants | PASS, 16/16 behavior-killed, including 7 regenerated `.om` mutants |
| Freestanding session/platform fixture | PASS, 16 passed, 0 failed |
| Hosted Linux reference with simulated driver | PASS, 17 passed, 0 failed |
| VA-slot checks | PASS, 10 passed, 0 failed |
| `sh om/units.sh --check` | PASS, rc 0, byte-for-byte regeneration |
| `crumb compile . && crumb verify .` | PASS, rc 0; 41 crumbs current |

`acceptance.log` records the final combined make run. `units-check.log` records reproducibility. `crumb.log` records the crumb result. Baseline logs retain the original complete output. Absolute home paths in these copied logs were normalized to `~`.

`harness-site-error.log` records an intermediate harness error: an ambiguous `open-while-latched` pattern aborted with rc 2, without reporting that mutant killed. The site was then narrowed to its complete function prefix; the final run above is the passing evidence. This is a negative control for distinguishing a broken mutation site from a behavioral kill.

**QUALIFIED on hardware, MERGED, DEPLOYED: NOT_RUN.** Existing chip qualification procedures are unchanged.
