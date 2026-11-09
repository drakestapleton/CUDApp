# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-graph-residue
GRAPH_RESIDUE_DEPS = tests/test_graph_residue.c examples/hello_gpu/hello_gpu.c src/omega_gpu_graph_residue.h src/cudapp_native.h $(OUT_DIR)/libomega_gpu.a
GRAPH_RESIDUE_FLAGS = $(filter-out -MMD -MP,$(CFLAGS)) -ffp-contract=off -Isrc
$(OUT_DIR)/graph-residue: $(GRAPH_RESIDUE_DEPS)
	$(CC) $(GRAPH_RESIDUE_FLAGS) $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm -o $@
$(OUT_DIR)/graph-residue-oracle: $(GRAPH_RESIDUE_DEPS)
	$(CC) $(GRAPH_RESIDUE_FLAGS) -O0 --coverage -DCUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE=1 -DCUDAPP_MIGRATION_PENDING_HELLO_HOST=1 $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm -o $@
$(OUT_DIR)/graph-residue-red: $(GRAPH_RESIDUE_DEPS)
	$(CC) $(GRAPH_RESIDUE_FLAGS) -DCUDAPP_GRAPH_DIFF_RED=1 $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm -o $@
.PHONY: test-graph-residue test-graph-residue-mutants
test-graph-residue: $(OUT_DIR)/graph-residue $(OUT_DIR)/graph-residue-oracle $(OUT_DIR)/graph-residue-red
	$(OUT_DIR)/graph-residue > $(OUT_DIR)/graph-residue.log
	$(OUT_DIR)/graph-residue-oracle > $(OUT_DIR)/graph-residue-oracle.log
	cmp $(OUT_DIR)/graph-residue.log $(OUT_DIR)/graph-residue-oracle.log
	$(OUT_DIR)/graph-residue-red > $(OUT_DIR)/graph-residue-red.log
	@if cmp -s $(OUT_DIR)/graph-residue.log $(OUT_DIR)/graph-residue-red.log; then echo 'FAIL differential red check'; exit 1; fi
	@echo 'PASS graph residue differential and red check'
test-graph-residue-mutants: $(OUT_DIR)/graph-residue $(OUT_DIR)/d8_mutants
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" GRAPH_RESIDUE_FLAGS="$(GRAPH_RESIDUE_FLAGS)" OUT_DIR="$(OUT_DIR)" python3 om/graph/residue-mutants.py
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" $(OUT_DIR)/d8_mutants $(OUT_DIR) $(CC) $(GRAPH_RESIDUE_FLAGS)
$(OUT_DIR)/hello_gpu-oracle: examples/hello_gpu/hello_gpu.c src/cudapp_native.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(GRAPH_RESIDUE_FLAGS) -O0 --coverage -DCUDAPP_MIGRATION_PENDING_HELLO_HOST=1 -DCUDAPP_MIGRATION_PENDING_HELLO_CONTROL=1 $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm -o $@
.PHONY: test-graph-residue-coverage
test-graph-residue-coverage: test-graph-residue $(OUT_DIR)/hello_gpu-oracle
	$(OUT_DIR)/hello_gpu-oracle --sim > $(OUT_DIR)/hello-oracle-sim.log
	@for bug in past-end short; do $(OUT_DIR)/hello_gpu-oracle --sim --bug=$$bug > $(OUT_DIR)/hello-oracle-$$bug.log; rc=$$?; test $$rc = 1 || exit 1; done
	gcov -b -c -o $(OUT_DIR)/graph-residue-oracle-test_graph_residue.gcno tests/test_graph_residue.c
	gcov -b -c -o $(OUT_DIR)/hello_gpu-oracle-hello_gpu.gcno examples/hello_gpu/hello_gpu.c

.PHONY: test-graph-table-perm
CARRIED_HOST_TESTS += test-graph-table-perm
# Both graph mutant runners convert .om through the bridge's seeded converter (pinned AArch64 omc).
CUDAPP_TARGET_ONLY_TESTS += $(if $(CUDAPP_OMC_HOST),,test-graph-table-perm test-graph-headscale)
test-graph-table-perm:
	@$(CUDAPP_REQUIRE_OMC)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/graph/table-mutants.py

.PHONY: test-graph-headscale
CARRIED_HOST_TESTS += test-graph-headscale
test-graph-headscale:
	@$(CUDAPP_REQUIRE_OMC)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/graph/headscale-mutants.py
