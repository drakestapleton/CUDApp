# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# C4 streams (CUDApp#115): the method words of a cmdlist submission (setup, wait_for ACQUIRE, barrier choice, stamps,
# PCAS or inline QMD headers, completion tail), the submit_pair marker layout, AIEN_OMEGA_GPU_CHANNELS and the extra
# queue admission come from seeded Omega, om/streams/streams.om, through the generated src/cudapp_om_streams.c.
# The C rules are migration-pending oracles (CUDAPP_MIGRATION_PENDING_C_STREAMS, CUDAPP_MIGRATION_PENDING_STREAMS_DIFF).
#   make test-om-streams           (in `make test`, no Omega tools needed)
#       1. gpu_streams_test --host-only / --sim and gpu_isolation_test --host-only on the Omega path (the CI tiers);
#       2. tests/streams_diff_test.c built with CUDAPP_MIGRATION_PENDING_STREAMS_DIFF (the host and sim tiers never
#          submit a cmdlist, so a sweep drives emit_chunk): every chunk configuration, both pair markers, channel
#          values and extra queue indexes are computed both ways and abort on any difference; only SAME log lines;
#       3. red check: the diff build with one flipped Omega word must abort.
#   make test-om-streams-mutants   mutant streams.om units, each compiled by the pinned omc, must turn the diff red
#       (needs OMEGA_MV and OMEGA_BRIDGE_DIR as in mk/om-encoder.mk; without them exit 3, NOT_RUN, never PASS).
OM_STREAMS_DIR = $(OUT_DIR)/om-streams
OM_STREAMS_DIFF = -DCUDAPP_MIGRATION_PENDING_STREAMS_DIFF
OM_STREAMS_SRCS = src/omega_gpu_cmdlist.c src/omega_gpu_session.c src/omega_gpu_platform_linux.c
OM_STREAMS_HDRS = src/omega_gpu_cmdlist.h src/omega_gpu_session.h src/cudapp_native.h tests/bw_warp_sim.h src/omega_bw_em.h
# $(1) output, $(2) test source, $(3) extra flags, $(4) binding source: the diff-built streams sources go ahead of the
# archive, so its own copies of those objects are never pulled in.
om_streams_link = $(CC) $(CFLAGS) $(OM_STREAMS_DIFF) $(3) -ffp-contract=off -Itests -o $(1) $(2) $(OM_STREAMS_SRCS) $(4) \
	$(OUT_DIR)/libomega_gpu.a -lpthread -lm

.PHONY: test-om-streams test-om-streams-mutants
$(OM_STREAMS_DIR)/streams_diff: tests/streams_diff_test.c $(OM_STREAMS_SRCS) $(OM_STREAMS_HDRS) src/cudapp_om_streams.c $(OUT_DIR)/libomega_gpu.a
	@mkdir -p $(OM_STREAMS_DIR)
	$(call om_streams_link,$@,tests/streams_diff_test.c,,)
$(OM_STREAMS_DIR)/streams_diff_red: tests/streams_diff_test.c $(OM_STREAMS_SRCS) $(OM_STREAMS_HDRS) src/cudapp_om_streams.c $(OUT_DIR)/libomega_gpu.a
	@mkdir -p $(OM_STREAMS_DIR)
	$(call om_streams_link,$@,tests/streams_diff_test.c,-DCUDAPP_STREAMS_DIFF_SELFTEST,)

test-om-streams: $(GPU_STREAMS_TEST) $(GPU_ISOLATION_TEST) $(OM_STREAMS_DIR)/streams_diff $(OM_STREAMS_DIR)/streams_diff_red
	./$(GPU_STREAMS_TEST) --host-only
	./$(GPU_STREAMS_TEST) --sim
	./$(GPU_ISOLATION_TEST) --host-only
	@rm -f $(OM_STREAMS_DIR)/diff.log
	CUDAPP_STREAMS_DIFF_LOG=$(OM_STREAMS_DIR)/diff.log ./$(OM_STREAMS_DIR)/streams_diff
	@other=$$(grep -vc '^SAME ' $(OM_STREAMS_DIR)/diff.log || true); \
	if [ "$$other" -ne 0 ]; then echo "FAIL streams diff: $$other non-SAME lines"; exit 1; fi; \
	echo "streams diff: Omega and C identical on $$(grep -c '^SAME chunk' $(OM_STREAMS_DIR)/diff.log) chunks ($$(grep '^SAME chunk' $(OM_STREAMS_DIR)/diff.log | sed 's/.*words=//' | awk '{s+=$$1} END {print s}') words), $$(grep -c '^SAME pair' $(OM_STREAMS_DIR)/diff.log) pair markers"
	@if ./$(OM_STREAMS_DIR)/streams_diff_red > /dev/null 2>&1; then \
		echo "RED CHECK BROKEN: a flipped Omega word passed the streams diff"; exit 1; \
	else echo "red check: one flipped Omega word aborts the streams diff, as it must"; fi

