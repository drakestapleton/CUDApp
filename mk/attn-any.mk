# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-gpu-attn-any test-gpu-attn-any-mutants
.PHONY: test-gpu-attn-any test-gpu-attn-any-mutants
$(OUT_DIR)/gpu_attn_any_test: tests/gpu_attn_any_test.c tests/gpu_attn_any_ref.h tests/bw_warp_sim_k.h src/omega_gpu_attn_any_api.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-gpu-attn-any: $(OUT_DIR)/gpu_attn_any_test
	./$(OUT_DIR)/gpu_attn_any_test --sim
test-gpu-attn-any-mutants: $(OUT_DIR)/gpu_attn_any_test
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" OUT_DIR="$(OUT_DIR)" CC="$(CC)" sh tests/gpu_attn_any_mutants.sh
