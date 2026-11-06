# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CPU-only; invoke every target under the lane's bwrap device isolation.
API_KGEN_DIR ?= build/api-kgen
API_KGEN_CORE = omega_blackwell_codegen omega_gpu_om_ir omega_cubin sha256
API_KGEN_OBJS = $(addprefix $(API_KGEN_DIR)/,$(addsuffix .o,$(API_KGEN_CORE))) \
 $(patsubst src/%.c,$(API_KGEN_DIR)/%.o,$(CUDAPP_BINDING_SRCS))
API_KGEN_CC = $(CC) $(CFLAGS) -ffunction-sections -fdata-sections

include tests/attention_kgen.mk

$(API_KGEN_DIR)/%.o: src/%.c src/cudapp_native.h
	@mkdir -p $(API_KGEN_DIR)
	$(API_KGEN_CC) -c $< -o $@
$(API_KGEN_DIR)/ew_diff.o: tests/api_kgen_ew_diff_test.c src/omega_gpu_elementwise_api.c
	@mkdir -p $(API_KGEN_DIR)
	$(API_KGEN_CC) -c $< -o $@
$(API_KGEN_DIR)/ew_red.o: tests/api_kgen_ew_diff_test.c src/omega_gpu_elementwise_api.c
	@mkdir -p $(API_KGEN_DIR)
	$(API_KGEN_CC) -DCUDAPP_API_KGEN_RED -c $< -o $@
$(API_KGEN_DIR)/ew_diff: $(API_KGEN_DIR)/ew_diff.o $(API_KGEN_OBJS)
	$(CC) -Wl,--gc-sections -o $@ $^ -lpthread -lm
$(API_KGEN_DIR)/ew_red: $(API_KGEN_DIR)/ew_red.o $(API_KGEN_OBJS)
	$(CC) -Wl,--gc-sections -o $@ $^ -lpthread -lm
.PHONY: test-api-kgen-ew test-api-kgen-ew-mutants
test-api-kgen-ew: $(API_KGEN_DIR)/ew_diff $(API_KGEN_DIR)/ew_red
	$(API_KGEN_DIR)/ew_diff
	@set +e; $(API_KGEN_DIR)/ew_red; rc=$$?; set -e; test "$$rc" = 1 && echo 'PASS API EW red control detected an altered IR word'

test-api-kgen-ew-mutants: $(API_KGEN_DIR)/ew_diff
	python3 tests/api_kgen_ew_mutants.py $(API_KGEN_DIR)
