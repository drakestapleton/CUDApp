# Standalone zero-centered RMSNorm receipt

Operator intent: the input/post-attention/final RMSNorm over the pinned model's hidden dimension, with (1+weight).

IMPLEMENTED and EXERCISED on the host. Exact commands, exits, complete logs, immutable diagnostic logs, object/control bytes, source digests and per-output numerical intervals are filed here. The host gate includes LLVM instruction/metadata/descriptor equality, byte-identical builds and the mandatory all-object scheduling check. The executor gate retains all prior cases and checks the new norm's full outputs, guards and genuine wrong-object controls.

No seeded-Omega compiler error occurred. The first norm run was correctly refused with exit66: the authoring stream used an SGPR as VOP3 source0, outside the executor's existing audited VGPR-source0 subset. Moving the row index into a VGPR before the multiply fixed the program without expanding that subset. The failed executor log is retained. A later test-instrument edit initially selected the wrong line boundary; the edit tool reported the syntax issue, and the fixture and timing placement were corrected before acceptance.

This receipt also supersedes the previous generated MLP notebook section, not its immutable receipt or object: correctness cases still deliberately overdispatch, but timing uses each kernel's exact production grid. Both refreshed generated sections are filed here and exported to the requested notebook sections directory. The numerical payload is regenerated from the host oracle; the generator now compiles the emitted Python source before writing it. Hardware execution of these sections remains NOT_RUN.

Notebook object keys are the object filenames without `.hsaco`. All norm variants load the symbol `rmsnorm5120`. Kernel ABI and exact arithmetic order are in PROVENANCE §19. No merge, deployment, GPU timing or full-model serving is claimed.
