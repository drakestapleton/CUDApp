# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only: no driver or device objects.
CARRIED_HOST_TESTS += test-codegen-tern-diff
.PHONY: test-codegen-tern-diff
$(OUT_DIR)/codegen_tern_diff_test: tests/codegen_tern_diff_test.c src/omega_blackwell_codegen_tern.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c src/cudapp_om_cg_tern.c src/cudapp_om_cg_tern_sched.c src/cudapp_om_cg_tern_policy.c src/cudapp_om_bw_enc_sm121.c src/omega_gpu_fmt.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-codegen-tern-diff: $(OUT_DIR)/codegen_tern_diff_test
	./$(OUT_DIR)/codegen_tern_diff_test
