// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Independent LLVM18 oracle, never producer/executor input. Hardware NOT_RUN.
// Row-major C[M,N] = A[M,K]*B[K,N]; increasing-k single-rounded fmac.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl sgemm
.p2align 8
.type sgemm,@function
sgemm:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx2 s[8:9], s[0:1], 16
  s_load_dwordx2 s[10:11], s[0:1], 24
  s_load_dword s12, s[0:1], 32
  v_and_b32 v0, 0x3ff, v0
  s_waitcnt lgkmcnt(0)
  s_sub_u32 s14, s3, s10
  s_cbranch_scc0 .Lend
  s_lshl_b32 s14, s2, 6
  v_add_u32 v1, s14, v0
  v_cmp_gt_u32 vcc, s11, v1
  s_and_saveexec_b64 s[20:21], vcc
  s_cbranch_execz .Lend
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  s_mul_i32 s14, s3, s12
  s_mul_hi_u32 s15, s3, s12
  s_lshl_b64 s[14:15], s[14:15], 2
  s_add_u32 s14, s14, s4
  s_addc_u32 s15, s15, s5
  v_mov_b32 v8, s14
  v_mov_b32 v9, s15
  v_mov_b32 v7, s7
  v_add_co_u32 v10, vcc, s6, v2
  v_addc_co_u32 v11, vcc, v7, v3, vcc
  s_mul_i32 s16, s3, s11
  s_mul_hi_u32 s17, s3, s11
  s_lshl_b64 s[16:17], s[16:17], 2
  s_add_u32 s16, s16, s8
  s_addc_u32 s17, s17, s9
  v_mov_b32 v7, s17
  v_add_co_u32 v12, vcc, s16, v2
  v_addc_co_u32 v13, vcc, v7, v3, vcc
  s_mov_b32 s18, s11
  s_mov_b32 s19, 0
  s_lshl_b64 s[18:19], s[18:19], 2
  v_mov_b32 v7, s19
  v_mov_b32 v4, 0
  s_sub_u32 s13, s12, 1
  s_cbranch_scc1 .Lstore
.Lloop:
  global_load_dword v14, v[8:9], off
  global_load_dword v6, v[10:11], off
  s_waitcnt vmcnt(0)
  v_fmac_f32 v4, v14, v6
  v_add_co_u32 v8, vcc, 4, v8
  v_addc_co_u32 v9, vcc, 0, v9, vcc
  v_add_co_u32 v10, vcc, s18, v10
  v_addc_co_u32 v11, vcc, v7, v11, vcc
  s_sub_u32 s13, s13, 1
  s_cbranch_scc0 .Lloop
.Lstore:
  global_store_dword v[12:13], v4, off
.Lend:
  s_endpgm
.Lsgemm_end:
.size sgemm, .Lsgemm_end-sgemm
.rodata
.p2align 6
.amdhsa_kernel sgemm
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 36
  .amdhsa_next_free_vgpr 15
  .amdhsa_next_free_sgpr 22
  .amdhsa_accum_offset 16
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: sgemm
    .symbol: sgemm.kd
    .kernarg_segment_size: 36
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 28
    .vgpr_count: 15
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - { .name: A, .offset: 0, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: B, .offset: 8, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: C, .offset: 16, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: M, .offset: 24, .size: 4, .value_kind: by_value }
      - { .name: !str N, .offset: 28, .size: 4, .value_kind: by_value }
      - { .name: K, .offset: 32, .size: 4, .value_kind: by_value }
...
.end_amdgpu_metadata
