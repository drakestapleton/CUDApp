// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl attn256_part_step
.p2align 8
.type attn256_part_step,@function
attn256_part_step:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_load_dwordx4 s[12:15], s[0:1], 32
  s_waitcnt lgkmcnt(0)
  s_load_dwordx2 s[16:17], s[0:1], 40
  s_waitcnt lgkmcnt(0)
  v_mov_b32 v46, s16
  v_mov_b32 v47, s17
  global_load_dword v45, v[46:47], off
  s_waitcnt vmcnt(0)
  v_readfirstlane_b32 s14, v45
  s_nop 1
  s_load_dword s15, s[0:1], 48
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s28, s14, s15
  s_cbranch_scc0 .L99
  s_add_u32 s14, s14, 1
  s_sub_u32 s28, s3, 24
  s_cbranch_scc0 .L99
  v_and_b32 v0, 0x3ff, v0
  v_lshlrev_b32 v1, 2, v0
  s_mov_b32 s18, 0
  s_sub_u32 s28, s14, 1
  s_cbranch_scc1 .L2
  s_lshr_b32 s18, s28, 6
  s_add_u32 s18, s18, 1
.L2:
  v_mov_b32 v12, 0
  v_mov_b32 v13, 0
  v_mov_b32 v14, 0
  v_mov_b32 v15, 0
  v_mov_b32 v17, 0
  v_mov_b32 v16, 0xff800000
  v_mov_b32 v29, 0x3fb8aa3b
  v_mov_b32 v30, 0x3d800000
  s_sub_u32 s28, s2, s18
  s_cbranch_scc0 .L99
  s_mov_b32 s19, 0
  s_mov_b32 s20, s3
.L3:
  s_sub_u32 s28, s20, 6
  s_cbranch_scc1 .L4
  s_mov_b32 s20, s28
  s_add_u32 s19, s19, 1
  s_sub_u32 s28, 0, 0
  s_cbranch_scc0 .L3
.L4:
  v_mov_b32 v31, s19
  v_mul_lo_u32 v2, v31, s15
  v_mul_hi_u32 v3, v31, s15
  v_lshrrev_b32 v27, 23, v2
  v_lshlrev_b32 v3, 9, v3
  v_add_u32 v3, v27, v3
  v_lshlrev_b32 v2, 9, v2
  v_mov_b32 v27, s7
  v_add_co_u32 v2, vcc, s6, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_mov_b32 v31, s19
  v_mul_lo_u32 v4, v31, s15
  v_mul_hi_u32 v5, v31, s15
  v_lshrrev_b32 v27, 23, v4
  v_lshlrev_b32 v5, 9, v5
  v_add_u32 v5, v27, v5
  v_lshlrev_b32 v4, 9, v4
  v_mov_b32 v27, s9
  v_add_co_u32 v4, vcc, s8, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_mov_b32 v31, s2
  v_lshlrev_b32 v31, 15, v31
  v_add_co_u32 v2, vcc, v31, v2
  v_addc_co_u32 v3, vcc, 0, v3, vcc
  v_add_co_u32 v4, vcc, v31, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  v_lshlrev_b32 v31, 3, v0
  v_add_co_u32 v2, vcc, v31, v2
  v_addc_co_u32 v3, vcc, 0, v3, vcc
  v_add_co_u32 v4, vcc, v31, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  v_mov_b32 v34, s3
  v_mov_b32 v35, 0
  v_lshrrev_b32 v27, 22, v34
  v_lshlrev_b32 v35, 10, v35
  v_add_u32 v35, v27, v35
  v_lshlrev_b32 v34, 10, v34
  v_mov_b32 v27, s5
  v_add_co_u32 v34, vcc, s4, v34
  v_addc_co_u32 v35, vcc, v27, v35, vcc
  v_lshlrev_b32 v31, 4, v0
  v_add_co_u32 v34, vcc, v31, v34
  v_addc_co_u32 v35, vcc, 0, v35, vcc
  global_load_dwordx4 v[8:11], v[34:35], off
  s_waitcnt vmcnt(0)
  s_lshl_b32 s24, s2, 6
  s_mov_b32 s25, 0
