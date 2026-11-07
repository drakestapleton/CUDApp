# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
.PHONY: test-chip-enum-om
# The seven .om mutants regenerate chip_enum through om/units.sh, which executes
# the pinned AArch64 omc. Off a CUDAPP_OMC_HOST they print NOT_RUN; the
# differential, controls and fixture scan still run.
test-chip-enum-om:
	CHIP_ENUM_MUTANTS='$(if $(CUDAPP_OMC_HOST),1,0)' OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" \
		python3 tests/chip_enum_check.py
CARRIED_HOST_TESTS += test-chip-enum-om
