# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-gpu-dnn-rows test-gpu-dnn-rows-mutants
.PHONY: test-gpu-dnn-rows test-gpu-dnn-rows-mutants
$(OUT_DIR)/gpu_dnn_rows_test: tests/gpu_dnn_rows_test.c tests/gpu_dnn_rows_ref.h tests/bw_warp_sim_k.h src/omega_gpu_dnn_rows_api.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-gpu-dnn-rows: $(OUT_DIR)/gpu_dnn_rows_test
	./$(OUT_DIR)/gpu_dnn_rows_test --sim
test-gpu-dnn-rows-mutants: $(OUT_DIR)/gpu_dnn_rows_test
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" OUT_DIR="$(OUT_DIR)" CC="$(CC)" sh tests/gpu_dnn_rows_mutants.sh
