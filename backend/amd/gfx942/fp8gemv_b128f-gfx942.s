// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM is an independent encoding oracle, never the production object producer.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.p2align 8
.globl fp8gemv_b128f
.type fp8gemv_b128f,@function
fp8gemv_b128f:
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
  v_add_co_u32 v6, vcc, v2, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_lshlrev_b32 v8, 2, v2
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
// Scale pointer starts at row block plus lane/32; v45 is two-block byte stride.
  v_mov_b32 v45, 4
  v_mov_b32 v46, 2
  v_lshrrev_b32 v5, 5, v0
  v_mul_lo_u32 v5, v5, v46
  v_add_co_u32 v12, vcc, v5, v12
  v_addc_co_u32 v13, vcc, 0, v13, vcc
  v_mov_b32 v14, 0
  v_mov_b32 v42, 126
  s_mov_b32 s16, s13
  s_sub_u32 s17, s16, 1024
  s_cbranch_scc1 .L2
.L1:
// Four W dwords, sixteen x dwords, and four scale values in flight per lane.
  global_load_dword v16, v[6:7], off
  global_load_dwordx4 v[24:27], v[8:9], off
  v_mov_b32 v20, 0
  global_load_short_d16_hi v20, v[12:13], off
  v_add_co_u32 v12, vcc, v45, v12
  v_addc_co_u32 v13, vcc, 0, v13, vcc
  global_load_dword v17, v[6:7], off offset:256
  global_load_dwordx4 v[28:31], v[8:9], off offset:1024
  v_mov_b32 v21, 0
  global_load_short_d16_hi v21, v[12:13], off
  v_add_co_u32 v12, vcc, v45, v12
  v_addc_co_u32 v13, vcc, 0, v13, vcc
  global_load_dword v18, v[6:7], off offset:512
  global_load_dwordx4 v[32:35], v[8:9], off offset:2048
  v_mov_b32 v22, 0
  global_load_short_d16_hi v22, v[12:13], off
  v_add_co_u32 v12, vcc, v45, v12
  v_addc_co_u32 v13, vcc, 0, v13, vcc
  global_load_dword v19, v[6:7], off offset:768
  global_load_dwordx4 v[36:39], v[8:9], off offset:3072
  v_mov_b32 v23, 0
  global_load_short_d16_hi v23, v[12:13], off
  v_add_co_u32 v12, vcc, v45, v12
  v_addc_co_u32 v13, vcc, 0, v13, vcc
  s_waitcnt vmcnt(0)
  v_mov_b32 v4, 0
  v_mov_b32 v3, v16
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v24
  v_lshrrev_b32 v3, 8, v16
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v25
  v_lshrrev_b32 v3, 16, v16
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v26
  v_lshrrev_b32 v3, 24, v16
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v27
  v_fmac_f32 v14, v20, v4
  v_mov_b32 v4, 0
  v_mov_b32 v3, v17
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v28
  v_lshrrev_b32 v3, 8, v17
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v29
  v_lshrrev_b32 v3, 16, v17
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v30
  v_lshrrev_b32 v3, 24, v17
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v31
  v_fmac_f32 v14, v21, v4
  v_mov_b32 v4, 0
  v_mov_b32 v3, v18
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v32
  v_lshrrev_b32 v3, 8, v18
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v33
  v_lshrrev_b32 v3, 16, v18
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v34
  v_lshrrev_b32 v3, 24, v18
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v35
  v_fmac_f32 v14, v22, v4
  v_mov_b32 v4, 0
  v_mov_b32 v3, v19
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v36
  v_lshrrev_b32 v3, 8, v19
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v37
  v_lshrrev_b32 v3, 16, v19
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v38
  v_lshrrev_b32 v3, 24, v19
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v39
  v_fmac_f32 v14, v23, v4
  v_add_co_u32 v6, vcc, 1024, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_add_co_u32 v8, vcc, 4096, v8
  v_addc_co_u32 v9, vcc, 0, v9, vcc
  s_mov_b32 s16, s17
  s_sub_u32 s17, s16, 1024
  s_cbranch_scc0 .L1
.L2:
  s_sub_u32 s17, s16, 1
  s_cbranch_scc1 .L8
