# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# API shells and generated units are collected by existing Makefile wildcards.
CARRIED_HOST_TESTS += test-gpu-dnn-activations test-gpu-dnn-activations-mutants
.PHONY: test-gpu-dnn-activations test-gpu-dnn-activations-mutants
DNN_ACTIVATIONS_WRAP = -Wl,--wrap=omega_gpu_session_open -Wl,--wrap=omega_gpu_session_lock -Wl,--wrap=omega_gpu_session_unlock -Wl,--wrap=omega_gpu_session_free -Wl,--wrap=omega_gpu_session_alloc
$(OUT_DIR)/gpu_dnn_activations_test: tests/gpu_dnn_activations_test.c tests/gpu_dnn_activations_ref.h tests/bw_warp_sim_k.h src/omega_gpu_dnn_activations_api.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests $(DNN_ACTIVATIONS_WRAP) -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-gpu-dnn-activations: $(OUT_DIR)/gpu_dnn_activations_test
	./$(OUT_DIR)/gpu_dnn_activations_test --sim
	./$(OUT_DIR)/gpu_dnn_activations_test --shell-contract
test-gpu-dnn-activations-mutants: $(OUT_DIR)/gpu_dnn_activations_test
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" OUT_DIR="$(OUT_DIR)" CC="$(CC)" sh tests/gpu_dnn_activations_mutants.sh

# Build only; deliberately not a CARRIED_HOST_TESTS target and never run by make.
$(OUT_DIR)/gpu_dnn_activations_chip_test: tests/gpu_dnn_activations_test.c tests/gpu_dnn_activations_ref.h tests/bw_warp_sim_k.h src/omega_gpu_dnn_activations_api.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -DDA_CHIP_RUNNER -Itests $(DNN_ACTIVATIONS_WRAP) -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm