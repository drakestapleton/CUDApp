# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-moe-diff
.PHONY: test-codegen-moe-diff
CG_MOE_SRCS = src/omega_blackwell_codegen_moe.c src/omega_blackwell_codegen_moe2.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c src/cudapp_om_cg_moe.c src/cudapp_om_cg_moe2.c src/cudapp_om_bw_enc_sm121.c src/omega_gpu_fmt.c src/sha256.c
$(OUT_DIR)/codegen_moe_diff_test: tests/codegen_moe_diff_test.c $(CG_MOE_SRCS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-codegen-moe-diff: $(OUT_DIR)/codegen_moe_diff_test
	./$(OUT_DIR)/codegen_moe_diff_test
