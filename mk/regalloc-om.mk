# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CPU-only differential; never enters the GPU execution path.
CARRIED_HOST_TESTS += test-regalloc-om
.PHONY: test-regalloc-om
# The units check executes the pinned AArch64 omc; off a CUDAPP_OMC_HOST run.sh prints NOT_RUN for it.
test-regalloc-om:
	OMEGA_OMC="$(if $(CUDAPP_OMC_HOST),$(OMEGA_OMC))" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" sh tests/regalloc_om/run.sh
