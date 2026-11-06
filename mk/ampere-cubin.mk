# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Include from Makefile. Hosted .om packer, no GPU; regenerate the checked
# portable TUs with om/units.sh after changing om/cubin/cubin_elf.om.
AMPERE_CUBIN_DIR = $(OUT_DIR)/ampere-cubin
AMPERE_NVDISASM ?= /usr/local/cuda/bin/nvdisasm
AMPERE_CUBIN_SOURCES = tests/ampere_cubin_test.c src/omega_cubin.c $(CUDAPP_BINDING_SRCS)
AMPERE_CUBIN_DEPS = $(AMPERE_CUBIN_SOURCES) src/omega_cubin.h src/cudapp_native.h

.PHONY: test-ampere-cubin test-ampere-cubin-disasm
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
