# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
.PHONY: test-disasm-words
CARRIED_HOST_TESTS += test-disasm-words
# The word check regenerates units through om/units.sh (pinned AArch64 omc).
CUDAPP_TARGET_ONLY_TESTS += $(if $(CUDAPP_OMC_HOST),,test-disasm-words)
test-disasm-words:
	@$(CUDAPP_REQUIRE_OMC)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" python3 tests/dis_words_run.py $(OUT_DIR)/dis-words
