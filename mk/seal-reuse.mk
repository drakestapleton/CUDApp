# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-seal-reuse
# Integer-only generated Omega unit. Use distinct OUT_DIRs when comparing settings.
CUDAPP_SEAL_OPT ?= -O3
$(OUT_DIR)/cudapp_om_closure_reseal.o: private CFLAGS += $(CUDAPP_SEAL_OPT)
.PHONY: test-seal-reuse
$(OUT_DIR)/seal-reuse.so: src/cudapp_om_closure_reseal.c src/cudapp_om_closure_seal.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(filter-out -MMD -MP,$(CFLAGS)) $(CUDAPP_SEAL_OPT) -shared -fPIC $^ -o $@
test-seal-reuse: $(OUT_DIR)/seal-reuse.so
	python3 om/pipeline/seal-reuse-check.py $(abspath $(OUT_DIR))/seal-reuse.so
ifeq ($(CUDAPP_OMC_HOST),1)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" CUDAPP_SEAL_OPT="$(CUDAPP_SEAL_OPT)" python3 om/pipeline/seal-reuse-mutants.py
endif
