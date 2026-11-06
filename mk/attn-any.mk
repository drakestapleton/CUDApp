# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-gpu-attn-any test-gpu-attn-any-mutants
.PHONY: test-gpu-attn-any test-gpu-attn-any-mutants
ATTN_ANY_HOST_WRAP := -Wl,--wrap=omega_gpu_session_open,--wrap=omega_gpu_session_lock,--wrap=omega_gpu_session_unlock,--wrap=omega_gpu_session_free,--wrap=omega_gpu_session_alloc
$(OUT_DIR)/gpu_attn_any_test: tests/gpu_attn_any_test.c tests/gpu_attn_any_ref.h tests/bw_warp_sim_k.h src/omega_gpu_attn_any_api.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm $(ATTN_ANY_HOST_WRAP)
test-gpu-attn-any: $(OUT_DIR)/gpu_attn_any_test
	./$(OUT_DIR)/gpu_attn_any_test --sim
	./$(OUT_DIR)/gpu_attn_any_test --shell-contract
test-gpu-attn-any-mutants: $(OUT_DIR)/gpu_attn_any_test
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" OUT_DIR="$(OUT_DIR)" CC="$(CC)" sh tests/gpu_attn_any_mutants.sh
