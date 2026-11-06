# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-om-ir-boundary

.PHONY: test-om-ir-boundary
$(OUT_DIR)/om_ir_boundary_test: tests/gpu_om_ir_boundary_test.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Isrc -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm

test-om-ir-boundary: $(OUT_DIR)/om_ir_boundary_test
	$(OUT_DIR)/om_ir_boundary_test
	CC='$(CC)' OMEGA_TEST_LIB='$(OUT_DIR)/libomega_gpu.a' python3 tests/gpu_om_ir_boundary_mutant.py
