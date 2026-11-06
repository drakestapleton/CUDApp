# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
.PHONY: test-dagsim-jstr-om
test-dagsim-jstr-om:
	python3 tests/dagsim/jstr_check.py
CARRIED_HOST_TESTS += test-dagsim-jstr-om