# Each mutant is one edit of streams.om (1-4) or xq.om (5-7), compiled by the pinned omc into its own binding; the diff
# build linked with it must abort. 1 ACQUIRE method offset, 2 ACQUIRE payload, 3 completion marker payload, 4 pair
# marker stride, 5 extra-queue index bound, 6 queue 2 order check, 7 queue 3 order check.
OM_STREAMS_MUTANTS = 's/^k = pw o 0 mthd 0 92 5$$/k = pw o 0 mthd 0 96 5/' \
	's/^const DONE ulong 6$$/const DONE ulong 7/' \
	's/^const MARKER1 ulong 1145324612$$/const MARKER1 ulong 1145324613/' \
	's/^ret add add get a 0 256 mul k 128$$/ret add add get a 0 256 mul k 64/'
OM_STREAMS_XQ_MUTANTS = 's/^if or eq i 0 gt i 3$$/if or eq i 0 gt i 2/' \
	's/^if eq q1 0$$/if ne q1 0/' \
	's/^if or eq q1 0 eq q2 0$$/if eq q1 0/'
test-om-streams-mutants: $(OUT_DIR)/libomega_gpu.a
	@[ -x $(OMEGA_MV)/seed/out/omc ] && [ -f $(OMEGA_BRIDGE_DIR)/src/native.om ] || { echo "NOT_RUN: set OMEGA_MV and OMEGA_BRIDGE_DIR"; exit 3; }
	@n=0; for m in $(OM_STREAMS_MUTANTS); do n=$$((n+1)); d=$(OM_STREAMS_DIR)/mutant$$n; rm -rf $$d; mkdir -p $$d; \
	sed "$$m" om/streams/streams.om > $$d/streams.om; \
	if cmp -s $$d/streams.om om/streams/streams.om; then echo "MUTANT $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	OMEGA_UNITS=$$(pwd)/$$d/streams.om OMEGA_UNITS_DEST=$$d OMEGA_OMC=$(OMEGA_MV)/seed/out/omc \
		OMEGA_BRIDGE=$(OMEGA_BRIDGE_DIR) sh om/units.sh > /dev/null || exit 1; \
	$(call om_streams_link,$$d/streams,tests/streams_diff_test.c,,$$d/cudapp_om_streams.c) || exit 1; \
	if $$d/streams > $$d/streams.log 2>&1; then echo "MUTANT $$n SURVIVED: '$$m'"; exit 1; fi; \
	echo "mutant $$n red ('$$m'): $$(grep -m1 -e 'streams diff' -e FAIL $$d/streams.log)"; \
	done; \
	for m in $(OM_STREAMS_XQ_MUTANTS); do n=$$((n+1)); d=$(OM_STREAMS_DIR)/mutant$$n; rm -rf $$d; mkdir -p $$d; \
	sed "$$m" om/streams/xq.om > $$d/xq.om; \
	if cmp -s $$d/xq.om om/streams/xq.om; then echo "MUTANT $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	OMEGA_SEEDED_OMC=$(OMEGA_MV)/seed/out/omc sh $(OMEGA_BRIDGE_DIR)/tools/bridge-freest c $$d/xq.om \
		> $$d/cudapp_freest_streams_xq.c || exit 1; \
	$(call om_streams_link,$$d/streams,tests/streams_diff_test.c,,$$d/cudapp_freest_streams_xq.c) || exit 1; \
	if $$d/streams > $$d/streams.log 2>&1; then echo "MUTANT $$n SURVIVED: '$$m'"; exit 1; fi; \
	echo "mutant $$n red ('$$m'): $$(grep -m1 -e 'streams diff' -e FAIL $$d/streams.log)"; \
	done
