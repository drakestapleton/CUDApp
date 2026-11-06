# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CUB/Thrust row and bar C3: reductions, prefix sums (f32, u32) and the stable radix sort (keys, pairs), whose kernels
# (om/scan/scan_ir.om) and launch plans (om/scan/scan_plan.om) are seeded Omega through their generated units. The C
# generator and level drivers in src/omega_gpu_scan.c are migration-pending: compiled only with
# CUDAPP_MIGRATION_PENDING_C_SCAN (C primary) or CUDAPP_MIGRATION_PENDING_SCAN_DIFF (both, abort on any difference).
#   make test-gpu-scan                 (mk/c3.mk) host tier and warp-simulator tier of build/gpu_scan_test
#   make test-gpu-scan-diff            the same simulator tier with every kernel and plan built by both C and Omega;
#                                      the log must hold only SAME lines
#   make test-gpu-scan-diff-selftest   the diff build must abort when one Omega instruction word is flipped
#                                      (CUDAPP_SCAN_DIFF_TAMPER=codegen) and when one plan word is (=plan)
#   make test-gpu-scan-mutants         mutant .om sources, each compiled by the pinned omc, must turn the simulator
#                                      tier red (needs OMEGA_MV and OMEGA_BRIDGE_DIR; without them exit 3, NOT_RUN)
# gpu_scan_test links src/omega_numeric.c, whose host reference uses AArch64 FP instructions, so these join
# `make test` on AArch64 hosts only (docs/OMEGA_HOST_TESTS.md).
SCAN_SORT_DIR = $(OUT_DIR)/scan-sort
SCAN_SORT_DIFF = -DCUDAPP_MIGRATION_PENDING_SCAN_DIFF
SCAN_SORT_SRCS = tests/gpu_scan_test.c src/omega_numeric_reduce.c src/omega_numeric.c
SCAN_SORT_DEPS = $(SCAN_SORT_SRCS) tests/bw_warp_sim.h src/omega_gpu_scan.h src/omega_gpu_scan.c $(OUT_DIR)/libomega_gpu.a

ifneq ($(CUDAPP_HOST_IS_AARCH64),)
CARRIED_HOST_TESTS += test-gpu-scan test-gpu-scan-diff test-gpu-scan-diff-selftest
endif
.PHONY: test-gpu-scan-diff test-gpu-scan-diff-selftest test-gpu-scan-mutants

