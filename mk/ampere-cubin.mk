# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Include from Makefile. Hosted .om packer, no GPU; regenerate the checked
# portable TUs with om/units.sh after changing om/cubin/cubin_elf.om.
AMPERE_CUBIN_DIR = $(OUT_DIR)/ampere-cubin
AMPERE_NVDISASM ?= /usr/local/cuda/bin/nvdisasm
AMPERE_CUBIN_SOURCES = tests/ampere_cubin_test.c src/omega_cubin.c $(CUDAPP_BINDING_SRCS)
AMPERE_CUBIN_DEPS = $(AMPERE_CUBIN_SOURCES) src/omega_cubin.h src/cudapp_native.h

.PHONY: test-ampere-cubin test-ampere-cubin-disasm test-om-ampere-elf-admission
$(AMPERE_CUBIN_DIR)/host: $(AMPERE_CUBIN_DEPS)
	@mkdir -p $(AMPERE_CUBIN_DIR)
	$(CC) $(CFLAGS) -Isrc -pthread -o $@ $(AMPERE_CUBIN_SOURCES)

$(AMPERE_CUBIN_DIR)/diff: $(AMPERE_CUBIN_DEPS)
	@mkdir -p $(AMPERE_CUBIN_DIR)
	$(CC) $(CFLAGS) -Isrc -pthread -DCUDAPP_MIGRATION_PENDING_CUBIN_DIFF -o $@ $(AMPERE_CUBIN_SOURCES)

test-ampere-cubin: $(AMPERE_CUBIN_DIR)/host $(AMPERE_CUBIN_DIR)/diff
	$(AMPERE_CUBIN_DIR)/host $(AMPERE_CUBIN_DIR)/sm86.cubin
	$(AMPERE_CUBIN_DIR)/diff

test-ampere-cubin-disasm: test-ampere-cubin
	@test -x "$(AMPERE_NVDISASM)" || { echo 'NOT_RUN: missing offline nvdisasm'; exit 3; }
	"$(AMPERE_NVDISASM)" -c $(AMPERE_CUBIN_DIR)/sm86.cubin > $(AMPERE_CUBIN_DIR)/sm86.sass
	@grep -q 'EXIT' $(AMPERE_CUBIN_DIR)/sm86.sass || { echo 'FAIL: sm86 EXIT not decoded'; exit 1; }
	@echo 'AMPERE CUBIN NVDISASM PASS; device NOT_RUN'

# tests/cubin_elf_admission.om, composed after om/components/artifact_elf.om, runs the existing ELF admission unit on
# the image test-ampere-cubin writes: original accepted, wrong text section size and truncated ELF refused inside the
# program; a 200-byte prefix of the image must be refused as a file (exit 7). A prerequisite of test-om-cubin-mutants.
test-om-ampere-elf-admission: test-ampere-cubin om/components/artifact_elf.om tests/cubin_elf_admission.om
	@[ -x $(OMEGA_MV)/seed/out/omc ] || { echo "NOT_RUN: set OMEGA_MV"; exit 3; }
	cat om/components/artifact_elf.om tests/cubin_elf_admission.om | $(OMEGA_MV)/seed/out/omc > $(AMPERE_CUBIN_DIR)/elf-admission
	chmod +x $(AMPERE_CUBIN_DIR)/elf-admission
	$(AMPERE_CUBIN_DIR)/elf-admission $(AMPERE_CUBIN_DIR)/sm86.cubin
	head -c 200 $(AMPERE_CUBIN_DIR)/sm86.cubin > $(AMPERE_CUBIN_DIR)/sm86-trunc.cubin
	@rc=0; $(AMPERE_CUBIN_DIR)/elf-admission $(AMPERE_CUBIN_DIR)/sm86-trunc.cubin > /dev/null || rc=$$?; \
	if [ $$rc != 7 ]; then echo "RED CHECK BROKEN: truncated image exited $$rc, not 7"; exit 1; fi; \
	echo "red check: a 200-byte image prefix exits 7, as it must"

# Numeric defaults to the scalar, tile and remainder units in om/numeric/.
# c selects both default-OFF C oracle flags; diff checks the preserved scalar
# wrappers. test-numeric-rest-om-diff separately compares remainder algorithms.
CUDAPP_NUMERIC ?= om
CUDAPP_NUMERIC_DEFS_om :=
CUDAPP_NUMERIC_DEFS_c := -DCUDAPP_MIGRATION_PENDING_C_NUMERIC -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_REST
CUDAPP_NUMERIC_DEFS_diff := -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF
CUDAPP_NUMERIC_DEFS = $(CUDAPP_NUMERIC_DEFS_$(CUDAPP_NUMERIC))
ifeq ($(filter om c diff,$(CUDAPP_NUMERIC)),)
$(error CUDAPP_NUMERIC must be om, c or diff)
endif
NUMERIC_SRCS := src/omega_numeric.c src/omega_numeric_reduce.c src/cudapp_om_numeric.c src/cudapp_om_numeric_tile.c src/cudapp_om_numeric_rest.c src/cudapp_om_numeric_patch.c src/cudapp_om_numeric_check.c src/cudapp_om_numeric_trace.c src/cudapp_om_numeric_reduce.c
NUMERIC_DEPS := src/omega_numeric.h src/omega_numeric_reduce.h src/omega_numeric_om.h src/cudapp_native.h src/omega_numeric_rest.inc src/omega_numeric_reduce_rest.inc

# Independently runnable on a native x86 host before the bridge prerequisite.
.PHONY: test-ampere-numeric-host
$(AMPERE_CUBIN_DIR)/numeric-host: tests/ampere_numeric_host_test.c $(NUMERIC_SRCS) $(NUMERIC_DEPS) src/cudapp_freest_leftovers.c
	@mkdir -p $(AMPERE_CUBIN_DIR)
	$(CC) $(CFLAGS) -Isrc -ffp-contract=off -ffunction-sections -fdata-sections -Wl,--gc-sections $(CUDAPP_NUMERIC_DEFS) -o $@ tests/ampere_numeric_host_test.c $(NUMERIC_SRCS) src/cudapp_freest_leftovers.c -lm

test-ampere-numeric-host: $(AMPERE_CUBIN_DIR)/numeric-host
	$(AMPERE_CUBIN_DIR)/numeric-host

# The native floating cast is an independent verification oracle only.
.PHONY: test-ampere-f16-host
$(AMPERE_CUBIN_DIR)/f16-host: tests/ampere/test_f16_behavior.c src/cudapp_freest_f16.c
	@mkdir -p $(AMPERE_CUBIN_DIR)
	$(CC) $(CFLAGS) -O3 -o $@ tests/ampere/test_f16_behavior.c src/cudapp_freest_f16.c

test-ampere-f16-host: $(AMPERE_CUBIN_DIR)/f16-host
	$(AMPERE_CUBIN_DIR)/f16-host
