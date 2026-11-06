# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-moe-diff test-codegen-moe-sim
.PHONY: test-codegen-moe-diff test-codegen-moe-sim
CG_MOE_SRC ?= src/cudapp_om_cg_moe.c
CG_MOE2_SRC ?= src/cudapp_om_cg_moe2.c
CG_MOE_SCHED_SRC ?= src/cudapp_om_cg_moe_sched.c
CG_MOE_SRCS = src/omega_blackwell_codegen_moe.c src/omega_blackwell_codegen_moe2.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c $(CG_MOE_SRC) $(CG_MOE2_SRC) $(CG_MOE_SCHED_SRC) src/cudapp_om_bw_enc_sm121.c src/omega_gpu_fmt.c src/sha256.c tests/codegen_moe_oracle.c tests/codegen_moe2_oracle.c
CG_MOE_SRCS += src/omega_cubin.c src/cudapp_om_cubin_elf.c src/cudapp_om_core_cubinmsg.c $(BW_CODEGEN_BINDING)
$(OUT_DIR)/codegen_moe_diff_test: tests/codegen_moe_diff_test.c $(CG_MOE_SRCS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-codegen-moe-diff: $(OUT_DIR)/codegen_moe_diff_test
	./$(OUT_DIR)/codegen_moe_diff_test
$(OUT_DIR)/codegen_moe_sim_test: tests/codegen_moe_sim_test.c $(CG_MOE_SRCS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -Itests -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-codegen-moe-sim: $(OUT_DIR)/codegen_moe_sim_test
	./$(OUT_DIR)/codegen_moe_sim_test
