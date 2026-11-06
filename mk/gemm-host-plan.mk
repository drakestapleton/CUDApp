# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Config, grids, spans, launch words and scratch policy default to seeded Omega.
# The hand-written C is a default-OFF migration oracle, independent of the kernel builder oracle.
CUDAPP_MIGRATION_PENDING_C_GEMM_HOST_PLAN ?= 0
$(OUT_DIR)/omega_gpu_gemm.o: CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_GEMM_HOST_PLAN=$(CUDAPP_MIGRATION_PENDING_C_GEMM_HOST_PLAN)
.PHONY: test-gemm-host-plan-policy
test-gemm-host-plan-policy:
	python3 tests/gemm_host_plan_check.py
