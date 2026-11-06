# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# The general matrix multiply omega_gpu_gemm_ex (src/omega_gpu_gemm_ex.h): any M, N, K >= 1, op N/T, leading dimensions,
# alpha/beta, strided batches, f32/bf16/f16/e4m3/e5m2 inputs, fp8 scales, over the C3 kernels of src/omega_gpu_gemm.c.
# The plan, routes, k parts, operand conversion, k-part sums, host recompute, epilogue and fp8 dequantization are seeded
# Omega (om/gemmex/*.om -> src/cudapp_om_gemm_ex_*.c by om/units.sh, CUDApp#144); src/omega_gpu_gemm_ex.c is their memory
# shell. MIGRATION-PENDING C, kept as the differential oracle: CUDAPP_MIGRATION_PENDING_GEMM_EX=1 builds the old C path
# instead (default 0, the Omega path); CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF=1 runs both on every call and aborts on any
# difference in return code, info or output bytes.
#   make test-gpu-gemm-ex / test-gpu-gemm-ex-sim   the host and simulator tiers on the library (the Omega path)
#   make test-gpu-gemm-ex-c                         the same two tiers on the C oracle
#   make test-gpu-gemm-ex-diff                      both tiers with every call run both ways (DIFF), plus its red check:
#                                                   one flipped bit of the C copy must abort
#   make test-gemm-ex-unfused                       no fused multiply-add in the C oracle, the shell or the units, plus its
#                                                   red check: the oracle built without -ffp-contract=off has one (AArch64)
#   make test-gemm-ex-om-num / test-gemm-ex-om-mutants   (not in `make test`: need OMEGA_MV and OMEGA_BRIDGE_DIR as in
#       mk/om-encoder.mk; without them exit 3, NOT_RUN, never PASS) the exhaustive number-format test of om/gemmex/num.om,
#       and mutant .om units, each compiled by the pinned omc, that must turn the host tier red.
# Picked up by the `-include mk/*.mk` line of the Makefile, after CFLAGS and before HOST_TESTS.
CUDAPP_MIGRATION_PENDING_GEMM_EX ?= 0
CFLAGS += -DCUDAPP_MIGRATION_PENDING_GEMM_EX=$(CUDAPP_MIGRATION_PENDING_GEMM_EX)
# The C oracle sums in double; a fused multiply-add rounds once where the C rounds twice, so contraction stays off.
$(OUT_DIR)/omega_gpu_gemm_ex.o: CFLAGS += -ffp-contract=off

GEMM_EX_DIR = $(OUT_DIR)/gemm-ex
GEMM_EX_TEST_DEPS = tests/gpu_gemm_ex_test.c tests/bw_warp_sim_tc.h tests/bw_hmma_model.h tests/bw_qmma_model.h \
	src/omega_gpu_gemm_ex.h src/omega_gpu_gemm.h
GEMM_EX_CFLAGS = $(filter-out -DCUDAPP_MIGRATION_PENDING_GEMM_EX=%,$(CFLAGS)) -ffp-contract=off -Itests
# a test binary over src/omega_gpu_gemm_ex.c built with the given defines; the archive supplies everything else
GEMM_EX_LINK = $(CC) $(GEMM_EX_CFLAGS) $(2) -o $(1) tests/gpu_gemm_ex_test.c src/omega_gpu_gemm_ex.c $(3) \
	$(OUT_DIR)/libomega_gpu.a -lpthread -lm

GPU_GEMM_EX_TEST = $(OUT_DIR)/gpu_gemm_ex_test
$(GPU_GEMM_EX_TEST): $(GEMM_EX_TEST_DEPS) $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -ffp-contract=off -Itests -o $@ tests/gpu_gemm_ex_test.c $(OUT_DIR)/libomega_gpu.a -lpthread -lm
$(GEMM_EX_DIR)/test_c: $(GEMM_EX_TEST_DEPS) src/omega_gpu_gemm_ex.c $(OUT_DIR)/libomega_gpu.a
	@mkdir -p $(GEMM_EX_DIR)
	$(call GEMM_EX_LINK,$@,-DCUDAPP_MIGRATION_PENDING_GEMM_EX=1)
$(GEMM_EX_DIR)/test_diff: $(GEMM_EX_TEST_DEPS) src/omega_gpu_gemm_ex.c $(OUT_DIR)/libomega_gpu.a
	@mkdir -p $(GEMM_EX_DIR)
	$(call GEMM_EX_LINK,$@,-DCUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF=1)
