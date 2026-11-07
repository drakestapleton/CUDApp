# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CUDApp#79: the sm_121 instruction encoder in seeded Omega (om/encoder/bw_enc_sm121.om), linked through the
# portable generated C units $(CUDAPP_BINDING_SRCS) that om/units.sh generates. The C encoder in
# src/omega_blackwell_codegen.c is migration-pending: the freestanding build and the oracle builds only.
#   make test-om-encoder          (in `make test`, no Omega tools needed)
#       1. the golden-word fixture verifiers (nvcc/nvdisasm oracles) through the .om encoder;
#       2. a build with CUDAPP_MIGRATION_PENDING_ENCODER_DIFF encodes every codegen fixture both ways and must log
#          only SAME lines; 3. its red check: the same build with one flipped output bit must abort.
#   make test-om-encoder-corpus   every kernel of the sm121dis corpus (tools/sm121dis/corpus.sh) built three ways:
#       .om (OUT_DIR), C oracle ($(OUT_DIR)-enc-c), diff ($(OUT_DIR)-enc-diff); the .om and C kernel sets must be
#       identical file for file, and the diff build must log no DIFFER line.
#   make test-om-units            the committed binding regenerates byte for byte from the pinned omc + bridge
#   make test-om-encoder-mutants  three mutant .om encoders, each compiled by the pinned omc, must go red
#   make test-om-encoder-ldg      CUDApp#373: om/encoder/ldg_offset_test.om, composed after the encoder and compiled by
#       the pinned omc, checks the exact LDG.E words for byte offsets 0, +0x40, -0x100000, +0x7fffff, -0x800000 and
#       the refusal of +0x800000, -0x800001, 0x7fffffff, 0x80000000; ORACLE NVIDIA nvdisasm 13.0.85 (offline,
#       read-only, never shipped) must decode the accepted words as those offsets. Red checks: the pre-fix op-11
#       line (offset dropped), a field bound widened by one, and the oracle text without its -0x800000 line.
#   make test-om-encoder-atomg    op 64 ATOMG.E.MIN.64.STRONG.SYS (Omega NG trap word): om/encoder/atomg_min_test.om,
#       composed after the encoder and compiled by the pinned omc, checks the exact words of three rows (one with RZ
#       as destination); the nvdisasm 13.0.85 ORACLE must decode them as those instructions. Red checks: the signed
#       type (.S64), RZ dropped for an absent destination, and the oracle text without its R14 line.
#   make test-om-encoder-guard    DRA-483: om/encoder/guard_pred_test.om, the guard predicate (@P0 / @!P0) of every
#       op 0..64 through the seeded encoder, LDCU / LDCU.64 refused, the omega-mv #1044 bool/trap rows; ORACLE
#       nvdisasm 13.0.85 must print each guard; om/encoder/guard_spec_test.om: sp_enc equals enc1 on the guards.
#       Red checks: the guard ignored, forced to 7, @!P0 as @P0, the LDCU refusal removed (enc1 and sp_enc), oracle
#       lines missing.
# The last five accept OMEGA_OMC, units and mutants also OMEGA_BRIDGE; their defaults retain the
# OMEGA_MV and OMEGA_BRIDGE_DIR checkout convention. Missing pins refuse.
OMEGA_MV ?= ../omega-mv
OMEGA_BRIDGE_DIR ?= ../omega-bridge
OMEGA_OMC ?= $(OMEGA_MV)/seed/out/omc
OMEGA_BRIDGE ?= $(OMEGA_BRIDGE_DIR)
OM_ENC_DIR = $(OUT_DIR)/om-encoder
OM_ENC_TEST_DEPS = tests/test_omega_codegen_freestanding.c src/omega_blackwell_codegen.h src/omega_blackwell_matmul.h
OM_ENC_SRCS = src/omega_blackwell_codegen.c src/omega_blackwell_matmul.c src/sha256.c
OM_ENC_BUILD = gcc -std=gnu11 -O2 -Wall -Wextra -Werror -pthread -Isrc

.PHONY: test-om-encoder test-om-encoder-corpus test-om-units test-om-encoder-mutants
$(OM_ENC_DIR)/codegen_om: $(OM_ENC_TEST_DEPS) $(OM_ENC_SRCS) $(CUDAPP_BINDING_SRCS) src/cudapp_native.h
	@mkdir -p $(OM_ENC_DIR)
	$(OM_ENC_BUILD) -o $@ tests/test_omega_codegen_freestanding.c $(OM_ENC_SRCS) $(CUDAPP_BINDING_SRCS)
