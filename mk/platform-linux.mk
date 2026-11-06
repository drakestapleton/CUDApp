# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Linux platform policy differential. No device is opened.
CARRIED_HOST_TESTS += test-platform-linux-policy test-platform-linux-fake-device
.PHONY: test-platform-linux-policy test-platform-linux-fake-device

$(OUT_DIR)/platform_linux_policy: tests/platform_linux_policy.c src/cudapp_om_plat_policy.c src/omega_gpu_platform_linux_cpolicy.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -Itests -Isrc -o $@ tests/platform_linux_policy.c

test-platform-linux-policy: $(OUT_DIR)/platform_linux_policy
	$(OUT_DIR)/platform_linux_policy

$(OUT_DIR)/platform_linux_fake_devctl: om/platform-linux/fake_devctl.om
	@mkdir -p $(OUT_DIR)
	$(OMEGA_MV)/seed/out/omc < om/platform-linux/fake_devctl.om > $@
	test -s $@
	chmod +x $@

test-platform-linux-fake-device: $(OUT_DIR)/platform_linux_fake_devctl
	$(OUT_DIR)/platform_linux_fake_devctl
	@sed 's/uchar len msg/uchar 18/' om/platform-linux/fake_devctl.om > $(OUT_DIR)/fake_devctl_mutant.om
	@if cmp -s $(OUT_DIR)/fake_devctl_mutant.om om/platform-linux/fake_devctl.om; then echo "MUTANT BROKEN"; exit 1; fi
	@$(OMEGA_MV)/seed/out/omc < $(OUT_DIR)/fake_devctl_mutant.om > $(OUT_DIR)/fake_devctl_mutant
	@chmod +x $(OUT_DIR)/fake_devctl_mutant
	@$(OUT_DIR)/fake_devctl_mutant; test $$? -eq 4
	@echo "PASS platform linux fake-device fixture (mutant exit 4)"
