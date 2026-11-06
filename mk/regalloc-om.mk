# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CPU-only differential; never enters the GPU execution path.
CARRIED_HOST_TESTS += test-regalloc-om
.PHONY: test-regalloc-om
test-regalloc-om:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE_DIR)" CC="$(CC)" sh tests/regalloc_om/run.sh
