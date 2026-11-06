# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only: no driver or device objects.
CARRIED_HOST_TESTS += test-codegen-gdn-diff
.PHONY: test-codegen-gdn-diff test-codegen-gdn-mutants test-codegen-gdn-sim
CG_GDN_SRC ?= src/cudapp_om_cg_gdn.c
CG_GDN_POLICY_SRC ?= src/cudapp_om_cg_gdn_policy.c
CG_GDN_HOST_SRCS = src/omega_blackwell_codegen_gdn.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c $(CG_GDN_SRC) $(CG_GDN_POLICY_SRC) src/cudapp_om_bw_enc_sm121.c src/omega_gpu_fmt.c src/sha256.c $(BW_CODEGEN_BINDING)
$(OUT_DIR)/codegen_gdn_diff_test: tests/codegen_gdn_diff_test.c $(CG_GDN_HOST_SRCS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-codegen-gdn-diff: $(OUT_DIR)/codegen_gdn_diff_test
	./$(OUT_DIR)/codegen_gdn_diff_test
test-codegen-gdn-mutants: $(OUT_DIR)/codegen_gdn_diff_test
	python3 om/codegen/gdn_mutants.py
# The existing warp-simulator batteries, linked against the default (Omega) builders.
test-codegen-gdn-sim: $(GPU_GDN_TEST) $(GPU_GDN_MULTI_TEST)
	./$(GPU_GDN_TEST) --host-only
	./$(GPU_GDN_TEST) --sim
	./$(GPU_GDN_MULTI_TEST) --sim
