// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Test oracle only; never producer/executor input. Exact BF16 expansion; ascending-k FP32 FMA.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl bf16gemv
.p2align 8
.type bf16gemv,@function
bf16gemv:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  v_and_b32 v0, 0x3ff, v0
  s_waitcnt lgkmcnt(0)
  s_lshl_b32 s12, s2, 6
  v_add_u32 v1, s12, v0
  v_cmp_gt_u32 vcc, s10, v1
  s_and_saveexec_b64 s[14:15], vcc
  s_cbranch_execz .Lend
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  v_mul_lo_u32 v8, v1, s11
  v_mul_hi_u32 v9, v1, s11
  v_add_co_u32 v8, vcc, v8, v8
  v_addc_co_u32 v9, vcc, v9, v9, vcc
  v_mov_b32 v5, s5
  v_add_co_u32 v8, vcc, s4, v8
  v_addc_co_u32 v9, vcc, v5, v9, vcc
  v_mov_b32 v10, s6
  v_mov_b32 v11, s7
  v_mov_b32 v5, s9
  v_add_co_u32 v12, vcc, s8, v2
  v_addc_co_u32 v13, vcc, v5, v3, vcc
  v_mov_b32 v4, 0
  v_mov_b32 v7, 0
  s_sub_u32 s13, s11, 1
  s_cbranch_scc1 .Lstore
.Lloop:
  global_load_short_d16_hi v7, v[8:9], off
  global_load_dword v6, v[10:11], off
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v7, v6
  v_add_co_u32 v8, vcc, 2, v8
  v_addc_co_u32 v9, vcc, 0, v9, vcc
  v_add_co_u32 v10, vcc, 4, v10
  v_addc_co_u32 v11, vcc, 0, v11, vcc
  s_sub_u32 s13, s13, 1
  s_cbranch_scc0 .Lloop
.Lstore:
  global_store_dword v[12:13], v4, off
.Lend:
  s_endpgm
.Lbf16gemv_end:
.size bf16gemv, .Lbf16gemv_end-bf16gemv
.rodata
.p2align 6
.amdhsa_kernel bf16gemv
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 32
  .amdhsa_next_free_vgpr 14
  .amdhsa_next_free_sgpr 16
  .amdhsa_accum_offset 16
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: bf16gemv
    .symbol: bf16gemv.kd
    .kernarg_segment_size: 32
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
      - { .name: !str y, .offset: 16, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: M, .offset: 24, .size: 4, .value_kind: by_value }
      - { .name: K, .offset: 28, .size: 4, .value_kind: by_value }
...
.end_amdgpu_metadata
