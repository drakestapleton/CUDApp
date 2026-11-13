// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl residual5120
.p2align 8
.type residual5120,@function
residual5120:
  s_load_dwordx2 s[4:5], s[0:1], 0
  s_load_dwordx2 s[6:7], s[0:1], 8
  s_load_dwordx2 s[8:9], s[0:1], 16
  s_waitcnt lgkmcnt(0)
  s_mov_b32 s29, 0x50
  s_sub_u32 s28, s2, s29
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  v_and_b32 v0, 0x3ff, v0
  v_mov_b32 v1, s2
  v_lshlrev_b32 v1, 6, v1
  v_add_u32 v1, v0, v1
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  v_mov_b32 v27, s5
  v_add_co_u32 v2, vcc, s4, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_lshlrev_b32 v4, 2, v1
  v_lshrrev_b32 v5, 30, v1
  v_mov_b32 v27, s7
  v_add_co_u32 v4, vcc, s6, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_lshlrev_b32 v6, 2, v1
  v_lshrrev_b32 v7, 30, v1
  v_mov_b32 v27, s9
  v_add_co_u32 v6, vcc, s8, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  global_load_dword v8, v[2:3], off
  global_load_dword v9, v[4:5], off
  s_waitcnt vmcnt(0)
  // BF16_ROUND_RESIDUAL
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
  // END_BF16_ROUND_RESIDUAL
  // BF16_ROUND_PROJECTION
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
  // END_BF16_ROUND_PROJECTION
  v_add_f32 v9, v8, v9
  // BF16_ROUND_OUTPUT
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
  // END_BF16_ROUND_OUTPUT
  global_store_dword v[6:7], v9, off
.L99:
  s_endpgm
.Lresidual5120_end:
.size residual5120, .Lresidual5120_end-residual5120
.rodata
.p2align 6
.amdhsa_kernel residual5120
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 24
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
  - .name: residual5120
    .symbol: residual5120.kd
    .kernarg_segment_size: 24
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 36
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: residual
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: projection
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: output
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
...
.end_amdgpu_metadata
