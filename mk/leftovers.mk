# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
LEFTOVERS_DIR = $(OUT_DIR)/leftovers
LEFTOVERS_DEPS = tests/leftovers_test.c src/omega_path_check.h src/omega_numeric_lifecycle.h src/omega_gpu_q36_native.h src/cudapp_native.h src/cudapp_freest_leftovers.c src/cudapp_freest_pathcopy.c
.PHONY: test-leftovers test-leftovers-mutants test-leftovers-coverage
$(LEFTOVERS_DIR)/diff: $(LEFTOVERS_DEPS)
	@mkdir -p $(LEFTOVERS_DIR)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/leftovers_test.c src/cudapp_freest_leftovers.c src/cudapp_freest_pathcopy.c
$(LEFTOVERS_DIR)/red: $(LEFTOVERS_DEPS)
	@mkdir -p $(LEFTOVERS_DIR)
	$(CC) $(CFLAGS) -Isrc -DLEFTOVERS_RED -o $@ tests/leftovers_test.c src/cudapp_freest_leftovers.c src/cudapp_freest_pathcopy.c
test-leftovers: $(LEFTOVERS_DIR)/diff $(LEFTOVERS_DIR)/red
	$(LEFTOVERS_DIR)/diff
	@if $(LEFTOVERS_DIR)/red > $(LEFTOVERS_DIR)/red.log 2>&1; then echo 'FAIL red differential survived'; exit 1; else echo 'LEFTOVERS RED PASS: deliberately unequal result refused'; fi
test-leftovers-mutants:
	python3 tests/leftovers_mutants.py $(LEFTOVERS_DIR)/mutants
	python3 tests/pathcopy_mutants.py $(LEFTOVERS_DIR)/pathcopy-mutants
test-leftovers-coverage:
	@mkdir -p $(LEFTOVERS_DIR)/coverage
	rm -f $(LEFTOVERS_DIR)/coverage/*.gcda
	$(CC) $(CFLAGS) -O0 --coverage -DLEFTOVERS_ORACLE -Isrc -o $(LEFTOVERS_DIR)/coverage/diff tests/leftovers_test.c src/cudapp_freest_leftovers.c src/cudapp_freest_pathcopy.c
	$(LEFTOVERS_DIR)/coverage/diff
	gcov -b -c -t $(LEFTOVERS_DIR)/coverage/diff-leftovers_test.gcno
