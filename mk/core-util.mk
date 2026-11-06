# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
CARRIED_HOST_TESTS += test-core-util
.PHONY: test-core-util
$(OUT_DIR)/core-util-sha-ref.o: src/sha256.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_CORE -Dsha256_init=ref_sha256_init -Dsha256_update=ref_sha256_update -Dsha256_final=ref_sha256_final -Dsha256_hash=ref_sha256_hash -c $< -o $@
$(OUT_DIR)/test-core-util: tests/test_core_util.c src/sha256.c $(OUT_DIR)/core-util-sha-ref.o src/cudapp_om_core_sha.c | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread
 test-core-util: $(OUT_DIR)/test-core-util
	./$(OUT_DIR)/test-core-util
	@if ./$(OUT_DIR)/test-core-util --flip; then echo 'FAIL record flip survived'; exit 1; else echo 'CORE-UTIL negative control caught'; fi
