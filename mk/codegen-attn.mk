# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-attn
.PHONY: test-codegen-attn
$(OUT_DIR)/codegen_attn_diff: tests/codegen_attn_diff.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Itests -Isrc -o $@ tests/codegen_attn_diff.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-codegen-attn: $(OUT_DIR)/codegen_attn_diff
	$(OUT_DIR)/codegen_attn_diff
