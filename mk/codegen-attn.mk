# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-attn
.PHONY: test-codegen-attn
$(OUT_DIR)/codegen_attn_oracle.o: src/omega_blackwell_codegen_attn.c
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_ATTN -Domega_blackwell_codegen_decode_attn_prog=omega_codegen_attn_oracle -Domega_decode_attn_prog_stats=omega_codegen_attn_oracle_stats -c $< -o $@
$(OUT_DIR)/codegen_pattn_oracle.o: src/omega_blackwell_codegen_pattn.c
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_PATTN -Domega_blackwell_codegen_prefill_attn_prog=omega_codegen_pattn_oracle -Domega_prefill_attn_prog_stats=omega_codegen_pattn_oracle_stats -Domega_pattn_spec_ok=omega_codegen_pattn_oracle_spec_ok -c $< -o $@
$(OUT_DIR)/codegen_attn_diff: tests/codegen_attn_diff.c $(OUT_DIR)/codegen_attn_oracle.o $(OUT_DIR)/codegen_pattn_oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Itests -Isrc -o $@ tests/codegen_attn_diff.c $(OUT_DIR)/codegen_attn_oracle.o $(OUT_DIR)/codegen_pattn_oracle.o $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-codegen-attn: $(OUT_DIR)/codegen_attn_diff
	$(OUT_DIR)/codegen_attn_diff
