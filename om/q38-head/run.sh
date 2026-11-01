#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External verification harness; not a runtime dependency.
set -eu
: "${OMEGA_OMC:?set the pinned seeded compiler}"
: "${Q38_GPU_OUT:?set an external output directory}"
test "$(sha256sum < "$OMEGA_OMC" | cut -c1-64)" = 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
mkdir -p "$Q38_GPU_OUT"
cat om/components/bw_alloc.om om/components/bw_effect.om \
    om/components/bw_check.om om/components/bw_project.om \
    om/encoder/bw_enc_sm121.om om/encoder/bw_spec_sm121.om \
    om/components/encode.om om/q38-head/bounds.om om/q38-head/driver.om \
    > "$Q38_GPU_OUT/combined.om"
"$OMEGA_OMC" < "$Q38_GPU_OUT/combined.om" > "$Q38_GPU_OUT/emit"
chmod +x "$Q38_GPU_OUT/emit"
"$Q38_GPU_OUT/emit" > "$Q38_GPU_OUT/bounds.frame"
${CC:-cc} -O2 -std=gnu11 -Wall -Wextra -Werror -Wno-unused-function \
    -ffp-contract=off -Isrc -Itests om/q38-head/check.c \
    "${Q38_GPU_LIB:-build/libomega_gpu.a}" -lpthread -lm -o "$Q38_GPU_OUT/check"
"$Q38_GPU_OUT/check" "$Q38_GPU_OUT/bounds.frame" "$@"
cat om/q38-head/envelope.om om/q38-head/envelope-driver.om > "$Q38_GPU_OUT/envelope.om"
"$OMEGA_OMC" < "$Q38_GPU_OUT/envelope.om" > "$Q38_GPU_OUT/envelope"
chmod +x "$Q38_GPU_OUT/envelope"
python3 om/q38-head/envelope-check.py "$Q38_GPU_OUT/envelope"
python3 om/q38-head/envelope-mutants.py "$OMEGA_OMC"
${CC:-cc} -O2 -shared -fPIC src/cudapp_om_q38_runtime.c src/cudapp_om_q38_emit.c \
    -o "$Q38_GPU_OUT/plan.so"
python3 om/q38-head/plan-check.py "$Q38_GPU_OUT/plan.so" "$Q38_GPU_OUT/bounds.frame"
${CC:-cc} -O2 -Isrc om/q38-head/budget-check.c "${Q38_GPU_LIB:-build/libomega_gpu.a}" \
    -lpthread -lm -o "$Q38_GPU_OUT/budget-check"
"$Q38_GPU_OUT/budget-check"
