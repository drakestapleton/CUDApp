# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# HOST_ONLY: GPFIFO entry, USERD GPPut and doorbell stores (native/streams/submit.om)
# on memfd fake-device pages; C oracle (CUDAPP_MIGRATION_PENDING_C_SUBMIT=1) vs Omega.
CUDAPP_MIGRATION_PENDING_C_SUBMIT ?= 0
CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_SUBMIT=$(CUDAPP_MIGRATION_PENDING_C_SUBMIT)
CARRIED_HOST_TESTS += test-streams-submit
.PHONY: test-streams-submit test-streams-submit-mutants
STREAMS_SUBMIT_DEPS = nvrm/submit_test.c nvrm/native_client.h nvrm/nvrm.c nvrm/nvrm.h src/cudapp_nvrm_native.h src/cudapp_om_nvrm_transport.c src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c src/cudapp_om_amp_qmd.c $(NVRM_SERIALIZER_SRCS) $(NVRM_ADMIT_SRCS)
STREAMS_SUBMIT_LINK = src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c src/cudapp_om_amp_qmd.c $(NVRM_SERIALIZER_SRCS) $(NVRM_ADMIT_SRCS)
$(OUT_DIR)/streams-submit-omega: $(STREAMS_SUBMIT_DEPS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_NVRM -DCUDAPP_MIGRATION_PENDING_C_NVRM=0 -UCUDAPP_MIGRATION_PENDING_C_SUBMIT -DCUDAPP_MIGRATION_PENDING_C_SUBMIT=0 -o $@ nvrm/submit_test.c src/cudapp_om_nvrm_transport.c $(STREAMS_SUBMIT_LINK)
$(OUT_DIR)/streams-submit-c: $(STREAMS_SUBMIT_DEPS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_NVRM -DCUDAPP_MIGRATION_PENDING_C_NVRM=0 -UCUDAPP_MIGRATION_PENDING_C_SUBMIT -DCUDAPP_MIGRATION_PENDING_C_SUBMIT=1 -o $@ nvrm/submit_test.c src/cudapp_om_nvrm_transport.c $(STREAMS_SUBMIT_LINK)
test-streams-submit: $(OUT_DIR)/streams-submit-omega $(OUT_DIR)/streams-submit-c
	./$(OUT_DIR)/streams-submit-c $(OUT_DIR)/streams-submit-c.img
	./$(OUT_DIR)/streams-submit-omega $(OUT_DIR)/streams-submit-omega.img
	cmp $(OUT_DIR)/streams-submit-c.img $(OUT_DIR)/streams-submit-omega.img
	@cp $(OUT_DIR)/streams-submit-omega.img $(OUT_DIR)/streams-submit-red.img
	@printf '\001' | dd of=$(OUT_DIR)/streams-submit-red.img bs=1 seek=8 conv=notrunc status=none
	@if cmp -s $(OUT_DIR)/streams-submit-c.img $(OUT_DIR)/streams-submit-red.img; then echo "FAIL: image compare accepted a changed GP entry byte"; exit 1; fi
	@echo "STREAMS SUBMIT PASS (2000x2 submits, wrap at 2^32 on a 6-entry ring: C and Omega stores byte-identical; red image refused; HOST_ONLY)"
# Needs the pinned generator toolchain (native/nvrm/units.sh variables); regenerates
# AArch64 only from a mutated submit.om and expects the fixture to fail.
test-streams-submit-mutants:
	STREAMS_SUBMIT_LINK='$(STREAMS_SUBMIT_LINK)' sh tests/streams_submit_mutants.sh
