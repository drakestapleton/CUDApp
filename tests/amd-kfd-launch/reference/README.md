# Pinned primary sources for the KFD topology conversion

These are unmodified upstream files, retaining their upstream license headers.
`sources.json` is the canonical source of URLs, digests and line ranges.
`SHA256SUMS` and the table below are derived views checked by the gate. The launcher gate checks every digest and the cited
producer statements offline before compiling. Retrieve each file from the URL
below and verify its listed digest before replacing a reference.

<!-- generated from sources.json -->
| Local file | Upstream URL | Relevant lines |
| --- | --- | --- |
| topology-rocm-6.2.0.c | https://raw.githubusercontent.com/ROCm/ROCT-Thunk-Interface/rocm-6.2.0/src/topology.c | 1147-1150, 1204-1205 |
| topology-rocm-7.2.2.c | https://raw.githubusercontent.com/ROCm/rocm-systems/rocm-7.2.2/projects/rocr-runtime/libhsakmt/src/topology.c | 1171-1174, 1234-1235 |
| queues-rocm-6.2.0.c | https://raw.githubusercontent.com/ROCm/ROCT-Thunk-Interface/rocm-6.2.0/src/queues.c | 292-298 |
| kfd_topology-v6.18.c | https://raw.githubusercontent.com/torvalds/linux/v6.18/drivers/gpu/drm/amd/amdkfd/kfd_topology.c | 461-465, 2082-2083 |
| kfd_crat-v6.18.c | https://raw.githubusercontent.com/torvalds/linux/v6.18/drivers/gpu/drm/amd/amdkfd/kfd_crat.c | 1041, 2286-2287 |
<!-- end generated source table -->

ROCT maps `array_count` to a local SIMD-array count and
`simd_arrays_per_engine` to `NumArrays`. It derives `NumShaderBanks` by dividing
that count by `NumArrays`; the queue calculation divides by `NumArrays` again.
The native launcher deliberately retains both divisions. The gate includes an
array-bound fixture that distinguishes the formulas and a missing-division
mutation control.
