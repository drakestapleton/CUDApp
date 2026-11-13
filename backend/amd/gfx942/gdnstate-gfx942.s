// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl gdnstate
.p2align 8
.type gdnstate,@function
gdnstate:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_load_dwordx4 s[12:15], s[0:1], 32
  s_load_dwordx2 s[16:17], s[0:1], 48
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s28, s2, 3
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 16
  s_cbranch_scc0 .L99
  s_mul_i32 s28, s3, 3
  s_add_u32 s28, s28, s2
  v_and_b32 v0, 0x3ff, v0
  v_lshlrev_b32 v1, 2, v0
  s_lshl_b32 s29, s28, 16
  v_add_u32 v2, s29, v1
  v_mov_b32 v3, 0
  v_mov_b32 v4, v2
  v_mov_b32 v5, 0
  v_mov_b32 v27, s9
  v_add_co_u32 v2, vcc, s8, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_mov_b32 v27, s11
  v_add_co_u32 v4, vcc, s10, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_mov_b32 v30, v4
  v_mov_b32 v31, v5
  s_lshl_b32 s29, s3, 9
  v_mov_b32 v6, s29
  v_mov_b32 v7, 0
  v_mov_b32 v8, v6
  v_mov_b32 v9, 0
  v_mov_b32 v27, s5
  v_add_co_u32 v6, vcc, s4, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  v_add_co_u32 v6, vcc, 0x2000, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_mov_b32 v27, s5
  v_add_co_u32 v8, vcc, s4, v8
  v_addc_co_u32 v9, vcc, v27, v9, vcc
  s_lshl_b32 s29, s28, 9
  v_add_u32 v10, s29, v1
  v_mov_b32 v11, 0
  v_mov_b32 v12, v10
  v_mov_b32 v13, 0
  v_mov_b32 v27, s7
  v_add_co_u32 v10, vcc, s6, v10
  v_addc_co_u32 v11, vcc, v27, v11, vcc
  v_mov_b32 v27, s13
  v_add_co_u32 v12, vcc, s12, v12
  v_addc_co_u32 v13, vcc, v27, v13, vcc
  s_lshl_b32 s29, s28, 2
  v_mov_b32 v14, s29
  v_mov_b32 v15, 0
  v_mov_b32 v27, s15
  v_add_co_u32 v14, vcc, s14, v14
  v_addc_co_u32 v15, vcc, v27, v15, vcc
  global_load_dword v20, v[14:15], off
  v_mov_b32 v14, s29
  v_mov_b32 v15, 0
  v_mov_b32 v27, s17
  v_add_co_u32 v14, vcc, s16, v14
  v_addc_co_u32 v15, vcc, v27, v15, vcc
  global_load_dword v21, v[14:15], off
  global_load_dword v18, v[10:11], off
  global_load_dword v19, v[10:11], off offset:256
  s_waitcnt vmcnt(0)
  v_mov_b32 v22, 0
  v_mov_b32 v23, 0
  s_mov_b32 s18, 127
.L10:
  global_load_dword v16, v[2:3], off
  global_load_dword v17, v[2:3], off offset:256
  global_load_dword v24, v[6:7], off
  s_waitcnt vmcnt(0)
  v_mul_f32 v16, v20, v16
  v_mul_f32 v17, v20, v17
  v_mul_f32 v26, v24, v16
  v_add_f32 v22, v26, v22
  v_mul_f32 v26, v24, v17
  v_add_f32 v23, v26, v23
  global_store_dword v[4:5], v16, off
  global_store_dword v[4:5], v17, off offset:256
  v_add_co_u32 v2, vcc, 0x200, v2
  v_addc_co_u32 v3, vcc, 0, v3, vcc
  v_add_co_u32 v4, vcc, 0x200, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  v_add_co_u32 v6, vcc, 4, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  s_sub_u32 s18, s18, 1
  s_cbranch_scc0 .L10
  s_waitcnt vmcnt(0)
  v_sub_f32 v18, v18, v22
  v_mul_f32 v18, v21, v18
  v_sub_f32 v19, v19, v23
  v_mul_f32 v19, v21, v19
  v_mov_b32 v4, v30
  v_mov_b32 v5, v31
  v_mov_b32 v6, v8
  v_mov_b32 v7, v9
  v_add_co_u32 v6, vcc, 0x2000, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_mov_b32 v22, 0
  v_mov_b32 v23, 0
  s_mov_b32 s18, 127
.L20:
  global_load_dword v16, v[4:5], off
  global_load_dword v17, v[4:5], off offset:256
  global_load_dword v24, v[6:7], off
  global_load_dword v25, v[8:9], off
  s_waitcnt vmcnt(0)
  v_mul_f32 v26, v24, v18
  v_add_f32 v16, v26, v16
  v_mul_f32 v26, v24, v19
  v_add_f32 v17, v26, v17
  v_mul_f32 v26, v25, v16
  v_add_f32 v22, v26, v22
  v_mul_f32 v26, v25, v17
  v_add_f32 v23, v26, v23
  global_store_dword v[4:5], v16, off
  global_store_dword v[4:5], v17, off offset:256
  v_add_co_u32 v4, vcc, 0x200, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  v_add_co_u32 v6, vcc, 4, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_add_co_u32 v8, vcc, 4, v8
  v_addc_co_u32 v9, vcc, 0, v9, vcc
  s_sub_u32 s18, s18, 1
  s_cbranch_scc0 .L20
  // BF16_ROUND_CORE0
  v_lshrrev_b32 v28, 16, v22
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v22, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v22
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v22, v28
  // END_BF16_ROUND_CORE0
  global_store_dword v[12:13], v22, off
  // BF16_ROUND_CORE1
  v_lshrrev_b32 v28, 16, v23
  v_and_b32 v28, 1, v28
  v_add_u32 v28, 0x7fff, v28
  v_add_u32 v28, v23, v28
  v_and_b32 v28, 0xffff0000, v28
  v_and_b32 v29, 0x7fffffff, v23
  v_mov_b32 v30, 0x7f800000
  v_cmp_gt_u32 vcc, v29, v30
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v28, 0x7fc00000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v23, v28
  // END_BF16_ROUND_CORE1
  global_store_dword v[12:13], v23, off offset:256
.L99:
  s_endpgm
.Lgdnstate_end:
.size gdnstate, .Lgdnstate_end-gdnstate
.rodata
.p2align 6
.amdhsa_kernel gdnstate
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 56
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
  - .name: gdnstate
    .symbol: gdnstate.kd
    .kernarg_segment_size: 56
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 35
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: qk
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: value
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: oldstate
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: newstate
        .offset: 24
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: output
        .offset: 32
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: decay
        .offset: 40
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: beta
        .offset: 48
        .size: 8
        .value_kind: global_buffer
        .address_space: global
...
.end_amdgpu_metadata
