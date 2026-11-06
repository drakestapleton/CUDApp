# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only: no driver or device objects.
CARRIED_HOST_TESTS += test-codegen-tern-diff
.PHONY: test-codegen-tern-diff
CG_TERN_SRC ?= src/cudapp_om_cg_tern.c
CG_TERN_SCHED_SRC ?= src/cudapp_om_cg_tern_sched.c
CG_TERN_HOST_SRCS = src/omega_blackwell_codegen_tern.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c $(CG_TERN_SRC) $(CG_TERN_SCHED_SRC) src/cudapp_om_cg_tern_policy.c src/cudapp_om_bw_enc_sm121.c src/omega_gpu_fmt.c src/sha256.c $(BW_CODEGEN_BINDING) src/cudapp_om_apish_shp.c src/cudapp_om_ar_tern.c src/cudapp_om_ar_segb.c
$(OUT_DIR)/codegen_tern_diff_test: tests/codegen_tern_diff_test.c $(CG_TERN_HOST_SRCS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-codegen-tern-diff: $(OUT_DIR)/codegen_tern_diff_test
	./$(OUT_DIR)/codegen_tern_diff_test

# Optional existing simulator/algebra oracle; never part of the production build.
.PHONY: test-codegen-tern-sim test-codegen-tern-mutants
test-codegen-tern-sim: | $(OUT_DIR)
	test -n "$(OMEGA_MV)"
	test "$$(git -C "$(OMEGA_MV)" rev-parse HEAD)" = e5b32567a7aff3ce594e3254a622a846a6cd190b
	git -C "$(OMEGA_MV)" diff --quiet HEAD -- tests/gpu_tern_test.c src/algebra/realize_common.c src/algebra/realize_common.h
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -ffp-contract=off -I"$(OMEGA_MV)/src" -DOMEGA_TERN_REFERENCE='"$(OMEGA_MV)/tests/gpu_tern_test.c"' -o $(OUT_DIR)/codegen_tern_sim_test tests/codegen_tern_sim_test.c $(OMEGA_MV)/src/algebra/realize_common.c src/omega_gpu_tern_api.c $(CG_TERN_HOST_SRCS) -lpthread -lm
	./$(OUT_DIR)/codegen_tern_sim_test
test-codegen-tern-mutants: $(OUT_DIR)/codegen_tern_diff_test
	python3 om/codegen/tern_mutants.py
