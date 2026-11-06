# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# cudapp-smi host tier (fixtures, fake resource manager, devcall bound probe, mutants). It never opens
# a device; the live comparison tests/smi/live.sh is a receipt tool and is not part of make test.
# native/smi/util.om issues a 55888-byte devcall, which needs the devcall bound of
# aien-dev/omega-mv#1042 (draft PR #1048, not yet approved). Until that compiler is the pin, the
# tier runs only with CUDAPP_SMI_OMC set to the #1048 candidate (sha256 390629fd...); OMEGA_OMC
# (e5b32567) is passed as BASE_OMC for the probe's negative control.
.PHONY: test-smi
SMI_RUN := $(and $(filter 1,$(CUDAPP_OMC_HOST)),$(CUDAPP_SMI_OMC))
ifneq ($(SMI_RUN),)
CARRIED_HOST_TESTS += test-smi
test-smi:
	sh tests/smi/run.sh "$(CUDAPP_SMI_OMC)" "$(OMEGA_OMC)"
else
test-smi:
	@echo "test-smi: NOT_RUN (requires CUDAPP_OMC_HOST=1 and CUDAPP_SMI_OMC, the aien-dev/omega-mv#1048 candidate omc)"
	@exit 2
endif
