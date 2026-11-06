#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
: "${OMEGA_NATIVE_OMC:=$HOME/work/cudastack/lanes/bh-tools/omc-native}"
: "${OMEGA_NATIVE_OMC2:=$HOME/work/cudastack/omega-mv/seed/out/omc}"
: "${OMEGA_BRIDGE:=$HOME/work/cudastack/omega-bridge}"
: "${CC:=cc}"
OUT="$ROOT/build/regalloc-liveness"
export OMEGA_NATIVE_OMC OMEGA_NATIVE_OMC2
mkdir -p "$OUT/include"
"$OMEGA_BRIDGE/tools/native" "$OUT/native" om/codegen/regalloc.om om/codegen/regalloc_ir.om om/codegen/regalloc_live.om
cp "$OUT/native/omega_native.h" "$OUT/include/cudapp_hosted_native.h"
$CC -std=gnu11 -O2 -Wall -Wextra -Werror -ffunction-sections -fdata-sections -Isrc -I"$OUT/include" \
  -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_MATMUL tests/regalloc_om/liveness.c \
  src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c -Wl,--gc-sections \
  -L"$OUT/native" -Wl,-rpath,"$OUT/native" -lomega_native -o "$OUT/liveness"
"$OUT/liveness"
