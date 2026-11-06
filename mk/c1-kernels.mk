# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Opt-in because the pinned language/checker source is a separate repository.
# Host only; does not compile a device launcher or open /dev/nvidia*.
C1_LANG ?= $(OMEGA_C1_SOURCE)
C1_DIR = $(OUT_DIR)/c1
C1_FLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -ffp-contract=off -Isrc -Itests -I$(C1_LANG)/src -I$(C1_LANG)/src/compiler -include src/omega_blackwell_codegen.h
C1_COMPILER = $(filter-out $(C1_LANG)/src/compiler/oscc_main.c,$(wildcard $(C1_LANG)/src/compiler/*.c)) $(C1_LANG)/src/omega_om_units.c
C1_SRCS = $(C1_LANG)/src/omega_osc_gpu.c $(C1_LANG)/src/omega_osc_launch_args.c src/omega_blackwell_codegen.c src/omega_blackwell_matmul.c src/sha256.c $(C1_COMPILER) $(CUDAPP_BINDING_SRCS)
.PHONY: test-c1 c1-source-check
c1-source-check:
	@sh om/c1/source-check.sh "$(C1_LANG)"
$(C1_DIR)/sim: om/c1/sim.c $(C1_SRCS) src/omega_cubin.c | c1-source-check
	@mkdir -p $(C1_DIR)
	$(CC) $(C1_FLAGS) -DC1_TEST_SOURCE='"$(abspath $(C1_LANG))/tests/osc_gpu/test_osc_gpu.c"' -o $@ om/c1/sim.c src/omega_cubin.c $(C1_SRCS) -lm
$(C1_DIR)/oracle: $(C1_SRCS) | c1-source-check
	@mkdir -p $(C1_DIR)
	$(CC) $(C1_FLAGS) -o $@ $(C1_LANG)/tests/osc_gpu/om_oracle.c $(C1_SRCS) -lm

test-c1: c1-source-check
	$(MAKE) $(C1_DIR)/sim $(C1_DIR)/oracle
	$(MAKE) -C "$(C1_LANG)" OUT_DIR=build -j3 test-om-ports
	cd "$(C1_LANG)" && sh tests/osc_gpu/om_diff.sh seed/out/omc "$(abspath $(C1_DIR))/oracle" "$(abspath $(C1_DIR))/sim" build/osc_gpu/ir "$(abspath $(C1_DIR))/om"
	$(C1_DIR)/sim "$(C1_LANG)/build/osc_gpu/ir" "$(abspath $(C1_DIR))/om/om"