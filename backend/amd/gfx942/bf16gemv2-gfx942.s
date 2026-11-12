// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM oracle only. No assembler output is production input.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl bf16gemv2
.p2align 8
.type bf16gemv2,@function
bf16gemv2:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_waitcnt lgkmcnt(0)
  s_mov_b32 s12, s10
  s_mov_b32 s13, s11
  s_sub_u32 s17, s2, s12
  s_cbranch_scc0 .L9
  v_and_b32 v0, 0x3ff, v0
  v_mov_b32 v1, s2
  v_lshlrev_b32 v2, 2, v0
  v_mul_lo_u32 v6, v1, s13
  v_mul_hi_u32 v7, v1, s13
  v_add_co_u32 v6, vcc, v6, v6
  v_addc_co_u32 v7, vcc, v7, v7, vcc
  v_mov_b32 v5, s5
  v_add_co_u32 v6, vcc, s4, v6
  v_addc_co_u32 v7, vcc, v5, v7, vcc
  v_lshlrev_b32 v5, 1, v2
  v_add_co_u32 v6, vcc, v5, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_lshlrev_b32 v8, 2, v2
  v_mov_b32 v9, 0
  v_mov_b32 v5, s7
  v_add_co_u32 v8, vcc, s6, v8
  v_addc_co_u32 v9, vcc, v5, v9, vcc
  v_lshlrev_b32 v10, 2, v1
  v_lshrrev_b32 v11, 30, v1
  v_mov_b32 v5, s9
  v_add_co_u32 v10, vcc, s8, v10
  v_addc_co_u32 v11, vcc, v5, v11, vcc
  v_mov_b32 v4, 0
  s_mov_b32 s16, s13
  s_sub_u32 s17, s16, 1024
  s_cbranch_scc1 .L2
.L1:
  global_load_dwordx2 v[16:17], v[6:7], off
  global_load_dwordx4 v[24:27], v[8:9], off
  global_load_dwordx2 v[18:19], v[6:7], off offset:512
  global_load_dwordx4 v[28:31], v[8:9], off offset:1024
  global_load_dwordx2 v[20:21], v[6:7], off offset:1024
  global_load_dwordx4 v[32:35], v[8:9], off offset:2048
  global_load_dwordx2 v[22:23], v[6:7], off offset:1536
  global_load_dwordx4 v[36:39], v[8:9], off offset:3072
  s_waitcnt vmcnt(0)
  v_lshlrev_b32 v3, 16, v16
  v_fmac_f32 v4, v3, v24
  v_and_b32 v3, 0xffff0000, v16
  v_fmac_f32 v4, v3, v25
  v_lshlrev_b32 v3, 16, v17
  v_fmac_f32 v4, v3, v26
  v_and_b32 v3, 0xffff0000, v17
  v_fmac_f32 v4, v3, v27
  v_lshlrev_b32 v3, 16, v18
  v_fmac_f32 v4, v3, v28
  v_and_b32 v3, 0xffff0000, v18
  v_fmac_f32 v4, v3, v29
  v_lshlrev_b32 v3, 16, v19
  v_fmac_f32 v4, v3, v30
  v_and_b32 v3, 0xffff0000, v19
  v_fmac_f32 v4, v3, v31
  v_lshlrev_b32 v3, 16, v20
  v_fmac_f32 v4, v3, v32
  v_and_b32 v3, 0xffff0000, v20
  v_fmac_f32 v4, v3, v33
  v_lshlrev_b32 v3, 16, v21
  v_fmac_f32 v4, v3, v34
  v_and_b32 v3, 0xffff0000, v21
  v_fmac_f32 v4, v3, v35
  v_lshlrev_b32 v3, 16, v22
  v_fmac_f32 v4, v3, v36
  v_and_b32 v3, 0xffff0000, v22
  v_fmac_f32 v4, v3, v37
  v_lshlrev_b32 v3, 16, v23
  v_fmac_f32 v4, v3, v38
  v_and_b32 v3, 0xffff0000, v23
  v_fmac_f32 v4, v3, v39
  v_add_co_u32 v6, vcc, 0x800, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_add_co_u32 v8, vcc, 0x1000, v8
  v_addc_co_u32 v9, vcc, 0, v9, vcc
  s_mov_b32 s16, s17
  s_sub_u32 s17, s16, 1024
  s_cbranch_scc0 .L1
.L2:
  s_sub_u32 s17, s16, 1
  s_cbranch_scc1 .L3
  v_add_u32 v15, 0, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off
  global_load_dword v5, v[8:9], off
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 1, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:2
  global_load_dword v5, v[8:9], off offset:4
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 2, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:4
  global_load_dword v5, v[8:9], off offset:8
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 3, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:6
  global_load_dword v5, v[8:9], off offset:12
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x100, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:512
  global_load_dword v5, v[8:9], off offset:1024
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x101, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:514
  global_load_dword v5, v[8:9], off offset:1028
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x102, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:516
  global_load_dword v5, v[8:9], off offset:1032
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x103, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:518
  global_load_dword v5, v[8:9], off offset:1036
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x200, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:1024
  global_load_dword v5, v[8:9], off offset:2048
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x201, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:1026
  global_load_dword v5, v[8:9], off offset:2052
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x202, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:1028
  global_load_dword v5, v[8:9], off offset:2056
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x203, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:1030
  global_load_dword v5, v[8:9], off offset:2060
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x300, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:1536
  global_load_dword v5, v[8:9], off offset:3072
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x301, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:1538
  global_load_dword v5, v[8:9], off offset:3076
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x302, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:1540
  global_load_dword v5, v[8:9], off offset:3080
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x303, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_short_d16_hi v3, v[6:7], off offset:1542
  global_load_dword v5, v[8:9], off offset:3084
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
.L3:
  v_lshlrev_b32 v2, 2, v0
  v_xor_b32 v15, 0x80, v2
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
  v_cmp_gt_u32 vcc, 1, v0
  s_and_saveexec_b64 s[14:15], vcc
  global_store_dword v[10:11], v4, off
.L9:
  s_endpgm
.Lbf16gemv2_end:
.size bf16gemv2, .Lbf16gemv2_end-bf16gemv2
.rodata
.p2align 6
.amdhsa_kernel bf16gemv2
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 32
  .amdhsa_next_free_vgpr 40
  .amdhsa_next_free_sgpr 18
  .amdhsa_accum_offset 40
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: bf16gemv2
    .symbol: bf16gemv2.kd
    .kernarg_segment_size: 32
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 24
    .vgpr_count: 40
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
