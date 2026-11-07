# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only first-party builder/oracle parity. No driver or device objects.
CARRIED_HOST_TESTS += test-codegen-qmma-diff
.PHONY: test-codegen-qmma-diff
QMMA_REST_SRC = src/omega_blackwell_codegen_qmma.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c src/cudapp_om_cg_qmma.c src/cudapp_om_cg_qmma_patch.c src/cudapp_om_qm_host.c src/cudapp_om_qm_self.c src/cudapp_om_bw_enc_sm121.c src/cudapp_om_cubin_elf.c src/omega_cubin.c src/omega_gpu_fmt.c src/sha256.c src/cudapp_freest_core_cubinmsg.c src/cudapp_freest_core_cubadmit.c src/cudapp_om_core_fmt.c src/cudapp_om_core_fmtnum.c src/cudapp_om_core_fmtclip.c $(BW_CODEGEN_BINDING)
$(OUT_DIR)/codegen_qmma_diff_test: tests/codegen_qmma_diff_test.c $(QMMA_REST_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-codegen-qmma-diff: $(OUT_DIR)/codegen_qmma_diff_test
	./$(OUT_DIR)/codegen_qmma_diff_test
CARRIED_HOST_TESTS += test-codegen-tgemm-diff
.PHONY: test-codegen-tgemm-diff test-codegen-tgemm-exhaustive
$(OUT_DIR)/codegen_tgemm_diff_test: tests/codegen_tgemm_diff_test.c src/omega_blackwell_codegen_tgemm.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c src/cudapp_om_cg_tgemm.c src/cudapp_om_cg_tgemm_spec.c src/cudapp_om_bw_enc_sm121.c src/cudapp_om_cubin_elf.c src/omega_cubin.c src/cudapp_freest_core_cubinmsg.c src/cudapp_freest_core_cubadmit.c src/omega_gpu_fmt.c src/sha256.c $(BW_CODEGEN_BINDING) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-codegen-tgemm-diff: $(OUT_DIR)/codegen_tgemm_diff_test
	./$(OUT_DIR)/codegen_tgemm_diff_test
test-codegen-tgemm-exhaustive: $(OUT_DIR)/codegen_tgemm_diff_test
	./$(OUT_DIR)/codegen_tgemm_diff_test --exhaustive
$(OUT_DIR)/gpu_tgemm_test: tests/gpu_tgemm_test.c tests/bw_hmma_model.h tests/bw_warp_sim_tc.h src/omega_blackwell_codegen_tgemm.h src/omega_gpu_tgemm_api.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -DWS_MAX_THREADS=512 -Itests -o $@ tests/gpu_tgemm_test.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
.PHONY: test-codegen-tgemm-sim
test-codegen-tgemm-sim: $(OUT_DIR)/gpu_tgemm_test
	./$(OUT_DIR)/gpu_tgemm_test --host-only
	./$(OUT_DIR)/gpu_tgemm_test --sim
CARRIED_HOST_TESTS += test-codegen-tgemm-sim
.PHONY: test-codegen-qmma-host
test-codegen-qmma-host: $(OUT_DIR)/gpu_qmma_probe_test
	./$(OUT_DIR)/gpu_qmma_probe_test --host-only
CARRIED_HOST_TESTS += test-codegen-qmma-host

CARRIED_HOST_TESTS += test-qmma-probe-rest test-qmma-probe-rest-mutants
# The mutants regenerate qm_host/qm_self through the bridge's seeded converter,
# which executes the pinned AArch64 omc.
CUDAPP_TARGET_ONLY_TESTS += $(if $(CUDAPP_OMC_HOST),,test-qmma-probe-rest-mutants)
.PHONY: test-qmma-probe-rest test-qmma-probe-rest-mutants
$(OUT_DIR)/qmma_probe_rest_test: tests/qmma_probe_rest_test.c $(QMMA_REST_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -Wl,--gc-sections -o $@ $^ -lm
test-qmma-probe-rest: $(OUT_DIR)/qmma_probe_rest_test
	./$(OUT_DIR)/qmma_probe_rest_test
	@if ./$(OUT_DIR)/qmma_probe_rest_test --red; then echo 'QMMA-REST RED FAIL'; exit 1; else echo 'QMMA-REST RED PASS'; fi
ifeq ($(CUDAPP_OMC_HOST),1)
test-qmma-probe-rest-mutants:
	OUT_DIR=$(OUT_DIR) QMMA_REST_SRC='$(QMMA_REST_SRC)' OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" \
		python3 tests/qmma_probe_rest_mutants.py
else
test-qmma-probe-rest-mutants:
	@echo "$@: NOT_RUN (requires the pinned AArch64 omc; set OMEGA_OMC to a pin that executes on this host; not part of the portable host suite)"
	@exit 2
endif
