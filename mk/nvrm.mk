# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# HOST_ONLY: vendor byte oracle and fake replies; no device nodes opened.
CUDAPP_MIGRATION_PENDING_C_NVRM ?= 0
CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_NVRM=$(CUDAPP_MIGRATION_PENDING_C_NVRM)
CUDAPP_MIGRATION_PENDING_C_NVRM_STRUCT ?= 0
CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_NVRM_STRUCT=$(CUDAPP_MIGRATION_PENDING_C_NVRM_STRUCT)
CUDAPP_MIGRATION_PENDING_C_NVRM_ALLOC ?= 0
CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_NVRM_ALLOC=$(CUDAPP_MIGRATION_PENDING_C_NVRM_ALLOC)
.PHONY: test-nvrm-wire test-nvrm-native test-nvrm-differential test-nvrm-boundary test-nvrm-policy
CARRIED_HOST_TESTS += test-nvrm-wire test-nvrm-native test-nvrm-differential test-nvrm-boundary test-nvrm-policy
NVRM_WIRE_SRCS = src/cudapp_om_nvrm_wire_pack.c src/cudapp_om_nvrm_wire_reply.c src/cudapp_om_nvrm_heap_wire.c
$(OUT_DIR)/nvrm-wire-test: nvrm/wire_test.c $(NVRM_WIRE_SRCS) src/cudapp_native.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ nvrm/wire_test.c $(NVRM_WIRE_SRCS)
$(OUT_DIR)/nvrm-wire-mutant: nvrm/wire_test.c $(NVRM_WIRE_SRCS) src/cudapp_native.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -DNVRM_WIRE_MUTANT=1 -o $@ nvrm/wire_test.c $(NVRM_WIRE_SRCS)
$(OUT_DIR)/nvrm-uvm-wire-test: nvrm/uvm_wire_test.c src/cudapp_om_nvrm_uvm_wire.c src/cudapp_om_nvrm_uvm_map_wire.c src/cudapp_native.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ nvrm/uvm_wire_test.c src/cudapp_om_nvrm_uvm_wire.c src/cudapp_om_nvrm_uvm_map_wire.c
test-nvrm-wire: $(OUT_DIR)/nvrm-wire-test $(OUT_DIR)/nvrm-wire-mutant $(OUT_DIR)/nvrm-uvm-wire-test
	./$(OUT_DIR)/nvrm-wire-test
	./$(OUT_DIR)/nvrm-uvm-wire-test
	@if ./$(OUT_DIR)/nvrm-wire-mutant; then echo "FAIL: malformed request serializer accepted"; exit 1; else echo "NVRM wire mutant caught"; fi