$(OM_ENC_DIR)/codegen_diff: $(OM_ENC_TEST_DEPS) $(OM_ENC_SRCS) $(CUDAPP_BINDING_SRCS) src/cudapp_native.h
	@mkdir -p $(OM_ENC_DIR)
	$(OM_ENC_BUILD) -DCUDAPP_MIGRATION_PENDING_ENCODER_DIFF -o $@ tests/test_omega_codegen_freestanding.c \
		$(OM_ENC_SRCS) $(CUDAPP_BINDING_SRCS)
$(OM_ENC_DIR)/codegen_diff_red: $(OM_ENC_TEST_DEPS) $(OM_ENC_SRCS) $(CUDAPP_BINDING_SRCS) src/cudapp_native.h
	@mkdir -p $(OM_ENC_DIR)
	$(OM_ENC_BUILD) -DCUDAPP_MIGRATION_PENDING_ENCODER_DIFF -DCUDAPP_ENCODER_DIFF_SELFTEST -o $@ \
		tests/test_omega_codegen_freestanding.c $(OM_ENC_SRCS) $(CUDAPP_BINDING_SRCS)

test-om-encoder: $(OM_ENC_DIR)/codegen_om $(OM_ENC_DIR)/codegen_diff $(OM_ENC_DIR)/codegen_diff_red
	$(OM_ENC_DIR)/codegen_om --golden
	rm -f $(OM_ENC_DIR)/diff.log
	CUDAPP_ENCODER_DIFF_LOG=$(OM_ENC_DIR)/diff.log $(OM_ENC_DIR)/codegen_diff --golden
	CUDAPP_ENCODER_DIFF_LOG=$(OM_ENC_DIR)/diff.log $(OM_ENC_DIR)/codegen_diff --dump > $(OM_ENC_DIR)/dump_diff.txt
	@same=$$(grep -c ' SAME$$' $(OM_ENC_DIR)/diff.log); other=$$(grep -vc ' SAME$$' $(OM_ENC_DIR)/diff.log); \
	if [ "$$same" -eq 0 ] || [ "$$other" -ne 0 ]; then echo "FAIL encoder diff: $$same SAME, $$other other lines"; exit 1; fi; \
	echo "encoder diff: .om and C byte-identical on $$(grep -c '^insn ' $(OM_ENC_DIR)/diff.log) golden instructions and $$(grep -c '^program ' $(OM_ENC_DIR)/diff.log) fixture programs"
	@if CUDAPP_ENCODER_DIFF_LOG=$(OM_ENC_DIR)/diff_red.log $(OM_ENC_DIR)/codegen_diff_red --dump > /dev/null 2>&1; then \
		echo "RED CHECK BROKEN: a flipped bit passed the encoder diff"; exit 1; \
	else echo "red check: one flipped bit aborts the encoder diff, as it must"; fi

test-om-encoder-corpus:
	$(MAKE) OUT_DIR=$(OUT_DIR) CUDAPP_ENCODER=om sm121dis-corpus-bins
	$(MAKE) OUT_DIR=$(OUT_DIR)-enc-c CUDAPP_ENCODER=c sm121dis-corpus-bins
	$(MAKE) OUT_DIR=$(OUT_DIR)-enc-diff CUDAPP_ENCODER=diff sm121dis-corpus-bins
	rm -rf $(OM_ENC_DIR)/k-om $(OM_ENC_DIR)/k-c $(OM_ENC_DIR)/k-diff $(OM_ENC_DIR)/corpus-diff.log
	@mkdir -p $(OM_ENC_DIR)
	sh tools/sm121dis/corpus.sh $(OUT_DIR) $(OM_ENC_DIR)/k-om
	sh tools/sm121dis/corpus.sh $(OUT_DIR)-enc-c $(OM_ENC_DIR)/k-c
	CUDAPP_ENCODER_DIFF_LOG=$(abspath $(OM_ENC_DIR))/corpus-diff.log sh tools/sm121dis/corpus.sh $(OUT_DIR)-enc-diff $(OM_ENC_DIR)/k-diff
	diff -r $(OM_ENC_DIR)/k-om $(OM_ENC_DIR)/k-c
	diff -r $(OM_ENC_DIR)/k-om $(OM_ENC_DIR)/k-diff
	@if grep -v ' SAME$$' $(OM_ENC_DIR)/corpus-diff.log; then echo "FAIL corpus encoder diff"; exit 1; fi
	@echo "encoder corpus: $$(ls $(OM_ENC_DIR)/k-om/*.bin | wc -l) distinct kernels identical .om vs C; diff build $$(grep -c '^program ' $(OM_ENC_DIR)/corpus-diff.log) program encodes and $$(grep -c '^insn ' $(OM_ENC_DIR)/corpus-diff.log) single-instruction encodes, all SAME"

