# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-core-util test-core-mutants test-core-cubin-message test-core-residue test-core-residue-mutants
.PHONY: test-core-util test-core-algorithms test-core-mutants test-core-sha-state
$(OUT_DIR)/core-util-sha-ref.o: src/sha256.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_CORE -Dsha256_init=ref_sha256_init -Dsha256_update=ref_sha256_update -Dsha256_final=ref_sha256_final -Dsha256_hash=ref_sha256_hash -c $< -o $@
$(OUT_DIR)/test-core-util: tests/test_core_util.c src/sha256.c $(OUT_DIR)/core-util-sha-ref.o src/cudapp_om_core_sha.c src/omega_core_binding.h src/cudapp_native.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.c %.o,$^) -lpthread
test-core-util: $(OUT_DIR)/test-core-util $(OUT_DIR)/test-core-algorithms $(OUT_DIR)/test-core-sha-state
	./$(OUT_DIR)/test-core-util
	./$(OUT_DIR)/test-core-sha-state
	@if ./$(OUT_DIR)/test-core-util --flip; then echo 'FAIL record flip survived'; exit 1; else echo 'CORE-UTIL negative control caught'; fi
	./$(OUT_DIR)/test-core-algorithms

CORE_UTIL_ORACLES := omega_core omega_canonical omega_vector omega_gpu_fmt omega_gpu_hostmath
CORE_UTIL_REFS := $(addprefix $(OUT_DIR)/core-util-ref-,$(addsuffix .o,$(CORE_UTIL_ORACLES)))
CORE_UTIL_SOURCES := $(addprefix src/,$(addsuffix .c,$(CORE_UTIL_ORACLES))) src/sha256.c
CORE_UTIL_GENERATED := $(wildcard src/cudapp_om_core_*.c)
$(OUT_DIR)/core-util-ref-%.o: src/%.c tests/core_util_rename.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -DCUDAPP_MIGRATION_PENDING_C_CORE -include tests/core_util_rename.h -c $< -o $@
$(OUT_DIR)/test-core-algorithms: tests/test_core_algorithms.c $(CORE_UTIL_SOURCES) $(CORE_UTIL_REFS) $(OUT_DIR)/core-util-sha-ref.o $(CORE_UTIL_GENERATED) src/cudapp_freest_f16.c src/omega_core_binding.h src/omega_core_om.inc src/cudapp_native.h tests/core_util_refs.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -o $@ $(filter %.c %.o,$^) -lpthread
test-core-algorithms: $(OUT_DIR)/test-core-algorithms
	./$(OUT_DIR)/test-core-algorithms

test-core-mutants: $(OUT_DIR)/test-core-util
	python3 tests/test_core_mutants.py --out-dir $(OUT_DIR) --receipt $(OUT_DIR)/core-util-mutants

$(OUT_DIR)/test-core-sha-state: tests/test_core_sha_state.c src/sha256.c $(OUT_DIR)/core-util-sha-ref.o src/cudapp_om_core_sha.c src/omega_core_binding.h src/cudapp_native.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.c %.o,$^) -lpthread
test-core-sha-state: $(OUT_DIR)/test-core-sha-state
	./$(OUT_DIR)/test-core-sha-state

$(OUT_DIR)/test-core-cubin-message: tests/test_core_cubin_message.c tests/core_cubin_refusal_ref.inc src/cudapp_om_core_cubinmsg.c src/cudapp_native.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^) -lpthread
.PHONY: test-core-cubin-message
test-core-cubin-message: $(OUT_DIR)/test-core-cubin-message
	./$(OUT_DIR)/test-core-cubin-message
	@if ./$(OUT_DIR)/test-core-cubin-message --flip; then echo 'FAIL cubin message flip survived'; exit 1; else echo 'CORE-UTIL cubin message negative control caught'; fi

$(OUT_DIR)/test-core-residue: tests/test_core_residue.c src/omega_cubin.c src/cudapp_om_cubin_elf.c \
		src/cudapp_om_core_cubinmsg.c src/cudapp_om_core_cubadmit.c src/cudapp_om_core_ampack.c \
		src/cudapp_om_core_fmtnum.c src/cudapp_native.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^) -lpthread
$(OUT_DIR)/test-core-residue-diff: tests/test_core_residue.c src/omega_cubin.c src/cudapp_om_cubin_elf.c \
		src/cudapp_om_core_cubinmsg.c src/cudapp_om_core_cubadmit.c src/cudapp_om_core_ampack.c \
		src/cudapp_om_core_fmtnum.c src/cudapp_native.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_CUBIN_DIFF -o $@ $(filter %.c,$^) -lpthread
$(OUT_DIR)/test-core-residue-red: tests/test_core_residue.c src/omega_cubin.c src/cudapp_om_cubin_elf.c \
		src/cudapp_om_core_cubinmsg.c src/cudapp_om_core_cubadmit.c src/cudapp_om_core_ampack.c \
		src/cudapp_om_core_fmtnum.c src/cudapp_native.h | $(OUT_DIR)
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_CUBIN_DIFF -DCUDAPP_CUBIN_DIFF_SELFTEST -o $@ $(filter %.c,$^) -lpthread
.PHONY: test-core-residue test-core-residue-mutants
test-core-residue: $(OUT_DIR)/test-core-residue $(OUT_DIR)/test-core-residue-diff $(OUT_DIR)/test-core-residue-red
	./$(OUT_DIR)/test-core-residue
	./$(OUT_DIR)/test-core-residue-diff
	@if ./$(OUT_DIR)/test-core-residue --flip >/dev/null; then echo 'FAIL ampack flip survived'; exit 1; else echo 'CORE-RESIDUE ampack negative control caught'; fi
	@if ./$(OUT_DIR)/test-core-residue-red >/dev/null 2>&1; then echo 'RED CHECK BROKEN: flipped cubin bit passed'; exit 1; else echo 'CORE-RESIDUE red check: one flipped image bit aborts'; fi
test-core-residue-mutants: $(OUT_DIR)/test-core-residue
	OMEGA_OMC="$(or $(OMEGA_OMC),$(HOME)/work/cudastack/omega-mv/seed/out/omc)" OMEGA_BRIDGE="$(or $(OMEGA_BRIDGE),$(HOME)/work/cudastack/omega-bridge)" python3 tests/test_core_residue_mutants.py
