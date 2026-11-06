# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-codegen-rest
.PHONY: test-codegen-rest

CARRIED_HOST_TESTS += test-om-ir-limit
.PHONY: test-om-ir-limit
test-om-ir-limit:
	$(CC) $(CFLAGS) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Isrc \
	  tests/gpu_om_ir_limit_test.c src/omega_gpu_om_ir.c src/omega_blackwell_codegen.c $(CUDAPP_BINDING_SRCS) \
	  -Wl,--gc-sections -o /tmp/cudapp-om-ir-limit
	/tmp/cudapp-om-ir-limit
