// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl attn256_prep_step
.p2align 8
.type attn256_prep_step,@function
attn256_prep_step:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx4 s[8:11], s[0:1], 16
  s_load_dwordx4 s[12:15], s[0:1], 32
  s_load_dwordx4 s[16:19], s[0:1], 48
  s_load_dwordx4 s[20:23], s[0:1], 64
  s_waitcnt lgkmcnt(0)
  s_load_dwordx2 s[24:25], s[0:1], 72
  s_waitcnt lgkmcnt(0)
  v_mov_b32 v46, s24
  v_mov_b32 v47, s25
  global_load_dword v45, v[46:47], off
  s_waitcnt vmcnt(0)
  v_readfirstlane_b32 s22, v45
  s_nop 1
  s_load_dword s23, s[0:1], 80
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s28, s2, 24
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s22, s23
  s_cbranch_scc0 .L99
  v_mov_b32 v45, s22
  v_lshrrev_b32 v47, 24, v45
  v_lshlrev_b32 v46, 8, v45
  v_add_co_u32 v46, vcc, s14, v46
  v_mov_b32 v45, s15
  v_addc_co_u32 v47, vcc, v45, v47, vcc
  v_readfirstlane_b32 s14, v46
  v_readfirstlane_b32 s15, v47
  s_nop 1
  v_and_b32 v0, 0x3ff, v0
  v_lshlrev_b32 v1, 2, v0
  v_mov_b32 v2, s2
  v_mov_b32 v3, 0
  v_lshrrev_b32 v27, 21, v2
  v_lshlrev_b32 v3, 11, v3
  v_add_u32 v3, v27, v3
  v_lshlrev_b32 v2, 11, v2
  v_mov_b32 v27, s5
  v_add_co_u32 v2, vcc, s4, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_lshlrev_b32 v31, 4, v0
  v_add_co_u32 v2, vcc, v31, v2
  v_addc_co_u32 v3, vcc, 0, v3, vcc
  v_mov_b32 v4, s2
  v_mov_b32 v5, 0
  v_lshrrev_b32 v27, 22, v4
  v_lshlrev_b32 v5, 10, v5
  v_add_u32 v5, v27, v5
  v_lshlrev_b32 v4, 10, v4
  v_mov_b32 v27, s17
  v_add_co_u32 v4, vcc, s16, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_lshlrev_b32 v31, 4, v0
  v_add_co_u32 v4, vcc, v31, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  v_mov_b32 v6, 0
  v_mov_b32 v7, 0
  v_mov_b32 v27, s11
  v_add_co_u32 v6, vcc, s10, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  v_lshlrev_b32 v31, 4, v0
  v_add_co_u32 v6, vcc, v31, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  global_load_dwordx4 v[8:11], v[2:3], off
  global_load_dwordx4 v[12:15], v[6:7], off
  s_waitcnt vmcnt(0)
  v_mov_b32 v16, 0
  v_fmac_f32 v16, v8, v8
  v_fmac_f32 v16, v9, v9
  v_fmac_f32 v16, v10, v10
  v_fmac_f32 v16, v11, v11
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
  v_mul_f32 v16, 0x3b800000, v16
  v_add_f32 v16, 0x358637bd, v16
  v_rsq_f32 v16, v16
  v_mov_b32 v44, 0x3f800000
  v_add_f32 v12, v44, v12
  v_mul_f32 v8, v16, v8
  v_mul_f32 v8, v12, v8
  v_add_f32 v13, v44, v13
  v_mul_f32 v9, v16, v9
  v_mul_f32 v9, v13, v9
  v_add_f32 v14, v44, v14
  v_mul_f32 v10, v16, v10
  v_mul_f32 v10, v14, v10
  v_add_f32 v15, v44, v15
  v_mul_f32 v11, v16, v11
  v_mul_f32 v11, v15, v11
  v_cmp_gt_u32 vcc, 16, v0
  s_and_saveexec_b64 s[26:27], vcc
  v_and_b32 v30, 7, v0
  v_lshlrev_b32 v30, 4, v30
  v_mov_b32 v31, 0
  v_mov_b32 v27, s15
  v_add_co_u32 v30, vcc, s14, v30
  v_addc_co_u32 v31, vcc, v27, v31, vcc
  global_load_dwordx4 v[18:21], v[30:31], off
  global_load_dwordx4 v[22:25], v[30:31], off offset:128
  s_waitcnt vmcnt(0)
  v_xor_b32 v28, 32, v1
  ds_bpermute_b32 v34, v28, v8
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, 8, v0
  s_and_saveexec_b64 s[24:25], vcc
  v_xor_b32 v34, 0x80000000, v34
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v8, v18, v8
  v_mul_f32 v34, v22, v34
  v_add_f32 v8, v34, v8
  ds_bpermute_b32 v34, v28, v9
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, 8, v0
  s_and_saveexec_b64 s[24:25], vcc
  v_xor_b32 v34, 0x80000000, v34
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v9, v19, v9
  v_mul_f32 v34, v23, v34
  v_add_f32 v9, v34, v9
  ds_bpermute_b32 v34, v28, v10
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, 8, v0
  s_and_saveexec_b64 s[24:25], vcc
  v_xor_b32 v34, 0x80000000, v34
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v10, v20, v10
  v_mul_f32 v34, v24, v34
  v_add_f32 v10, v34, v10
  ds_bpermute_b32 v34, v28, v11
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, 8, v0
  s_and_saveexec_b64 s[24:25], vcc
  v_xor_b32 v34, 0x80000000, v34
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v11, v21, v11
  v_mul_f32 v34, v25, v34
  v_add_f32 v11, v34, v11
  s_mov_b64 exec, s[26:27]
  global_store_dword v[4:5], v8, off
  global_store_dword v[4:5], v9, off offset:4
  global_store_dword v[4:5], v10, off offset:8
  global_store_dword v[4:5], v11, off offset:12
  s_sub_u32 s28, s2, 4
  s_cbranch_scc0 .L99
  v_mov_b32 v2, s2
  v_mov_b32 v3, 0
  v_lshrrev_b32 v27, 22, v2
  v_lshlrev_b32 v3, 10, v3
  v_add_u32 v3, v27, v3
  v_lshlrev_b32 v2, 10, v2
  v_mov_b32 v27, s7
  v_add_co_u32 v2, vcc, s6, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  v_lshlrev_b32 v31, 4, v0
  v_add_co_u32 v2, vcc, v31, v2
  v_addc_co_u32 v3, vcc, 0, v3, vcc
  v_mov_b32 v32, s2
  v_mov_b32 v33, 0
  v_lshrrev_b32 v27, 22, v32
  v_lshlrev_b32 v33, 10, v33
  v_add_u32 v33, v27, v33
  v_lshlrev_b32 v32, 10, v32
  v_mov_b32 v27, s9
  v_add_co_u32 v32, vcc, s8, v32
  v_addc_co_u32 v33, vcc, v27, v33, vcc
  v_lshlrev_b32 v31, 4, v0
  v_add_co_u32 v32, vcc, v31, v32
  v_addc_co_u32 v33, vcc, 0, v33, vcc
  global_load_dwordx4 v[40:43], v[32:33], off
  v_mov_b32 v6, 0
  v_mov_b32 v7, 0
  v_mov_b32 v27, s13
  v_add_co_u32 v6, vcc, s12, v6
  v_addc_co_u32 v7, vcc, v27, v7, vcc
  v_lshlrev_b32 v31, 4, v0
  v_add_co_u32 v6, vcc, v31, v6
  v_addc_co_u32 v7, vcc, 0, v7, vcc
  global_load_dwordx4 v[8:11], v[2:3], off
  global_load_dwordx4 v[12:15], v[6:7], off
  s_waitcnt vmcnt(0)
  v_mov_b32 v16, 0
  v_fmac_f32 v16, v8, v8
  v_fmac_f32 v16, v9, v9
  v_fmac_f32 v16, v10, v10
  v_fmac_f32 v16, v11, v11
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
  v_mul_f32 v16, 0x3b800000, v16
  v_add_f32 v16, 0x358637bd, v16
  v_rsq_f32 v16, v16
  v_mov_b32 v44, 0x3f800000
  v_add_f32 v12, v44, v12
  v_mul_f32 v8, v16, v8
  v_mul_f32 v8, v12, v8
  v_add_f32 v13, v44, v13
  v_mul_f32 v9, v16, v9
  v_mul_f32 v9, v13, v9
  v_add_f32 v14, v44, v14
  v_mul_f32 v10, v16, v10
  v_mul_f32 v10, v14, v10
  v_add_f32 v15, v44, v15
  v_mul_f32 v11, v16, v11
  v_mul_f32 v11, v15, v11
  v_cmp_gt_u32 vcc, 16, v0
  s_and_saveexec_b64 s[26:27], vcc
  v_and_b32 v30, 7, v0
  v_lshlrev_b32 v30, 4, v30
  v_mov_b32 v31, 0
  v_mov_b32 v27, s15
  v_add_co_u32 v30, vcc, s14, v30
  v_addc_co_u32 v31, vcc, v27, v31, vcc
  global_load_dwordx4 v[18:21], v[30:31], off
  global_load_dwordx4 v[22:25], v[30:31], off offset:128
  s_waitcnt vmcnt(0)
  v_xor_b32 v28, 32, v1
  ds_bpermute_b32 v34, v28, v8
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, 8, v0
  s_and_saveexec_b64 s[24:25], vcc
  v_xor_b32 v34, 0x80000000, v34
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v8, v18, v8
  v_mul_f32 v34, v22, v34
  v_add_f32 v8, v34, v8
  ds_bpermute_b32 v34, v28, v9
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, 8, v0
  s_and_saveexec_b64 s[24:25], vcc
  v_xor_b32 v34, 0x80000000, v34
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v9, v19, v9
  v_mul_f32 v34, v23, v34
  v_add_f32 v9, v34, v9
  ds_bpermute_b32 v34, v28, v10
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, 8, v0
  s_and_saveexec_b64 s[24:25], vcc
  v_xor_b32 v34, 0x80000000, v34
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v10, v20, v10
  v_mul_f32 v34, v24, v34
  v_add_f32 v10, v34, v10
  ds_bpermute_b32 v34, v28, v11
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, 8, v0
  s_and_saveexec_b64 s[24:25], vcc
  v_xor_b32 v34, 0x80000000, v34
  s_mov_b64 exec, s[24:25]
  v_mul_f32 v11, v21, v11
  v_mul_f32 v34, v25, v34
  v_add_f32 v11, v34, v11
  s_mov_b64 exec, s[26:27]
  v_mov_b32 v31, s2
  v_mul_lo_u32 v4, v31, s23
  v_mul_hi_u32 v5, v31, s23
  v_add_co_u32 v4, vcc, s22, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  v_lshrrev_b32 v27, 23, v4
  v_lshlrev_b32 v5, 9, v5
  v_add_u32 v5, v27, v5
  v_lshlrev_b32 v4, 9, v4
  v_mov_b32 v27, s19
  v_add_co_u32 v4, vcc, s18, v4
  v_addc_co_u32 v5, vcc, v27, v5, vcc
  v_lshlrev_b32 v31, 3, v0
  v_add_co_u32 v4, vcc, v31, v4
  v_addc_co_u32 v5, vcc, 0, v5, vcc
  v_mov_b32 v31, s2
  v_mul_lo_u32 v38, v31, s23
  v_mul_hi_u32 v39, v31, s23
  v_add_co_u32 v38, vcc, s22, v38
  v_addc_co_u32 v39, vcc, 0, v39, vcc
  v_lshrrev_b32 v27, 23, v38
  v_lshlrev_b32 v39, 9, v39
  v_add_u32 v39, v27, v39
  v_lshlrev_b32 v38, 9, v38
  v_mov_b32 v27, s21
  v_add_co_u32 v38, vcc, s20, v38
  v_addc_co_u32 v39, vcc, v27, v39, vcc
  v_lshlrev_b32 v31, 3, v0
  v_add_co_u32 v38, vcc, v31, v38
  v_addc_co_u32 v39, vcc, 0, v39, vcc
  v_lshrrev_b32 v16, 16, v8
  v_and_b32 v17, 1, v16
  v_add_u32 v17, 0x7fff, v17
  v_add_u32 v17, v8, v17
  v_lshrrev_b32 v17, 16, v17
  v_and_b32 v18, 0x7fffffff, v8
  v_mov_b32 v44, 0x7f800000
  v_cmp_gt_u32 vcc, v18, v44
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v17, 0x7fc0
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v19, v17
  v_lshrrev_b32 v16, 16, v9
  v_and_b32 v17, 1, v16
  v_add_u32 v17, 0x7fff, v17
  v_add_u32 v17, v9, v17
  v_lshrrev_b32 v17, 16, v17
  v_and_b32 v18, 0x7fffffff, v9
  v_mov_b32 v44, 0x7f800000
  v_cmp_gt_u32 vcc, v18, v44
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v17, 0x7fc0
  s_mov_b64 exec, s[24:25]
  v_lshlrev_b32 v17, 16, v17
  v_add_u32 v19, v17, v19
  global_store_dword v[4:5], v19, off
  v_lshrrev_b32 v16, 16, v10
  v_and_b32 v17, 1, v16
  v_add_u32 v17, 0x7fff, v17
  v_add_u32 v17, v10, v17
  v_lshrrev_b32 v17, 16, v17
  v_and_b32 v18, 0x7fffffff, v10
  v_mov_b32 v44, 0x7f800000
  v_cmp_gt_u32 vcc, v18, v44
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v17, 0x7fc0
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v19, v17
  v_lshrrev_b32 v16, 16, v11
  v_and_b32 v17, 1, v16
  v_add_u32 v17, 0x7fff, v17
  v_add_u32 v17, v11, v17
  v_lshrrev_b32 v17, 16, v17
  v_and_b32 v18, 0x7fffffff, v11
  v_mov_b32 v44, 0x7f800000
  v_cmp_gt_u32 vcc, v18, v44
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v17, 0x7fc0
  s_mov_b64 exec, s[24:25]
  v_lshlrev_b32 v17, 16, v17
  v_add_u32 v19, v17, v19
  global_store_dword v[4:5], v19, off offset:4
  v_lshrrev_b32 v16, 16, v40
  v_and_b32 v17, 1, v16
  v_add_u32 v17, 0x7fff, v17
  v_add_u32 v17, v40, v17
  v_lshrrev_b32 v17, 16, v17
  v_and_b32 v18, 0x7fffffff, v40
  v_mov_b32 v44, 0x7f800000
  v_cmp_gt_u32 vcc, v18, v44
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v17, 0x7fc0
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v19, v17
  v_lshrrev_b32 v16, 16, v41
  v_and_b32 v17, 1, v16
  v_add_u32 v17, 0x7fff, v17
  v_add_u32 v17, v41, v17
  v_lshrrev_b32 v17, 16, v17
  v_and_b32 v18, 0x7fffffff, v41
  v_mov_b32 v44, 0x7f800000
  v_cmp_gt_u32 vcc, v18, v44
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v17, 0x7fc0
  s_mov_b64 exec, s[24:25]
  v_lshlrev_b32 v17, 16, v17
  v_add_u32 v19, v17, v19
  global_store_dword v[38:39], v19, off
  v_lshrrev_b32 v16, 16, v42
  v_and_b32 v17, 1, v16
  v_add_u32 v17, 0x7fff, v17
  v_add_u32 v17, v42, v17
  v_lshrrev_b32 v17, 16, v17
  v_and_b32 v18, 0x7fffffff, v42
  v_mov_b32 v44, 0x7f800000
  v_cmp_gt_u32 vcc, v18, v44
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v17, 0x7fc0
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v19, v17
  v_lshrrev_b32 v16, 16, v43
  v_and_b32 v17, 1, v16
  v_add_u32 v17, 0x7fff, v17
  v_add_u32 v17, v43, v17
  v_lshrrev_b32 v17, 16, v17
  v_and_b32 v18, 0x7fffffff, v43
  v_mov_b32 v44, 0x7f800000
  v_cmp_gt_u32 vcc, v18, v44
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v17, 0x7fc0
  s_mov_b64 exec, s[24:25]
  v_lshlrev_b32 v17, 16, v17
  v_add_u32 v19, v17, v19
  global_store_dword v[38:39], v19, off offset:4
.L99:
  s_endpgm
.Lattn256_prep_step_end:
.size attn256_prep_step, .Lattn256_prep_step_end-attn256_prep_step
.rodata
.p2align 6
.amdhsa_kernel attn256_prep_step
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 84
  .amdhsa_next_free_vgpr 48
  .amdhsa_next_free_sgpr 29
  .amdhsa_accum_offset 48
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: attn256_prep_step
    .symbol: attn256_prep_step.kd
    .kernarg_segment_size: 84
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 35
    .vgpr_count: 48
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: Q_gate
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: K_raw
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: V_raw
        .offset: 16
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: Q_weight
        .offset: 24
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: K_weight
        .offset: 32
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: rope_table
        .offset: 40
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: Q_out
        .offset: 48
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: K_cache
        .offset: 56
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: V_cache
        .offset: 64
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: position
        .offset: 72
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: capacity
        .offset: 80
        .size: 4
        .value_kind: by_value
...
.end_amdgpu_metadata
