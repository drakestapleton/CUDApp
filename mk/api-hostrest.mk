# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
.PHONY: test-api-hostrest
test-api-hostrest:
	bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 python3 tests/api_hostrest_check.py
