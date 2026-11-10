# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CUDApp#79: the sm_121 instruction encoder in seeded Omega (om/encoder/bw_enc_sm121.om), linked through the
# portable generated C units $(CUDAPP_BINDING_SRCS) that om/units.sh generates. The C encoder in
# src/omega_blackwell_codegen.c is migration-pending: explicit default-OFF oracle builds only.
#   make test-om-encoder          (in `make test`, no Omega tools needed)
#       1. the golden-word fixture verifiers (nvcc/nvdisasm oracles) through the .om encoder;
#       2. a build with CUDAPP_MIGRATION_PENDING_ENCODER_DIFF encodes every codegen fixture both ways and must log
#          only SAME lines; 3. its red check: the same build with one flipped output bit must abort.
#   make test-om-encoder-corpus   every kernel of the sm121dis corpus (tools/sm121dis/corpus.sh) built three ways:
#       .om (OUT_DIR), C oracle ($(OUT_DIR)-enc-c), diff ($(OUT_DIR)-enc-diff); the .om and C kernel sets must be
#       identical file for file, and the diff build must log no DIFFER line.
#   make test-om-units            the committed binding regenerates byte for byte from the pinned omc + bridge
#   make test-om-encoder-mutants  three mutant .om encoders, each compiled by the pinned omc, must go red
#     (after its prerequisites test-om-encoder-ldg, test-om-encoder-ldg-spec and test-om-encoder-ldg-c, so the
#     CI om-units job, which already has the pinned omc, omega-bridge and nvdisasm 13.0.85, runs the LDG gates)
# The last two accept OMEGA_OMC and OMEGA_BRIDGE; their defaults retain the
# OMEGA_MV and OMEGA_BRIDGE_DIR checkout convention. Missing pins refuse.
OMEGA_MV ?= ../omega-mv
OMEGA_BRIDGE_DIR ?= ../omega-bridge
OMEGA_OMC ?= $(OMEGA_MV)/seed/out/omc
OMEGA_BRIDGE ?= $(OMEGA_BRIDGE_DIR)
OM_ENC_DIR = $(OUT_DIR)/om-encoder
OM_ENC_TEST_DEPS = tests/test_omega_codegen_freestanding.c src/omega_blackwell_codegen.h src/omega_blackwell_matmul.h
OM_ENC_SRCS = src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c src/omega_blackwell_matmul.c src/sha256.c
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

# The samples are read back by Omega's own decoder ($(OUT_DIR)/sm121dis, mk/sm121dis.mk, read after this file). The
# -nvdisasm-oracle variant reads them with NVIDIA's offline nvdisasm instead; it is not in `make test`.
.PHONY: test-om-spec-encoder test-om-spec-encoder-nvdisasm-oracle
ifeq ($(CUDAPP_OMC_HOST),1)
# On a non-AArch64 host the native AArch64 function-byte probe cannot execute; the script exits 3 (NOT_RUN) for
# it unless told the row is accepted as NOT_RUN, which this target does there and the log says so.
test-om-spec-encoder: $(OUT_DIR)/sm121dis
	OMEGA_BRIDGE="$(OMEGA_BRIDGE)" SM121DIS="$(OUT_DIR)/sm121dis" $(if $(CUDAPP_HOST_IS_AARCH64),,BW_SPEC_NATIVE_PROBE_NOT_RUN_OK=1) sh tools/test_bw_spec_encoder.sh
test-om-spec-encoder-nvdisasm-oracle:
	OMEGA_BRIDGE="$(OMEGA_BRIDGE)" SPEC_ENCODER_ORACLE=nvdisasm NVDISASM="$(NVDISASM)" $(if $(CUDAPP_HOST_IS_AARCH64),,BW_SPEC_NATIVE_PROBE_NOT_RUN_OK=1) sh tools/test_bw_spec_encoder.sh
else
test-om-spec-encoder test-om-spec-encoder-nvdisasm-oracle:
	@echo "$@: NOT_RUN (requires the pinned AArch64 omc; set OMEGA_OMC to a pin that executes on this host; not part of the portable host suite)"
	@exit 2
