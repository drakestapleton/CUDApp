# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Seeded list policy; no device is opened by these host and simulator checks.
CUDAPP_MIGRATION_PENDING_CMDLIST ?= 0
ifeq ($(CUDAPP_MIGRATION_PENDING_CMDLIST),1)
CFLAGS += -DCUDAPP_MIGRATION_PENDING_CMDLIST=1
endif
OM_CMDLIST_DIR = $(OUT_DIR)/om-cmdlist
OM_CMDLIST_TEST = $(OM_CMDLIST_DIR)/planner
CARRIED_HOST_TESTS += test-om-cmdlist
OM_CMDLIST_BINDINGS = src/cudapp_om_cmdlist_record.c src/cudapp_om_cmdlist_plan.c src/cudapp_om_cmdlist_layout.c
$(OM_CMDLIST_TEST): tests/cmdlist_plan_test.c $(OM_CMDLIST_BINDINGS) src/omega_blackwell_qmd.c src/cudapp_native.h
	@mkdir -p $(OM_CMDLIST_DIR)
	$(CC) $(CFLAGS) -o $@ tests/cmdlist_plan_test.c $(OM_CMDLIST_BINDINGS) src/omega_blackwell_qmd.c
$(OM_CMDLIST_DIR)/sim: tests/cmdlist_sim_test.c src/cudapp_om_cmdlist_plan.c $(OUT_DIR)/libomega_gpu.a
	@mkdir -p $(OM_CMDLIST_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ tests/cmdlist_sim_test.c src/cudapp_om_cmdlist_plan.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
$(OM_CMDLIST_DIR)/shell: tests/cmdlist_shell_test.c src/omega_gpu_cmdlist.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ tests/cmdlist_shell_test.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
$(OM_CMDLIST_DIR)/cost: tests/cmdlist_cost_test.c src/cudapp_om_cmdlist_plan.c
	@mkdir -p $(OM_CMDLIST_DIR)
	$(CC) $(CFLAGS) -o $@ tests/cmdlist_cost_test.c src/cudapp_om_cmdlist_plan.c
.PHONY: test-om-cmdlist
test-om-cmdlist: $(OM_CMDLIST_TEST) $(OM_CMDLIST_DIR)/sim $(OM_CMDLIST_DIR)/shell $(OM_CMDLIST_DIR)/cost
	./$(OM_CMDLIST_TEST)
	./$(OM_CMDLIST_DIR)/sim
	./$(OM_CMDLIST_DIR)/shell
	./$(OM_CMDLIST_DIR)/cost
test-om-cmdlist-mutants: $(OM_CMDLIST_TEST) $(OM_CMDLIST_DIR)/sim
	python3 tests/check_cmdlist_mutants.py
.PHONY: test-om-cmdlist-mutants
