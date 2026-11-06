# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Scalar f16 conversion and guarded encoding verification; no device is opened.
CARRIED_HOST_TESTS += test-dnn-half

.PHONY: test-dnn-half
$(OUT_DIR)/dnn_half_test: tests/gpu_dnn_half_test.c tests/bw_warp_sim_k.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Isrc -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm

test-dnn-half: $(OUT_DIR)/dnn_half_test
	$(OUT_DIR)/dnn_half_test
