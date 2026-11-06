# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Library GPU admission (developer bars D2, D6; ROADMAP C2). HOST_ONLY: simulated driver, fake PCI trees, no
# /dev/nvidia opens.
#   make test-chip-admission           .om decision vs the C host reference, refusals with zero device opens,
#                                      the admitted path and the driver-card cross-checks in nvrm_open
#   make test-chip-admission-mutants   each mutant (.om decision or the nvrm_open call sites) must turn the test red
#       (needs OMEGA_MV and OMEGA_BRIDGE_DIR as in mk/om-encoder.mk; without them exit 3, NOT_RUN, never PASS).
CHIP_ADMIT_DIR = $(OUT_DIR)/chip-admission
# The simulated driver replaces open/ioctl/close/mmap by macro; since NVRM #281 nvrm.c only takes those through
# the explicitly selected C test boundary (nvrm/native_client.h), as mk/ampere-runtime.mk does.
CHIP_ADMIT_BUILD = $(CC) -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -DCUDAPP_NVRM_TEST_TRANSPORT=1 -Isrc $(GPU_ENGINE_PHYS_INC)
CHIP_ADMIT_SRCS = src/omega_gpu_chip.c src/omega_ampere_qmd.c
CHIP_ADMIT_OTHER_UNITS = $(filter-out src/cudapp_om_chip_admit.c,$(CUDAPP_BINDING_SRCS))
CHIP_ADMIT_OM_MUTANTS = 's/eq device 11794/eq device 11795/' 's/ne shr cls 16 3/ne shr cls 16 4/' \
	's/eq device 9353/eq device 9354/' 's/if eq t 0/if eq t 1/' 's/if gt hit 1/if gt hit 2/'
CHIP_ADMIT_C_MUTANTS = 's/if (omega_gpu_chip_admit_open(bdf, rm->err, sizeof rm->err) != 0) return -1;/(void)0;/' \
	's/\&admitted_sm, rm->err, sizeof rm->err) != 0) return -1;/\&admitted_sm, rm->err, sizeof rm->err) != 0) (void)0;/' \
	's/if (rm->target_sm != admitted_sm)/if (0)/'

.PHONY: test-chip-admission test-chip-admission-mutants
CARRIED_HOST_TESTS += test-chip-admission

$(CHIP_ADMIT_DIR)/test_chip_admission: tests/test_chip_admission.c nvrm/nvrm.c nvrm/nvrm.h src/omega_gpu_chip.h \
		src/cudapp_native.h $(CHIP_ADMIT_SRCS) $(CUDAPP_BINDING_SRCS)
	@mkdir -p $(CHIP_ADMIT_DIR)
	$(CHIP_ADMIT_BUILD) -o $@ tests/test_chip_admission.c $(CHIP_ADMIT_SRCS) $(CUDAPP_BINDING_SRCS)

test-chip-admission: $(CHIP_ADMIT_DIR)/test_chip_admission
	TMPDIR=$(abspath $(CHIP_ADMIT_DIR)) ./$(CHIP_ADMIT_DIR)/test_chip_admission

test-chip-admission-mutants:
	@[ -x $(OMEGA_MV)/seed/out/omc ] && [ -f $(OMEGA_BRIDGE_DIR)/src/native.om ] || { echo "NOT_RUN: set OMEGA_MV and OMEGA_BRIDGE_DIR"; exit 3; }
	@n=0; for m in $(CHIP_ADMIT_OM_MUTANTS); do n=$$((n+1)); d=$(CHIP_ADMIT_DIR)/mutant$$n; rm -rf $$d; mkdir -p $$d; \
	sed "$$m" om/admission/chip_admit.om > $$d/chip_admit.om; \
	if cmp -s $$d/chip_admit.om om/admission/chip_admit.om; then echo "MUTANT $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	OMEGA_UNITS=$$(pwd)/$$d/chip_admit.om OMEGA_UNITS_DEST=$$d OMEGA_OMC=$(OMEGA_MV)/seed/out/omc \
		OMEGA_BRIDGE=$(OMEGA_BRIDGE_DIR) sh om/units.sh > /dev/null || exit 1; \
	$(CHIP_ADMIT_BUILD) -o $$d/test tests/test_chip_admission.c $(CHIP_ADMIT_SRCS) $(CHIP_ADMIT_OTHER_UNITS) \
		$$d/cudapp_om_chip_admit.c || exit 1; \
	if TMPDIR=$$(pwd)/$$d $$d/test > $$d/test.log 2>&1; then echo "MUTANT $$n SURVIVED: '$$m'"; exit 1; fi; \
	echo "mutant $$n red ('$$m'): $$(grep -m1 -e 'CHIP ADMIT DIFF' -e FAIL $$d/test.log)"; \
	done; \
	for m in $(CHIP_ADMIT_C_MUTANTS); do n=$$((n+1)); d=$(CHIP_ADMIT_DIR)/mutant$$n; rm -rf $$d; mkdir -p $$d; \
	sed "$$m" nvrm/nvrm.c > $$d/nvrm.c; \
	if cmp -s $$d/nvrm.c nvrm/nvrm.c; then echo "MUTANT $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	$(CHIP_ADMIT_BUILD) "-DNVRM_C=\"$$(pwd)/$$d/nvrm.c\"" -o $$d/test tests/test_chip_admission.c $(CHIP_ADMIT_SRCS) \
		$(CUDAPP_BINDING_SRCS) || exit 1; \
	if TMPDIR=$$(pwd)/$$d $$d/test > $$d/test.log 2>&1; then echo "MUTANT $$n SURVIVED: '$$m'"; exit 1; fi; \
	echo "mutant $$n red ('$$m'): $$(grep -m1 -e 'CHIP ADMIT DIFF' -e FAIL $$d/test.log)"; \
	done; echo "chip admission mutants: $$n of $$n red"

