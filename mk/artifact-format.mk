# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-artifact-format
.PHONY: test-artifact-format
test-artifact-format: test-native-artifact-manifest test-native-artifact
	CC="$(CC)" CFLAGS="$(CFLAGS)" python3 tests/runtime/artifact_format_gate.py
