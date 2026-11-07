# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
.PHONY: test-dagsim-jstr-om
# The jstr check ends with .om mutants regenerated through om/units.sh (pinned AArch64 omc).
CUDAPP_TARGET_ONLY_TESTS += $(if $(CUDAPP_OMC_HOST),,test-dagsim-jstr-om)
test-dagsim-jstr-om:
	@$(CUDAPP_REQUIRE_OMC)
	python3 tests/dagsim/jstr_check.py
CARRIED_HOST_TESTS += test-dagsim-jstr-om
