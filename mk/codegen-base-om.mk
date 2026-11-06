# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CPU-only replay and byte-identical SASS/cubin comparison. Caller must use the documented bwrap wrapper.
CARRIED_HOST_TESTS += test-codegen-base-om
.PHONY: test-codegen-base-om test-codegen-base-om-mutants
test-codegen-base-om:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE_DIR)" CC="$(CC)" sh tests/codegen_base/run.sh
test-codegen-base-om-mutants:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE_DIR)" CC="$(CC)" sh tests/codegen_base/run.sh --mutants
