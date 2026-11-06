# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Zero-copy sharing (native/share/share.om) against the fake RM: host reference, suite,
# read-only trap control, mutants, and a compile-only chip gate. Opens no device file.
.PHONY: test-zero-copy-share
ifeq ($(CUDAPP_OMC_HOST),1)
CARRIED_HOST_TESTS += test-zero-copy-share
test-zero-copy-share:
	OMEGA_OMC="$(OMEGA_OMC)" ZERO_COPY_OUT="$(abspath $(OUT_DIR))/zero-copy" CC="$(CC)" \
	  sh tests/share/run-share-host.sh
else
test-zero-copy-share:
	@echo "test-zero-copy-share: NOT_RUN (requires a runnable pinned AArch64 omc)"
	@exit 2
endif
