# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-dec-diff test-gdn-admit-diff
.PHONY: test-codegen-dec-diff test-gdn-admit-diff
$(OUT_DIR)/codegen_dec_oracle.o: src/omega_blackwell_codegen_dec.c
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_DEC -Domega_blackwell_codegen_dec_prog=omega_codegen_dec_oracle -Domega_dec_prog_stats=omega_codegen_dec_oracle_stats -c $< -o $@
$(OUT_DIR)/codegen_dec_diff: tests/codegen_dec_diff.c $(OUT_DIR)/codegen_dec_oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Itests -Isrc -o $@ tests/codegen_dec_diff.c $(OUT_DIR)/codegen_dec_oracle.o $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-codegen-dec-diff: $(OUT_DIR)/codegen_dec_diff
	$(OUT_DIR)/codegen_dec_diff
GDN_ADMIT_RENAMES = $(foreach f,shape alloc layer finite multi tokens layout move,-Domega_gdn_admit_$(f)=omega_gdn_admit_$(f)_oracle)
$(OUT_DIR)/gdn_admit_oracle.o: src/omega_gpu_gdn_admit.c
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_GDN_ADMIT $(GDN_ADMIT_RENAMES) -c $< -o $@
$(OUT_DIR)/gdn_admit_diff: tests/gdn_admit_diff.c $(OUT_DIR)/gdn_admit_oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Itests -Isrc -o $@ tests/gdn_admit_diff.c $(OUT_DIR)/gdn_admit_oracle.o $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-gdn-admit-diff: $(OUT_DIR)/gdn_admit_diff
	$(OUT_DIR)/gdn_admit_diff
.PHONY: test-dec-mutants
test-dec-mutants: $(OUT_DIR)/codegen_dec_oracle.o $(OUT_DIR)/gdn_admit_oracle.o $(OUT_DIR)/libomega_gpu.a
	python3 tests/dec_mutants.py $(OUT_DIR)
CARRIED_HOST_TESTS += test-codegen-dec-sim
.PHONY: test-codegen-dec-sim
$(OUT_DIR)/codegen_dec_sim: tests/codegen_dec_sim.c tests/bw_warp_sim_k.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -Isrc -o $@ tests/codegen_dec_sim.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-codegen-dec-sim: $(OUT_DIR)/codegen_dec_sim
	$(OUT_DIR)/codegen_dec_sim
