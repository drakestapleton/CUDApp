# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only f64 encoding, simulation, pair pressure and seeded mutation gate.
CARRIED_HOST_TESTS += test-codegen-f64
.PHONY: test-codegen-f64
$(OUT_DIR)/codegen_f64_test: tests/codegen_f64_test.c tests/bw_warp_sim.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Isrc -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm

test-codegen-f64: $(OUT_DIR)/codegen_f64_test
	$(OUT_DIR)/codegen_f64_test
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" OMEGA_TEST_LIB="$(OUT_DIR)/libomega_gpu.a" python3 tests/codegen_f64_mutants.py