# src/omega_gpu_scan.c comes before the library, so its diff-mode object replaces the library's
$(SCAN_SORT_DIR)/gpu_scan_diff: $(SCAN_SORT_DEPS)
	@mkdir -p $(SCAN_SORT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -Itests $(SCAN_SORT_DIFF) -o $@ $(SCAN_SORT_SRCS) src/omega_gpu_scan.c \
		$(OUT_DIR)/libomega_gpu.a -lpthread -lm
$(SCAN_SORT_DIR)/gpu_scan_diff_red: $(SCAN_SORT_DEPS)
	@mkdir -p $(SCAN_SORT_DIR)
	$(CC) $(CFLAGS) -ffp-contract=off -Itests $(SCAN_SORT_DIFF) -DCUDAPP_SCAN_DIFF_SELFTEST -o $@ $(SCAN_SORT_SRCS) \
		src/omega_gpu_scan.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm

test-gpu-scan-diff: $(SCAN_SORT_DIR)/gpu_scan_diff
	@rm -f $(SCAN_SORT_DIR)/diff.log
	CUDAPP_SCAN_DIFF_LOG=$(SCAN_SORT_DIR)/diff.log $(SCAN_SORT_DIR)/gpu_scan_diff --sim --quick > $(SCAN_SORT_DIR)/diff-run.log
	@tail -1 $(SCAN_SORT_DIR)/diff-run.log | grep -q '^C3 scan sim: PASS' || { tail -5 $(SCAN_SORT_DIR)/diff-run.log; exit 1; }
	@same=$$(grep -c ' SAME$$' $(SCAN_SORT_DIR)/diff.log); other=$$(grep -vc ' SAME$$' $(SCAN_SORT_DIR)/diff.log); \
	prog=$$(grep -c '^codegen kind ' $(SCAN_SORT_DIR)/diff.log); plan=$$(grep -c '^plan op ' $(SCAN_SORT_DIR)/diff.log); \
	if [ "$$same" -eq 0 ] || [ "$$other" -ne 0 ] || [ "$$prog" -eq 0 ] || [ "$$plan" -eq 0 ]; then \
		echo "FAIL scan diff: $$same SAME ($$prog kernels, $$plan plans), $$other other lines"; exit 1; fi; \
	echo "scan diff: C and Omega identical on $$prog kernel builds and $$plan launch plans (kinds 0..4, ops 0..3); $$(tail -1 $(SCAN_SORT_DIR)/diff-run.log)"

test-gpu-scan-diff-selftest: $(SCAN_SORT_DIR)/gpu_scan_diff_red
	@for t in codegen plan; do \
	if CUDAPP_SCAN_DIFF_TAMPER=$$t CUDAPP_SCAN_DIFF_LOG=/dev/null $(SCAN_SORT_DIR)/gpu_scan_diff_red --host-only \
		> $(SCAN_SORT_DIR)/diff-red-$$t.log 2>&1; then echo "RED CHECK BROKEN: a flipped $$t word passed the scan diff"; exit 1; fi; \
	grep -q "^omega_gpu_scan differential: $$t .* differs between C and Omega" $(SCAN_SORT_DIR)/diff-red-$$t.log || \
		{ echo "RED CHECK BROKEN: the $$t tamper failed for another reason"; tail -3 $(SCAN_SORT_DIR)/diff-red-$$t.log; exit 1; }; \
	echo "red check: $$(grep -m1 'differs between C and Omega' $(SCAN_SORT_DIR)/diff-red-$$t.log)"; done

# Each mutant is one edit of a .om source, compiled by the pinned omc into its own unit; the simulator tier linked
# with it (the unit's object before the library) must fail.
SCAN_SORT_MUTANTS = 'scan_ir s/^shri ins m DIG X 28$$/shri ins m DIG X 27/' \
	'scan_ir s/^stg ins m A64 4 X$$/stg ins m A64 0 X/' \
	'scan_ir s/^shri ins m X X 24$$/shri ins m X X 25/' \
	'scan_ir s/^d = ulong 16$$/d = ulong 8/' \
	'scan_plan s/^k = shl sub 28 mul p 4 8$$/k = shl sub 28 mul p 3 8/' \
	'scan_plan s/^ret shr add nw 31 5$$/ret shr add nw 30 5/'
test-gpu-scan-mutants: $(OUT_DIR)/libomega_gpu.a
	@[ -x $(OMEGA_MV)/seed/out/omc ] && [ -f $(OMEGA_BRIDGE_DIR)/src/native.om ] || { echo "NOT_RUN: set OMEGA_MV and OMEGA_BRIDGE_DIR"; exit 3; }
	@n=0; for m in $(SCAN_SORT_MUTANTS); do n=$$((n+1)); u=$${m%% *}; e=$${m#* }; d=$(SCAN_SORT_DIR)/mutant$$n; \
	rm -rf $$d; mkdir -p $$d; sed "$$e" om/scan/$$u.om > $$d/$$u.om; \
	if cmp -s $$d/$$u.om om/scan/$$u.om; then echo "MUTANT $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	OMEGA_UNITS=$$(pwd)/$$d/$$u.om OMEGA_UNITS_DEST=$$d OMEGA_OMC=$(OMEGA_MV)/seed/out/omc \
		OMEGA_BRIDGE=$(OMEGA_BRIDGE_DIR) sh om/units.sh > /dev/null || exit 1; \
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $$d/gpu_scan_test $(SCAN_SORT_SRCS) $$d/cudapp_om_$$u.c \
		$(OUT_DIR)/libomega_gpu.a -lpthread -lm || exit 1; \
	if $$d/gpu_scan_test --sim --quick > $$d/sim.log 2>&1; then echo "MUTANT $$n SURVIVED: '$$m'"; exit 1; fi; \
	echo "mutant $$n red ('$$m'): $$(grep -m1 -e FAIL $$d/sim.log | cut -c1-200)"; \
	done
