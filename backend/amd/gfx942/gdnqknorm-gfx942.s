// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl gdnqknorm
.p2align 8
.type gdnqknorm,@function
gdnqknorm:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s28, s2, 32
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  v_and_b32 v0, 0x3ff, v0
  v_lshlrev_b32 v1, 2, v0
  s_lshl_b32 s13, s2, 9
  v_add_u32 v2, s13, v1
  v_mov_b32 v3, 0
  v_mov_b32 v4, v2
  v_mov_b32 v5, 0
  v_mov_b32 v27, s5
  v_add_co_u32 v2, vcc, s4, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_mov_b32 v27, s7
  v_add_co_u32 v4, vcc, s6, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  global_load_dword v8, v[2:3], off
  global_load_dword v9, v[2:3], off offset:256
  s_waitcnt vmcnt(0)
  v_mul_f32 v10, v8, v8
  // BF16_ROUND_SQUARE0
  v_lshrrev_b32 v28, 16, v10
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v10, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v10
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v10, v28
  // END_BF16_ROUND_SQUARE0
  v_mul_f32 v11, v9, v9
  // BF16_ROUND_SQUARE1
  v_lshrrev_b32 v28, 16, v11
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v11, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v11
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v11, v28
  // END_BF16_ROUND_SQUARE1
  v_add_f32 v16, v10, v11
  v_xor_b32 v28, 0x80, v1
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 64, v1
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 32, v1
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 16, v1
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 8, v1
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  v_xor_b32 v28, 4, v1
  ds_bpermute_b32 v27, v28, v16
  s_waitcnt lgkmcnt(0)
  v_add_f32 v16, v16, v27
  // BF16_ROUND_SUM
  v_lshrrev_b32 v28, 16, v16
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v16, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v16
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v16, v28
  // END_BF16_ROUND_SUM
  v_add_f32 v16, 0x35860000, v16
  // BF16_ROUND_EPSILON
  v_lshrrev_b32 v28, 16, v16
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v16, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v16
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v16, v28
  // END_BF16_ROUND_EPSILON
  v_rsq_f32 v16, v16
  s_nop 0
  // BF16_ROUND_RSQ
  v_lshrrev_b32 v28, 16, v16
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v16, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v16
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v16, v28
  // END_BF16_ROUND_RSQ
  v_mul_f32 v8, v16, v8
  // BF16_ROUND_NORM0
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
  // END_BF16_ROUND_NORM0
  v_mul_f32 v9, v16, v9
  // BF16_ROUND_NORM1
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
  // END_BF16_ROUND_NORM1
  s_sub_u32 s28, s2, 16
  s_cbranch_scc0 .L10
  v_mul_f32 v8, 0x3db504f3, v8
  v_mul_f32 v9, 0x3db504f3, v9
.L10:
  global_store_dword v[4:5], v8, off
  global_store_dword v[4:5], v9, off offset:256
.L99:
  s_endpgm
.Lgdnqknorm_end:
.size gdnqknorm, .Lgdnqknorm_end-gdnqknorm
.rodata
.p2align 6
.amdhsa_kernel gdnqknorm
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 16
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
  - .name: gdnqknorm
    .symbol: gdnqknorm.kd
    .kernarg_segment_size: 16
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
      - .name: output
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
...
.end_amdgpu_metadata
