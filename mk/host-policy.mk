# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-host-policy
.PHONY: test-host-policy
$(OUT_DIR)/host_policy_unit: tests/runtime/host_policy_unit.c src/omega_host_policy.h src/cudapp_native.h src/cudapp_om_api_policy.c src/cudapp_om_artifact_policy.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/host_policy_unit.c src/cudapp_om_api_policy.c src/cudapp_om_artifact_policy.c

$(OUT_DIR)/cm_sort_unit: tests/runtime/cm_sort_unit.c src/runtime/rx_cm_sort.h src/cudapp_native.h src/cudapp_om_cm_sort.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/cm_sort_unit.c src/cudapp_om_cm_sort.c

$(OUT_DIR)/artifact_text_unit: tests/runtime/artifact_text_unit.c src/cudapp_native.h src/cudapp_om_artifact_parse.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/artifact_text_unit.c src/cudapp_om_artifact_parse.c

$(OUT_DIR)/cm_math_unit: tests/runtime/cm_math_unit.c src/cudapp_native.h src/cudapp_om_cm_math.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/cm_math_unit.c src/cudapp_om_cm_math.c -lm

test-host-policy: $(OUT_DIR)/host_policy_unit $(OUT_DIR)/cm_sort_unit $(OUT_DIR)/artifact_text_unit $(OUT_DIR)/cm_math_unit
	./$(OUT_DIR)/host_policy_unit
	./$(OUT_DIR)/cm_sort_unit
	./$(OUT_DIR)/artifact_text_unit
	./$(OUT_DIR)/cm_math_unit
	CC="$(CC)" python3 tests/runtime/host_policy_mutants.py
