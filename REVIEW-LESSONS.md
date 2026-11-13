# Review lessons

Before requesting review of the native AMD model path, run the launcher gate on
the complete release tree. Its Qwen object manifest must rebuild byte-identically
to the objects exercised on the card. The notebook once carried device-stepped
kernels that were missing from the proposed release tree; smoke tests alone did
not detect that gap. The enforced source-manifest check catches missing kernels,
changed object bytes and changed kernarg sizes before review. Update its fixture
only with a new corresponding hardware receipt.
