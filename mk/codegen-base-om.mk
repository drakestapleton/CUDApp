# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CPU-only replay and byte-identical SASS/cubin comparison. Caller must use the documented bwrap wrapper.
CARRIED_HOST_TESTS += test-codegen-base-om
# The units check and the mutants execute the pinned AArch64 omc. Off a
# CUDAPP_OMC_HOST the comparison runs without them and run.sh prints NOT_RUN.
.PHONY: test-codegen-base-om test-codegen-base-om-mutants
test-codegen-base-om:
	OMEGA_OMC="$(if $(CUDAPP_OMC_HOST),$(OMEGA_OMC))" OMEGA_BRIDGE="$(OMEGA_BRIDGE_DIR)" CC="$(CC)" sh tests/codegen_base/run.sh
ifeq ($(CUDAPP_OMC_HOST),1)
test-codegen-base-om-mutants:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE_DIR)" CC="$(CC)" sh tests/codegen_base/run.sh --mutants
else
test-codegen-base-om-mutants:
	@echo "$@: NOT_RUN (requires the pinned AArch64 omc; set OMEGA_OMC to a pin that executes on this host; not part of the portable host suite)"
	@exit 2
endif
