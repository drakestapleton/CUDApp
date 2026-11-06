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

# native/share/share_native.om (OM-2 function mode, for the one hosted native library) built as
# a single-unit test library and run against a seccomp fake RM, then its mutants. Needs the
# pinned OM-2 toolchain (see tests/share/run-share-native.sh); without it the script reports
# NOT_RUN and exits 2, so it is not a carried host test.
.PHONY: test-zero-copy-share-native
test-zero-copy-share-native:
	OMEGA_NATIVE_OMC="$(OMEGA_NATIVE_OMC)" OMEGA_NATIVE_OMC2="$(OMEGA_NATIVE_OMC2)" \
	  ZERO_COPY_NATIVE_BRIDGE="$(ZERO_COPY_NATIVE_BRIDGE)" \
	  ZERO_COPY_OUT="$(abspath $(OUT_DIR))/zero-copy" CC="$(CC)" \
	  sh tests/share/run-share-native.sh
