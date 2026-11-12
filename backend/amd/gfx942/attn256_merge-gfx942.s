// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl attn256_merge
.p2align 8
.type attn256_merge,@function
attn256_merge:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_load_dwordx4 s[12:15], s[0:1], 32
  s_waitcnt lgkmcnt(0)
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
  v_mov_b32 v31, s3
  v_mul_lo_u32 v31, v31, s18
  s_mov_b32 s21, 1040
  v_mul_lo_u32 v32, v31, s21
  v_mul_hi_u32 v33, v31, s21
  v_mov_b32 v27, s11
  v_add_co_u32 v32, vcc, s10, v32
  v_addc_co_u32 v33, vcc, v27, v33, vcc
  v_mov_b32 v6, s3
  v_mov_b32 v7, 0
  v_lshrrev_b32 v27, 22, v6
  v_lshlrev_b32 v7, 10, v7
  v_add_u32 v7, v27, v7
  v_lshlrev_b32 v6, 10, v6
  v_mov_b32 v27, s13
  v_add_co_u32 v6, vcc, s12, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  v_lshlrev_b32 v31, 4, v0
  v_add_co_u32 v6, vcc, v31, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_mov_b32 v40, v32
  v_mov_b32 v41, v33
  v_add_co_u32 v40, vcc, v31, v40
  v_addc_co_u32 v41, vcc, 0, v41, vcc
  s_mov_b32 s24, 0
.L30:
  s_sub_u32 s28, s24, s18
  s_cbranch_scc0 .L40
  global_load_dwordx2 v[22:23], v[32:33], off
  global_load_dwordx4 v[36:39], v[40:41], off offset:16
  s_waitcnt vmcnt(0)
  v_max_f32 v19, v16, v22
  v_sub_f32 v20, v16, v19
  v_mul_f32 v20, v29, v20
  v_exp_f32 v20, v20
  v_sub_f32 v21, v22, v19
  v_mul_f32 v21, v29, v21
  v_exp_f32 v21, v21
  v_mul_f32 v17, v20, v17
  v_fmac_f32 v17, v21, v23
  v_mul_f32 v12, v20, v12
  v_fmac_f32 v12, v21, v36
  v_mul_f32 v13, v20, v13
  v_fmac_f32 v13, v21, v37
  v_mul_f32 v14, v20, v14
  v_fmac_f32 v14, v21, v38
  v_mul_f32 v15, v20, v15
  v_fmac_f32 v15, v21, v39
  v_mov_b32 v16, v19
  v_add_co_u32 v32, vcc, 0x410, v32
  v_addc_co_u32 v33, vcc, 0, v33, vcc
  v_add_co_u32 v40, vcc, 0x410, v40
  v_addc_co_u32 v41, vcc, 0, v41, vcc
  s_add_u32 s24, s24, 1
  s_sub_u32 s28, 0, 0
  s_cbranch_scc0 .L30
.L40:
  s_sub_u32 s28, s18, 1
  s_cbranch_scc1 .L41
  v_rcp_f32 v20, v17
  v_mul_f32 v12, v20, v12
  v_mul_f32 v13, v20, v13
  v_mul_f32 v14, v20, v14
  v_mul_f32 v15, v20, v15
.L41:
  global_store_dword v[6:7], v12, off
  global_store_dword v[6:7], v13, off offset:4
  global_store_dword v[6:7], v14, off offset:8
  global_store_dword v[6:7], v15, off offset:12
.L99:
  s_endpgm
.Lattn256_merge_end:
.size attn256_merge, .Lattn256_merge_end-attn256_merge
.rodata
.p2align 6
.amdhsa_kernel attn256_merge
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 48
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
  - .name: attn256_merge
    .symbol: attn256_merge.kd
    .kernarg_segment_size: 48
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
      - .name: T
        .offset: 40
        .size: 4
        .value_kind: by_value
      - .name: capacity
        .offset: 44
        .size: 4
        .value_kind: by_value
...
.end_amdgpu_metadata