endif

test-om-units:
	OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" sh om/units.sh --check

# Each mutant is one edit of the .om source, compiled by the pinned omc into its own binding; the hosted
# fixture test linked with it must fail the golden words or differ from the C-encoded freestanding dump.
OM_ENC_MUTANTS = 's/^ret or base shl rb o 1 16$$/ret or base shl rb o 1 17/' \
	's/^p = 262144$$/p = 0/' \
	's/^const NOPC ulong 1032192$$/const NOPC ulong 1032193/'
test-om-encoder-mutants: build/test_codegen_freestanding test-om-encoder-ldg test-om-encoder-ldg-spec test-om-encoder-ldg-c
	@[ -x "$(OMEGA_OMC)" ] && [ -f "$(OMEGA_BRIDGE)/src/toc.om" ] && [ -f "$(OMEGA_BRIDGE)/src/freest.om" ] || { echo "NOT_RUN: set pinned OMEGA_OMC and OMEGA_BRIDGE"; exit 3; }
	@mkdir -p $(OM_ENC_DIR)
	./build/test_codegen_freestanding --dump > $(OM_ENC_DIR)/c-dump.txt
	@n=0; for m in $(OM_ENC_MUTANTS); do n=$$((n+1)); d=$(OM_ENC_DIR)/mutant$$n; rm -rf $$d; mkdir -p $$d; \
	sed "$$m" om/encoder/bw_enc_sm121.om > $$d/bw_enc_sm121.om; \
	if cmp -s $$d/bw_enc_sm121.om om/encoder/bw_enc_sm121.om; then echo "MUTANT $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	OMEGA_UNITS=$$(pwd)/$$d/bw_enc_sm121.om OMEGA_UNITS_DEST=$$d OMEGA_OMC="$(OMEGA_OMC)" \
		OMEGA_BRIDGE="$(OMEGA_BRIDGE)" sh om/units.sh > /dev/null || exit 1; \
	$(OM_ENC_BUILD) -o $$d/codegen tests/test_omega_codegen_freestanding.c $(OM_ENC_SRCS) \
		$(filter-out src/cudapp_om_bw_enc_sm121.c,$(CUDAPP_BINDING_SRCS)) $$d/cudapp_om_bw_enc_sm121.c || exit 1; \
	g=0; $$d/codegen --golden > $$d/golden.log 2>&1 || g=1; \
	c=0; $$d/codegen --dump > $$d/dump.txt 2>&1; cmp -s $$d/dump.txt $(OM_ENC_DIR)/c-dump.txt || c=1; \
	if [ $$g = 0 ] && [ $$c = 0 ]; then echo "MUTANT $$n SURVIVED: '$$m'"; exit 1; fi; \
	echo "mutant $$n red ('$$m'): golden words $$( [ $$g = 1 ] && echo FAIL || echo pass ), dump vs C $$( [ $$c = 1 ] && echo DIFFERS || echo same )"; \
	done
NVDISASM ?= /usr/local/cuda/bin/nvdisasm

.PHONY: test-om-encoder-ldg-c
$(OM_ENC_DIR)/ldg-c: gpu-engine-freestanding $(GPU_NO_POSIX_OBJS) tests/test_omega_codegen_freestanding.c src/omega_blackwell_codegen.c
	@mkdir -p $(OM_ENC_DIR)
	$(OM_ENC_BUILD) -DOMEGA_GPU_NO_POSIX -DCUDAPP_MIGRATION_PENDING_C_REGALLOC -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_MATMUL \
		-DCUDAPP_MIGRATION_PENDING_C_ENCODER -DCUDAPP_MIGRATION_PENDING_C_BIND -DCUDAPP_MIGRATION_PENDING_C_PACK -DCUDAPP_MIGRATION_PENDING_C_CORE \
		-I$(GPU_NO_POSIX_DIR) -o $@ tests/test_omega_codegen_freestanding.c src/omega_blackwell_codegen.c src/omega_blackwell_matmul.c \
		src/sha256.c src/cudapp_freest_f16.c src/cudapp_freest_bw_scalar.c src/cudapp_freest_fixture_verify.c $(GPU_NO_POSIX_OBJS)
