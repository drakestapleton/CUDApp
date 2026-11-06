# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-rest
.PHONY: test-codegen-rest
test-codegen-rest:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" CC="$(CC)" \
	  python3 tests/codegen_rest_check.py --coverage --mutants --build-dir $(OUT_DIR)/codegen-rest

.PHONY: test-codegen-rest-sim
test-codegen-rest-sim: $(OUT_DIR)/gpu_decode_attn256_test $(OUT_DIR)/gpu_prefill_attn256_test $(OUT_DIR)/gpu_router_test $(OUT_DIR)/gpu_fp8gemm_test $(OUT_DIR)/gpu_bf16_gemv_test $(OUT_DIR)/gpu_qgemm_test
	$(GPU_DATTN256_TEST) --host-only
	$(GPU_DATTN256_TEST) --sim
	$(GPU_PATTN256_TEST) --host-only
	$(GPU_PATTN256_TEST) --sim
	$(GPU_ROUTER_TEST) --host-only
	$(GPU_ROUTER_TEST) --sim
	$(GPU_FP8GEMM_TEST) --host-only
	$(GPU_FP8GEMM_TEST) --sim
	$(GPU_BF16_TEST) --host-only
	$(GPU_BF16_TEST) --sim
	$(GPU_QGEMM_TEST) --host-only
	$(GPU_QGEMM_TEST) --sim

CARRIED_HOST_TESTS += test-om-ir-limit
.PHONY: test-om-ir-limit
test-om-ir-limit:
	$(CC) $(CFLAGS) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Isrc \
	  tests/gpu_om_ir_limit_test.c src/omega_gpu_om_ir.c src/omega_blackwell_codegen.c \
	  -Wl,--gc-sections -o /tmp/cudapp-om-ir-limit
	/tmp/cudapp-om-ir-limit
