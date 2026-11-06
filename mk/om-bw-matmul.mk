# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
BW_DIFF_SRCS = tests/om_bw_diff_test.c tests/om_bw_matmul_oracle.c tests/om_bw_realize_oracle.c
BW_DIFF_FLAGS = -std=gnu11 -O0 -g -Wall -Wextra -Werror -ffp-contract=off -Isrc -DCUDAPP_MIGRATION_PENDING_BW_MATMUL=1
BW_DIFF_DEPS = $(BW_DIFF_SRCS) src/omega_bw_policy.h src/omega_blackwell_matmul.h $(OUT_DIR)/libomega_gpu.a
.PHONY: test-om-bw-matmul test-om-bw-matmul-mutants test-om-bw-matmul-coverage
$(OUT_DIR)/om_bw_diff: $(BW_DIFF_DEPS)
	$(CC) $(BW_DIFF_FLAGS) -o $@ $(BW_DIFF_SRCS) $(OUT_DIR)/libomega_gpu.a -lpthread -lm
$(OUT_DIR)/om_bw_diff_red: $(BW_DIFF_DEPS)
	$(CC) $(BW_DIFF_FLAGS) -DOMEGA_BW_DIFF_RED=1 -o $@ $(BW_DIFF_SRCS) $(OUT_DIR)/libomega_gpu.a -lpthread -lm
test-om-bw-matmul: $(OUT_DIR)/om_bw_diff $(OUT_DIR)/om_bw_diff_red
	./$(OUT_DIR)/om_bw_diff
	@rc=0; ./$(OUT_DIR)/om_bw_diff_red >$(OUT_DIR)/om_bw_diff_red.log 2>&1 || rc=$$?; test $$rc = 1
	@echo 'PASS bw differential red check killed'
test-om-bw-matmul-mutants: $(OUT_DIR)/libomega_gpu.a
	python3 tests/om_bw_mutants.py
test-om-bw-matmul-coverage: $(OUT_DIR)/libomega_gpu.a
	@mkdir -p $(OUT_DIR)/om-bw-coverage
	rm -f $(OUT_DIR)/om-bw-coverage/diff-*.gcda
	$(CC) $(BW_DIFF_FLAGS) --coverage -o $(OUT_DIR)/om-bw-coverage/diff $(foreach s,$(BW_DIFF_SRCS),$(abspath $(s))) $(OUT_DIR)/libomega_gpu.a -lpthread -lm
	./$(OUT_DIR)/om-bw-coverage/diff
	cd $(OUT_DIR)/om-bw-coverage && gcov -b -c diff-om_bw_matmul_oracle.gcno diff-om_bw_realize_oracle.gcno
