# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Opt-in because the pinned language/checker source is a separate repository.
# Host only; does not compile a device launcher or open /dev/nvidia*.
C1_LANG ?= $(OMEGA_C1_SOURCE)
C1_DIR = $(OUT_DIR)/c1
C1_FLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -ffp-contract=off -Isrc -Itests -I$(C1_LANG)/src -I$(C1_LANG)/src/compiler -include src/omega_blackwell_codegen.h
C1_COMPILER = $(filter-out $(C1_LANG)/src/compiler/oscc_main.c,$(wildcard $(C1_LANG)/src/compiler/*.c)) $(C1_LANG)/src/omega_om_units.c
C1_FOREIGN_SRCS = $(C1_LANG)/src/omega_osc_gpu.c $(C1_LANG)/src/omega_osc_launch_args.c $(C1_COMPILER)
C1_FOREIGN_OBJS = $(patsubst $(C1_LANG)/src/%.c,$(C1_DIR)/foreign/%.o,$(C1_FOREIGN_SRCS))
C1_SRCS = src/omega_blackwell_codegen.c src/omega_blackwell_matmul.c src/sha256.c $(CUDAPP_BINDING_SRCS)
C1_HEADERS = src/omega_blackwell_codegen.h src/omega_cubin.h tests/bw_warp_sim.h $(wildcard tests/bw_warp_sim_k.h)
# Isolate the retained oracle library's native exports. Encoding/packaging still
# resolve the unrenamed CUDApp exports, never its duplicate historical encoder.
C1_FOREIGN_NAMES = -Domega_bw_enc_sm121=c1_mv_bw_enc_sm121 -Dsha256_init=c1_mv_sha256_init -Dsha256_update=c1_mv_sha256_update -Dsha256_final=c1_mv_sha256_final -Dsha256_hash=c1_mv_sha256_hash
.PHONY: test-c1 test-c1-predicate c1-source-check
c1-source-check:
	@sh om/c1/source-check.sh "$(C1_LANG)"
$(C1_DIR)/foreign/%.o: $(C1_LANG)/src/%.c src/omega_blackwell_codegen.h | c1-source-check
	@mkdir -p $(@D)
	$(CC) $(C1_FLAGS) $(C1_FOREIGN_NAMES) -c $< -o $@
$(C1_DIR)/sim: om/c1/sim.c $(C1_SRCS) $(C1_FOREIGN_OBJS) $(C1_HEADERS) | c1-source-check
	@mkdir -p $(C1_DIR)
	$(CC) $(C1_FLAGS) -DC1_TEST_SOURCE='"$(abspath $(C1_LANG))/tests/osc_gpu/test_osc_gpu.c"' -o $@ om/c1/sim.c src/omega_cubin.c $(C1_SRCS) $(C1_FOREIGN_OBJS) -lm
$(C1_DIR)/oracle: $(C1_SRCS) $(C1_FOREIGN_OBJS) $(C1_HEADERS) | c1-source-check
	@mkdir -p $(C1_DIR)
	$(CC) $(C1_FLAGS) -o $@ $(C1_LANG)/tests/osc_gpu/om_oracle.c $(C1_SRCS) $(C1_FOREIGN_OBJS) -lm
$(C1_DIR)/predicate: om/c1/predicate.c $(C1_HEADERS)
	@mkdir -p $(C1_DIR)
	$(CC) $(C1_FLAGS) -Wno-unused-function -o $@ om/c1/predicate.c -lm
test-c1-predicate: $(C1_DIR)/predicate
	$(C1_DIR)/predicate

test-c1: c1-source-check
	$(MAKE) $(C1_DIR)/sim $(C1_DIR)/oracle test-c1-predicate
	$(MAKE) -C "$(C1_LANG)" OUT_DIR=build -j3 test-osc-gpu test-checker2-om-bounds
	cd "$(C1_LANG)" && for name in vadd gemm attn; do build/checker2/shared-bounds/checker2.om.exe check "build/osc_gpu/ir/kernel_om_$$name.ir" || exit $$?; done
	cd "$(C1_LANG)" && sh tests/osc_gpu/om_diff.sh seed/out/omc "$(abspath $(C1_DIR))/oracle" "$(abspath $(C1_DIR))/sim" build/osc_gpu/ir "$(abspath $(C1_DIR))/om"
	$(C1_DIR)/sim "$(C1_LANG)/build/osc_gpu/ir" "$(abspath $(C1_DIR))/om/om"