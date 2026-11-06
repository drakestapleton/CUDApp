# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CodegenB host differentials. Registered through CARRIED_HOST_TESTS so the
# Makefile HOST_TESTS list stays shared-lane clean.
CARRIED_HOST_TESTS += test-codegen-pf
.PHONY: test-codegen-pf
$(OUT_DIR)/codegen_pf_diff: tests/codegen_pf_diff.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Itests -Isrc -o $@ tests/codegen_pf_diff.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-codegen-pf: $(OUT_DIR)/codegen_pf_diff
	$(OUT_DIR)/codegen_pf_diff
