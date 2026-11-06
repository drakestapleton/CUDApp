# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Developer bar D8 (docs/issues/D8-process-tax.md): what a CUDA++ process costs beyond its own buffers.
# tests/run_d8_process_tax.sh holds the bar and the negative controls; it builds hello_gpu itself.
#   d8-process-tax-dry-run  host only, in bwrap with the GPU devices hidden; never opens a device
#   d8-process-tax          GB10: takes one spark-window lease (gb10 exclusive) and measures inside it
.PHONY: d8-process-tax-dry-run d8-process-tax
d8-process-tax-dry-run:
	bash tests/run_d8_process_tax.sh --dry-run
d8-process-tax:
	bash tests/run_d8_process_tax.sh --gpu
