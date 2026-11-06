# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Blackwell QMD packing in seeded Omega (om/qmd/bw_qmd.om).
#   make test-om-qmd          cited constants, field boundaries, 10000 random cases, source-derived partition
#   make test-om-qmd-diff     same calls packed by .om and by the C oracle; abort on any difference
#   make test-om-qmd-red      one flipped output word must abort
#   make test-om-qmd-cov      gcov line and branch report of the C oracle under the differential sweep
#   make test-om-qmd-mutants  recompiled bw_qmd.om mutants must fail the sweep (needs OMEGA_OMC, OMEGA_BRIDGE)
# Host only. No device.
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

# Engine decisions in seeded Omega (om/engine/eng.om), differential against the C oracles.
#   make test-om-eng-diff     om build and C build (every CUDAPP_MIGRATION_PENDING_C_<PART>) give identical transcripts
#   make test-om-eng-red      one flipped unit answer must make the transcripts differ
#   make test-om-eng-mutants  recompiled eng.om mutants must fail the differential or the backend test
#                             (needs OMEGA_OMC, OMEGA_BRIDGE and the eng_c transcript of test-om-eng-diff)
OM_ENG_C_PARTS = $(foreach p,ENGINE WAIT MEMRESV MEMCACHE SERVING M16NATIVE M16WAIT PLATFORM SUBMIT BWENGINE REQUALIFY CONCURRENT VISIBILITY LIFECYCLE ENGINE_FIXTURE_REST CLOCK_SCALARS,-DCUDAPP_MIGRATION_PENDING_C_$(p))
OM_ENG_SRCS = tests/om_eng_diff_test.c src/omega_gpu_engine.c src/omega_gpu_wait.c src/omega_gpu_platform.c \
	src/omega_gpu_mem_reserve.c src/omega_gpu_mem_cached.c src/omega_gpu_serving.c src/omega_gpu_platform_linux.c \
	src/omega_gpu_fmt.c src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c m16/m16_gpu_wait.c m16/m16_native.c nvrm/nvrm.c
OM_ENG_UNIT ?= src/cudapp_om_eng.c
OM_ENG_BUILD = gcc -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -Isrc $(GPU_ENGINE_PHYS_INC) -Wl,--wrap=omega_eng
OM_ENG_DEPS = $(OM_ENG_SRCS) $(OM_ENG_UNIT) src/omega_eng_dec.h src/omega_eng_gates.h src/omega_fixture_rest.h

.PHONY: test-om-eng-diff test-om-eng-red
$(OM_QMD_DIR)/eng_om: $(OM_ENG_DEPS)
	@mkdir -p $(OM_QMD_DIR)
	$(OM_ENG_BUILD) -o $@ $(OM_ENG_SRCS) $(OM_ENG_UNIT)
$(OM_QMD_DIR)/eng_c: $(OM_ENG_DEPS)
	@mkdir -p $(OM_QMD_DIR)
	$(OM_ENG_BUILD) $(OM_ENG_C_PARTS) -DENG_EXPECT_C -o $@ $(OM_ENG_SRCS) $(OM_ENG_UNIT)
$(OM_QMD_DIR)/eng_flip: $(OM_ENG_DEPS)
	@mkdir -p $(OM_QMD_DIR)
	$(OM_ENG_BUILD) -DENG_TRACE_FLIP=5000 -o $@ $(OM_ENG_SRCS) $(OM_ENG_UNIT)
test-om-eng-diff: $(OM_QMD_DIR)/eng_om $(OM_QMD_DIR)/eng_c
	$(OM_QMD_DIR)/eng_om $(OM_QMD_DIR)/eng_om.txt 2>$(OM_QMD_DIR)/eng_om.ops
	$(OM_QMD_DIR)/eng_c $(OM_QMD_DIR)/eng_c.txt 2>$(OM_QMD_DIR)/eng_c.ops
	$(OM_QMD_DIR)/eng_om --compare $(OM_QMD_DIR)/eng_om.txt $(OM_QMD_DIR)/eng_c.txt
test-om-eng-red: $(OM_QMD_DIR)/eng_flip $(OM_QMD_DIR)/eng_c
	$(OM_QMD_DIR)/eng_flip $(OM_QMD_DIR)/eng_flip.txt 2>/dev/null
	$(OM_QMD_DIR)/eng_c $(OM_QMD_DIR)/eng_c.txt 2>/dev/null
	@rc=0; $(OM_QMD_DIR)/eng_om --compare $(OM_QMD_DIR)/eng_flip.txt $(OM_QMD_DIR)/eng_c.txt || rc=$$?; \
	if [ "$$rc" != 1 ]; then echo "FAIL flipped unit answer was not detected (rc=$$rc)"; exit 1; fi; \
	echo "PASS red: flipped unit answer detected"

.PHONY: test-om-eng-mutants test-om-qmd-mutants
test-om-eng-mutants: $(OM_QMD_DIR)/eng_c
	$(OM_QMD_DIR)/eng_c $(OM_QMD_DIR)/eng_c.txt 2>/dev/null
	OM_ENG_BUILD_LINE='$(OM_ENG_BUILD) $(OM_ENG_SRCS)' sh tests/om_eng_mutants.sh
test-om-qmd-mutants:
	sh tests/om_qmd_mutants.sh

# gcov of the C QMD oracle (src/omega_blackwell_qmd.c) under the differential sweep.
.PHONY: test-om-qmd-cov
test-om-qmd-cov: tests/om_qmd_sweep_test.c $(OM_QMD_DEPS)
	@rm -rf $(OM_QMD_DIR)/cov && mkdir -p $(OM_QMD_DIR)/cov
	cd $(OM_QMD_DIR)/cov && gcc -std=gnu11 -O0 --coverage -Wall -Wextra -Werror -I$(CURDIR)/src $(OM_QMD_DIFF) -o sweep_cov \
		$(CURDIR)/tests/om_qmd_sweep_test.c $(CURDIR)/src/omega_blackwell_qmd.c $(CURDIR)/src/cudapp_om_bw_qmd.c
	cd $(OM_QMD_DIR)/cov && ./sweep_cov && gcov -b -o . $$(ls *omega_blackwell_qmd.gcno) | tee gcov.txt | grep -A4 "omega_blackwell_qmd.c'"

CARRIED_HOST_TESTS += test-engine-fixture-rest
.PHONY: test-engine-fixture-rest
test-engine-fixture-rest:
	python3 tests/om_fixture_rest_test.py