.PHONY: test-om-spec-encoder
ifeq ($(CUDAPP_OMC_HOST),1)
# On a non-AArch64 host the native AArch64 function-byte probe cannot execute; the script exits 3 (NOT_RUN) for
# it unless told the row is accepted as NOT_RUN, which this target does there and the log says so.
test-om-spec-encoder:
	OMEGA_BRIDGE="$(OMEGA_BRIDGE)" $(if $(CUDAPP_HOST_IS_AARCH64),,BW_SPEC_NATIVE_PROBE_NOT_RUN_OK=1) sh tools/test_bw_spec_encoder.sh
else
test-om-spec-encoder:
	@echo "test-om-spec-encoder: NOT_RUN (requires the pinned AArch64 omc and nvdisasm; set OMEGA_OMC to a pin that executes on this host; not part of the portable host suite)"
	@exit 2
endif

test-om-units:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" sh om/units.sh --check

# Each mutant is one edit of the .om source, compiled by the pinned omc into its own binding; the hosted
# fixture test linked with it must fail the golden words or differ from the C-encoded freestanding dump.
OM_ENC_MUTANTS = 's/^ret or base shl rb o 1 16$$/ret or base shl rb o 1 17/' \
	's/^p = 262144$$/p = 0/' \
	's/^const NOPC ulong 1032192$$/const NOPC ulong 1032193/'
test-om-encoder-mutants: build/test_codegen_freestanding
	@[ -x "$(OMEGA_OMC)" ] && [ -f "$(OMEGA_BRIDGE)/src/toc.om" ] && [ -f "$(OMEGA_BRIDGE)/src/freest.om" ] || { echo "NOT_RUN: set pinned OMEGA_OMC and OMEGA_BRIDGE"; exit 3; }
	@mkdir -p $(OM_ENC_DIR)
	./build/test_codegen_freestanding --dump > $(OM_ENC_DIR)/c-dump.txt
	@n=0; for m in $(OM_ENC_MUTANTS); do n=$$((n+1)); d=$(OM_ENC_DIR)/mutant$$n; rm -rf $$d; mkdir -p $$d; \
	sed "$$m" om/encoder/bw_enc_sm121.om > $$d/bw_enc_sm121.om; \
	if cmp -s $$d/bw_enc_sm121.om om/encoder/bw_enc_sm121.om; then echo "MUTANT $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	OMEGA_UNITS=$$(pwd)/$$d/bw_enc_sm121.om OMEGA_UNITS_DEST=$$d OMEGA_OMC="$(OMEGA_OMC)" \
		OMEGA_BRIDGE="$(OMEGA_BRIDGE)" sh om/units.sh > /dev/null || exit 1; \
	$(OM_ENC_BUILD) -o $$d/codegen tests/test_omega_codegen_freestanding.c $(OM_ENC_SRCS) $$d/cudapp_om_bw_enc_sm121.c || exit 1; \
	g=0; $$d/codegen --golden > $$d/golden.log 2>&1 || g=1; \
	c=0; $$d/codegen --dump > $$d/dump.txt 2>&1; cmp -s $$d/dump.txt $(OM_ENC_DIR)/c-dump.txt || c=1; \
	if [ $$g = 0 ] && [ $$c = 0 ]; then echo "MUTANT $$n SURVIVED: '$$m'"; exit 1; fi; \
	echo "mutant $$n red ('$$m'): golden words $$( [ $$g = 1 ] && echo FAIL || echo pass ), dump vs C $$( [ $$c = 1 ] && echo DIFFERS || echo same )"; \
	done

