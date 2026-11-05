# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Issue #205: the C encoder's BW_IR_LDG_MMIO word, pinned and read through Mesa NAK's sm_80+ memory-access fields
# (tests/bw_ldg_mmio_test.c). The red check feeds omega-mv's word (w[2] 0x0c0e1900, LDG.E.EF) to the same contract
# check and requires it to fail. Host only; GB10 behaviour NOT_RUN. Picked up by the `-include mk/*.mk` line.
BW_LDG_MMIO_TEST = $(OUT_DIR)/bw_ldg_mmio_test
$(BW_LDG_MMIO_TEST): tests/bw_ldg_mmio_test.c src/omega_blackwell_codegen.h src/omega_sm121_disasm.h $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ tests/bw_ldg_mmio_test.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm

.PHONY: test-bw-ldg-mmio
test-bw-ldg-mmio: $(BW_LDG_MMIO_TEST)
	$(BW_LDG_MMIO_TEST)
	@if $(BW_LDG_MMIO_TEST) --word 0x0c0e1900; then \
		echo "RED CHECK BROKEN: the EF word kept the LDG_MMIO contract"; exit 1; \
	else rc=$$?; test "$$rc" = 1 || exit "$$rc"; \
		echo "red check: omega-mv's LDG.E.EF word FAILS the LDG_MMIO contract, as it must"; fi
CARRIED_HOST_TESTS += test-bw-ldg-mmio
