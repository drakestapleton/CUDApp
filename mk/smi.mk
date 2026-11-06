# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# cudapp-smi host tier (fixtures, fake resource manager, mutants). It never opens a device;
# the live comparison tests/smi/live.sh is a receipt tool and is not part of make test.
.PHONY: test-smi
ifeq ($(CUDAPP_OMC_HOST),1)
CARRIED_HOST_TESTS += test-smi
test-smi:
	sh tests/smi/run.sh "$(OMEGA_OMC)"
else
test-smi:
	@echo "test-smi: NOT_RUN (requires a runnable pinned AArch64 omc)"
	@exit 2
endif