NVDISASM ?= /usr/local/cuda/bin/nvdisasm
OM_ENC_LDG = $(OM_ENC_DIR)/ldg-offset
# Red checks: each pair is the exit the gate must give, then one edit of the encoder. 11: the pre-fix op-11 line
# drops the offset (row 1, +0x40). 20: a field bound widened by one accepts +0x800000 (refused row 0).
OM_ENC_LDG_RED = 11 's/^put4 w wda o 31105 or rb o 5 shl and imm 16777215 8 203299072 ctl o 991744$$/put4 w wda o 31105 rb o 5 203299072 ctl o 991744/' \
	20 's/^ret or lt imm 8388608 ge imm 4286578688$$/ret or lt imm 8388609 ge imm 4286578687/'
.PHONY: test-om-encoder-ldg
test-om-encoder-ldg: om/encoder/bw_enc_sm121.om om/encoder/ldg_offset_test.om
	@[ -x "$(OMEGA_OMC)" ] && [ "$$(sha256sum < $(OMEGA_OMC) | cut -c1-64)" = 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f ] || { echo "NOT_RUN: set OMEGA_OMC to the pinned seeded compiler"; exit 3; }
	@case "$$($(NVDISASM) --version 2>/dev/null)" in *V13.0.85*) ;; *) echo "NOT_RUN: nvdisasm version must be 13.0.85"; exit 3;; esac
	@mkdir -p $(OM_ENC_DIR)
	cat om/encoder/bw_enc_sm121.om om/encoder/ldg_offset_test.om > $(OM_ENC_LDG).om
	$(OMEGA_OMC) < $(OM_ENC_LDG).om > $(OM_ENC_LDG)
	chmod +x $(OM_ENC_LDG)
	$(OM_ENC_LDG)
	$(OM_ENC_LDG) bin > $(OM_ENC_LDG).bin
	$(NVDISASM) -b SM121 -hex $(OM_ENC_LDG).bin > $(OM_ENC_LDG).nv
	$(OM_ENC_LDG) check oracle < $(OM_ENC_LDG).nv
	@set -- $(OM_ENC_LDG_RED); n=0; while [ $$# -gt 0 ]; do want=$$1; m=$$2; shift 2; n=$$((n+1)); d=$(OM_ENC_LDG)-red$$n; \
	sed "$$m" om/encoder/bw_enc_sm121.om > $$d.om; \
	if cmp -s $$d.om om/encoder/bw_enc_sm121.om; then echo "RED CHECK $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	cat $$d.om om/encoder/ldg_offset_test.om | $(OMEGA_OMC) > $$d || exit 1; \
	chmod +x $$d; rc=0; $$d || rc=$$?; \
	if [ $$rc != $$want ]; then echo "RED CHECK $$n BROKEN: '$$m' exited $$rc, not $$want"; exit 1; fi; \
	echo "red check $$n: '$$m' exits $$rc, as it must"; done
	@rc=0; grep -v -- '-0x800000' $(OM_ENC_LDG).nv | $(OM_ENC_LDG) check oracle || rc=$$?; \
	if [ $$rc != 4 ]; then echo "RED CHECK BROKEN: oracle text without -0x800000 exited $$rc, not 4"; exit 1; fi; \
	echo "red check: oracle text without the -0x800000 line exits 4, as it must"
	@echo "PASS LDG.E offset: 5 exact words, 4 refusals, nvdisasm 13.0.85 decodes all 5. GB10 DEVICE NOT_RUN"

OM_ENC_ATOMG = $(OM_ENC_DIR)/atomg-min
# Red checks as for test-om-encoder-ldg. 10: type 2 -> 3 encodes the signed .S64 min (row 0). 11: the destination
# field ignores its presence flag, so an absent destination is not RZ (row 1).
OM_ENC_ATOMG_RED = 10 's/^put4 w a or rb o 3 2147483648 or 144659712 rb o 5 ctl o 3284480$$/put4 w a or rb o 3 2147483648 or 144660224 rb o 5 ctl o 3284480/' \
	11 's/^a = or or 31144 shl rz o 1 15 16 shl rb o 2 24$$/a = or or 31144 shl rb o 1 16 shl rb o 2 24/'
.PHONY: test-om-encoder-atomg
test-om-encoder-atomg: om/encoder/bw_enc_sm121.om om/encoder/atomg_min_test.om
	@[ -x "$(OMEGA_OMC)" ] && [ "$$(sha256sum < $(OMEGA_OMC) | cut -c1-64)" = 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f ] || { echo "NOT_RUN: set OMEGA_OMC to the pinned seeded compiler"; exit 3; }
	@case "$$($(NVDISASM) --version 2>/dev/null)" in *V13.0.85*) ;; *) echo "NOT_RUN: nvdisasm version must be 13.0.85"; exit 3;; esac
	@mkdir -p $(OM_ENC_DIR)
	cat om/encoder/bw_enc_sm121.om om/encoder/atomg_min_test.om > $(OM_ENC_ATOMG).om
	$(OMEGA_OMC) < $(OM_ENC_ATOMG).om > $(OM_ENC_ATOMG)
	chmod +x $(OM_ENC_ATOMG)
	$(OM_ENC_ATOMG)
	$(OM_ENC_ATOMG) bin > $(OM_ENC_ATOMG).bin
	$(NVDISASM) -b SM121 -hex $(OM_ENC_ATOMG).bin > $(OM_ENC_ATOMG).nv
	$(OM_ENC_ATOMG) check oracle < $(OM_ENC_ATOMG).nv
	@set -- $(OM_ENC_ATOMG_RED); n=0; while [ $$# -gt 0 ]; do want=$$1; m=$$2; shift 2; n=$$((n+1)); d=$(OM_ENC_ATOMG)-red$$n; \
	sed "$$m" om/encoder/bw_enc_sm121.om > $$d.om; \
	if cmp -s $$d.om om/encoder/bw_enc_sm121.om; then echo "RED CHECK $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	cat $$d.om om/encoder/atomg_min_test.om | $(OMEGA_OMC) > $$d || exit 1; \
	chmod +x $$d; rc=0; $$d || rc=$$?; \
	if [ $$rc != $$want ]; then echo "RED CHECK $$n BROKEN: '$$m' exited $$rc, not $$want"; exit 1; fi; \
	echo "red check $$n: '$$m' exits $$rc, as it must"; done
	@rc=0; grep -v -- 'R14' $(OM_ENC_ATOMG).nv | $(OM_ENC_ATOMG) check oracle || rc=$$?; \
	if [ $$rc != 4 ]; then echo "RED CHECK BROKEN: oracle text without the R14 line exited $$rc, not 4"; exit 1; fi; \
	echo "red check: oracle text without the R14 line exits 4, as it must"
	@echo "PASS ATOMG.E.MIN.64.STRONG.SYS: 3 exact rows, nvdisasm 13.0.85 decodes all 3. GB10 DEVICE NOT_RUN"


OM_ENC_GUARD = $(OM_ENC_DIR)/guard-pred
# DRA-483 (C1b): om/encoder/guard_pred_test.om, composed after the encoder and compiled by the pinned omc. Every op
# 0..64 encoded @P0 and @!P0 must carry guard 0 / 8 in w0 bits 12..15 and equal the unpredicated words otherwise;
# LDCU / LDCU.64 (uniform datapath, guard bits name UP0..UPT) must refuse both forms; the omega-mv #1044 bool and
# trap rows through the program entry must get guards 7, 0, 8, 8, 8. ORACLE NVIDIA nvdisasm 13.0.85 (offline,
# read-only, never shipped) must print @P0 / @!P0 on the 126 P-guarded words, @UP0 / @!UP0 on the 4 LDCU probe
# words and the five #1044 guards. Then om/encoder/guard_spec_test.om: the spec encoder sp_enc (bc_encode's path)
# must equal enc1 on every op 0..59 unpredicated, @P0 and @!P0.
# Red checks: each triple is the file edited, the failure count the gate must print (exit 1), then one edit.
# enc 121: the guard left as each op encodes it (the pre-fix behaviour except the LDCU refusal). enc 130: the guard
# forced to 7 (always) again. enc 66: @!P0 encoded as @P0. enc 4: LDCU / LDCU.64 accept a P guard. spec 106: sp_enc
# without the guard. spec 4: sp_enc accepts a P guard on LDCU / LDCU.64. Oracle red: no @!UP0 lines exits 4.
OM_ENC_GUARD_RED = enc 121 's/^put w 0 or sub x and x 61440 shl pred o 12$$/put w 0 x/' \
	enc 130 's/^put w 0 or sub x and x 61440 shl pred o 12$$/put w 0 or sub x and x 61440 shl ulong 7 12/' \
	enc 66 's/^put w 0 or sub x and x 61440 shl pred o 12$$/put w 0 or sub x and x 61440 shl and pred o 7 12/' \
	enc 4 's/^if isuld op$$/if eq op 99/' \
	spec 106 's/^put w 0 or sub gx and gx 61440 shl sp_val R_PRED 0 o 12$$/put w 0 gx/' \
	spec 4 's/^if or eq get o 0 6 eq get o 0 7$$/if eq get o 0 99/'
.PHONY: test-om-encoder-guard
test-om-encoder-guard: om/encoder/bw_enc_sm121.om om/encoder/bw_spec_sm121.om om/encoder/guard_pred_test.om \
		om/encoder/guard_spec_test.om
	@[ -x "$(OMEGA_OMC)" ] && [ "$$(sha256sum < $(OMEGA_OMC) | cut -c1-64)" = 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f ] || { echo "NOT_RUN: set OMEGA_OMC to the pinned seeded compiler"; exit 3; }
	@case "$$($(NVDISASM) --version 2>/dev/null)" in *V13.0.85*) ;; *) echo "NOT_RUN: nvdisasm version must be 13.0.85"; exit 3;; esac
	@mkdir -p $(OM_ENC_DIR)
	cat om/encoder/bw_enc_sm121.om om/encoder/guard_pred_test.om > $(OM_ENC_GUARD).om
	$(OMEGA_OMC) < $(OM_ENC_GUARD).om > $(OM_ENC_GUARD)
	chmod +x $(OM_ENC_GUARD)
	$(OM_ENC_GUARD)
	$(OM_ENC_GUARD) bin > $(OM_ENC_GUARD).bin
	$(NVDISASM) -b SM121 -hex $(OM_ENC_GUARD).bin > $(OM_ENC_GUARD).nv
	$(OM_ENC_GUARD) check oracle < $(OM_ENC_GUARD).nv
	cat om/encoder/bw_enc_sm121.om om/encoder/bw_spec_sm121.om om/encoder/guard_spec_test.om > $(OM_ENC_GUARD)-spec.om
	$(OMEGA_OMC) < $(OM_ENC_GUARD)-spec.om > $(OM_ENC_GUARD)-spec
	chmod +x $(OM_ENC_GUARD)-spec
	$(OM_ENC_GUARD)-spec
	@set -- $(OM_ENC_GUARD_RED); n=0; while [ $$# -gt 0 ]; do f=$$1; want=$$2; m=$$3; shift 3; n=$$((n+1)); d=$(OM_ENC_GUARD)-red$$n; \
	if [ $$f = enc ]; then src=om/encoder/bw_enc_sm121.om; else src=om/encoder/bw_spec_sm121.om; fi; \
	sed "$$m" $$src > $$d.om; \
	if cmp -s $$d.om $$src; then echo "RED CHECK $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	if [ $$f = enc ]; then cat $$d.om om/encoder/guard_pred_test.om; \
	else cat om/encoder/bw_enc_sm121.om $$d.om om/encoder/guard_spec_test.om; fi | $(OMEGA_OMC) > $$d || exit 1; \
	chmod +x $$d; rc=0; $$d > $$d.txt || rc=$$?; got=$$(tail -n 1 $$d.txt); \
	if [ $$f = enc ]; then exp="guard predicate failures: $$want"; else exp="spec guard mismatches: $$want"; fi; \
	if [ $$rc != 1 ] || [ "$$got" != "$$exp" ]; then echo "RED CHECK $$n BROKEN: '$$m' exited $$rc, '$$got', not 1 with '$$exp'"; exit 1; fi; \
	echo "red check $$n ($$f): '$$m' exits 1 with $$want failures, as it must"; done
	@rc=0; grep -v -- '@!UP0' $(OM_ENC_GUARD).nv | $(OM_ENC_GUARD) check oracle > /dev/null || rc=$$?; \
	if [ $$rc != 4 ]; then echo "RED CHECK BROKEN: oracle text without @!UP0 exited $$rc, not 4"; exit 1; fi; \
	echo "red check: oracle text without the @!UP0 lines exits 4, as it must"
	@echo "PASS guard predicates: 63 ops x @P0/@!P0 exact guard, LDCU/LDCU.64 refused, #1044 bool/trap rows, nvdisasm 13.0.85 prints all 135 guards, sp_enc = enc1 on 180 forms. GB10 DEVICE NOT_RUN"
