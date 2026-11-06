# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-tc
CUDAPP_MIGRATION_PENDING_C_CODEGEN_TC ?= 0
CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC=$(CUDAPP_MIGRATION_PENDING_C_CODEGEN_TC)
TC_ORACLE_NAMES = omega_blackwell_codegen_tc_probe_prog omega_blackwell_codegen_tcgemm_prog omega_blackwell_codegen_tc_peak_prog omega_blackwell_codegen_tc_bw_prog omega_tcgemm_spec_ok omega_tcgemm_shared_bytes omega_tcgemm_span omega_tcgemm_permute_x omega_tcgemm_w_bf16
TC2_ORACLE_NAMES = omega_blackwell_codegen_tc2_prog omega_tc2_spec_ok omega_tc2_shared_bytes omega_tc2_span
$(OUT_DIR)/codegen_tc_oracle.o: src/omega_blackwell_codegen_tc.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC=1 $(foreach n,$(TC_ORACLE_NAMES),-D$(n)=oracle_$(n)) -c $< -o $@
$(OUT_DIR)/codegen_tc2_oracle.o: src/omega_blackwell_codegen_tc2.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC=1 $(foreach n,$(TC2_ORACLE_NAMES),-D$(n)=oracle_$(n)) -c $< -o $@
$(OUT_DIR)/codegen_tc_diff: tests/codegen_tc_diff.c $(OUT_DIR)/codegen_tc_oracle.o $(OUT_DIR)/codegen_tc2_oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lm
.PHONY: test-codegen-tc
test-codegen-tc: $(OUT_DIR)/codegen_tc_diff
	./$(OUT_DIR)/codegen_tc_diff
