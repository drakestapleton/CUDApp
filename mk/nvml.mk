# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CUDA++ NVML host tier: the libnvidia-ml C ABI subset (compat/nvml) over native/nvml against the
# resource-manager fake tests/nvml/rmfake.om, the CUDA nvml.h ABI comparison and mutants. Device-free:
# the fake transport replaces omega2_nvrm_*; nothing opens /dev/nvidia*.
# Needs the pinned OMEGA_OMC and an OMEGA_BRIDGE checkout (rmfake.om and the mutants are bridged).
.PHONY: test-nvml
NVML_RUN := $(and $(filter 1,$(CUDAPP_OMC_HOST)),$(OMEGA_BRIDGE))
ifneq ($(NVML_RUN),)
CARRIED_HOST_TESTS += test-nvml
test-nvml:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" sh tests/nvml/run.sh
else
test-nvml:
	@echo "test-nvml: NOT_RUN (requires CUDAPP_OMC_HOST=1 with OMEGA_OMC and OMEGA_BRIDGE)"
	@exit 2
endif
