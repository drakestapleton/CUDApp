# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-host-policy
CARRIED_HOST_TESTS += test-cm-decision-differential
CARRIED_HOST_TESTS += test-cm-source-mutants
.PHONY: test-host-policy
$(OUT_DIR)/host_policy_unit: tests/runtime/host_policy_unit.c src/omega_host_policy.h src/cudapp_native.h src/cudapp_om_api_policy.c src/cudapp_om_artifact_policy.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/host_policy_unit.c src/cudapp_om_api_policy.c src/cudapp_om_artifact_policy.c

$(OUT_DIR)/cm_sort_unit: tests/runtime/cm_sort_unit.c src/runtime/rx_cm_sort.h src/cudapp_native.h src/cudapp_om_cm_sort.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/cm_sort_unit.c src/cudapp_om_cm_sort.c

$(OUT_DIR)/artifact_text_unit: tests/runtime/artifact_text_unit.c src/cudapp_native.h src/cudapp_om_artifact_parse.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/artifact_text_unit.c src/cudapp_om_artifact_parse.c

$(OUT_DIR)/cm_math_unit: tests/runtime/cm_math_unit.c src/cudapp_native.h src/cudapp_om_cm_math.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/cm_math_unit.c src/cudapp_om_cm_math.c -lm

$(OUT_DIR)/cm_blob_unit: tests/runtime/cm_blob_unit.c tests/runtime/cm_oracle.c src/cudapp_native.h $(OUT_DIR)/rx_costmodel.o $(wildcard src/cudapp_om_cm_*.c) src/sha256.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/cm_blob_unit.c tests/runtime/cm_oracle.c $(RX_CM_OBJ) $(RX_CM_BINDING) src/sha256.c -lm

test-host-policy: $(OUT_DIR)/host_policy_unit $(OUT_DIR)/cm_sort_unit $(OUT_DIR)/artifact_text_unit $(OUT_DIR)/cm_math_unit $(OUT_DIR)/cm_blob_unit
	./$(OUT_DIR)/host_policy_unit
	./$(OUT_DIR)/cm_sort_unit
	./$(OUT_DIR)/artifact_text_unit
	./$(OUT_DIR)/cm_math_unit
	./$(OUT_DIR)/cm_blob_unit
	CC="$(CC)" python3 tests/runtime/host_policy_mutants.py

.PHONY: test-cm-decision-differential
$(OUT_DIR)/cm_decision_diff: tests/runtime/rx_costmodel_unit.c tests/runtime/cm_decision_diff.c tests/runtime/cm_oracle.c src/runtime/rx_costmodel.c src/runtime/rx_costmodel.h src/cudapp_native.h $(wildcard src/cudapp_om_cm_*.c) | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/rx_costmodel_unit.c tests/runtime/cm_decision_diff.c tests/runtime/cm_oracle.c src/runtime/rx_costmodel.c $(RX_CM_BINDING) src/sha256.c -Wl,--wrap=rx_cm_decide -Wl,--wrap=rx_cm_predict -lm
test-cm-decision-differential: $(OUT_DIR)/cm_decision_diff
	./$(OUT_DIR)/cm_decision_diff

.PHONY: test-cm-source-mutants
test-cm-source-mutants:
	CC="$(CC)" python3 tests/runtime/cm_source_mutants.py
