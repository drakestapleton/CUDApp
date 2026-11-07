# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Developer bar D8 (docs/issues/D8-process-tax.md): what a CUDA++ process costs beyond its own buffers.
# tests/d8_process_tax.om holds the bar and the negative controls; tests/d8_lib.om is its shared part. Both
# compile with the pinned seeded omc (OMEGA_OMC, omega-mv e5b32567), so off a CUDAPP_OMC_HOST the rows print NOT_RUN.
#   d8-process-tax-dry-run  host only, in bwrap with the GPU devices hidden; never opens a device
#   d8-process-tax          GB10: takes one spark-window lease (gb10 exclusive) and measures inside it
D8_HELLO = $(abspath $(OUT_DIR))/hello_gpu $(abspath $(OUT_DIR))/hello_gpu_control
$(OUT_DIR)/d8_process_tax $(OUT_DIR)/d8_mutants: $(OUT_DIR)/d8_%: tests/d8_lib.om tests/d8_%.om
	@mkdir -p $(@D)
	cat $^ > $@.om
	"$(OMEGA_OMC)" < $@.om > $@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@
.PHONY: d8-process-tax-dry-run d8-process-tax
d8-process-tax-dry-run: $(if $(CUDAPP_OMC_HOST),$(OUT_DIR)/d8_process_tax) $(OUT_DIR)/hello_gpu $(OUT_DIR)/hello_gpu_control
	@$(CUDAPP_REQUIRE_OMC)
	$(OUT_DIR)/d8_process_tax dry-run "$(D8_OUT)" $(D8_HELLO)
d8-process-tax: $(if $(CUDAPP_OMC_HOST),$(OUT_DIR)/d8_process_tax) $(OUT_DIR)/hello_gpu $(OUT_DIR)/hello_gpu_control
	@$(CUDAPP_REQUIRE_OMC)
	$(OUT_DIR)/d8_process_tax gpu "$(D8_OUT)" $(D8_HELLO)
