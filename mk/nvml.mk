# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Seeded Omega NVML gate: the libnvidia-ml.so.1 generated from native/nvml/api.om by the
# omega-bridge#83 C ABI emitter (OMEGA_CABI_BRIDGE, pinned by om/omega-bridge-cabi.lock),
# the Omega fake-RM model and its mutants. No device access.
.PHONY: test-nvml
NVML_RUN := $(and $(filter 1,$(CUDAPP_OMC_HOST)),$(OMEGA_BRIDGE),$(OMEGA_CABI_BRIDGE))
ifneq ($(NVML_RUN),)
CARRIED_HOST_TESTS += test-nvml
NVML_GATE := $(OUT_DIR)/nvml_gate
$(NVML_GATE): tests/components/proc.om tests/nvml/gate.om tests/nvml/mutants.om
	@mkdir -p $(OUT_DIR)
	cat $^ | "$(OMEGA_OMC)" > $@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@
test-nvml: $(NVML_GATE)
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" OMEGA_CABI_BRIDGE="$(OMEGA_CABI_BRIDGE)" $(NVML_GATE)
else
test-nvml:
	@echo "test-nvml: NOT_RUN (requires CUDAPP_OMC_HOST=1 with OMEGA_OMC, OMEGA_BRIDGE and OMEGA_CABI_BRIDGE)"
	@exit 2
endif
