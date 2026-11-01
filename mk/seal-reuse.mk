# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-seal-reuse
.PHONY: test-seal-reuse
$(OUT_DIR)/seal-reuse.so: src/cudapp_om_closure_reseal.c
	@mkdir -p $(OUT_DIR)
	$(CC) -O2 -shared -fPIC $< -o $@
test-seal-reuse: $(OUT_DIR)/seal-reuse.so
	python3 om/pipeline/seal-reuse-check.py $(abspath $(OUT_DIR))/seal-reuse.so
ifeq ($(CUDAPP_OMC_HOST),1)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/pipeline/seal-reuse-mutants.py
endif
