# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-pfhost
.PHONY: test-codegen-pfhost
PFHOST_ORACLE_RENAMES = -Domega_rl_spec_ok=omega_rl_spec_ok_c -Domega_rl_shared_bytes=omega_rl_shared_bytes_c -Domega_mc_spec_ok=omega_mc_spec_ok_c -Domega_blackwell_codegen_rl_prog=omega_blackwell_codegen_rl_prog_c -Domega_blackwell_codegen_mc_prog=omega_blackwell_codegen_mc_prog_c
$(OUT_DIR)/codegen_pfhost_oracle.o: src/omega_blackwell_codegen_pfhost.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_PFHOST=1 $(PFHOST_ORACLE_RENAMES) -c $< -o $@
$(OUT_DIR)/codegen_pfhost_diff: tests/codegen_pfhost_diff.c $(OUT_DIR)/codegen_pfhost_oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Itests -Isrc -o $@ $^ -lpthread -lm
test-codegen-pfhost: $(OUT_DIR)/codegen_pfhost_diff
	$(OUT_DIR)/codegen_pfhost_diff
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" python3 tests/codegen_pfhost_mutants.py $(OUT_DIR)