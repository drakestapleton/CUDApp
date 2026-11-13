// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl gdnconv4
.p2align 8
.type gdnconv4,@function
gdnconv4:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_load_dwordx2 s[12:13], s[0:1], 32
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s28, s2, 160
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  v_and_b32 v0, 0x3ff, v0
  v_mov_b32 v1, s2
  v_lshlrev_b32 v1, 6, v1
  v_add_u32 v1, v0, v1
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
  v_lshrrev_b32 v27, 29, v4
  v_lshlrev_b32 v5, 3, v5
  v_add_u32 v5, v27, v5
  v_lshlrev_b32 v4, 3, v4
  v_mov_b32 v27, s7
  v_add_co_u32 v4, vcc, s6, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_mov_b32 v6, v1
  v_mov_b32 v7, 0
  v_lshrrev_b32 v27, 29, v6
  v_lshlrev_b32 v7, 3, v7
  v_add_u32 v7, v27, v7
  v_lshlrev_b32 v6, 3, v6
  v_mov_b32 v27, s9
  v_add_co_u32 v6, vcc, s8, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  v_mov_b32 v8, v1
  v_mov_b32 v9, 0
  v_lshrrev_b32 v27, 29, v8
  v_lshlrev_b32 v9, 3, v9
  v_add_u32 v9, v27, v9
  v_lshlrev_b32 v8, 3, v8
  v_mov_b32 v27, s11
  v_add_co_u32 v8, vcc, s10, v8
  v_addc_co_u32 v9, vcc, v27, v9, vcc
  v_mov_b32 v10, v1
  v_mov_b32 v11, 0
  v_lshrrev_b32 v27, 30, v10
  v_lshlrev_b32 v11, 2, v11
  v_add_u32 v11, v27, v11
  v_lshlrev_b32 v10, 2, v10
  v_mov_b32 v27, s13
  v_add_co_u32 v10, vcc, s12, v10
  v_addc_co_u32 v11, vcc, v27, v11, vcc
  global_load_dword v12, v[2:3], off
  global_load_dwordx2 v[14:15], v[4:5], off
  global_load_dwordx2 v[16:17], v[6:7], off
  s_waitcnt vmcnt(0)
  // BF16_ROUND_INPUT
  v_lshrrev_b32 v28, 16, v12
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v12, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v12
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v12, v28
  // END_BF16_ROUND_INPUT
  v_and_b32 v20, 0xffff0000, v16
  v_lshlrev_b32 v21, 16, v17
  v_and_b32 v22, 0xffff0000, v17
  v_lshrrev_b32 v18, 16, v16
  v_lshlrev_b32 v19, 16, v17
  v_add_u32 v18, v19, v18
  global_store_dword v[8:9], v18, off
  v_lshrrev_b32 v19, 16, v17
  v_add_u32 v19, v12, v19
  global_store_dword v[8:9], v19, off offset:4
  v_lshlrev_b32 v23, 16, v14
  v_and_b32 v24, 0xffff0000, v14
  v_lshlrev_b32 v25, 16, v15
  v_and_b32 v26, 0xffff0000, v15
  v_mov_b32 v18, 0
  v_fmac_f32 v18, v23, v20
  v_fmac_f32 v18, v24, v21
  v_fmac_f32 v18, v25, v22
  v_fmac_f32 v18, v26, v12
  // BF16_ROUND_CONV
  v_lshrrev_b32 v28, 16, v18
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v18, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v18
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v18, v28
  // END_BF16_ROUND_CONV
  v_and_b32 v20, 0x7fffffff, v18
  v_xor_b32 v20, 0x80000000, v20
  v_mul_f32 v20, 0x3fb8aa3b, v20
  v_exp_f32 v20, v20
  s_nop 0
  v_add_f32 v21, 1.0, v20
  v_rcp_f32 v21, v21
  v_lshrrev_b32 v22, 31, v18
  v_mov_b32 v23, 0
  v_cmp_gt_u32 vcc, v22, v23
  s_and_saveexec_b64 s[24:25], vcc
  v_mul_f32 v21, v20, v21
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v18, v21, v18
  // BF16_ROUND_SILU
  v_lshrrev_b32 v28, 16, v18
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v18, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v18
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v18, v28
  // END_BF16_ROUND_SILU
  global_store_dword v[10:11], v18, off
.L99:
  s_endpgm
.Lgdnconv4_end:
.size gdnconv4, .Lgdnconv4_end-gdnconv4
.rodata
.p2align 6
.amdhsa_kernel gdnconv4
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 40
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
  - .name: gdnconv4
    .symbol: gdnconv4.kd
    .kernarg_segment_size: 40
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 35
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: input
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: weight
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: old_state
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: new_state
        .offset: 24
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: output
        .offset: 32
        .size: 8
        .value_kind: global_buffer
        .address_space: global
...
.end_amdgpu_metadata