test-om-encoder-ldg-c: $(OM_ENC_DIR)/ldg-c
	cat om/encoder/bw_enc_sm121.om om/encoder/ldg_mini.om > $(OM_ENC_DIR)/ldg-mini.om
	$(OMEGA_OMC) < $(OM_ENC_DIR)/ldg-mini.om > $(OM_ENC_DIR)/ldg-mini
	chmod +x $(OM_ENC_DIR)/ldg-mini
	$(OM_ENC_DIR)/ldg-mini > $(OM_ENC_DIR)/ldg-mini.tape
	$(OM_ENC_DIR)/ldg-mini expected > $(OM_ENC_DIR)/ldg-mini.om.bin
	$(OM_ENC_DIR)/ldg-c --encoder-tape < $(OM_ENC_DIR)/ldg-mini.tape > $(OM_ENC_DIR)/ldg-mini.c.bin
	cmp $(OM_ENC_DIR)/ldg-mini.om.bin $(OM_ENC_DIR)/ldg-mini.c.bin
	sed '$(OM_ENC_LDG_PREFIX_SED)' om/encoder/bw_enc_sm121.om > $(OM_ENC_DIR)/ldg-before.om
	@if cmp -s $(OM_ENC_DIR)/ldg-before.om om/encoder/bw_enc_sm121.om; then echo 'FAIL pre-fix LDG reversal changed nothing'; exit 1; fi
	cat om/encoder/ldg_mini.om >> $(OM_ENC_DIR)/ldg-before.om
	$(OMEGA_OMC) < $(OM_ENC_DIR)/ldg-before.om > $(OM_ENC_DIR)/ldg-before
	chmod +x $(OM_ENC_DIR)/ldg-before
	$(OM_ENC_DIR)/ldg-before expected > $(OM_ENC_DIR)/ldg-mini.before.bin
	@if cmp -s $(OM_ENC_DIR)/ldg-mini.before.bin $(OM_ENC_DIR)/ldg-mini.c.bin; then echo 'FAIL pre-fix LDG witness unexpectedly agrees'; exit 1; fi
	@echo 'PASS one-instruction LDG C parity; pre-fix seeded encoder differs; public Mesa field selects C as correct'
OM_ENC_LDG = $(OM_ENC_DIR)/ldg-offset
# The pre-fix op-11 line (base cb3395aa) differs from the fixed line only by the dropped offset term, so
# test-om-encoder-ldg-c and red check 11 rebuild it from the current source: neither needs Git history
# (shallow CI checkout). Red checks: each pair is the exit the gate must give, then one edit of the encoder.
# 11: the pre-fix op-11 line drops the offset (row 1, +0x40). 20: a field bound widened by one accepts
# +0x800000 (refused row 0).
OM_ENC_LDG_PREFIX_SED = s/^put4 w wda o 31105 or rb o 5 shl and imm 16777215 8 203299072 ctl o 991744$$/put4 w wda o 31105 rb o 5 203299072 ctl o 991744/
OM_ENC_LDG_RED = 11 '$(OM_ENC_LDG_PREFIX_SED)' \
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

.PHONY: test-om-encoder-ldg-spec
test-om-encoder-ldg-spec:
	@mkdir -p $(OM_ENC_DIR)
	cat om/encoder/bw_enc_sm121.om om/encoder/bw_spec_sm121.om om/encoder/ldg_spec_test.om > $(OM_ENC_DIR)/ldg-spec.om
	$(OMEGA_OMC) < $(OM_ENC_DIR)/ldg-spec.om > $(OM_ENC_DIR)/ldg-spec
	chmod +x $(OM_ENC_DIR)/ldg-spec
	$(OM_ENC_DIR)/ldg-spec
	@rc=0; $(OM_ENC_DIR)/ldg-spec red || rc=$$?; test $$rc = 1
	@echo 'PASS LDG spec: nine signed-offset/boundary cases equal primitive encoder; omitted-field control refused'

