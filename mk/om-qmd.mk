# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Blackwell QMD packing in seeded Omega (om/qmd/bw_qmd.om).
#   make test-om-qmd          cited constants, field boundaries, 10000 random cases
#   make test-om-qmd-diff     same calls packed by .om and by the C oracle; abort on any difference
#   make test-om-qmd-red      one flipped output word must abort
# Host only. No device. Mutants: tests/om_qmd_mutants.sh (needs OMEGA_OMC).
OM_QMD_DIR = $(OUT_DIR)/om-qmd
OM_QMD_BUILD = gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc
OM_QMD_DEPS = src/omega_blackwell_qmd.c src/omega_blackwell_qmd.h src/cudapp_om_bw_qmd.c src/cudapp_native.h
OM_QMD_DIFF = -DCUDAPP_MIGRATION_PENDING_QMD_DIFF

.PHONY: test-om-qmd test-om-qmd-diff test-om-qmd-red
$(OM_QMD_DIR)/sweep: tests/om_qmd_sweep_test.c $(OM_QMD_DEPS)
	@mkdir -p $(OM_QMD_DIR)
	$(OM_QMD_BUILD) -o $@ tests/om_qmd_sweep_test.c src/omega_blackwell_qmd.c src/cudapp_om_bw_qmd.c
$(OM_QMD_DIR)/sweep_diff: tests/om_qmd_sweep_test.c $(OM_QMD_DEPS)
	@mkdir -p $(OM_QMD_DIR)
	$(OM_QMD_BUILD) $(OM_QMD_DIFF) -o $@ tests/om_qmd_sweep_test.c src/omega_blackwell_qmd.c src/cudapp_om_bw_qmd.c
$(OM_QMD_DIR)/sweep_red: tests/om_qmd_sweep_test.c $(OM_QMD_DEPS)
	@mkdir -p $(OM_QMD_DIR)
	$(OM_QMD_BUILD) $(OM_QMD_DIFF) -DQMD_DIFF_FLIP -o $@ tests/om_qmd_sweep_test.c src/omega_blackwell_qmd.c src/cudapp_om_bw_qmd.c
test-om-qmd: $(OM_QMD_DIR)/sweep
	$(OM_QMD_DIR)/sweep
test-om-qmd-diff: $(OM_QMD_DIR)/sweep_diff
	$(OM_QMD_DIR)/sweep_diff
test-om-qmd-red: $(OM_QMD_DIR)/sweep_red
	@rc=0; $(OM_QMD_DIR)/sweep_red >/dev/null 2>$(OM_QMD_DIR)/red.log || rc=$$?; \
	if [ "$$rc" = 0 ]; then echo "FAIL red sweep exited 0"; exit 1; fi; \
	grep -q "QMD DIFF" $(OM_QMD_DIR)/red.log; echo "PASS red sweep aborted rc=$$rc"
