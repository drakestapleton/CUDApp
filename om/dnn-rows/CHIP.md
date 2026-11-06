# Row operators: chip gate NOT_RUN

No GPU run was authorized or performed. Host, generated-code and simulator
results are not hardware qualification. Do not execute this runbook from an
agent assignment without current explicit exclusive-window authorization.

Operator prerequisites:

1. Arrange an authorized GB10 window through the current operator procedure.
   Confirm the actual serving model and all service owners before any changes.
   This runbook does not authorize stopping services, acquiring a GPU lock,
   changing a gate, rebooting, or altering NVIDIA modules.
2. Record the candidate commit, compiler digest, generated source identities,
   machine, driver and device, and preserve the preflight output in a new
   receipt directory. Never overwrite historical receipts.
3. Build the focused binary with the pinned compiler/bridge as in README.md.
   The build is host-only. Run the normal repository operator preflight.
4. Only inside that approved window, execute:

       taskset -c 10-14 nice -n 10 ./build/gpu_dnn_rows_test --chip

   This runs the full listed shape/type/epsilon/constant/varied sweep with
   the same independent double oracle. Preserve command, exact rc, full output
   and final line. Record any timeout as inconclusive/failed, never PASS.
5. Require the wrong-result and refusal controls to pass, and compare chip
   output against the documented modeled test envelope. A passing finite
   fixture sweep qualifies only that measured domain on the named chip.
6. Operator restores any operator-managed state under its own runbook and
   records restoration evidence. No merge or deployment is authorized here.

Current status: CHIP NOT_RUN; qualified NOT_RUN; merged NOT_RUN; deployed NOT_RUN.
