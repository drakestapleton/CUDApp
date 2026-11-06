# Explicit compiler integration seam

At NvrmAllocOm's request, each gcc invocation in tests/run_m16_requalification.sh and tests/run_nvrm_lifecycle_gates.sh now forwards `CUDAPP_MIGRATION_PENDING_C_NVRM_ALLOC`, defaulting to zero. This selects that lane's retained allocator/packet/header oracle when explicitly set to one, without changing the default path. It is an integration selector, not an additional implementation of allocator policy.

The hardware scripts were NOT_RUN: no GPU window is authorized. The previously recorded host and simulated-family results precede this compile-argument-only addition. The integrator owns verification of the combined allocator lane and these explicit build scripts. Their chip execution remains NOT_RUN.
