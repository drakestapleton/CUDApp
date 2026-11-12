// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// vadd-gfx942.s: ORACLE input, never read by the producer. The first-party code object is written by
// backend/amd/gfx942/hsaco.om (seeded Omega) from the instruction table backend/amd/gfx942/vadd.isa.
// Only tests/amd/run.sh assembles this file, with llvm-mc-18 -triple=amdgcn-amd-amdhsa -mcpu=gfx942
// -filetype=obj, into a relocatable object. No linker is installed on the Spark (no ld.lld), so the
// gate compares sections (.text bytes, the objdump listing, the kernel descriptor apart from its
// relocated entry offset, and the metadata note), not whole linked files.
// Copied from the ended t_bd34e75b lane's untracked backend/amd/gfx942/vadd-gfx942.s. The address
// path now computes full 64-bit addresses: the old gid<<2 in a single VGPR lost the high bits.
// Hardware NOT_RUN: no AMD GPU on this host.
// FP32 c[gid] = a[gid] + b[gid] for gid < count; 64 work-items per group (one wave64 on CDNA3).
// Kernarg layout (same as backend/amd/vadd-gfx1151.s): a @0, b @8, c @16 (global float pointers),
// count @24 (uint), 28 bytes. SGPR inputs: s[0:1] kernarg segment pointer, s2 workgroup id x.
// VGPR input: v0 work-item ids packed 10 bits each (x in bits 0..9).
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl vadd
.p2align 8
.type vadd,@function
vadd:
  s_load_dwordx4 s[4:7], s[0:1], 0x0
  s_load_dwordx2 s[8:9], s[0:1], 0x10
  s_load_dword s10, s[0:1], 0x18
  v_and_b32 v0, 0x3ff, v0
  s_lshl_b32 s11, s2, 6
  v_add_u32 v1, s11, v0
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, s10, v1
  s_and_saveexec_b64 s[12:13], vcc
  s_cbranch_execz .Lend
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  v_mov_b32 v5, s5
  v_add_co_u32 v6, vcc, s4, v2
  v_addc_co_u32 v7, vcc, v5, v3, vcc
  global_load_dword v4, v[6:7], off
  v_mov_b32 v5, s7
  v_add_co_u32 v6, vcc, s6, v2
  v_addc_co_u32 v7, vcc, v5, v3, vcc
  global_load_dword v8, v[6:7], off
  s_waitcnt vmcnt(0)
  v_add_f32 v4, v4, v8
  v_mov_b32 v5, s9
  v_add_co_u32 v6, vcc, s8, v2
  v_addc_co_u32 v7, vcc, v5, v3, vcc
  global_store_dword v[6:7], v4, off
.Lend:
  s_endpgm
.Lvadd_end:
.size vadd, .Lvadd_end-vadd

.rodata
.p2align 6
.amdhsa_kernel vadd
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 28
  .amdhsa_next_free_vgpr 9
  .amdhsa_next_free_sgpr 14
  .amdhsa_accum_offset 12
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel

.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: vadd
    .symbol: vadd.kd
    .kernarg_segment_size: 28
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 20
    .vgpr_count: 9
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - { .name: a, .offset: 0, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: b, .offset: 8, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: c, .offset: 16, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: count, .offset: 24, .size: 4, .value_kind: by_value }
...
.end_amdgpu_metadata
