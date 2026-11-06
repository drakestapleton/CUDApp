# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only first-party builder/oracle parity. No driver or device objects.
CARRIED_HOST_TESTS += test-codegen-qmma-diff
.PHONY: test-codegen-qmma-diff
$(OUT_DIR)/codegen_qmma_diff_test: tests/codegen_qmma_diff_test.c src/omega_blackwell_codegen_qmma.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c src/cudapp_om_cg_qmma.c src/cudapp_om_cg_qmma_patch.c src/cudapp_om_bw_enc_sm121.c src/cudapp_om_cubin_elf.c src/omega_cubin.c src/omega_gpu_fmt.c src/sha256.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-codegen-qmma-diff: $(OUT_DIR)/codegen_qmma_diff_test
	./$(OUT_DIR)/codegen_qmma_diff_test
