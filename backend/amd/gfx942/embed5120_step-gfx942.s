// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl embed5120_step
.p2align 8
.type embed5120_step,@function
embed5120_step:
  s_load_dwordx2 s[4:5], s[0:1], 0
  s_load_dwordx2 s[6:7], s[0:1], 8
  s_waitcnt lgkmcnt(0)
  s_load_dwordx2 s[8:9], s[0:1], 16
  s_waitcnt lgkmcnt(0)
  v_mov_b32 v30, s8
  v_mov_b32 v31, s9
  global_load_dword v29, v[30:31], off
  s_waitcnt vmcnt(0)
  v_readfirstlane_b32 s8, v29
  s_nop 1
  s_load_dword s10, s[0:1], 24
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s28, s8, s10
  s_cbranch_scc0 .L99
  s_mov_b32 s28, 10240
  v_mul_lo_u32 v30, v29, s28
  v_mul_hi_u32 v31, v29, s28
  v_add_co_u32 v30, vcc, s4, v30
  v_mov_b32 v29, s5
  v_addc_co_u32 v31, vcc, v29, v31, vcc
  v_readfirstlane_b32 s4, v30
  v_readfirstlane_b32 s5, v31
  s_nop 1
  s_mov_b32 s29, 0x50
  s_sub_u32 s28, s2, s29
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  v_and_b32 v0, 0x3ff, v0
  v_mov_b32 v1, s2
  v_lshlrev_b32 v1, 6, v1
  v_add_u32 v1, v0, v1
  v_lshlrev_b32 v2, 1, v1
  v_lshrrev_b32 v3, 31, v1
  v_mov_b32 v27, s5
  v_add_co_u32 v2, vcc, s4, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_lshlrev_b32 v4, 2, v1
  v_lshrrev_b32 v5, 30, v1
  v_mov_b32 v27, s7
  v_add_co_u32 v4, vcc, s6, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_mov_b32 v8, 0
  global_load_short_d16_hi v8, v[2:3], off
  s_waitcnt vmcnt(0)
  global_store_dword v[4:5], v8, off
.L99:
  s_endpgm
.Lembed5120_step_end:
.size embed5120_step, .Lembed5120_step_end-embed5120_step
.rodata
.p2align 6
.amdhsa_kernel embed5120_step
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 28
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
  - .name: embed5120_step
    .symbol: embed5120_step.kd
    .kernarg_segment_size: 28
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 36
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: table
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: output
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: token
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: vocab
        .offset: 24
        .size: 4
        .value_kind: by_value
...
.end_amdgpu_metadata