# The runtime API version a program linked (omega_runtime_api_version, API 0.14.0), host only. hello_gpu --sim must print
# the manifest's MAJOR.MINOR for both the library and the program; the negative control builds hello_gpu against a header
# one MINOR ahead of the library, which must stop with exit 2 and its FAIL sentence before anything runs.
.PHONY: test-runtime-api-version
CARRIED_HOST_TESTS += test-runtime-api-version
RUNTIME_VERSION_DIR = $(OUT_DIR)/runtime-api-version
test-runtime-api-version: $(OUT_DIR)/hello_gpu $(OUT_DIR)/libomega_gpu.a
	@mkdir -p $(RUNTIME_VERSION_DIR)/ahead
	@v=$$(sed -n 's/^abi omega_runtime \([0-9]*\)\.\([0-9]*\)\.[0-9]*$$/\1.\2/p' $(RUNTIME_API_MANIFEST)); \
	[ -n "$$v" ] || { echo "FAIL: no abi line in $(RUNTIME_API_MANIFEST)"; exit 1; }; \
	nm $(OUT_DIR)/libomega_gpu.a | grep -q ' T omega_runtime_api_version$$' || { echo "FAIL: libomega_gpu.a has no omega_runtime_api_version"; exit 1; }; \
	./$(OUT_DIR)/hello_gpu --sim > $(RUNTIME_VERSION_DIR)/hello.log 2>&1 || { cat $(RUNTIME_VERSION_DIR)/hello.log; exit 1; }; \
	grep -qx "runtime API $$v (library), program built against $$v" $(RUNTIME_VERSION_DIR)/hello.log || \
		{ echo "FAIL: hello_gpu did not report runtime API $$v for library and program"; exit 1; }; \
	minor=$$(sed -n 's/^#define OMEGA_RUNTIME_API_VERSION_MINOR \([0-9]*\)$$/\1/p' src/omega_runtime_api.h); \
	sed "s/^#define OMEGA_RUNTIME_API_VERSION_MINOR $$minor$$/#define OMEGA_RUNTIME_API_VERSION_MINOR $$((minor+1))/" \
		src/omega_runtime_api.h > $(RUNTIME_VERSION_DIR)/ahead/omega_runtime_api.h; \
	cmp -s src/omega_runtime_api.h $(RUNTIME_VERSION_DIR)/ahead/omega_runtime_api.h && { echo "FAIL: control header unchanged"; exit 1; }; \
	$(CC) -I$(RUNTIME_VERSION_DIR)/ahead $(CFLAGS) -o $(RUNTIME_VERSION_DIR)/hello_ahead examples/hello_gpu/hello_gpu.c \
		$(OUT_DIR)/libomega_gpu.a -lpthread -lm || exit 1; \
	./$(RUNTIME_VERSION_DIR)/hello_ahead --sim > $(RUNTIME_VERSION_DIR)/ahead.log 2>&1; rc=$$?; \
	if [ $$rc -eq 2 ] && grep -q '^FAIL: the linked CUDA++ library has runtime API ' $(RUNTIME_VERSION_DIR)/ahead.log \
		&& ! grep -q '^kernel built' $(RUNTIME_VERSION_DIR)/ahead.log; then \
		echo "NEGATIVE CONTROL program one MINOR ahead of the library: RED as it must (exit 2)"; \
	else cat $(RUNTIME_VERSION_DIR)/ahead.log; echo "NEGATIVE CONTROL not caught (exit $$rc)"; exit 1; fi; \
	echo "runtime API version tests passed: library and program $$v (HOST_ONLY, --sim)"
