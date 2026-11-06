# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Included after mk/api-kgen.mk; all executions require the documented bwrap wrapper.
ATTN_KGEN_UNIT ?= src/cudapp_om_attention_ir.c
$(API_KGEN_DIR)/attention_unit.o: $(ATTN_KGEN_UNIT)
	@mkdir -p $(API_KGEN_DIR)
	$(API_KGEN_CC) -c $< -o $@
$(API_KGEN_DIR)/attention_diff.o: tests/api_kernel_gen_attention.c src/omega_gpu_attention_api.c
	@mkdir -p $(API_KGEN_DIR)
	$(API_KGEN_CC) -O0 --coverage -ffp-contract=off -c $< -o $@
$(API_KGEN_DIR)/attention_diff: $(API_KGEN_DIR)/attention_diff.o $(filter-out $(API_KGEN_DIR)/cudapp_om_attention_ir.o,$(API_KGEN_OBJS)) $(API_KGEN_DIR)/attention_unit.o
	$(CC) --coverage -Wl,--gc-sections -o $@ $^ -lpthread -lm
.PHONY: test-api-kgen-attention test-api-kgen-attention-mutants
test-api-kgen-attention: $(API_KGEN_DIR)/attention_diff
	$(API_KGEN_DIR)/attention_diff
	@set +e; $(API_KGEN_DIR)/attention_diff --perturb; rc=$$?; set -e; test "$$rc" = 1 && echo 'PASS attention red control detected perturbed IR record'
test-api-kgen-attention-mutants: $(API_KGEN_DIR)/attention_diff
	python3 tests/attention_kgen_mutants.py $(API_KGEN_DIR)
