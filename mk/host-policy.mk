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
	$(CC) $(CFLAGS) -ffp-contract=off -o $@ tests/runtime/cm_math_unit.c src/cudapp_om_cm_math.c -lm

$(OUT_DIR)/cm_oracle_diff.o: tests/runtime/cm_oracle.c src/runtime/rx_costmodel.c src/runtime/rx_costmodel.h src/runtime/rx_cm_math.h src/runtime/rx_cm_sort.h src/cudapp_native.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -c -o $@ $<
$(OUT_DIR)/cm_oracle_contracted.o: tests/runtime/cm_oracle.c src/runtime/rx_costmodel.c src/runtime/rx_costmodel.h src/runtime/rx_cm_math.h src/runtime/rx_cm_sort.h src/cudapp_native.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=fast -c -o $@ $<
$(OUT_DIR)/cm_blob_unit: tests/runtime/cm_blob_unit.c $(OUT_DIR)/cm_oracle_diff.o src/cudapp_native.h $(RX_CM_OBJ) $(wildcard src/cudapp_om_cm_*.c) $(DS_SHA_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -o $@ tests/runtime/cm_blob_unit.c $(OUT_DIR)/cm_oracle_diff.o $(RX_CM_OBJ) $(RX_CM_BINDING) $(DS_SHA_SRC) -lm

test-host-policy: $(OUT_DIR)/host_policy_unit $(OUT_DIR)/cm_sort_unit $(OUT_DIR)/artifact_text_unit $(OUT_DIR)/cm_math_unit $(OUT_DIR)/cm_blob_unit
	./$(OUT_DIR)/host_policy_unit
	./$(OUT_DIR)/cm_sort_unit
	./$(OUT_DIR)/artifact_text_unit
	./$(OUT_DIR)/cm_math_unit
	./$(OUT_DIR)/cm_blob_unit
	CC="$(CC)" python3 tests/runtime/host_policy_mutants.py

.PHONY: test-cm-decision-differential
CM_DIFF_WRAP = -Wl,--wrap=rx_cm_decide -Wl,--wrap=rx_cm_predict -Wl,--wrap=rx_cm_init -Wl,--wrap=rx_cm_observe -Wl,--wrap=rx_cm_calibrate -Wl,--wrap=rx_cm_charge
$(OUT_DIR)/cm_decision_diff: tests/runtime/rx_costmodel_unit.c tests/runtime/cm_decision_diff.c $(OUT_DIR)/cm_oracle_diff.o $(RX_CM_OBJ) src/runtime/rx_costmodel.h src/cudapp_native.h $(wildcard src/cudapp_om_cm_*.c) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -o $@ tests/runtime/rx_costmodel_unit.c tests/runtime/cm_decision_diff.c $(OUT_DIR)/cm_oracle_diff.o $(RX_CM_OBJ) $(RX_CM_BINDING) $(DS_SHA_SRC) $(CM_DIFF_WRAP) -lm
$(OUT_DIR)/cm_decision_replay: tests/runtime/cm_decision_diff.c $(OUT_DIR)/cm_oracle_diff.o $(RX_CM_OBJ) src/cudapp_native.h $(wildcard src/cudapp_om_cm_*.c) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -DCM_DIFF_REPLAY_MAIN -o $@ tests/runtime/cm_decision_diff.c $(OUT_DIR)/cm_oracle_diff.o $(RX_CM_OBJ) $(RX_CM_BINDING) $(DS_SHA_SRC) $(CM_DIFF_WRAP) -lm
$(OUT_DIR)/cm_decision_contracted: tests/runtime/cm_decision_diff.c $(OUT_DIR)/cm_oracle_contracted.o $(RX_CM_OBJ) src/cudapp_native.h $(wildcard src/cudapp_om_cm_*.c) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -DCM_DIFF_REPLAY_MAIN -DCM_DIFF_OUTPUT_ONLY -o $@ tests/runtime/cm_decision_diff.c $(OUT_DIR)/cm_oracle_contracted.o $(RX_CM_OBJ) $(RX_CM_BINDING) $(DS_SHA_SRC) -Wl,--wrap=rx_cm_decide -Wl,--wrap=rx_cm_predict -lm
test-cm-decision-differential: $(OUT_DIR)/cm_decision_diff $(OUT_DIR)/cm_decision_replay $(OUT_DIR)/cm_decision_contracted
	./$(OUT_DIR)/cm_decision_diff
	@ulimit -c 0; if ./$(OUT_DIR)/cm_decision_contracted >$(OUT_DIR)/cm_contraction_control.log 2>&1; then \
	  cat $(OUT_DIR)/cm_contraction_control.log; echo "CM contraction control missed drift"; exit 1; fi; \
	  if ! grep -q "CM DECISION floating differential FAIL" $(OUT_DIR)/cm_contraction_control.log; then \
	  cat $(OUT_DIR)/cm_contraction_control.log; echo "CM contraction control failed for an unrelated reason"; exit 1; fi; \
	  echo "CM contraction control PASS (contraction-on oracle rejected)"
	CM_DIFF_REPLAY=$(OUT_DIR)/cm_decision_max.case CM_DIFF_THRESHOLD_PROBE=1 ./$(OUT_DIR)/cm_decision_replay

.PHONY: test-cm-source-mutants
test-cm-source-mutants:
	CC="$(CC)" python3 tests/runtime/cm_source_mutants.py
