# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Kernel-family API shell differential. The committed generated units are the
# default path. Mutants recompile .om and need the pinned omc.
CARRIED_HOST_TESTS += test-api-shells-diff
.PHONY: test-api-shells-diff test-api-shells-mutants
test-api-shells-diff:
	bash tests/api_shells_run.sh
ifneq ($(OMEGA_OMC),)
CARRIED_HOST_TESTS += test-api-shells-mutants
test-api-shells-mutants:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" python3 tests/api_shells_mutants.py
endif
