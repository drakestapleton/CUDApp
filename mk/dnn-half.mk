# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Scalar f16 conversion and guarded encoding verification; no device is opened.
CARRIED_HOST_TESTS += test-dnn-half test-dnn-half-mutants

.PHONY: test-dnn-half test-dnn-half-mutants
$(OUT_DIR)/dnn_half_test: tests/gpu_dnn_half_test.c tests/bw_warp_sim_k.h src/omega_numeric.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Isrc -Itests -o $@ $< src/omega_numeric.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm

$(OUT_DIR)/dnn_half_alt_test: tests/gpu_dnn_half_test.c tests/bw_warp_sim.h src/omega_numeric.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -DDNN_HALF_ALT_SIM -Isrc -Itests -o $@ $< src/omega_numeric.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm

test-dnn-half: $(OUT_DIR)/dnn_half_test $(OUT_DIR)/dnn_half_alt_test
	$(OUT_DIR)/dnn_half_test
	$(OUT_DIR)/dnn_half_alt_test

# Converts .om through the bridge's seeded converter, which executes the pinned AArch64 omc.
CUDAPP_TARGET_ONLY_TESTS += $(if $(CUDAPP_OMC_HOST),,test-dnn-half-mutants)
test-dnn-half-mutants: $(OUT_DIR)/dnn_half_test
	@$(CUDAPP_REQUIRE_OMC)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" OMEGA_TEST_LIB="$(OUT_DIR)/libomega_gpu.a" python3 tests/gpu_dnn_half_mutants.py
