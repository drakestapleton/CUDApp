# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host check that the zero-CUDA ldd and nm probes pass binary_path as argv.
# A path with spaces or shell metacharacters stays one pathname.
.PHONY: test-zero-cuda-tool-argv
ZERO_CUDA_TOOL_STUB = $(OUT_DIR)/zero_cuda_tool_stub
ZERO_CUDA_ARGV_TEST = $(OUT_DIR)/zero_cuda_tool_argv_test

$(ZERO_CUDA_TOOL_STUB): tests/zero_cuda_tool_stub.c src/cudapp_freest_leftovers.c src/cudapp_freest_pathcopy.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(ZERO_CUDA_ARGV_TEST): tests/zero_cuda_tool_argv_test.c src/omega_blackwell_submit.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ tests/zero_cuda_tool_argv_test.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm

test-zero-cuda-tool-argv: $(ZERO_CUDA_ARGV_TEST) $(ZERO_CUDA_TOOL_STUB)
	ZERO_CUDA_TOOL_STUB=$(ZERO_CUDA_TOOL_STUB) $(ZERO_CUDA_ARGV_TEST)
