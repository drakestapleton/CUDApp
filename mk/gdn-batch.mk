# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-gdn-batch-policy
.PHONY: test-gdn-batch-policy
$(OUT_DIR)/gdn-batch-policy.so: src/cudapp_om_gdn_batch_plan.c src/cudapp_om_gdn_batch_step.c src/cudapp_om_gdn_batch_stamp.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(filter-out -MMD -MP,$(CFLAGS)) -shared -fPIC $^ -o $@
test-gdn-batch-policy: $(OUT_DIR)/gdn-batch-policy.so
	python3 om/gdn-batch/check.py $(abspath $(OUT_DIR))/gdn-batch-policy.so
ifeq ($(CUDAPP_OMC_HOST),1)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/gdn-batch/mutants.py
endif

CARRIED_HOST_TESTS += test-gdn-batch-dispatch
.PHONY: test-gdn-batch-dispatch
GDN_BATCH_WRAP = omega_gpu_session_lock omega_gpu_session_unlock omega_gpu_session_open omega_gpu_session_on_close omega_gpu_session_scratch omega_gpu_session_scratch_free omega_gpu_session_sync_to_gpu omega_gpu_session_sync_to_cpu omega_gpu_cmdlist_create omega_gpu_cmdlist_free omega_gpu_cmdlist_reset omega_gpu_cmdlist_count omega_gpu_cmdlist_add omega_gpu_cmdlist_submit omega_gpu_cmdlist_stamps omega_gpu_session_launch
$(OUT_DIR)/gdn-batch-dispatch: om/gdn-batch/dispatch-check.c src/omega_gpu_gdn_api.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ $< $(OUT_DIR)/libomega_gpu.a $(foreach f,$(GDN_BATCH_WRAP),-Wl,--wrap=$(f)) -lpthread -lm
test-gdn-batch-dispatch: $(OUT_DIR)/gdn-batch-dispatch $(OUT_DIR)/gdn-batch-selector
	$(abspath $(OUT_DIR))/gdn-batch-dispatch
	$(abspath $(OUT_DIR))/gdn-batch-selector
	CC="$(CC)" python3 om/gdn-batch/selector-mutants.py $(abspath $(OUT_DIR))/libomega_gpu.a
	CC="$(CC)" python3 om/gdn-batch/dispatch-mutants.py $(abspath $(OUT_DIR))/libomega_gpu.a

# Explicit binary only. It opens a GPU unless --host-only or --sim is passed.
$(OUT_DIR)/gdn-batch-chip: om/gdn-batch/chip-check.c tests/gpu_gdn_test.c tests/bw_warp_sim_k.h src/omega_gpu_gdn_batch.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm

$(OUT_DIR)/gdn-batch-selector: om/gdn-batch/selector-check.c om/gdn-batch/chip-check.c tests/gpu_gdn_test.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -Wl,--wrap=omega_gpu_gdn_prefill_scoped -lpthread -lm
