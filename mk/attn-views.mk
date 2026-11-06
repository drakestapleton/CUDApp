# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only attention-family typed-view differential; caller runs under bwrap.
CARRIED_HOST_TESTS += test-attn-views
.PHONY: test-attn-views test-attn-views-mutants test-attn-views-coverage bench-attn-views
test-attn-views:
	bash tests/attn_views_run.sh
test-attn-views-mutants:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" bash tests/attn_views_run.sh --mutants
test-attn-views-coverage:
	bash tests/attn_views_run.sh --coverage --out "$(OUT_DIR)/attn-views-coverage"
bench-attn-views:
	bash tests/attn_views_run.sh --bench --out "$(OUT_DIR)/attn-views-bench"
