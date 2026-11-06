# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
.PHONY: test-chip-enum-om
test-chip-enum-om:
	python3 tests/chip_enum_check.py
CARRIED_HOST_TESTS += test-chip-enum-om
