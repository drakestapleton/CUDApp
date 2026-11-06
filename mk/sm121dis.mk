# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# C6 of docs/OMEGA.md: Omega's own sm_121 disassembler (om/disasm/sm121_dis.om through src/cudapp_om_sm121_dis.c;
# the migration-pending C oracle src/omega_sm121_disasm.c; the command line tools/sm121dis/).
#   make test-sm121dis   the nvdisasm oracle target (not in `make test`): collect every kernel the library builds (no
#                        GPU), compare our text with nvdisasm's, word by word; then the same kernels through the C
#                        oracle build, which must print the same text as the .om build; then the red check: a decoder
#                        built from a mutant .om (first source register off by one bit) must FAIL the nvdisasm comparison.
#                        Needs nvdisasm and the pinned OMEGA_OMC and OMEGA_BRIDGE (the mutant binding).
# Picked up by the `-include mk/*.mk` line of the Makefile. The test binaries are named as files, not through the
# GPU_*_TEST variables: this file is read before the Makefile defines them. Recipes read CUDAPP_DISASM_DEFS late.
SM121DIS = $(OUT_DIR)/sm121dis
SM121DIS_RED = $(OUT_DIR)/sm121dis_red
SM121DIS_K = $(OUT_DIR)/sm121dis-kernels

SM121DIS_C = $(OUT_DIR)/sm121dis_c
SM121DIS_DEPS = tools/sm121dis/sm121dis.c src/omega_sm121_disasm.c src/omega_sm121_disasm.h src/cudapp_om_sm121_dis.c \
	src/cudapp_native.h
SM121DIS_BUILD = $(CC) -std=gnu11 -Wall -Wextra -Werror -O2 -Isrc
SM121DIS_RED_DIR = $(OUT_DIR)/om-disasm/red
# The red decoder's fault, ported from the former C mutant: field a (first source register) prints one bit off.
SM121DIS_RED_SED = 's/^if or or eq c 100 eq c 97 or eq c 98 eq c 99$$/if and eq c 97 ne v 255\nret preg o p 0 xor v 1\nend\n&/'

$(SM121DIS): $(SM121DIS_DEPS)
	@mkdir -p $(OUT_DIR)
	$(SM121DIS_BUILD) $(CUDAPP_DISASM_DEFS) -o $@ tools/sm121dis/sm121dis.c src/omega_sm121_disasm.c src/cudapp_om_sm121_dis.c
$(SM121DIS_C): $(SM121DIS_DEPS)
	@mkdir -p $(OUT_DIR)
	$(SM121DIS_BUILD) -DCUDAPP_MIGRATION_PENDING_C_DISASM -o $@ tools/sm121dis/sm121dis.c src/omega_sm121_disasm.c
$(SM121DIS_RED): $(SM121DIS_DEPS) om/disasm/sm121_dis.om om/units.sh
	@[ -x "$(OMEGA_OMC)" ] && [ -f "$(OMEGA_BRIDGE)/src/toc.om" ] || { echo "NOT_RUN: set pinned OMEGA_OMC and OMEGA_BRIDGE"; exit 3; }
	@rm -rf $(SM121DIS_RED_DIR) && mkdir -p $(SM121DIS_RED_DIR)
	sed $(SM121DIS_RED_SED) om/disasm/sm121_dis.om > $(SM121DIS_RED_DIR)/sm121_dis.om
	@if cmp -s $(SM121DIS_RED_DIR)/sm121_dis.om om/disasm/sm121_dis.om; then echo "RED DECODER BROKEN: the mutant changed nothing"; exit 1; fi
	OMEGA_UNITS=$$(pwd)/$(SM121DIS_RED_DIR)/sm121_dis.om OMEGA_UNITS_DEST=$(SM121DIS_RED_DIR) OMEGA_OMC="$(OMEGA_OMC)" \
		OMEGA_BRIDGE="$(OMEGA_BRIDGE)" sh om/units.sh > /dev/null
	$(SM121DIS_BUILD) -o $@ tools/sm121dis/sm121dis.c src/omega_sm121_disasm.c $(SM121DIS_RED_DIR)/cudapp_om_sm121_dis.c

sm121dis-corpus-bins: $(addprefix $(OUT_DIR)/,gpu_matmul_api_test gpu_elementwise_test gpu_reconv_test gpu_attention_test \
	gpu_moe_window_test gpu_fp8_api_test gpu_decode_attn_test gpu_router_test gpu_bf16_gemv_test gpu_fp8gemm_test \
	gpu_tc_probe_test gpu_tc_peak_test gpu_pf_mem_test gpu_qmma_probe_test gpu_cmdlist_test gpu_decode_graph_test gpu_tcgemm_test \
	gpu_prefill_attn_test gpu_pfgemm_test gpu_qgemm_test gpu_pfhost_test gpu_gpr_boundary_test \
	gpu_scan_test gpu_gemm_test gpu_streams_test gpu_isolation_test bw_hazard_test gpu_mathlibs_test)

.PHONY: test-sm121dis sm121dis-corpus-bins test-sm121dis-corpus-controls test-sm121dis-check-controls
test-sm121dis-corpus-controls:
	sh tools/sm121dis/corpus_selftest.sh

test-sm121dis-check-controls:
	sh tools/sm121dis/check_selftest.sh

