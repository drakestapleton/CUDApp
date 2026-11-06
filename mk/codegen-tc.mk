# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-tc test-codegen-tc-mutants test-codegen-tc-sim
CUDAPP_MIGRATION_PENDING_C_CODEGEN_TC ?= 0
CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC=$(CUDAPP_MIGRATION_PENDING_C_CODEGEN_TC)
TC_ORACLE_NAMES = omega_blackwell_codegen_tc_probe_prog omega_blackwell_codegen_tcgemm_prog omega_blackwell_codegen_tc_peak_prog omega_blackwell_codegen_tc_bw_prog omega_tcgemm_spec_ok omega_tcgemm_shared_bytes omega_tcgemm_span omega_tcgemm_permute_x omega_tcgemm_w_bf16
TC2_ORACLE_NAMES = omega_blackwell_codegen_tc2_prog omega_tc2_spec_ok omega_tc2_shared_bytes omega_tc2_span
$(OUT_DIR)/codegen_tc_oracle.o: src/omega_blackwell_codegen_tc.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC=1 -UCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC_HOST -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC_HOST=1 $(foreach n,$(TC_ORACLE_NAMES),-D$(n)=oracle_$(n)) -c $< -o $@
$(OUT_DIR)/codegen_tc2_oracle.o: src/omega_blackwell_codegen_tc2.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC=1 -UCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC_HOST -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC_HOST=1 $(foreach n,$(TC2_ORACLE_NAMES),-D$(n)=oracle_$(n)) -c $< -o $@
$(OUT_DIR)/codegen_tc_diff: tests/codegen_tc_diff.c $(OUT_DIR)/codegen_tc_oracle.o $(OUT_DIR)/codegen_tc2_oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lm
.PHONY: test-codegen-tc
test-codegen-tc: $(OUT_DIR)/codegen_tc_diff
	./$(OUT_DIR)/codegen_tc_diff

.PHONY: test-codegen-tc-mutants
test-codegen-tc-mutants: $(OUT_DIR)/codegen_tc_diff
	OUT_DIR="$(OUT_DIR)" CC="$(CC)" python3 tests/codegen_tc_mutants.py

$(OUT_DIR)/codegen_tc_sim: tests/codegen_tc_sim.c tests/bw_warp_sim_tc.h tests/bw_hmma_model.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -DWS_MAX_THREADS=64 -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm

.PHONY: test-codegen-tc-sim
test-codegen-tc-sim: $(OUT_DIR)/codegen_tc_sim $(OUT_DIR)/gpu_tc_peak_test $(OUT_DIR)/gpu_tcgemm_test $(OUT_DIR)/gpu_pfgemm_test
	./$(OUT_DIR)/codegen_tc_sim
	./$(OUT_DIR)/gpu_tc_peak_test --host-only
	./$(OUT_DIR)/gpu_tcgemm_test --sim
	./$(OUT_DIR)/gpu_pfgemm_test --sim

CUDAPP_MIGRATION_PENDING_C_CODEGEN_TC_HOST ?= 0
CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_TC_HOST=$(CUDAPP_MIGRATION_PENDING_C_CODEGEN_TC_HOST)
CARRIED_HOST_TESTS += test-codegen-tc-host
$(OUT_DIR)/codegen_tc_host_diff: tests/codegen_tc_host_diff.c $(OUT_DIR)/codegen_tc_oracle.o $(OUT_DIR)/codegen_tc2_oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -o $@ $^ -lpthread -lm
.PHONY: test-codegen-tc-host
test-codegen-tc-host: $(OUT_DIR)/codegen_tc_host_diff
	./$(OUT_DIR)/codegen_tc_host_diff
	@if ./$(OUT_DIR)/codegen_tc_host_diff --red; then exit 1; else echo 'TC-HOST-RED PASS'; fi

$(OUT_DIR)/codegen_tc_host_cubin: tests/codegen_tc_host_cubin.c $(OUT_DIR)/codegen_tc_oracle.o $(OUT_DIR)/codegen_tc2_oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lm
.PHONY: test-codegen-tc-host-cubin test-codegen-tc-host-mutants
test-codegen-tc-host-cubin: $(OUT_DIR)/codegen_tc_host_cubin
	./$(OUT_DIR)/codegen_tc_host_cubin
test-codegen-tc-host-mutants: $(OUT_DIR)/codegen_tc_host_diff
	OUT_DIR="$(OUT_DIR)" python3 tests/codegen_tc_host_mutants.py
CARRIED_HOST_TESTS += test-codegen-tc-host-cubin test-codegen-tc-host-mutants
