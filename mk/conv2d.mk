# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
.PHONY: test-gpu-conv2d test-gpu-conv2d-mutants test-gpu-conv2d-gemm-f32
$(OUT_DIR)/gpu_conv2d_test: tests/gpu_conv2d_test.c tests/gpu_conv2d_ref.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-gpu-conv2d: $(OUT_DIR)/gpu_conv2d_test
	$< --sim
test-gpu-conv2d-mutants: $(OUT_DIR)/gpu_conv2d_test
	sh tests/gpu_conv2d_mutants.sh
test-gpu-conv2d-gemm-f32: $(OUT_DIR)/gpu_conv2d_test
	$< --gemm-f32
CARRIED_HOST_TESTS += test-gpu-conv2d test-gpu-conv2d-mutants test-gpu-conv2d-gemm-f32
