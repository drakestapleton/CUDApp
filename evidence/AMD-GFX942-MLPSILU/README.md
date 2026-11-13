# Fused MLP activation receipt

Operator intent: SiLU(gate)*up between GEMV2 gate/up and GEMV2 down.

IMPLEMENTED and EXERCISED on the host. The exact commands, exits, complete logs, final gate lines, source digests, objects, negative-control objects and generated per-output intervals are filed here. Both existing gates include all earlier cases. LLVM parity includes the metadata, descriptor and full instruction stream. The mandatory static scheduling check includes the new object.

No seeded-Omega compiler error occurred. The first LLVM oracle assembly rejected metadata because the unquoted argument name `N` is a YAML boolean. Renaming that argument to `count` in both the production table and oracle assembly fixed it; the rejected log remains under diagnostics. The kernel ABI and instruction stream did not change for that repair.

`mlpsilu.py` is the generated v7 notebook section; the canonical authoring sources are `tests/amd-exec/token.py` and `notebook_section.py`. Its object-map keys are `mlpsilu`, `mlpsilu-no-silu` and `mlpsilu-wrong-up`. All three load the symbol `mlpsilu`. The section takes `ctx` as the notebook's globals, including its existing `objs`, HIP allocation/free helpers, timing helpers and controls dictionary. It checks every timed output, not sampled rows, and keeps all input and guard bytes checked. Generated copies are not edited by hand.

Hardware execution of this kernel and the notebook section: NOT_RUN here. This is not full serving, a timing claim, a merge or a deployment. The independent MI300X attention-core receipt in the adjacent device-evidence directory does not qualify this new kernel.
