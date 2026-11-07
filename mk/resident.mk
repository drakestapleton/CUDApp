# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-gpu-resident
$(OUT_DIR)/gpu_resident_test: tests/gpu_resident_test.c tests/gpu_resident_fixture.h tests/gpu_resident_grid.h tests/bw_warp_sim_k.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm
$(OUT_DIR)/gpu_resident_chip: tests/gpu_resident_chip.c tests/gpu_resident_fixture.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ $< $(OUT_DIR)/libomega_gpu.a -lpthread -lm
.PHONY: test-gpu-resident
test-gpu-resident: $(OUT_DIR)/gpu_resident_test $(OUT_DIR)/gpu_resident_chip
	./$(OUT_DIR)/gpu_resident_test --sim
	$(call CUDAPP_OMC_STEP,python3 tests/gpu_resident_mutants.py $(OUT_DIR)/libomega_gpu.a,resident .om mutants)
