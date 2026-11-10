# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Linux platform policy differential. No device is opened.
CARRIED_HOST_TESTS += test-platform-linux-policy test-platform-linux-fake-device test-platform-linux-effects test-platform-linux-wait
# The fake device controller is an Omega program compiled by the pinned AArch64 omc.
CUDAPP_TARGET_ONLY_TESTS += $(if $(CUDAPP_OMC_HOST),,test-platform-linux-fake-device)
.PHONY: test-platform-linux-policy test-platform-linux-fake-device test-platform-linux-effects test-platform-linux-wait

$(OUT_DIR)/platform_linux_policy: tests/platform_linux_policy.c src/cudapp_om_plat_policy.c src/omega_gpu_platform_linux_cpolicy.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -Itests -Isrc -o $@ tests/platform_linux_policy.c

test-platform-linux-policy: $(OUT_DIR)/platform_linux_policy
	$(OUT_DIR)/platform_linux_policy

$(OUT_DIR)/plat_fx_effects.o: om/platform-linux/gen/link/plat_fx_effects.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT_DIR)/plat_fx_wait.o: om/platform-linux/gen/link/plat_fx_wait.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT_DIR)/platform_linux_effects: tests/platform_linux_effects.c om/platform-linux/gen/plat_effects.c src/cudapp_plat_effects.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/platform_linux_effects.c om/platform-linux/gen/plat_effects.c -lpthread

test-platform-linux-effects: $(OUT_DIR)/platform_linux_effects
	$(OUT_DIR)/platform_linux_effects

$(OUT_DIR)/platform_linux_wait: tests/platform_linux_wait.c om/platform-linux/gen/plat_wait.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/platform_linux_wait.c om/platform-linux/gen/plat_wait.c -lpthread

test-platform-linux-wait: $(OUT_DIR)/platform_linux_wait $(OUT_DIR)/platform_linux_wait_mutant
	$(OUT_DIR)/platform_linux_wait
	@set +e; $(OUT_DIR)/platform_linux_wait_mutant; rc=$$?; set -e; test $$rc -ne 0
	@echo "PASS platform linux wait mutant caught"

# The mutant (2^31 bound -> 0) is committed by om/platform-linux/gen_effects.sh like
# plat_wait.c, so this carried gate needs no unpinned compiler.
$(OUT_DIR)/platform_linux_wait_mutant: tests/platform_linux_wait.c om/platform-linux/gen/plat_wait_mutant.c
	@mkdir -p $(OUT_DIR)
	grep -q 2147483648 om/platform-linux/plat_wait.om
	! cmp -s om/platform-linux/gen/plat_wait.c om/platform-linux/gen/plat_wait_mutant.c
	$(CC) $(CFLAGS) -o $@ tests/platform_linux_wait.c om/platform-linux/gen/plat_wait_mutant.c -lpthread

$(OUT_DIR)/platform_linux_fake_devctl: om/platform-linux/fake_devctl.om
	@mkdir -p $(OUT_DIR)
	$(OMEGA_OMC) < om/platform-linux/fake_devctl.om > $@
	test -s $@
	chmod +x $@

ifneq ($(CUDAPP_OMC_HOST),1)
test-platform-linux-fake-device:
	@$(CUDAPP_REQUIRE_OMC)
else
test-platform-linux-fake-device: $(OUT_DIR)/platform_linux_fake_devctl
	$(OUT_DIR)/platform_linux_fake_devctl
	@sed 's/uchar len msg/uchar 18/' om/platform-linux/fake_devctl.om > $(OUT_DIR)/fake_devctl_mutant.om
	@if cmp -s $(OUT_DIR)/fake_devctl_mutant.om om/platform-linux/fake_devctl.om; then echo "MUTANT BROKEN"; exit 1; fi
	@$(OMEGA_OMC) < $(OUT_DIR)/fake_devctl_mutant.om > $(OUT_DIR)/fake_devctl_mutant
	@chmod +x $(OUT_DIR)/fake_devctl_mutant
	@$(OUT_DIR)/fake_devctl_mutant; test $$? -eq 4
	@echo "PASS platform linux fake-device fixture (mutant exit 4)"
endif

# omega_gpu_live_marker (nvrm/nvrm.c) and its sole caller ref_poll_word. Host only, no device. The NVRM test maps
# memfd pages through the native client; the platform test replaces memfd_create so the poll must take the lookup.
# Each mutant breaks one rule in a copy and must fail: the lookup returning no/another context or ignoring the size,
# the caller polling plat_wait_ctx, passing NULL for rm or dropping the offset.
CARRIED_HOST_TESTS += test-live-marker
.PHONY: test-live-marker
LIVE_MARKER_MUT = $(OUT_DIR)/live_marker_mut
LIVE_MARKER_NVRM_LINK = src/cudapp_om_nvrm_transport.c src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c src/cudapp_om_amp_qmd.c $(NVRM_SERIALIZER_SRCS) \
	$(NVRM_ADMIT_SRCS) om/platform-linux/gen/link/plat_fx_wait.c
