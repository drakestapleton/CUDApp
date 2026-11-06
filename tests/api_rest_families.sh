#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Invoke only through the lane's bwrap /dev isolation and CPU affinity.
set -eu
: "${OMEGA_ROOT:?set OMEGA_ROOT to the pinned omega-mv oracle checkout}"
cc=${CC:-cc}
make -j3 build/gpu_moe_window_test build/gpu_tern_sched_test build/gpu_tcgemm_test build/gpu_fp8gemm_test build/gpu_pfgemm_test build/gpu_pfhost_test build/gpu_fp8_slots_test build/gpu_fp8_api_test
"$cc" -std=gnu11 -Wall -Wextra -Werror -O2 -ffp-contract=off -D_GNU_SOURCE -DWS_MAX_THREADS=512 -Isrc -Itests tests/gpu_tgemm_test.c build/libomega_gpu.a -lm -lpthread -o build/gpu_tgemm_test
# The external test's quoted include would otherwise select its stale simulator.
# Compile its untouched algebra oracle separately; the current CUDApp simulator
# is preincluded only into the test translation unit.
"$cc" -std=gnu11 -Wall -Wextra -Werror -O2 -ffunction-sections -fdata-sections -I"$OMEGA_ROOT/src" -c "$OMEGA_ROOT/src/algebra/realize_common.c" -o build/tern_realize_oracle.o
"$cc" -std=gnu11 -Wall -Wextra -Werror -O2 -ffp-contract=off -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE -Isrc -Itests -I"$OMEGA_ROOT/src" -include tests/bw_warp_sim_k.h "$OMEGA_ROOT/tests/gpu_tern_test.c" build/tern_realize_oracle.o build/libomega_gpu.a -lm -lpthread -o build/gpu_tern_test
build/gpu_moe_window_test --host-only
build/gpu_tern_sched_test
for family in tcgemm fp8gemm pfgemm pfhost tgemm tern fp8_api; do
    "build/gpu_${family}_test" --host-only
    "build/gpu_${family}_test" --sim
done
build/gpu_fp8_slots_test --host-only
printf '%s\n' 'PASS api-rest family host/sim; chip NOT_RUN'
