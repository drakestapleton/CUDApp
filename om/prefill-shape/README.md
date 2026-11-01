# Short-prompt matrix shape

The Omega policy admits the existing 64-row, eight-warp matrix kernel and one experimental 32-row, four-warp kernel. The default remains 64 rows. The current parameterized emitter, arithmetic order, wait distance, tail masks and device ownership checks are retained.

The 32-row shape reduces padded work for short prompts. This is a hypothesis until chip and full-model measurements establish a benefit. The emitter stages scales in paired row groups; a 16-row shape is not admitted. Wider and other untested shapes remain refused.

`make test-prefill-shape` checks the portable admission boundary, host numerical references and simulated direct/indexed matrix cases, including row tails, natural SwiGLU order and deliberate faults. It does not open a GPU. `om/units.sh --check` verifies generated bindings with the pinned Omega compiler and Bridge.

Hardware reference: NVIDIA PTX ISA9.4, physical page605, warp matrix instruction examples. This change reuses the existing instruction generator and changes its work grouping. The manual is not evidence that this implementation runs correctly; simulator and chip receipts supply that evidence.

The existing `omega_gpu_pf_qgemm_set_shape(2, 4, 45)` test knob selects the candidate. Call it before sizing prefill buffers or building tile tables. Never change shape while a model operation is in flight. No serving option or automatic policy is introduced here.

The separate serving screen in `serving-validation.json` passes two128-token reference runs with a seeded sampled request between them. First-visible latency remains about0.5seconds on the shared machine. The four-matrix chip comparison is mixed; this shape is not promoted as a speed improvement. Longer contexts, the full conversation/streaming gate and controlled pairs remain pending.
