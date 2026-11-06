# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER ?= 0
CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER=$(CUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER)
GPU_GEMM_ANY_TEST = $(OUT_DIR)/gpu_gemm_any_test
GPU_GEMM_ANY_C_ORACLE = $(OUT_DIR)/gpu_gemm_any_c_oracle
$(GPU_GEMM_ANY_TEST): tests/gpu_gemm_any_test.c tests/gpu_gemm_test.c tests/bw_warp_sim_tc.h tests/bw_hmma_model.h tests/bw_qmma_model.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ tests/gpu_gemm_any_test.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
$(GPU_GEMM_ANY_C_ORACLE): tests/gpu_gemm_any_test.c tests/gpu_gemm_test.c src/omega_gpu_gemm.c $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER -DCUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER=1 -ffp-contract=off -Itests -o $@ tests/gpu_gemm_any_test.c src/omega_gpu_gemm.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
.PHONY: test-gpu-gemm-any test-gpu-gemm-any-differential
test-gpu-gemm-any: $(GPU_GEMM_ANY_TEST)
	$(GPU_GEMM_ANY_TEST) --sim
test-gpu-gemm-any-differential: $(GPU_GEMM_ANY_TEST) $(GPU_GEMM_ANY_C_ORACLE)
	$(GPU_GEMM_ANY_TEST) --sim --digest > $(OUT_DIR)/gpu_gemm_any_omega.digest
	$(GPU_GEMM_ANY_C_ORACLE) --sim --digest > $(OUT_DIR)/gpu_gemm_any_c.digest
	cmp $(OUT_DIR)/gpu_gemm_any_omega.digest $(OUT_DIR)/gpu_gemm_any_c.digest
	@echo 'GEMM ANY C/OMEGA SIM-EQUAL PASS (not byte-identical kernels)'
CARRIED_HOST_TESTS += test-gpu-gemm-any test-gpu-gemm-any-differential
