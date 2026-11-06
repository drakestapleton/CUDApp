#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Chip tier for the Omega profiler. This script does not open a GPU device node.
# It is a refusal plus the runbook for a later Drake-authorized window.
set -eu
echo "NOT_RUN chip: profiler host and simulator tiers do not open a GPU."
echo "Runbook, Drake-authorized exclusive window only, after the current GPU lock procedure:"
echo "1. Build a command list with OmegaGpuCmdListOpts.stamps = 1. That asks Streams' emitter for one 6-word REPORT_SEMAPHORE per slot (method 0x0158, count 5)."
echo "2. Submit the list. omega_gpu_cmdlist_stamps returns one u64 timestamp per slot: out[0] before launch 0, out[i+1] after launch i."
echo "3. Pack the raw 16-byte slots (u64 payload, u64 timestamp) and the kernel descriptors into a PROF fixture. tools/prof/write_fix.c is the 176-byte layout."
echo "4. Run the pinned profiler: prof report <fixture> <out>. Exit 0, and time_ns must match the stamps() deltas. A fixture with two slots swapped must exit 3."
echo "5. This script is not that measurement. It always exits 2."
exit 2
