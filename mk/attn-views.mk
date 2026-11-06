# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only attention-family typed-view differential; caller runs under bwrap.
CARRIED_HOST_TESTS += test-attn-views
.PHONY: test-attn-views
test-attn-views:
	bash tests/attn_views_run.sh