test-sm121dis: $(SM121DIS) $(SM121DIS_C) $(SM121DIS_RED) sm121dis-corpus-bins test-sm121dis-corpus-controls test-sm121dis-check-controls
	rm -rf $(SM121DIS_K) && sh tools/sm121dis/corpus.sh $(OUT_DIR) $(SM121DIS_K)
	sh tools/sm121dis/check.sh $(SM121DIS) $(SM121DIS_K) $(OUT_DIR)/sm121dis-work
	sh tools/sm121dis/om_vs_c.sh $(SM121DIS) $(SM121DIS_C) $(SM121DIS_K) $(OUT_DIR)/sm121dis-om-vs-c
	@if sh tools/sm121dis/check.sh $(SM121DIS_RED) $(SM121DIS_K) $(OUT_DIR)/sm121dis-red > $(OUT_DIR)/sm121dis-red.log 2>&1; then \
		echo "RED CHECK BROKEN: the faulty decoder passed"; exit 1; \
	elif ! grep -q '^VERDICT FAIL' $(OUT_DIR)/sm121dis-red.log; then \
		echo "RED CHECK BROKEN: the faulty decoder's run did not reach a verdict"; tail -3 $(OUT_DIR)/sm121dis-red.log; exit 1; \
	else echo "red check: the faulty decoder FAILS the comparison, as it must ($$(grep -c '^\[FAIL\]' $(OUT_DIR)/sm121dis-red.log) kernels)"; fi

# The provenance checks (tools/divsqrt_nvdisasm_check.sh, tools/ldst_nvdisasm_check.sh, test-numeric-reduce-nvdisasm)
# call build/sm121dis by that name, whatever OUT_DIR is.
ifneq ($(OUT_DIR),build)
build/sm121dis: $(SM121DIS_DEPS)
	@mkdir -p build
	$(SM121DIS_BUILD) $(CUDAPP_DISASM_DEFS) -o $@ tools/sm121dis/sm121dis.c src/omega_sm121_disasm.c src/cudapp_om_sm121_dis.c
endif

# ZERO-NVDISASM (bar C6, second half: nothing calls nvdisasm). tools/sm121dis/zero_nvdisasm.sh runs every listing and
# provenance check with a decoy nvdisasm first on PATH, the CUDA directories removed from PATH, and strace on execve;
# then its own red check plants two calls to nvdisasm and must see the gate FAIL.
.PHONY: test-zero-nvdisasm
test-zero-nvdisasm: $(SM121DIS) build/sm121dis $(addprefix $(OUT_DIR)/,gpu_attention_test gpu_elementwise_test gpu_fp8_api_test gpu_reconv_test bw_hazard_test)
	PHYSICS_DIR="$(PHYSICS_DIR)" sh tools/sm121dis/zero_nvdisasm.sh $(OUT_DIR)
	@if PHYSICS_DIR="$(PHYSICS_DIR)" sh tools/sm121dis/zero_nvdisasm.sh $(OUT_DIR) --red > $(OUT_DIR)/zero-nvdisasm-red.log 2>&1; then \
		echo "RED CHECK BROKEN: the planted nvdisasm calls passed the gate"; exit 1; \
	else echo "red check: the planted nvdisasm calls FAIL the gate, as they must ($$(grep -c 'FAIL$$' $(OUT_DIR)/zero-nvdisasm-red.log) lines)"; fi

# Issue #171: the semantic fixture and expectation live in Omega. NVIDIA's
# pinned offline decoder is read-only; neither output nor the mutant enters a kernel.
NVDISASM ?= /usr/local/cuda/bin/nvdisasm
SM121_MMIO = $(OUT_DIR)/sm121-mmio
$(SM121_MMIO): om/encoder/bw_enc_sm121.om om/encoder/mmio_test.om
	@mkdir -p $(OUT_DIR)
	@test "$$(sha256sum < $(OMEGA_MV)/seed/out/omc | cut -c1-64)" = 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f || { echo "NOT_RUN: wrong seeded compiler"; exit 3; }
	cat $^ > $@.om
	$(OMEGA_MV)/seed/out/omc < $@.om > $@
	chmod +x $@

.PHONY: test-sm121-mmio
test-sm121-mmio: $(SM121_MMIO)
	@$(NVDISASM) --version > $(SM121_MMIO).version
	@case "$$(cat $(SM121_MMIO).version)" in *V13.0.85*) ;; *) echo "NOT_RUN: nvdisasm version must be 13.0.85"; exit 3;; esac
	@sha256sum $(NVDISASM)
	$(SM121_MMIO) > $(SM121_MMIO).bin
	$(NVDISASM) -b SM121 -hex $(SM121_MMIO).bin > $(SM121_MMIO).nv
	$(SM121_MMIO) check oracle < $(SM121_MMIO).nv
	$(SM121_MMIO) wrong-cache-policy > $(SM121_MMIO)-ef.bin
	$(NVDISASM) -b SM121 -hex $(SM121_MMIO)-ef.bin > $(SM121_MMIO)-ef.nv
	@if $(SM121_MMIO) check oracle < $(SM121_MMIO)-ef.nv; then \
		echo "FAIL: EF satisfied the MMIO.GPU contract"; exit 1; \
	else rc=$$?; test "$$rc" = 4 || exit "$$rc"; \
		echo "PASS: exact MMIO.GPU op and operands; wrong-EF negative refused. GB10 DEVICE NOT_RUN"; fi
