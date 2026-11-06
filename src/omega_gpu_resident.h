/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_RESIDENT_H
#define OMEGA_GPU_RESIDENT_H
#include "omega_gpu_om_ir.h"
/* INTERNAL, not runtime API. Immutable eight-u32 descriptors op,a,b,out,rows,cols,0,0.
 * a,b,out are f32 offsets into a size-element arena; ops 1 GEMV, 2 ADD, 3 MUL,
 * 4 SUM, 5 COPY. Admission and code generation are seeded Omega.
 * ABI c[0][896..]: descriptor, arena, two-u32 zeroed barrier state pointers.
 * Launch exactly ctas (1..2) blocks of 32 threads; only lane zero works.
 * All blocks must be simultaneously resident. Host/chip qualification are separate.
 */
int omega_gpu_resident_build(const uint32_t *descs, uint64_t n, uint64_t size, uint64_t ctas,
                             BlackwellIRProgram *p, uint64_t *err);
#endif