$(GEMM_EX_DIR)/test_diff_red: $(GEMM_EX_TEST_DEPS) src/omega_gpu_gemm_ex.c $(OUT_DIR)/libomega_gpu.a
	@mkdir -p $(GEMM_EX_DIR)
	$(call GEMM_EX_LINK,$@,-DCUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF=1 -DCUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF_SELFTEST)

# Host tiers: no device opened. The chip tier is the same binary with no flag, under the GB10 lock
# (flock -w 900 /tmp/aien-gb10.lock $(GPU_GEMM_EX_TEST) [--part I/N]); --list prints the predeclared sweep.
.PHONY: test-gpu-gemm-ex test-gpu-gemm-ex-sim test-gpu-gemm-ex-c test-gpu-gemm-ex-diff test-gemm-ex-unfused \
	test-gemm-ex-om-num test-gemm-ex-om-mutants
test-gpu-gemm-ex: $(GPU_GEMM_EX_TEST)
	./$(GPU_GEMM_EX_TEST) --host-only
test-gpu-gemm-ex-sim: $(GPU_GEMM_EX_TEST)
	./$(GPU_GEMM_EX_TEST) --sim
test-gpu-gemm-ex-c: $(GEMM_EX_DIR)/test_c
	$(GEMM_EX_DIR)/test_c --host-only
	$(GEMM_EX_DIR)/test_c --sim
test-gpu-gemm-ex-diff: $(GEMM_EX_DIR)/test_diff $(GEMM_EX_DIR)/test_diff_red
	$(GEMM_EX_DIR)/test_diff --host-only
	$(GEMM_EX_DIR)/test_diff --sim
	@if $(GEMM_EX_DIR)/test_diff_red --host-only > $(GEMM_EX_DIR)/diff_red.log 2>&1; then \
		echo "RED CHECK BROKEN: a flipped output bit passed the gemm_ex C-vs-Omega differential"; exit 1; fi; \
	grep -q 'FAIL gemm_ex C-vs-Omega differential: output bytes differ' $(GEMM_EX_DIR)/diff_red.log || \
		{ echo "RED CHECK BROKEN: the flipped bit failed without the differential's message"; exit 1; }; \
	echo "red check: one flipped output bit aborts the gemm_ex C-vs-Omega differential, as it must"

# fmadd/fmsub/fnmadd/fnmsub (AArch64), vfmadd.../vfnmsub... (x86)
GEMM_EX_FMA = '\bv?fn?m(add|sub)[0-9a-z]*\b'
test-gemm-ex-unfused: $(GEMM_EX_DIR)/test_c $(OUT_DIR)/libomega_gpu.a
	@mkdir -p $(GEMM_EX_DIR)
	$(CC) $(GEMM_EX_CFLAGS) -DCUDAPP_MIGRATION_PENDING_GEMM_EX=1 -c -o $(GEMM_EX_DIR)/oracle.o src/omega_gpu_gemm_ex.c
	@for o in $(GEMM_EX_DIR)/oracle.o $(OUT_DIR)/omega_gpu_gemm_ex.o $(patsubst src/%.c,$(OUT_DIR)/%.o,$(wildcard src/cudapp_om_gemm_ex_*.c)); do \
		[ -f $$o ] || { echo "FAIL gemm_ex unfused: $$o missing"; exit 1; }; \
		if objdump -d $$o | grep -Eq $(GEMM_EX_FMA); then echo "FAIL gemm_ex unfused: $$o holds a fused multiply-add"; \
			objdump -d $$o | grep -Em3 $(GEMM_EX_FMA); exit 1; fi; done; \
	echo "gemm_ex unfused: no fused multiply-add in the C oracle, the shell or the $(words $(wildcard src/cudapp_om_gemm_ex_*.c)) units"
	@if [ "$$(uname -m)" != aarch64 ]; then echo "NOT_RUN red check: no fused multiply-add in this host's base ISA"; exit 0; fi; \
	$(CC) $(filter-out -DCUDAPP_MIGRATION_PENDING_GEMM_EX=%,$(CFLAGS)) -DCUDAPP_MIGRATION_PENDING_GEMM_EX=1 -c \
		-o $(GEMM_EX_DIR)/oracle_fused.o src/omega_gpu_gemm_ex.c; \
	if objdump -d $(GEMM_EX_DIR)/oracle_fused.o | grep -Eq $(GEMM_EX_FMA); then \
		echo "red check: the oracle built without -ffp-contract=off holds a fused multiply-add, as the check must see"; \
	else echo "RED CHECK BROKEN: no fused multiply-add found even without -ffp-contract=off"; exit 1; fi

