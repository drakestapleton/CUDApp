# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
.PHONY: test-gpu-conv2d test-gpu-conv2d-mutants test-gpu-conv2d-gemm-f32
CONV2D_HOST_WRAP = -Wl,--wrap=omega_gpu_session_open,--wrap=omega_gpu_session_lock,--wrap=omega_gpu_session_unlock,--wrap=omega_gpu_session_free,--wrap=omega_gpu_session_alloc
$(OUT_DIR)/gpu_conv2d_test: tests/gpu_conv2d_test.c tests/gpu_conv2d_ref.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests $(CONV2D_HOST_WRAP) -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-gpu-conv2d: $(OUT_DIR)/gpu_conv2d_test
	$< --sim
	$< --shell-contract
test-gpu-conv2d-mutants: $(OUT_DIR)/gpu_conv2d_test
	sh tests/gpu_conv2d_mutants.sh
$(OUT_DIR)/gpu_conv2d_oracle_test: tests/gpu_conv2d_test.c tests/gpu_conv2d_ref.h src/omega_gpu_gemm.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests $(CONV2D_HOST_WRAP) -UCUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER -DCUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER=1 -o $@ $< src/omega_gpu_gemm.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-gpu-conv2d-gemm-f32: $(OUT_DIR)/gpu_conv2d_test $(OUT_DIR)/gpu_conv2d_oracle_test
	$(OUT_DIR)/gpu_conv2d_test --gemm-f32 > $(OUT_DIR)/conv2d-dense-seeded.log
	$(OUT_DIR)/gpu_conv2d_oracle_test --gemm-f32 > $(OUT_DIR)/conv2d-dense-oracle.log
	cmp $(OUT_DIR)/conv2d-dense-seeded.log $(OUT_DIR)/conv2d-dense-oracle.log
	cat $(OUT_DIR)/conv2d-dense-seeded.log
CARRIED_HOST_TESTS += test-gpu-conv2d test-gpu-conv2d-mutants test-gpu-conv2d-gemm-f32

# Build-only operator executable. Never part of carried/headless targets.
$(OUT_DIR)/gpu_conv2d_chip_test: tests/gpu_conv2d_test.c tests/gpu_conv2d_ref.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -DCONV2D_OPERATOR_CHIP=1 -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm
