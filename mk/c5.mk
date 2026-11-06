# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# C5/D14 pure policy, fake RM/channel simulator, and production scalar binding.
CUDAPP_FS_BINDING += src/cudapp_freest_c5.c
CARRIED_HOST_TESTS += test-c5-policy
CUDAPP_TARGET_ONLY_TESTS += $(if $(CUDAPP_OMC_HOST),,test-c5-policy)

.PHONY: test-c5-policy
test-c5-policy:
	OMEGA_OMC="$(OMEGA_OMC)" OUT_DIR="$(OUT_DIR)" sh tools/c5/policy-test.sh
