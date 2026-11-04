# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only verification. The --chip switch is never passed by make test.
.PHONY: test-q38-head-bounds
ifeq ($(CUDAPP_OMC_HOST),1)
CARRIED_HOST_TESTS += test-q38-head-bounds
test-q38-head-bounds: $(OUT_DIR)/libomega_gpu.a
	OMEGA_OMC="$(OMEGA_OMC)" Q38_GPU_OUT="$(abspath $(OUT_DIR))/q38-head" \
	  Q38_GPU_LIB="$(OUT_DIR)/libomega_gpu.a" CC="$(CC)" sh om/q38-head/run.sh
else
test-q38-head-bounds:
	@echo "test-q38-head-bounds: NOT_RUN (requires a runnable pinned AArch64 omc)"
	@exit 2
endif
