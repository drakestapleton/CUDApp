// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl gdnrmsgate
.p2align 8
.type gdnrmsgate,@function
gdnrmsgate:
  s_load_dwordx2 s[4:5], s[0:1], 0
  s_load_dwordx2 s[6:7], s[0:1], 8
  s_load_dwordx2 s[8:9], s[0:1], 16
  s_load_dwordx2 s[10:11], s[0:1], 24
  s_waitcnt lgkmcnt(0)
  s_mov_b32 s29, 48
  s_sub_u32 s28, s2, s29
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  v_and_b32 v0, 0x3ff, v0
  v_mov_b32 v1, s2
  v_lshlrev_b32 v1, 7, v1
  v_add_u32 v1, v0, v1
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  v_mov_b32 v27, s5
  v_add_co_u32 v2, vcc, s4, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_lshlrev_b32 v4, 2, v0
  v_lshrrev_b32 v5, 30, v0
  v_mov_b32 v27, s7
  v_add_co_u32 v4, vcc, s6, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_lshlrev_b32 v6, 2, v1
  v_lshrrev_b32 v7, 30, v1
  v_mov_b32 v27, s9
  v_add_co_u32 v6, vcc, s8, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  v_lshlrev_b32 v12, 2, v1
  v_lshrrev_b32 v13, 30, v1
  v_mov_b32 v27, s11
  v_add_co_u32 v12, vcc, s10, v12
  v_addc_co_u32 v13, vcc, v27, v13, vcc
  global_load_dword v8, v[2:3], off
  global_load_dword v9, v[2:3], off offset:256
  global_load_dword v10, v[4:5], off
  global_load_dword v11, v[4:5], off offset:256
  global_load_dword v14, v[6:7], off
  global_load_dword v15, v[6:7], off offset:256
  s_waitcnt vmcnt(0)
  // BF16_ROUND_Z_LOW
  v_lshrrev_b32 v28, 16, v14
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v14, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v14
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v14, v28
  // END_BF16_ROUND_Z_LOW
  // BF16_ROUND_Z_HIGH
  v_lshrrev_b32 v28, 16, v15
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v15, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v15
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v15, v28
  // END_BF16_ROUND_Z_HIGH
  v_mul_f32 v16, v8, v8
  v_mul_f32 v17, v9, v9
  v_add_f32 v16, v17, v16
  v_lshlrev_b32 v26, 2, v0
  v_xor_b32 v28, 0x80, v26
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 64, v26
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 32, v26
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 16, v26
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 8, v26
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 4, v26
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_mul_f32 v16, 0x3c000000, v16
  v_add_f32 v16, 0x358637bd, v16
  v_rsq_f32 v16, v16
  s_nop 0
  v_mul_f32 v8, v16, v8
  // BF16_ROUND_NORM_LOW
  v_lshrrev_b32 v28, 16, v8
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v8, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v8
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v8, v28
  // END_BF16_ROUND_NORM_LOW
  v_mul_f32 v8, v10, v8
  // BF16_ROUND_WEIGHT_LOW
  v_lshrrev_b32 v28, 16, v8
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v8, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v8
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v8, v28
  // END_BF16_ROUND_WEIGHT_LOW
  v_and_b32 v20, 0x7fffffff, v14
  v_xor_b32 v20, 0x80000000, v20
  v_mul_f32 v20, 0x3fb8aa3b, v20
  v_exp_f32 v20, v20
  s_nop 0
  v_add_f32 v18, 1.0, v20
  v_rcp_f32 v18, v18
  v_lshrrev_b32 v21, 31, v14
  v_mov_b32 v22, 0
  v_cmp_gt_u32 vcc, v21, v22
  s_and_saveexec_b64 s[24:25], vcc
  v_mul_f32 v18, v20, v18
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v18, v14, v18
  v_mul_f32 v8, v18, v8
  // BF16_ROUND_OUTPUT_LOW
  v_lshrrev_b32 v28, 16, v8
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v8, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v8
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v8, v28
  // END_BF16_ROUND_OUTPUT_LOW
  v_mul_f32 v9, v16, v9
  // BF16_ROUND_NORM_HIGH
  v_lshrrev_b32 v28, 16, v9
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v9, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v9
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v9, v28
  // END_BF16_ROUND_NORM_HIGH
  v_mul_f32 v9, v11, v9
  // BF16_ROUND_WEIGHT_HIGH
  v_lshrrev_b32 v28, 16, v9
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v9, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v9
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v9, v28
  // END_BF16_ROUND_WEIGHT_HIGH
  v_and_b32 v20, 0x7fffffff, v15
  v_xor_b32 v20, 0x80000000, v20
  v_mul_f32 v20, 0x3fb8aa3b, v20
  v_exp_f32 v20, v20
  s_nop 0
  v_add_f32 v18, 1.0, v20
  v_rcp_f32 v18, v18
  v_lshrrev_b32 v21, 31, v15
  v_mov_b32 v22, 0
  v_cmp_gt_u32 vcc, v21, v22
  s_and_saveexec_b64 s[24:25], vcc
  v_mul_f32 v18, v20, v18
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v18, v15, v18
  v_mul_f32 v9, v18, v9
  // BF16_ROUND_OUTPUT_HIGH
  v_lshrrev_b32 v28, 16, v9
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v9, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v9
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v9, v28
  // END_BF16_ROUND_OUTPUT_HIGH
  global_store_dword v[12:13], v8, off
  global_store_dword v[12:13], v9, off offset:256
.L99:
  s_endpgm
.Lgdnrmsgate_end:
.size gdnrmsgate, .Lgdnrmsgate_end-gdnrmsgate
.rodata
.p2align 6
.amdhsa_kernel gdnrmsgate
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 32
  .amdhsa_next_free_vgpr 32
  .amdhsa_next_free_sgpr 30
  .amdhsa_accum_offset 32
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: gdnrmsgate
    .symbol: gdnrmsgate.kd
    .kernarg_segment_size: 32
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 36
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: core
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: weight
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: z
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: output
        .offset: 24
        .size: 8
        .value_kind: global_buffer
        .address_space: global
...
.end_amdgpu_metadata