NVRM_ALLOC_SRCS = src/cudapp_om_nvrm_va.c src/cudapp_om_nvrm_counts.c src/cudapp_om_nvrm_alloc_scalar.c src/cudapp_om_nvrm_alloc_push.c
NVRM_SERIALIZER_SRCS = $(NVRM_WIRE_SRCS) src/cudapp_om_nvrm_uvm_wire.c src/cudapp_om_nvrm_uvm_map_wire.c src/cudapp_om_nvrm_struct.c $(NVRM_ALLOC_SRCS)
# nvrm_open admits the chip first (HelloC2 #296): omega_gpu_chip_admit_open/_card in src/omega_gpu_chip.c,
# whose decisions are the generated om/admission/chip_admit.om and chip_enum.om units. Every binary that links nvrm.c needs them.
NVRM_ADMIT_SRCS = src/omega_gpu_chip.c src/cudapp_om_chip_admit.c src/cudapp_om_chip_enum.c src/cudapp_om_nvrm_policy.c
$(OUT_DIR)/nvrm-native-test: nvrm/native_test.c nvrm/native_ioctl_fixture.h nvrm/native_client.h nvrm/nvrm.c nvrm/nvrm.h src/cudapp_nvrm_native.h src/cudapp_om_nvrm_transport.c src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c $(NVRM_SERIALIZER_SRCS) $(NVRM_ADMIT_SRCS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_NVRM -DCUDAPP_MIGRATION_PENDING_C_NVRM=0 -pthread -o $@ nvrm/native_test.c src/cudapp_om_nvrm_transport.c src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c $(NVRM_SERIALIZER_SRCS) $(NVRM_ADMIT_SRCS)
test-nvrm-native: $(OUT_DIR)/nvrm-native-test
	./$(OUT_DIR)/nvrm-native-test

$(OUT_DIR)/nvrm-c-oracle-test: tests/test_ampere_runtime.c nvrm/nvrm.c nvrm/nvrm.h nvrm/native_client.h $(AMPERE_RUNTIME_SRCS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_NVRM -DCUDAPP_MIGRATION_PENDING_C_NVRM=1 -UCUDAPP_MIGRATION_PENDING_C_NVRM_STRUCT -DCUDAPP_MIGRATION_PENDING_C_NVRM_STRUCT=1 -UCUDAPP_MIGRATION_PENDING_C_NVRM_ALLOC -DCUDAPP_MIGRATION_PENDING_C_NVRM_ALLOC=1 -pthread -o $@ tests/test_ampere_runtime.c $(AMPERE_RUNTIME_SRCS)
test-nvrm-differential: build/test_ampere_runtime $(OUT_DIR)/nvrm-c-oracle-test
	./build/test_ampere_runtime > $(OUT_DIR)/nvrm-omega-fixture.out
	./$(OUT_DIR)/nvrm-c-oracle-test > $(OUT_DIR)/nvrm-c-fixture.out
	cmp $(OUT_DIR)/nvrm-omega-fixture.out $(OUT_DIR)/nvrm-c-fixture.out
	@echo "NVRM C/OMEGA DIFFERENTIAL PASS (same fake RM lifecycle assertions and output; HOST_ONLY)"

$(OUT_DIR)/nvrm-boundary-test: nvrm/macro_boundary_test.c nvrm/native_client.h nvrm/nvrm.c nvrm/nvrm.h src/cudapp_nvrm_native.h src/cudapp_om_nvrm_transport.c src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c $(NVRM_SERIALIZER_SRCS) $(NVRM_ADMIT_SRCS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -UCUDAPP_MIGRATION_PENDING_C_NVRM -DCUDAPP_MIGRATION_PENDING_C_NVRM=0 -UCUDAPP_NVRM_TEST_TRANSPORT -pthread -o $@ nvrm/macro_boundary_test.c src/cudapp_om_nvrm_transport.c src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c $(NVRM_SERIALIZER_SRCS) $(NVRM_ADMIT_SRCS)
test-nvrm-boundary: $(OUT_DIR)/nvrm-boundary-test
	./$(OUT_DIR)/nvrm-boundary-test

$(OUT_DIR)/nvrm-policy-test: nvrm/policy_test.c src/cudapp_om_nvrm_policy.c src/cudapp_native.h nvrm/nvrm_policy.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ nvrm/policy_test.c src/cudapp_om_nvrm_policy.c
$(OUT_DIR)/nvrm-policy-red: nvrm/policy_test.c src/cudapp_om_nvrm_policy.c src/cudapp_native.h nvrm/nvrm_policy.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -DNVRM_POLICY_MUTANT=1 -o $@ nvrm/policy_test.c src/cudapp_om_nvrm_policy.c
test-nvrm-policy: $(OUT_DIR)/nvrm-policy-test $(OUT_DIR)/nvrm-policy-red
	./$(OUT_DIR)/nvrm-policy-test
	@if ./$(OUT_DIR)/nvrm-policy-red >/dev/null; then echo "FAIL: policy red check passed"; exit 1; else echo "NVRM policy red check caught"; fi
	sh tests/nvrm_policy_mutants.sh

$(OUT_DIR)/nvrm-struct-test: tests/nvrm_struct_test.c tests/nvrm_struct_layout.c src/cudapp_om_nvrm_struct.c nvrm/nvrm_struct.h
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ tests/nvrm_struct_test.c src/cudapp_om_nvrm_struct.c
.PHONY: test-nvrm-struct
CARRIED_HOST_TESTS += test-nvrm-struct
test-nvrm-struct: $(OUT_DIR)/nvrm-struct-test
	./$(OUT_DIR)/nvrm-struct-test
	sh tests/nvrm_struct_check.sh

.PHONY: test-nvrm-alloc
CARRIED_HOST_TESTS += test-nvrm-alloc
test-nvrm-alloc:
	sh tests/nvrm_alloc_check.sh
