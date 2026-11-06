# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-api-plan
.PHONY: test-api-plan
API_PLAN_SRCS = $(wildcard src/cudapp_om_api_*_plan.c)
$(OUT_DIR)/api_plan_unit: tests/runtime/api_plan_unit.c src/omega_api_plan.h src/cudapp_native.h $(API_PLAN_SRCS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/runtime/api_plan_unit.c $(API_PLAN_SRCS)
test-api-plan: $(OUT_DIR)/api_plan_unit
	./$(OUT_DIR)/api_plan_unit
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 tests/runtime/api_plan_mutants.py
