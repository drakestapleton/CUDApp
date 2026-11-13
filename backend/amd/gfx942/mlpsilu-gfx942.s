// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl mlpsilu
.p2align 8
.type mlpsilu,@function
mlpsilu:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx2 s[8:9], s[0:1], 16
  s_load_dword s10, s[0:1], 24
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s10, 1
  s_cbranch_scc1 .L99
  s_lshr_b32 s11, s28, 6
  s_sub_u32 s28, s11, s2
  s_cbranch_scc1 .L99
  v_and_b32 v0, 0x3ff, v0
  v_mov_b32 v1, s2
  v_lshlrev_b32 v1, 6, v1
  v_add_u32 v1, v0, v1
  v_cmp_gt_u32 vcc, s10, v1
  s_and_saveexec_b64 s[26:27], vcc
  s_cbranch_execz .L99
  v_mov_b32 v2, v1
  v_mov_b32 v3, 0
  v_lshrrev_b32 v27, 30, v2
  v_lshlrev_b32 v3, 2, v3
  v_add_u32 v3, v27, v3
  v_lshlrev_b32 v2, 2, v2
  v_mov_b32 v27, s5
  v_add_co_u32 v2, vcc, s4, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_mov_b32 v4, v1
  v_mov_b32 v5, 0
  v_lshrrev_b32 v27, 30, v4
  v_lshlrev_b32 v5, 2, v5
  v_add_u32 v5, v27, v5
  v_lshlrev_b32 v4, 2, v4
  v_mov_b32 v27, s7
  v_add_co_u32 v4, vcc, s6, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_mov_b32 v6, v1
  v_mov_b32 v7, 0
  v_lshrrev_b32 v27, 30, v6
  v_lshlrev_b32 v7, 2, v7
  v_add_u32 v7, v27, v7
  v_lshlrev_b32 v6, 2, v6
  v_mov_b32 v27, s9
  v_add_co_u32 v6, vcc, s8, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  global_load_dword v8, v[2:3], off
  global_load_dword v9, v[4:5], off
  s_waitcnt vmcnt(0)
  v_and_b32 v10, 0x7fffffff, v8
  v_xor_b32 v10, 0x80000000, v10
  v_mul_f32 v10, 0x3fb8aa3b, v10
  v_exp_f32 v10, v10
  s_nop 0
  v_add_f32 v11, 1.0, v10
  v_rcp_f32 v11, v11
  v_lshrrev_b32 v12, 31, v8
  v_mov_b32 v13, 0
  v_cmp_gt_u32 vcc, v12, v13
  s_and_saveexec_b64 s[24:25], vcc
  v_mul_f32 v11, v10, v11
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v8, v11, v8
  v_mul_f32 v9, v8, v9
  global_store_dword v[6:7], v9, off
.L99:
  s_endpgm
.Lmlpsilu_end:
.size mlpsilu, .Lmlpsilu_end-mlpsilu
.rodata
.p2align 6
.amdhsa_kernel mlpsilu
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 28
  .amdhsa_next_free_vgpr 32
  .amdhsa_next_free_sgpr 29
  .amdhsa_accum_offset 32
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: mlpsilu
    .symbol: mlpsilu.kd
    .kernarg_segment_size: 28
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 35
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: gate
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: up
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: out
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: count
        .offset: 24
        .size: 4
        .value_kind: by_value
...
.end_amdgpu_metadata
