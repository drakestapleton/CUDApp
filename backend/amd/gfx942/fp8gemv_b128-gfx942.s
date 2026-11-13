// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM is an independent encoding oracle, never the production object producer.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.p2align 8
.globl fp8gemv_b128
.type fp8gemv_b128,@function
fp8gemv_b128:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_load_dwordx2 s[12:13], s[0:1], 32
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s17, s2, s12
  s_cbranch_scc0 .L9
  v_and_b32 v0, 0x3ff, v0
  v_mov_b32 v1, s2
  v_mul_lo_u32 v6, v1, s13
  v_mul_hi_u32 v7, v1, s13
  v_mov_b32 v5, s5
  v_add_co_u32 v6, vcc, s4, v6
  v_addc_co_u32 v7, vcc, v5, v7, vcc
  v_add_co_u32 v6, vcc, v0, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_lshlrev_b32 v8, 2, v0
  v_mov_b32 v9, 0
  v_mov_b32 v5, s7
  v_add_co_u32 v8, vcc, s6, v8
  v_addc_co_u32 v9, vcc, v5, v9, vcc
  v_lshlrev_b32 v10, 2, v1
  v_lshrrev_b32 v11, 30, v1
  v_mov_b32 v5, s11
  v_add_co_u32 v10, vcc, s10, v10
  v_addc_co_u32 v11, vcc, v5, v11, vcc
  s_sub_u32 s4, s13, 1
  s_lshr_b32 s4, s4, 7
  s_add_u32 s4, s4, 1
  v_lshrrev_b32 v1, 7, v1
  v_mul_lo_u32 v12, v1, s4
  v_mul_hi_u32 v13, v1, s4
  v_lshrrev_b32 v5, 31, v12
  v_lshlrev_b32 v12, 1, v12
  v_lshlrev_b32 v13, 1, v13
  v_add_u32 v13, v5, v13
  v_mov_b32 v5, s9
  v_add_co_u32 v12, vcc, s8, v12
  v_addc_co_u32 v13, vcc, v5, v13, vcc
  v_mov_b32 v16, 0
  s_mov_b32 s16, s13
  s_sub_u32 s17, s16, 1
  s_cbranch_scc1 .L8
.L1:
  v_mov_b32 v4, 0
  v_cmp_gt_u32 vcc, s16, v0
  s_and_saveexec_b64 s[14:15], vcc
  global_load_ubyte v3, v[6:7], off
  global_load_dword v5, v[8:9], off
  s_waitcnt vmcnt(0)
  v_and_b32 v17, 128, v3
  v_lshlrev_b32 v17, 24, v17
  v_and_b32 v18, 127, v3
  v_mov_b32 v3, v18
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v17, v3
  v_mov_b32 v19, 126
  v_cmp_gt_u32 vcc, v18, v19
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 0x7fc00000
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 64, v0
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  global_load_ubyte v3, v[6:7], off offset:64
  global_load_dword v5, v[8:9], off offset:256
  s_waitcnt vmcnt(0)
  v_and_b32 v17, 128, v3
  v_lshlrev_b32 v17, 24, v17
  v_and_b32 v18, 127, v3
  v_mov_b32 v3, v18
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v17, v3
  v_mov_b32 v19, 126
  v_cmp_gt_u32 vcc, v18, v19
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 0x7fc00000
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_lshlrev_b32 v2, 2, v0
  v_xor_b32 v15, 128, v2
  ds_bpermute_b32 v5, v15, v4
  s_waitcnt lgkmcnt(0)
  v_add_f32 v4, v4, v5
  v_xor_b32 v15, 64, v2
  ds_bpermute_b32 v5, v15, v4
  s_waitcnt lgkmcnt(0)
  v_add_f32 v4, v4, v5
  v_xor_b32 v15, 32, v2
  ds_bpermute_b32 v5, v15, v4
  s_waitcnt lgkmcnt(0)
  v_add_f32 v4, v4, v5
  v_xor_b32 v15, 16, v2
  ds_bpermute_b32 v5, v15, v4
  s_waitcnt lgkmcnt(0)
  v_add_f32 v4, v4, v5
  v_xor_b32 v15, 8, v2
  ds_bpermute_b32 v5, v15, v4
  s_waitcnt lgkmcnt(0)
  v_add_f32 v4, v4, v5
  v_xor_b32 v15, 4, v2
  ds_bpermute_b32 v5, v15, v4
  s_waitcnt lgkmcnt(0)
  v_add_f32 v4, v4, v5
  v_mov_b32 v17, 0
  global_load_short_d16_hi v17, v[12:13], off
  s_waitcnt vmcnt(0)
  v_fmac_f32 v16, v17, v4
  s_sub_u32 s17, s16, 129
  s_cbranch_scc1 .L8
  s_sub_u32 s16, s16, 128
  v_add_co_u32 v6, vcc, 128, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_add_co_u32 v8, vcc, 512, v8
  v_addc_co_u32 v9, vcc, 0, v9, vcc
  v_add_co_u32 v12, vcc, 2, v12
  v_addc_co_u32 v13, vcc, 0, v13, vcc
  s_cbranch_scc0 .L1
.L8:
  v_cmp_gt_u32 vcc, 1, v0
  s_and_saveexec_b64 s[14:15], vcc
  global_store_dword v[10:11], v16, off
.L9:
  s_endpgm
.Lend:
.size fp8gemv_b128, .Lend-fp8gemv_b128
.rodata
.p2align 6
.amdhsa_kernel fp8gemv_b128
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 40
  .amdhsa_next_free_vgpr 24
  .amdhsa_next_free_sgpr 18
  .amdhsa_accum_offset 24
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: fp8gemv_b128
    .symbol: fp8gemv_b128.kd
    .kernarg_segment_size: 40
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 24
    .vgpr_count: 24
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - { .name: W, .offset: 0, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: x, .offset: 8, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: scale_inv, .offset: 16, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: !str y, .offset: 24, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: M, .offset: 32, .size: 4, .value_kind: by_value }
      - { .name: K, .offset: 36, .size: 4, .value_kind: by_value }
...
.end_amdgpu_metadata