CARRIED_HOST_TESTS += test-gpu-gemm-ex test-gpu-gemm-ex-sim test-gpu-gemm-ex-c test-gpu-gemm-ex-diff test-gemm-ex-unfused

GEMM_EX_OMC = $(OMEGA_MV)/seed/out/omc
GEMM_EX_TOOLS = [ -x $(GEMM_EX_OMC) ] && [ -f $(OMEGA_BRIDGE_DIR)/src/native.om ] || \
	{ echo "NOT_RUN: set OMEGA_MV and OMEGA_BRIDGE_DIR"; exit 3; }
test-gemm-ex-om-num:
	@$(GEMM_EX_TOOLS)
	@mkdir -p $(GEMM_EX_DIR)
	cat om/gemmex/num.om om/gemmex/probe.om > $(GEMM_EX_DIR)/num_probe.om
	OMEGA_SEEDED_OMC=$(GEMM_EX_OMC) sh $(OMEGA_BRIDGE_DIR)/tools/bridge-seeded c $(GEMM_EX_DIR)/num_probe.om > $(GEMM_EX_DIR)/num_probe.c
	gcc -std=gnu11 -O2 -ffp-contract=off -o $(GEMM_EX_DIR)/num_test tests/gemm_ex_om_num_test.c $(GEMM_EX_DIR)/num_probe.c -lm
	GX_STEP=13 $(GEMM_EX_DIR)/num_test

# Each mutant: unit=sed-expression on that unit's last source (om/gemmex/<unit>.om); the unit is regenerated by the
# pinned omc into its own directory and linked into the host tier in place of the committed one, which must go red.
GEMM_EX_MUTANTS = \
	'plan|s/^if or gt get x 18 2 eq get x 28 0$$/if or gt get x 18 3 eq get x 28 0/' \
	'row|s/^ret shr b 16$$/ret shr b 15/' \
	'acc|s/^f = f64 bitcast f32 gx_get view s add ix 1 4$$/f = f64 bitcast f32 gx_get view s add ix 2 4/' \
	'dot|s/^s = add s mul x f$$/s = add s x/' \
	'epi|s/^if ne bt bitcast f64 ulong 0$$/if eq bt bitcast f64 ulong 0/' \
	'fp8|s/^x = mul x f64 bitcast f32 uint sv$$/x = mul x f64 bitcast f32 uint get q 7/'
test-gemm-ex-om-mutants: $(OUT_DIR)/libomega_gpu.a
	@$(GEMM_EX_TOOLS)
	@n=0; for mu in $(GEMM_EX_MUTANTS); do n=$$((n+1)); u=$${mu%%|*}; m=$${mu#*|}; d=$$(pwd)/$(GEMM_EX_DIR)/mutant$$n; \
	rm -rf $$d; mkdir -p $$d; sed "$$m" om/gemmex/$$u.om > $$d/$$u.om; \
	if cmp -s $$d/$$u.om om/gemmex/$$u.om; then echo "MUTANT $$n BROKEN: $$u '$$m' changed nothing"; exit 1; fi; \
	OMEGA_UNITS=gemm_ex_$$u=$$(pwd)/om/gemmex/num.om,$$d/$$u.om OMEGA_UNITS_DEST=$$d OMEGA_OMC=$(GEMM_EX_OMC) \
		OMEGA_BRIDGE=$(OMEGA_BRIDGE_DIR) sh om/units.sh > /dev/null 2>&1 || { echo "MUTANT $$n BROKEN: $$u does not compile"; exit 1; }; \
	$(call GEMM_EX_LINK,$$d/test,,$$d/cudapp_om_gemm_ex_$$u.c) || exit 1; \
	if $$d/test --host-only > $$d/test.log 2>&1; then echo "MUTANT $$n SURVIVED: $$u '$$m'"; exit 1; fi; \
	echo "mutant $$n red ($$u '$$m'): $$(grep -m1 -e FAIL -e 'trap\|abort' $$d/test.log)"; \
	done
