# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-prefill-shape
.PHONY: test-prefill-shape
$(OUT_DIR)/prefill-shape.so: src/cudapp_om_prefill_shape.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(filter-out -MMD -MP,$(CFLAGS)) -shared -fPIC $< -o $@
$(OUT_DIR)/prefill-cache-check: om/prefill-shape/cache-check.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ $< $(OUT_DIR)/libomega_gpu.a -Wl,--wrap=omega_gpu_session_alloc_code -lpthread -lm
test-prefill-shape: $(OUT_DIR)/prefill-shape.so $(OUT_DIR)/gpu_qgemm_test $(OUT_DIR)/prefill-cache-check
	$(abspath $(OUT_DIR))/prefill-cache-check
	python3 om/prefill-shape/check.py $(abspath $(OUT_DIR))/prefill-shape.so
	$(abspath $(OUT_DIR))/gpu_qgemm_test --host-only
	$(abspath $(OUT_DIR))/gpu_qgemm_test --sim
ifeq ($(CUDAPP_OMC_HOST),1)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/prefill-shape/mutants.py
endif
