# BF16 causal convolution receipt

IMPLEMENTED and EXERCISED on the host. Hardware and notebook execution NOT_RUN. PROVENANCE §20 defines the ABI, exact order, cache pointer swap and all dtype boundaries with exact Transformers source lines.

The first host/executor acceptance attempt passed. No Omega compiler error occurred. Every output is checked against the manual-derived arithmetic enclosure and BF16 lattice; every new-state halfword and every input/guard byte is checked. Each omitted rounding boundary and the incorrect cache shift are genuine failing-object controls. The complete gate retains every older case, checks LLVM bytes/metadata/descriptor equality, byte-identical builds and all-object scheduling.

The generated gdnconv4.py section and compressed oracle payload are included and exported to the requested notebook sections directory. Object keys are filenames without .hsaco; every control uses symbol gdnconv4. Timing uses the exact160×1 workgroup grid, block64, immutable old state and separate new state. Its cold rotation checks every timed output/state/guard copy. The separately labeled torch reference is an AMD library measurement, not an Omega kernel.

Exact commands, exits, full logs, controls, source digests and object bytes are filed alongside. No merge/deployment/full-model serving is claimed.
