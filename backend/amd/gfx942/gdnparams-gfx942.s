// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl gdnparams
.p2align 8
.type gdnparams,@function
gdnparams:
  s_load_dwordx2 s[4:5], s[0:1], 0
  s_load_dwordx2 s[6:7], s[0:1], 8
  s_load_dwordx2 s[8:9], s[0:1], 16
  s_load_dwordx2 s[10:11], s[0:1], 24
  s_load_dwordx2 s[12:13], s[0:1], 32
  s_load_dwordx2 s[14:15], s[0:1], 40
  s_waitcnt lgkmcnt(0)
  s_mov_b32 s29, 1
  s_sub_u32 s28, s2, s29
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  v_and_b32 v0, 0x3ff, v0
  v_cmp_gt_u32 vcc, 48, v0
  s_and_saveexec_b64 s[26:27], vcc
  v_mov_b32 v1, v0
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  v_mov_b32 v27, s5
  v_add_co_u32 v2, vcc, s4, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  global_load_dword v8, v[2:3], off
  v_lshlrev_b32 v4, 2, v1
  v_lshrrev_b32 v5, 30, v1
  v_mov_b32 v27, s7
  v_add_co_u32 v4, vcc, s6, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  global_load_dword v9, v[4:5], off
  v_lshlrev_b32 v6, 2, v1
  v_lshrrev_b32 v7, 30, v1
  v_mov_b32 v27, s9
  v_add_co_u32 v6, vcc, s8, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  global_load_dword v10, v[6:7], off
  v_lshlrev_b32 v12, 2, v1
  v_lshrrev_b32 v13, 30, v1
  v_mov_b32 v27, s11
  v_add_co_u32 v12, vcc, s10, v12
  v_addc_co_u32 v13, vcc, v27, v13, vcc
  global_load_dword v11, v[12:13], off
  s_waitcnt vmcnt(0)
  // BF16_ROUND_A
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
  // END_BF16_ROUND_A
  // BF16_ROUND_B
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
  // END_BF16_ROUND_B
  v_and_b32 v20, 0x7fffffff, v9
  v_xor_b32 v20, 0x80000000, v20
  v_mul_f32 v20, 0x3fb8aa3b, v20
  v_exp_f32 v20, v20
  s_nop 0
  v_add_f32 v18, 1.0, v20
  v_rcp_f32 v18, v18
  v_lshrrev_b32 v21, 31, v9
  v_mov_b32 v22, 0
  v_cmp_gt_u32 vcc, v21, v22
  s_and_saveexec_b64 s[24:25], vcc
  v_mul_f32 v18, v20, v18
  s_mov_b64 exec, s[24:25]
  // BF16_ROUND_BETA
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
  // END_BF16_ROUND_BETA
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  v_mov_b32 v27, s15
  v_add_co_u32 v2, vcc, s14, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  global_store_dword v[2:3], v18, off
  v_add_f32 v8, v11, v8
  v_and_b32 v20, 0x7fffffff, v8
  v_xor_b32 v20, 0x80000000, v20
  v_mul_f32 v20, 0x3fb8aa3b, v20
  v_exp_f32 v20, v20
  s_nop 0
  v_mov_b32 v19, v20
  v_add_f32 v18, 1.0, v20
  v_mov_b32 v22, 1.0
  v_cmp_gt_u32 vcc, v18, v22
  s_and_saveexec_b64 s[24:25], vcc
  v_sub_f32 v21, v18, v22
  v_rcp_f32 v21, v21
  v_log_f32 v18, v18
  s_nop 0
  v_mul_f32 v18, 0x3f317218, v18
  v_mul_f32 v19, v20, v21
  v_mul_f32 v19, v18, v19
  s_mov_b64 exec, s[24:25]
  v_max_f32 v21, 0, v8
  v_add_f32 v19, v21, v19
  v_mov_b32 v22, 0x41a00000
  v_cmp_gt_u32 vcc, v21, v22
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v19, v8
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v10, 0x3fb8aa3b, v10
  v_exp_f32 v10, v10
  s_nop 0
  v_mul_f32 v19, v10, v19
  v_xor_b32 v19, 0x80000000, v19
  v_mul_f32 v19, 0x3fb8aa3b, v19
  v_exp_f32 v19, v19
  s_nop 0
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  v_mov_b32 v27, s13
  v_add_co_u32 v2, vcc, s12, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  global_store_dword v[2:3], v19, off
.L99:
  s_endpgm
.Lgdnparams_end:
.size gdnparams, .Lgdnparams_end-gdnparams
.rodata
.p2align 6
.amdhsa_kernel gdnparams
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 48
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
  - .name: gdnparams
    .symbol: gdnparams.kd
    .kernarg_segment_size: 48
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 36
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: a
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: b
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: A_log
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: dt_bias
        .offset: 24
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: decay
        .offset: 32
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: beta
        .offset: 40
        .size: 8
        .value_kind: global_buffer
        .address_space: global
...
.end_amdgpu_metadata
