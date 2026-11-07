# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Seeded Omega conductor and fake-transport model. The C ABI ORACLE remains while
# omega-bridge#83 adds the generated shared-library surface. No device access.
.PHONY: test-nvml
NVML_RUN := $(and $(filter 1,$(CUDAPP_OMC_HOST)),$(OMEGA_BRIDGE))
ifneq ($(NVML_RUN),)
CARRIED_HOST_TESTS += test-nvml
NVML_GATE := $(OUT_DIR)/nvml_gate
$(NVML_GATE): tests/components/proc.om tests/nvml/gate.om tests/nvml/mutants.om
	@mkdir -p $(OUT_DIR)
	cat $^ | "$(OMEGA_OMC)" > $@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@
test-nvml: $(NVML_GATE)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" $(NVML_GATE)
else
test-nvml:
	@echo "test-nvml: NOT_RUN (requires CUDAPP_OMC_HOST=1 with OMEGA_OMC and OMEGA_BRIDGE)"
	@exit 2
endif