.L10:
  s_sub_u32 s28, s24, s14
  s_cbranch_scc0 .L20
  s_sub_u32 s28, s25, 64
  s_cbranch_scc0 .L20
  global_load_dwordx2 v[22:23], v[2:3], off
  global_load_dwordx2 v[24:25], v[4:5], off
  s_waitcnt vmcnt(0)
  v_mov_b32 v18, 0
  v_lshlrev_b32 v26, 16, v22
  v_fmac_f32 v18, v26, v8
  v_and_b32 v26, 0xffff0000, v22
  v_fmac_f32 v18, v26, v9
  v_lshlrev_b32 v26, 16, v23
  v_fmac_f32 v18, v26, v10
  v_and_b32 v26, 0xffff0000, v23
  v_fmac_f32 v18, v26, v11
  v_xor_b32 v28, 0x80, v1
  ds_bpermute_b32 v27, v28, v18
  s_waitcnt lgkmcnt(0)
  v_add_f32 v18, v18, v27
  v_xor_b32 v28, 64, v1
  ds_bpermute_b32 v27, v28, v18
  s_waitcnt lgkmcnt(0)
  v_add_f32 v18, v18, v27
  v_xor_b32 v28, 32, v1
  ds_bpermute_b32 v27, v28, v18
  s_waitcnt lgkmcnt(0)
  v_add_f32 v18, v18, v27
  v_xor_b32 v28, 16, v1
  ds_bpermute_b32 v27, v28, v18
  s_waitcnt lgkmcnt(0)
  v_add_f32 v18, v18, v27
  v_xor_b32 v28, 8, v1
  ds_bpermute_b32 v27, v28, v18
  s_waitcnt lgkmcnt(0)
  v_add_f32 v18, v18, v27
  v_xor_b32 v28, 4, v1
  ds_bpermute_b32 v27, v28, v18
  s_waitcnt lgkmcnt(0)
  v_add_f32 v18, v18, v27
  v_mul_f32 v18, v30, v18
  v_max_f32 v19, v16, v18
  v_sub_f32 v20, v16, v19
  v_mul_f32 v20, v29, v20
  v_exp_f32 v20, v20
  v_sub_f32 v21, v18, v19
  v_mul_f32 v21, v29, v21
  v_exp_f32 v21, v21
  v_mul_f32 v17, v20, v17
  v_add_f32 v17, v17, v21
  v_lshlrev_b32 v26, 16, v24
  v_mul_f32 v12, v20, v12
  v_fmac_f32 v12, v21, v26
  v_and_b32 v26, 0xffff0000, v24
  v_mul_f32 v13, v20, v13
  v_fmac_f32 v13, v21, v26
  v_lshlrev_b32 v26, 16, v25
  v_mul_f32 v14, v20, v14
  v_fmac_f32 v14, v21, v26
  v_and_b32 v26, 0xffff0000, v25
  v_mul_f32 v15, v20, v15
  v_fmac_f32 v15, v21, v26
  v_mov_b32 v16, v19
  v_add_co_u32 v2, vcc, 0x200, v2
  v_addc_co_u32 v3, vcc, 0, v3, vcc
  v_add_co_u32 v4, vcc, 0x200, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  s_add_u32 s24, s24, 1
  s_add_u32 s25, s25, 1
  s_sub_u32 s28, 0, 0
  s_cbranch_scc0 .L10
.L20:
  v_mov_b32 v31, s3
  v_mul_lo_u32 v31, v31, s18
  v_mov_b32 v27, s2
  v_add_u32 v31, v27, v31
  s_mov_b32 s21, 1040
  v_mul_lo_u32 v32, v31, s21
  v_mul_hi_u32 v33, v31, s21
  v_mov_b32 v27, s11
  v_add_co_u32 v32, vcc, s10, v32
  v_addc_co_u32 v33, vcc, v27, v33, vcc
  v_mov_b32 v6, v32
  v_mov_b32 v7, v33
  v_lshlrev_b32 v31, 4, v0
  v_add_co_u32 v6, vcc, v31, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  global_store_dword v[6:7], v12, off offset:16
  global_store_dword v[6:7], v13, off offset:20
  global_store_dword v[6:7], v14, off offset:24
  global_store_dword v[6:7], v15, off offset:28
  v_cmp_gt_u32 vcc, 1, v0
  s_and_saveexec_b64 s[26:27], vcc
  global_store_dword v[32:33], v16, off
  global_store_dword v[32:33], v17, off offset:4
.L99:
  s_endpgm
.Lattn256_part_step_end:
.size attn256_part_step, .Lattn256_part_step_end-attn256_part_step
.rodata
.p2align 6
.amdhsa_kernel attn256_part_step
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 52
  .amdhsa_next_free_vgpr 48
  .amdhsa_next_free_sgpr 29
  .amdhsa_accum_offset 48
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: attn256_part_step
    .symbol: attn256_part_step.kd
    .kernarg_segment_size: 52
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 35
    .vgpr_count: 48
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: Q
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: K
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: V
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: partials
        .offset: 24
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: out
        .offset: 32
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: position
        .offset: 40
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: capacity
        .offset: 48
        .size: 4
        .value_kind: by_value
...
.end_amdgpu_metadata
