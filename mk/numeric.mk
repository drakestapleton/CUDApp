# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Numeric reference migration to Omega (om/numeric/numeric.om, om/numeric/tile.om); knob CUDAPP_NUMERIC in mk/ampere-cubin.mk.
# C-vs-Omega differential of the numeric reference (om/numeric/numeric.om, tile.om): every public function runs
# both and aborts on a bit difference; tools/numeric_om_mutants.sh shows a broken Omega unit is caught.

# The archive objects need the same rounding contract as the direct test links.
# Append even when callers supply CFLAGS, so a prior contraction flag cannot win.
$(addprefix $(OUT_DIR)/,$(notdir $(NUMERIC_SRCS:.c=.o))): override CFLAGS += -ffp-contract=off
.PHONY: test-numeric-om-diff
$(AMPERE_CUBIN_DIR)/numeric-om-diff: tests/numeric_om_diff_test.c $(NUMERIC_SRCS) $(NUMERIC_DEPS)
	@mkdir -p $(AMPERE_CUBIN_DIR)
	$(CC) $(CFLAGS) -Isrc -ffp-contract=off -ffunction-sections -fdata-sections -Wl,--gc-sections -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF -o $@ tests/numeric_om_diff_test.c $(NUMERIC_SRCS) -lm

test-numeric-om-diff: $(AMPERE_CUBIN_DIR)/numeric-om-diff
	$(AMPERE_CUBIN_DIR)/numeric-om-diff

.PHONY: test-numeric-om-mutants
test-numeric-om-mutants:
	@$(CUDAPP_REQUIRE_OMC)
	sh tools/numeric_om_mutants.sh

CARRIED_HOST_TESTS += test-numeric-om-diff
CARRIED_HOST_TESTS += test-numeric-om-mutants
# Both numeric mutant runners regenerate units with the pinned AArch64 omc.
CUDAPP_TARGET_ONLY_TESTS += $(if $(CUDAPP_OMC_HOST),,test-numeric-om-mutants test-numeric-rest-om-mutants)

.PHONY: test-numeric-rest-om-diff
$(AMPERE_CUBIN_DIR)/numeric-rest-om-diff: tests/numeric_rest_om_diff_test.c $(NUMERIC_SRCS) $(NUMERIC_DEPS) src/omega_blackwell_qmd.c src/omega_blackwell_qmd.h src/cudapp_om_bw_qmd.c
	@mkdir -p $(AMPERE_CUBIN_DIR)
	$(CC) $(CFLAGS) -Isrc -ffp-contract=off -ffunction-sections -fdata-sections -Wl,--gc-sections -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_REST_DIFF -o $@ tests/numeric_rest_om_diff_test.c $(NUMERIC_SRCS) src/omega_blackwell_qmd.c src/cudapp_om_bw_qmd.c -lm

test-numeric-rest-om-diff: $(AMPERE_CUBIN_DIR)/numeric-rest-om-diff
	$(AMPERE_CUBIN_DIR)/numeric-rest-om-diff

CARRIED_HOST_TESTS += test-numeric-rest-om-diff

.PHONY: test-numeric-rest-om-mutants
test-numeric-rest-om-mutants:
	@$(CUDAPP_REQUIRE_OMC)
	sh tools/numeric_rest_om_mutants.sh

CARRIED_HOST_TESTS += test-numeric-rest-om-mutants
