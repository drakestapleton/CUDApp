# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Included after mk/api-kgen.mk; all executions require the documented bwrap wrapper.
ifndef CUDAPP_ATTN_KGEN_INCLUDED
CUDAPP_ATTN_KGEN_INCLUDED := 1
ATTN_KGEN_UNIT ?= src/cudapp_freest_attention_ir.c
$(API_KGEN_DIR)/attention_unit.o: $(ATTN_KGEN_UNIT)
	@mkdir -p $(API_KGEN_DIR)
	$(API_KGEN_CC) -c $< -o $@
$(API_KGEN_DIR)/attention_diff.o: tests/api_kernel_gen_attention.c src/omega_gpu_attention_api.c tests/attention_kgen.mk
	@mkdir -p $(API_KGEN_DIR)
	@rm -f $(API_KGEN_DIR)/attention_diff.gcda
	$(API_KGEN_CC) -O0 --coverage -ffp-contract=off -c $< -o $@
# Simply expanded: the same list the attention_diff rule's prerequisites expand to here, before later
# includes append to CUDAPP_FS_BINDING.
ATTN_KGEN_LINK_OBJS := $(API_KGEN_DIR)/attention_diff.o $(filter-out $(API_KGEN_DIR)/cudapp_freest_attention_ir.o,$(API_KGEN_OBJS))
$(API_KGEN_DIR)/attention_diff: $(ATTN_KGEN_LINK_OBJS) $(API_KGEN_DIR)/attention_unit.o
	$(CC) --coverage -Wl,--gc-sections -o $@ $^ -lpthread -lm
.PHONY: test-api-kgen-attention test-api-kgen-attention-mutants
test-api-kgen-attention: $(API_KGEN_DIR)/attention_diff
	$(API_KGEN_DIR)/attention_diff
	@set +e; $(API_KGEN_DIR)/attention_diff --perturb; rc=$$?; set -e; test "$$rc" = 1 && echo 'PASS attention red control detected perturbed IR record'
	@set +e; $(API_KGEN_DIR)/attention_diff --cubin-red; rc=$$?; set -e; test "$$rc" = 1 && echo 'PASS attention red control detected perturbed final cubin'
test-api-kgen-attention-mutants: $(API_KGEN_DIR)/attention_diff
	python3 tests/attention_kgen_mutants.py $(API_KGEN_DIR) $(ATTN_KGEN_LINK_OBJS)
endif
