# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Scalar policy is portable. CUDA-header transports and native-emitter gates are
# explicit focused targets, not new dependencies of the portable host suite.
CARRIED_HOST_TESTS += test-standalone-oracles
.PHONY: test-standalone-oracles test-standalone-oracles-family test-standalone-oracles-mutants
$(OUT_DIR)/cudapp_om_standalone_oracles.o: CFLAGS += -ffp-contract=off
$(OUT_DIR)/standalone-oracles: om/standalone-oracles/scalar-check.c om/standalone-oracles/oracle-c.c om/standalone-oracles/oracle.h src/cudapp_om_standalone_oracles.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(filter-out -MMD -MP,$(CFLAGS)) -ffp-contract=off om/standalone-oracles/scalar-check.c om/standalone-oracles/oracle-c.c src/cudapp_om_standalone_oracles.c -o $@
test-standalone-oracles: $(OUT_DIR)/standalone-oracles
	$(abspath $(OUT_DIR))/standalone-oracles
test-standalone-oracles-family: $(OUT_DIR)/libomega_gpu.a test-q38-head-bounds
	CC="$(CC)" python3 om/standalone-oracles/family-check.py $(OUT_DIR)/libomega_gpu.a --frame $(OUT_DIR)/q38-head/bounds.frame
test-standalone-oracles-mutants:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" python3 om/standalone-oracles/mutants.py
