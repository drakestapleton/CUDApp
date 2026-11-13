// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl addnorm5120
.p2align 8
.type addnorm5120,@function
addnorm5120:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_load_dwordx2 s[12:13], s[0:1], 32
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s28, s2, 1
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  v_and_b32 v0, 0x3ff, v0
  v_lshlrev_b32 v1, 2, v0
  v_lshlrev_b32 v25, 4, v0
  v_mov_b32 v2, v25
  v_mov_b32 v3, 0
  v_mov_b32 v27, s5
  v_add_co_u32 v2, vcc, s4, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_mov_b32 v4, v25
  v_mov_b32 v5, 0
  v_mov_b32 v27, s7
  v_add_co_u32 v4, vcc, s6, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_mov_b32 v6, v25
  v_mov_b32 v7, 0
  v_mov_b32 v27, s11
  v_add_co_u32 v6, vcc, s10, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  v_mov_b32 v20, v25
  v_mov_b32 v21, 0
  v_mov_b32 v27, s9
  v_add_co_u32 v20, vcc, s8, v20
  v_addc_co_u32 v21, vcc, v27, v21, vcc
  v_mov_b32 v22, v25
  v_mov_b32 v23, 0
  v_mov_b32 v27, s13
  v_add_co_u32 v22, vcc, s12, v22
  v_addc_co_u32 v23, vcc, v27, v23, vcc
  v_mov_b32 v18, v6
  v_mov_b32 v19, v7
  v_mov_b32 v16, 0
  s_mov_b32 s18, 19
.L10:
  global_load_dwordx4 v[8:11], v[2:3], off
  global_load_dwordx4 v[12:15], v[4:5], off
  s_waitcnt vmcnt(0)
  // FUSED_ELEMENT_0
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
  v_add_f32 v12, v8, v12
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
  global_store_dword v[6:7], v12, off
  v_fmac_f32 v16, v12, v12
  // FUSED_ELEMENT_1
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
  v_lshrrev_b32 v28, 16, v13
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v13, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v13
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v13, v28
  v_add_f32 v13, v9, v13
  v_lshrrev_b32 v28, 16, v13
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v13, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v13
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v13, v28
  global_store_dword v[6:7], v13, off offset:4
  v_fmac_f32 v16, v13, v13
  // FUSED_ELEMENT_2
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
  v_add_f32 v14, v10, v14
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
  global_store_dword v[6:7], v14, off offset:8
  v_fmac_f32 v16, v14, v14
  // FUSED_ELEMENT_3
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
  v_add_f32 v15, v11, v15
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
  global_store_dword v[6:7], v15, off offset:12
  v_fmac_f32 v16, v15, v15
  v_add_co_u32 v2, vcc, 0x400, v2
  v_addc_co_u32 v3, vcc, 0, v3, vcc
  v_add_co_u32 v4, vcc, 0x400, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  v_add_co_u32 v6, vcc, 0x400, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  s_sub_u32 s18, s18, 1
  s_cbranch_scc0 .L10
  s_waitcnt vmcnt(0)
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
  v_mul_f32 v16, 0x394ccccd, v16
  v_add_f32 v16, 0x358637bd, v16
  v_rsq_f32 v17, v16
  s_nop 0
  v_mov_b32 v2, v18
  v_mov_b32 v3, v19
  v_mov_b32 v4, v20
  v_mov_b32 v5, v21
  v_mov_b32 v6, v22
  v_mov_b32 v7, v23
  s_mov_b32 s18, 19
.L20:
  global_load_dwordx4 v[8:11], v[2:3], off
  global_load_dwordx4 v[12:15], v[4:5], off
  s_waitcnt vmcnt(0)
  v_mul_f32 v8, v17, v8
  v_add_f32 v12, 1.0, v12
  v_mul_f32 v8, v12, v8
  global_store_dword v[6:7], v8, off
  v_mul_f32 v9, v17, v9
  v_add_f32 v13, 1.0, v13
  v_mul_f32 v9, v13, v9
  global_store_dword v[6:7], v9, off offset:4
  v_mul_f32 v10, v17, v10
  v_add_f32 v14, 1.0, v14
  v_mul_f32 v10, v14, v10
  global_store_dword v[6:7], v10, off offset:8
  v_mul_f32 v11, v17, v11
  v_add_f32 v15, 1.0, v15
  v_mul_f32 v11, v15, v11
  global_store_dword v[6:7], v11, off offset:12
  v_add_co_u32 v2, vcc, 0x400, v2
  v_addc_co_u32 v3, vcc, 0, v3, vcc
  v_add_co_u32 v4, vcc, 0x400, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  v_add_co_u32 v6, vcc, 0x400, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  s_sub_u32 s18, s18, 1
  s_cbranch_scc0 .L20
.L99:
  s_endpgm
.Laddnorm5120_end:
.size addnorm5120, .Laddnorm5120_end-addnorm5120
.rodata
.p2align 6
.amdhsa_kernel addnorm5120
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 40
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
  - .name: addnorm5120
    .symbol: addnorm5120.kd
    .kernarg_segment_size: 40
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 36
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: h
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: delta
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: weight
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: h_new
        .offset: 24
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: !str y
        .offset: 32
        .size: 8
        .value_kind: global_buffer
        .address_space: global
...
.end_amdgpu_metadata
