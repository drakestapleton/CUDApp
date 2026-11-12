// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM oracle only. No assembler output is production input.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl fp8gemv2
.p2align 8
.type fp8gemv2,@function
fp8gemv2:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_load_dwordx2 s[12:13], s[0:1], 32
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s17, s2, s12
  s_cbranch_scc0 .L9
  v_and_b32 v0, 0x3ff, v0
  v_mov_b32 v1, s2
  v_lshlrev_b32 v2, 2, v0
  v_mul_lo_u32 v6, v1, s13
  v_mul_hi_u32 v7, v1, s13
  v_mov_b32 v5, s5
  v_add_co_u32 v6, vcc, s4, v6
  v_addc_co_u32 v7, vcc, v5, v7, vcc
  v_mov_b32 v5, v2
  v_add_co_u32 v6, vcc, v5, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_lshlrev_b32 v8, 2, v2
  v_mov_b32 v9, 0
  v_mov_b32 v5, s7
  v_add_co_u32 v8, vcc, s6, v8
  v_addc_co_u32 v9, vcc, v5, v9, vcc
  v_lshlrev_b32 v10, 2, v1
  v_lshrrev_b32 v11, 30, v1
  v_mov_b32 v12, v10
  v_mov_b32 v13, v11
  v_mov_b32 v5, s9
  v_add_co_u32 v12, vcc, s8, v12
  v_addc_co_u32 v13, vcc, v5, v13, vcc
  v_mov_b32 v5, s11
  v_add_co_u32 v10, vcc, s10, v10
  v_addc_co_u32 v11, vcc, v5, v11, vcc
  v_mov_b32 v4, 0
  s_mov_b32 s16, s13
  s_sub_u32 s17, s16, 1024
  s_cbranch_scc1 .L2
.L1:
  global_load_dword v16, v[6:7], off
  global_load_dwordx4 v[24:27], v[8:9], off
  global_load_dword v18, v[6:7], off offset:256
  global_load_dwordx4 v[28:31], v[8:9], off offset:1024
  global_load_dword v20, v[6:7], off offset:512
  global_load_dwordx4 v[32:35], v[8:9], off offset:2048
  global_load_dword v22, v[6:7], off offset:768
  global_load_dwordx4 v[36:39], v[8:9], off offset:3072
  s_waitcnt vmcnt(0)
  v_mov_b32 v3, v16
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v24
  v_lshrrev_b32 v3, 8, v16
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v25
  v_lshrrev_b32 v3, 16, v16
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v26
  v_lshrrev_b32 v3, 24, v16
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v27
  v_mov_b32 v3, v18
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v28
  v_lshrrev_b32 v3, 8, v18
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v29
  v_lshrrev_b32 v3, 16, v18
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v30
  v_lshrrev_b32 v3, 24, v18
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v31
  v_mov_b32 v3, v20
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v32
  v_lshrrev_b32 v3, 8, v20
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v33
  v_lshrrev_b32 v3, 16, v20
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v34
  v_lshrrev_b32 v3, 24, v20
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v35
  v_mov_b32 v3, v22
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v36
  v_lshrrev_b32 v3, 8, v22
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v37
  v_lshrrev_b32 v3, 16, v22
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v38
  v_lshrrev_b32 v3, 24, v22
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v39
  v_add_co_u32 v6, vcc, 0x400, v6
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
  global_load_ubyte v3, v[6:7], off
  global_load_dword v5, v[8:9], off
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 1, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:1
  global_load_dword v5, v[8:9], off offset:4
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 2, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:2
  global_load_dword v5, v[8:9], off offset:8
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 3, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:3
  global_load_dword v5, v[8:9], off offset:12
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x100, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:256
  global_load_dword v5, v[8:9], off offset:1024
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x101, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:257
  global_load_dword v5, v[8:9], off offset:1028
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x102, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:258
  global_load_dword v5, v[8:9], off offset:1032
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x103, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:259
  global_load_dword v5, v[8:9], off offset:1036
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x200, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:512
  global_load_dword v5, v[8:9], off offset:2048
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x201, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:513
  global_load_dword v5, v[8:9], off offset:2052
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x202, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:514
  global_load_dword v5, v[8:9], off offset:2056
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x203, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:515
  global_load_dword v5, v[8:9], off offset:2060
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x300, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:768
  global_load_dword v5, v[8:9], off offset:3072
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x301, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:769
  global_load_dword v5, v[8:9], off offset:3076
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x302, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:770
  global_load_dword v5, v[8:9], off offset:3080
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 0x303, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v3, 0
  global_load_ubyte v3, v[6:7], off offset:771
  global_load_dword v5, v[8:9], off offset:3084
  s_waitcnt vmcnt(0)
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
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
  global_load_dword v14, v[12:13], off
  s_waitcnt vmcnt(0)
  v_mul_f32 v4, v14, v4
  global_store_dword v[10:11], v4, off
.L9:
  s_endpgm
.Lfp8gemv2_end:
.size fp8gemv2, .Lfp8gemv2_end-fp8gemv2
.rodata
.p2align 6
.amdhsa_kernel fp8gemv2
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 40
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
  - .name: fp8gemv2
    .symbol: fp8gemv2.kd
    .kernarg_segment_size: 40
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
      - { .name: s, .offset: 16, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: !str y, .offset: 24, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: M, .offset: 32, .size: 4, .value_kind: by_value }
      - { .name: K, .offset: 36, .size: 4, .value_kind: by_value }
...
.end_amdgpu_metadata
