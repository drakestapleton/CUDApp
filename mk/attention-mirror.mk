# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-attention-mirror-policy
.PHONY: test-attention-mirror-policy
$(OUT_DIR)/attention-mirror-policy.so: src/cudapp_om_attn_mirror_plan.c src/cudapp_om_attn_mirror_step.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(filter-out -MMD -MP,$(CFLAGS)) -shared -fPIC $^ -o $@
test-attention-mirror-policy: $(OUT_DIR)/attention-mirror-policy.so
	python3 om/attention-mirror/check.py $(abspath $(OUT_DIR))/attention-mirror-policy.so
ifeq ($(CUDAPP_OMC_HOST),1)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/attention-mirror/mutants.py
endif

CARRIED_HOST_TESTS += test-attention-mirror-dispatch
.PHONY: test-attention-mirror-dispatch
ATTN_MIRROR_WRAP = omega_gpu_session_lock omega_gpu_session_unlock omega_gpu_session_open omega_gpu_session_on_close omega_gpu_decode_attn256_cache_view omega_gpu_session_sync_to_gpu omega_gpu_session_sync_to_cpu omega_gpu_session_free omega_gpu_pf_alloc_dev_locked omega_gpu_pf_copy_va_locked omega_gpu_session_launch
$(OUT_DIR)/attention-mirror-dispatch: om/attention-mirror/dispatch-check.c src/omega_gpu_prefill_attn256_api.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ $< $(OUT_DIR)/libomega_gpu.a $(foreach f,$(ATTN_MIRROR_WRAP),-Wl,--wrap=$(f)) -lpthread -lm
test-attention-mirror-dispatch: $(OUT_DIR)/attention-mirror-dispatch
	$(abspath $(OUT_DIR))/attention-mirror-dispatch
	CC="$(CC)" python3 om/attention-mirror/dispatch-mutants.py $(abspath $(OUT_DIR))/libomega_gpu.a