LIVE_MARKER_NVRM_CC = $(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_NVRM -DCUDAPP_MIGRATION_PENDING_C_NVRM=0 -Invrm -Isrc
LIVE_MARKER_PLAT_CC = gcc -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -pthread -Isrc -Itests $(GPU_ENGINE_PHYS_INC)
$(OUT_DIR)/nvrm_live_marker: nvrm/live_marker_test.c nvrm/nvrm.c nvrm/nvrm.h nvrm/native_client.h $(LIVE_MARKER_NVRM_LINK)
	@mkdir -p $(OUT_DIR)
	$(LIVE_MARKER_NVRM_CC) -o $@ nvrm/live_marker_test.c $(LIVE_MARKER_NVRM_LINK)
$(OUT_DIR)/platform_linux_live_marker: tests/platform_linux_live_marker.c tests/fake_m16_native.c $(GPU_LINUX_REF_SRCS) $(GPU_LINUX_REF_CMDLIST_SRCS)
	@mkdir -p $(OUT_DIR)
	$(LIVE_MARKER_PLAT_CC) -o $@ tests/platform_linux_live_marker.c tests/fake_m16_native.c $(GPU_LINUX_REF_SRCS) $(GPU_LINUX_REF_CMDLIST_SRCS) -lm
test-live-marker: $(OUT_DIR)/nvrm_live_marker $(OUT_DIR)/platform_linux_live_marker
	./$(OUT_DIR)/nvrm_live_marker
	./$(OUT_DIR)/platform_linux_live_marker
	@mkdir -p $(LIVE_MARKER_MUT)
	@set -e; killed=0; \
	nmut() { name=$$1; script=$$2; d=$(LIVE_MARKER_MUT)/$$name; mkdir -p $$d; \
	  sed -e "$$script" nvrm/nvrm.c > $$d/nvrm.c; cp nvrm/live_marker_test.c $$d/; \
	  if cmp -s nvrm/nvrm.c $$d/nvrm.c; then echo "mutant $$name: pattern not found"; exit 1; fi; \
	  $(LIVE_MARKER_NVRM_CC) -o $$d/t $$d/live_marker_test.c $(LIVE_MARKER_NVRM_LINK); \
	  if ./$$d/t > $$d/log 2>&1; then echo "mutant $$name SURVIVED"; exit 1; fi; \
	  echo "mutant $$name killed"; killed=$$((killed+1)); }; \
	pmut() { name=$$1; script=$$2; d=$(LIVE_MARKER_MUT)/$$name; mkdir -p $$d; \
	  sed -e "$$script" src/omega_gpu_platform_linux.c > $$d/omega_gpu_platform_linux.c; \
	  if cmp -s src/omega_gpu_platform_linux.c $$d/omega_gpu_platform_linux.c; then echo "mutant $$name: pattern not found"; exit 1; fi; \
	  srcs=$$(echo $(GPU_LINUX_REF_SRCS) | tr ' ' '\n' | grep -v '^src/omega_gpu_platform_linux.c$$' | tr '\n' ' '); \
	  $(LIVE_MARKER_PLAT_CC) -o $$d/t tests/platform_linux_live_marker.c tests/fake_m16_native.c $$d/omega_gpu_platform_linux.c $$srcs $(GPU_LINUX_REF_CMDLIST_SRCS) -lm; \
	  if ./$$d/t > $$d/log 2>&1; then echo "mutant $$name SURVIVED"; exit 1; fi; \
	  echo "mutant $$name killed"; killed=$$((killed+1)); }; \
	nmut ctx-null 's|\*ctx = rm->omega_context;|*ctx = NULL;|'; \
	nmut ctx-foreign 's|\*ctx = rm->omega_context;|*ctx = (struct omega_native_context *)rm;|'; \
	nmut size-zero 's|nvrm_live_token(rm, cpu, sizeof(uint32_t),|nvrm_live_token(rm, cpu, 0,|'; \
	pmut plat-wait-ctx 's|            pctx = live;|            pctx = plat_wait_ctx;|'; \
	pmut ignore-rm 's|omega_gpu_live_marker(&r->m16.rm, |omega_gpu_live_marker(NULL, |'; \
	pmut no-offset 's|            map_off = byte_off;|            map_off = 0;|'; \
	test $$killed -eq 6; echo "LIVE MARKER PASS (NVRM lookup + platform caller; 6 mutants killed; HOST_ONLY)"
CARRIED_HOST_TESTS += test-platform-linux-rest
.PHONY: test-platform-linux-rest
$(OUT_DIR)/platform_linux_rest: tests/platform_linux_rest.c src/cudapp_om_plat_rest.c src/omega_gpu_platform_linux_rest_oracle.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -Isrc -o $@ $<
test-platform-linux-rest: $(OUT_DIR)/platform_linux_rest
	$(OUT_DIR)/platform_linux_rest
	@if $(OUT_DIR)/platform_linux_rest --red; then echo "FAIL platform rest red control survived"; exit 1; fi
	$(call CUDAPP_OMC_STEP,python3 tests/platform_linux_rest_mutants.py,platform rest .om mutants)
