# CodegenRest chip qualification runbook

**Current verdict: NOT_RUN. No GPU, live-RM, VFIO, or device window is authorized.**
Host IR equivalence and warp simulation are not hardware qualification.
Do not execute the prospective chip phase below under the present assignment.

## Preconditions requiring a new operator decision

1. Obtain current, explicit authorization naming the exclusive GPU window,
   exact executable revisions, device exposure, and rollback/cleanup owner.
   The existence or absence of a lock is not authorization.
2. Coordinate with Main/Integrator and the live-service owners. This lane must
   not stop or alter services, ports 18091 or 18173–18178, PID 68451, the
   omega-tool services, `/tmp/aien-gb10.lock`, driver modules, or VFIO state.
3. Record the source commit, generated-unit identities, pinned omc SHA256,
   compiler flags, simulator receipt, GPU/device identity and the operator's
   current authorization in a **new** evidence directory. Never overwrite an
   older receipt.
4. Require passing device-free differential, negative-control, compiled-mutant,
   host and warp-sim tiers first. Use the minimal `/dev` namespace in README.md
   for every host-tier command. Missing-device failures in a purported host or
   sim path are defects, not permission to expose the device.

## Prospective hardware matrix

The hardware phase requires authorization that explicitly supersedes the
current device-free-only test restriction. Main must choose the permitted
device namespace and the existing window/ownership procedure; this runbook
neither grants permission nor changes that procedure.

Use binaries built from the default Omega configuration, not the C-oracle
macros. The existing test entrypoints are:

| Family | Executable | Hardware mode after new authorization |
| --- | --- | --- |
| attn256 | gpu_decode_attn256_test | neither --host-only nor --sim; --out a fresh JSON |
| pattn256 | gpu_prefill_attn256_test | neither --host-only nor --sim; --out a fresh JSON |
| router | gpu_router_test | neither --host-only nor --sim; --out a fresh JSON |
| gemm | gpu_fp8gemm_test | neither --host-only nor --sim; --out a fresh JSON |
| bf16 | gpu_bf16_gemv_test | neither --host-only nor --sim; --out a fresh JSON |
| pq/qg | gpu_qgemm_test | explicit --chip; preserve its existing shape selection |

Use each program's existing hardware numerical and refusal assertions. Include
qg rt=8 to exercise the opt-in GPR limit, ordinary qg to exercise the unchanged
default limit, and the existing numerical mutants. Record exact arguments,
exit status, final line and produced receipts. Keep dtype/rounding and
non-contracting IEEE oracle rules unchanged.

## Stop and clean up

Any timeout, numerical discrepancy, device-open failure, NVRM/Xid symptom,
or ownership conflict stops qualification. Preserve the evidence and report
FAIL or INCONCLUSIVE as appropriate. Never rerun by weakening a gate or
changing a live service. Terminate only exact processes created by this
qualification. Main owns window closure and any service restoration.

Only passing newly authorized hardware evidence may change QUALIFIED from
NOT_RUN. Neither this file nor a successful host tier changes MERGED or
DEPLOYED status.