.L3:
// Predicated residual chunk: no wide access crosses K or the scale grid.
  v_mov_b32 v4, 0
  v_add_u32 v15, 0, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  global_load_ubyte v3, v[6:7], off
  global_load_dword v5, v[8:9], off
  s_waitcnt vmcnt(0)
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 1, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  global_load_ubyte v3, v[6:7], off offset:1
  global_load_dword v5, v[8:9], off offset:4
  s_waitcnt vmcnt(0)
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 2, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  global_load_ubyte v3, v[6:7], off offset:2
  global_load_dword v5, v[8:9], off offset:8
  s_waitcnt vmcnt(0)
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_add_u32 v15, 3, v2
  v_cmp_gt_u32 vcc, s16, v15
  s_and_saveexec_b64 s[14:15], vcc
  global_load_ubyte v3, v[6:7], off offset:3
  global_load_dword v5, v[8:9], off offset:12
  s_waitcnt vmcnt(0)
  v_and_b32 v40, 128, v3
  v_lshlrev_b32 v40, 24, v40
  v_and_b32 v41, 127, v3
  v_mov_b32 v3, v41
  v_cvt_f32_fp8 v3, v3 src0_sel:BYTE_0
  v_mul_f32 v3, 2.0, v3
  v_xor_b32 v3, v40, v3
  v_cmp_gt_u32 vcc, v41, v42
  s_and_saveexec_b64 s[4:5], vcc
  v_mov_b32 v3, 2143289344
  s_mov_b64 exec, s[4:5]
  v_fmac_f32 v4, v3, v5
  s_mov_b64 exec, s[14:15]
  v_cmp_gt_u32 vcc, s16, v2
  s_and_saveexec_b64 s[14:15], vcc
  v_mov_b32 v20, 0
  global_load_short_d16_hi v20, v[12:13], off
  s_waitcnt vmcnt(0)
  v_fmac_f32 v14, v20, v4
  s_mov_b64 exec, s[14:15]
  s_sub_u32 s17, s16, 257
  s_cbranch_scc1 .L8
  s_sub_u32 s16, s16, 256
  v_add_co_u32 v6, vcc, 256, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  v_add_co_u32 v8, vcc, 1024, v8
  v_addc_co_u32 v9, vcc, 0, v9, vcc
  v_add_co_u32 v12, vcc, v45, v12
  v_addc_co_u32 v13, vcc, 0, v13, vcc
  s_cbranch_scc0 .L3
.L8:
  v_xor_b32 v15, 128, v2
  ds_bpermute_b32 v5, v15, v14
  s_waitcnt lgkmcnt(0)
  v_add_f32 v14, v14, v5
  v_xor_b32 v15, 64, v2
  ds_bpermute_b32 v5, v15, v14
  s_waitcnt lgkmcnt(0)
  v_add_f32 v14, v14, v5
  v_xor_b32 v15, 32, v2
  ds_bpermute_b32 v5, v15, v14
  s_waitcnt lgkmcnt(0)
  v_add_f32 v14, v14, v5
  v_xor_b32 v15, 16, v2
  ds_bpermute_b32 v5, v15, v14
  s_waitcnt lgkmcnt(0)
  v_add_f32 v14, v14, v5
  v_xor_b32 v15, 8, v2
  ds_bpermute_b32 v5, v15, v14
  s_waitcnt lgkmcnt(0)
  v_add_f32 v14, v14, v5
  v_xor_b32 v15, 4, v2
  ds_bpermute_b32 v5, v15, v14
  s_waitcnt lgkmcnt(0)
  v_add_f32 v14, v14, v5
  v_cmp_gt_u32 vcc, 1, v0
  s_and_saveexec_b64 s[14:15], vcc
  global_store_dword v[10:11], v14, off
.L9:
  s_endpgm
.Lend:
.size fp8gemv_b128f, .Lend-fp8gemv_b128f
.rodata
.p2align 6
.amdhsa_kernel fp8gemv_b128f
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 40
  .amdhsa_next_free_vgpr 48
  .amdhsa_next_free_sgpr 18
  .amdhsa_accum_offset 48
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: fp8gemv_b128f
    .symbol: fp8gemv_b128f.kd
    .kernarg_segment_size: 40
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 24
    .vgpr_count: 48
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
