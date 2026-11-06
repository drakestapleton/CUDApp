# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-model-graphs-om
.PHONY: test-model-graphs-om
$(OUT_DIR)/model-graphs-check: tests/test_model_graphs_om.c src/cudapp_om_model_graph.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(filter-out -MMD -MP,$(CFLAGS)) -Isrc $^ -o $@
test-model-graphs-om: $(OUT_DIR)/model-graphs-check
	$(OUT_DIR)/model-graphs-check
ifeq ($(CUDAPP_OMC_HOST),1)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/graph/mutants.py
endif
