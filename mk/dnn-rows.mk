# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-gpu-dnn-rows test-gpu-dnn-rows-mutants
.PHONY: test-gpu-dnn-rows test-gpu-dnn-rows-mutants
DNN_ROWS_WRAP = -Wl,--wrap=omega_gpu_session_open,--wrap=omega_gpu_session_alloc,--wrap=omega_gpu_session_lock,--wrap=omega_gpu_session_unlock,--wrap=omega_gpu_session_free
$(OUT_DIR)/gpu_dnn_rows_test: tests/gpu_dnn_rows_test.c tests/gpu_dnn_rows_ref.h tests/bw_warp_sim_k.h src/omega_gpu_dnn_rows_api.h mk/dnn-rows.mk $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a $(DNN_ROWS_WRAP) -lpthread -lm
test-gpu-dnn-rows: $(OUT_DIR)/gpu_dnn_rows_test
	./$(OUT_DIR)/gpu_dnn_rows_test --sim
	./$(OUT_DIR)/gpu_dnn_rows_test --shell-contract
test-gpu-dnn-rows-mutants: $(OUT_DIR)/gpu_dnn_rows_test
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" OUT_DIR="$(OUT_DIR)" CC="$(CC)" sh tests/gpu_dnn_rows_mutants.sh
