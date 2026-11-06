# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-sim-predicates
$(OUT_DIR)/gpu_attn_any_predicate: tests/gpu_attn_any_predicate.c tests/bw_warp_sim_k.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm
.PHONY: test-sim-predicates
test-sim-predicates: $(OUT_DIR)/gpu_attn_any_predicate
	$(OUT_DIR)/gpu_attn_any_predicate
