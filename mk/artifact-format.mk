# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only: no GPU nodes, NVIDIA binaries, or live services.
CARRIED_HOST_TESTS += test-artifact-format
ARTIFACT_FORMAT_DIR = $(OUT_DIR)/artifact-format
ARTIFACT_FORMAT_RENAME = -Domega_native_artifact_export=artifact_diff_export \
 -Domega_native_artifact_admit=artifact_diff_admit -Domega_native_artifact_snapshot=artifact_diff_snapshot

$(ARTIFACT_FORMAT_DIR):
	mkdir -p $@
$(ARTIFACT_FORMAT_DIR)/manifest.o: tests/native_artifact_manifest_test.c | $(ARTIFACT_FORMAT_DIR)
	$(CC) $(CFLAGS) $(ARTIFACT_FORMAT_RENAME) -c $< -o $@
$(ARTIFACT_FORMAT_DIR)/export.o: tests/gpu_artifact_export_test.c | $(ARTIFACT_FORMAT_DIR)
	$(CC) $(CFLAGS) $(ARTIFACT_FORMAT_RENAME) -c $< -o $@
$(ARTIFACT_FORMAT_DIR)/tool.o: tools/artifact_export.c | $(ARTIFACT_FORMAT_DIR)
	$(CC) $(CFLAGS) $(ARTIFACT_FORMAT_RENAME) -c $< -o $@
$(ARTIFACT_FORMAT_DIR)/diff.o: tests/runtime/artifact_fixture_diff.c | $(ARTIFACT_FORMAT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(ARTIFACT_FORMAT_DIR)/oracle.o: tests/runtime/artifact_oracle.c src/omega_native_artifact.c | $(ARTIFACT_FORMAT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(ARTIFACT_FORMAT_DIR)/manifest-diff: $(ARTIFACT_FORMAT_DIR)/manifest.o $(ARTIFACT_FORMAT_DIR)/diff.o $(ARTIFACT_FORMAT_DIR)/oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -Wl,--wrap=fopen -o $@ $^ -lpthread -lm
$(ARTIFACT_FORMAT_DIR)/export-diff: $(ARTIFACT_FORMAT_DIR)/export.o $(ARTIFACT_FORMAT_DIR)/diff.o $(ARTIFACT_FORMAT_DIR)/oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lm
$(ARTIFACT_FORMAT_DIR)/tool-diff: $(ARTIFACT_FORMAT_DIR)/tool.o $(ARTIFACT_FORMAT_DIR)/diff.o $(ARTIFACT_FORMAT_DIR)/oracle.o $(OUT_DIR)/libomega_gpu.a
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lm
.PHONY: test-artifact-format test-artifact-format-pending test-artifact-format-mutants
test-artifact-format: test-native-artifact-manifest test-native-artifact $(ARTIFACT_FORMAT_DIR)/manifest-diff $(ARTIFACT_FORMAT_DIR)/export-diff $(ARTIFACT_FORMAT_DIR)/tool-diff
	./$(ARTIFACT_FORMAT_DIR)/manifest-diff
	rm -rf -- "$(ARTIFACT_FORMAT_DIR)/export-fixtures" "$(ARTIFACT_FORMAT_DIR)/shape-fixtures"
	./$(ARTIFACT_FORMAT_DIR)/export-diff $(ARTIFACT_FORMAT_DIR)/export-fixtures
	sh tools/artifact_export_shapes.sh ./$(ARTIFACT_FORMAT_DIR)/tool-diff $(ARTIFACT_FORMAT_DIR)/shape-fixtures
	CC="$(CC)" python3 tests/runtime/artifact_format_gate.py

test-artifact-format-pending:
	$(MAKE) -j3 OUT_DIR="$(OUT_DIR)/artifact-pending" CFLAGS="$(CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_ARTIFACT=1" test-native-artifact-manifest test-native-artifact

test-artifact-format-mutants:
	CC="$(CC)" python3 tests/runtime/artifact_format_mutants.py
