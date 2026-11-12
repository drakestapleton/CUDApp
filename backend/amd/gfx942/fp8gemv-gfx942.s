// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Test oracle only; never producer/executor input. E4M3FNUZ, row scale after ascending-k FMA.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl fp8gemv
.p2align 8
.type fp8gemv,@function
fp8gemv:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_load_dwordx2 s[12:13], s[0:1], 32
  v_and_b32 v0, 0x3ff, v0
  s_waitcnt lgkmcnt(0)
  s_lshl_b32 s14, s2, 6
  v_add_u32 v1, s14, v0
  v_cmp_gt_u32 vcc, s12, v1
  s_and_saveexec_b64 s[16:17], vcc
  s_cbranch_execz .Lend
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  v_mul_lo_u32 v8, v1, s13
  v_mul_hi_u32 v9, v1, s13
  v_mov_b32 v5, s5
  v_add_co_u32 v8, vcc, s4, v8
  v_addc_co_u32 v9, vcc, v5, v9, vcc
  v_mov_b32 v10, s6
  v_mov_b32 v11, s7
  v_mov_b32 v5, s9
  v_add_co_u32 v12, vcc, s8, v2
  v_addc_co_u32 v13, vcc, v5, v3, vcc
  v_mov_b32 v5, s11
  v_add_co_u32 v14, vcc, s10, v2
  v_addc_co_u32 v15, vcc, v5, v3, vcc
  v_mov_b32 v4, 0
  s_sub_u32 s15, s13, 1
  s_cbranch_scc1 .Lscale
.Lloop:
  global_load_ubyte v7, v[8:9], off
  global_load_dword v6, v[10:11], off
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v7, v7 src0_sel:BYTE_0
  v_fmac_f32 v4, v7, v6
  v_add_co_u32 v8, vcc, 1, v8
  v_addc_co_u32 v9, vcc, 0, v9, vcc
  v_add_co_u32 v10, vcc, 4, v10
  v_addc_co_u32 v11, vcc, 0, v11, vcc
  s_sub_u32 s15, s15, 1
  s_cbranch_scc0 .Lloop
.Lscale:
  global_load_dword v7, v[12:13], off
  s_waitcnt vmcnt(0)
  v_mul_f32 v4, v7, v4
  global_store_dword v[14:15], v4, off
.Lend:
  s_endpgm
.Lfp8gemv_end:
.size fp8gemv, .Lfp8gemv_end-fp8gemv
.rodata
.p2align 6
.amdhsa_kernel fp8gemv
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 40
  .amdhsa_next_free_vgpr 16
  .amdhsa_next_free_sgpr 18
  .amdhsa_accum_offset 16
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: fp8gemv
    .symbol: fp8gemv.kd
    .kernarg_segment_size: 40
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 24
    .vgpr_count: 16
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - { .name: W, .offset: 0, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: x, .offset: 8, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: s, .offset: 16, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: !str y, .offset: 24, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: M, .offset: 32, .size: 4, .value_kind: by_value }
      - { .name: K, .offset: 36, .size: 4, .value_kind: by_value }
...
.end_amdgpu_metadata
