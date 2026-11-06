# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
.PHONY: test-disasm-words
CARRIED_HOST_TESTS += test-disasm-words
test-disasm-words:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" python3 tests/dis_words_run.py $(OUT_DIR)/dis-words
