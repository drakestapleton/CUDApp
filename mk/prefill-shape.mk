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

CARRIED_HOST_TESTS += test-prefill-order
.PHONY: test-prefill-order
$(OUT_DIR)/prefill-order.so: src/cudapp_om_prefill_order.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(filter-out -MMD -MP,$(CFLAGS)) -shared -fPIC $< -o $@
test-prefill-order: $(OUT_DIR)/prefill-order.so $(OUT_DIR)/gpu_qgemm_test
	python3 om/prefill-shape/order-check.py $(abspath $(OUT_DIR))/prefill-order.so
ifeq ($(CUDAPP_OMC_HOST),1)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/prefill-shape/order-mutants.py
endif

CARRIED_HOST_TESTS += test-prefill-scoped
.PHONY: test-prefill-scoped
$(OUT_DIR)/prefill-scoped-check: om/prefill-shape/scoped-check.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ $< $(OUT_DIR)/libomega_gpu.a -Wl,--wrap=omega_gpu_pf_open_locked -Wl,--wrap=omega_gpu_pf_kernel_locked -lpthread -lm
test-prefill-scoped: $(OUT_DIR)/prefill-scoped-check
	$(abspath $(OUT_DIR))/prefill-scoped-check
	CC="$(CC)" python3 om/prefill-shape/scoped-mutants.py $(abspath $(OUT_DIR))/libomega_gpu.a

CARRIED_HOST_TESTS += test-prefill-reading
.PHONY: test-prefill-reading
$(OUT_DIR)/prefill-reading.so: src/cudapp_om_prefill_reading.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(filter-out -MMD -MP,$(CFLAGS)) -shared -fPIC $< -o $@
test-prefill-reading: $(OUT_DIR)/prefill-reading.so
	python3 om/prefill-shape/reading-check.py $(abspath $(OUT_DIR))/prefill-reading.so
ifeq ($(CUDAPP_OMC_HOST),1)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/prefill-shape/reading-mutants.py
endif
