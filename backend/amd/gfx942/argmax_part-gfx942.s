// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl argmax_part
.p2align 8
.type argmax_part,@function
argmax_part:
  s_load_dwordx2 s[4:5], s[0:1], 0
  s_load_dwordx2 s[6:7], s[0:1], 8
  s_waitcnt lgkmcnt(0)
  s_mov_b32 s29, 0xf28
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
  global_load_dword v8, v[2:3], off
  s_waitcnt vmcnt(0)
  v_mov_b32 v9, v1
  v_and_b32 v20, 0x7fffffff, v8
  v_lshrrev_b32 v21, 31, v8
  v_xor_b32 v8, 0x80000000, v8
  v_mov_b32 v22, 0
  v_cmp_gt_u32 vcc, v21, v22
  s_and_saveexec_b64 s[24:25], vcc
  v_xor_b32 v8, 0x7fffffff, v8
  s_mov_b64 exec, s[24:25]
  v_cmp_gt_u32 vcc, 1, v20
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v8, 0x80000000
  s_mov_b64 exec, s[24:25]
  v_mov_b32 v21, 0x7f800000
  v_cmp_gt_u32 vcc, v20, v21
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v8, -1
  s_mov_b64 exec, s[24:25]
  v_lshlrev_b32 v26, 2, v0
  v_xor_b32 v28, 0x80, v26
  ds_bpermute_b32 v10, v28, v8
  ds_bpermute_b32 v11, v28, v9
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, v10, v8
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v8, v10
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  v_xor_b32 v20, v10, v8
  v_cmp_gt_u32 vcc, 1, v20
  s_and_saveexec_b64 s[22:23], vcc
  v_cmp_gt_u32 vcc, v9, v11
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  s_mov_b64 exec, s[22:23]
  v_xor_b32 v28, 64, v26
  ds_bpermute_b32 v10, v28, v8
  ds_bpermute_b32 v11, v28, v9
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, v10, v8
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v8, v10
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  v_xor_b32 v20, v10, v8
  v_cmp_gt_u32 vcc, 1, v20
  s_and_saveexec_b64 s[22:23], vcc
  v_cmp_gt_u32 vcc, v9, v11
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  s_mov_b64 exec, s[22:23]
  v_xor_b32 v28, 32, v26
  ds_bpermute_b32 v10, v28, v8
  ds_bpermute_b32 v11, v28, v9
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, v10, v8
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v8, v10
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  v_xor_b32 v20, v10, v8
  v_cmp_gt_u32 vcc, 1, v20
  s_and_saveexec_b64 s[22:23], vcc
  v_cmp_gt_u32 vcc, v9, v11
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  s_mov_b64 exec, s[22:23]
  v_xor_b32 v28, 16, v26
  ds_bpermute_b32 v10, v28, v8
  ds_bpermute_b32 v11, v28, v9
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, v10, v8
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v8, v10
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  v_xor_b32 v20, v10, v8
  v_cmp_gt_u32 vcc, 1, v20
  s_and_saveexec_b64 s[22:23], vcc
  v_cmp_gt_u32 vcc, v9, v11
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  s_mov_b64 exec, s[22:23]
  v_xor_b32 v28, 8, v26
  ds_bpermute_b32 v10, v28, v8
  ds_bpermute_b32 v11, v28, v9
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, v10, v8
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v8, v10
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  v_xor_b32 v20, v10, v8
  v_cmp_gt_u32 vcc, 1, v20
  s_and_saveexec_b64 s[22:23], vcc
  v_cmp_gt_u32 vcc, v9, v11
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  s_mov_b64 exec, s[22:23]
  v_xor_b32 v28, 4, v26
  ds_bpermute_b32 v10, v28, v8
  ds_bpermute_b32 v11, v28, v9
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, v10, v8
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v8, v10
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  v_xor_b32 v20, v10, v8
  v_cmp_gt_u32 vcc, 1, v20
  s_and_saveexec_b64 s[22:23], vcc
  v_cmp_gt_u32 vcc, v9, v11
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v9, v11
  s_mov_b64 exec, s[24:25]
  s_mov_b64 exec, s[22:23]
  v_cmp_gt_u32 vcc, 1, v0
  s_and_saveexec_b64 s[24:25], vcc
  v_mov_b32 v1, s2
  v_lshlrev_b32 v2, 3, v1
  v_lshrrev_b32 v3, 29, v1
  v_mov_b32 v27, s7
  v_add_co_u32 v2, vcc, s6, v2
  v_addc_co_u32 v3, vcc, v27, v3, vcc
  global_store_dword v[2:3], v8, off
  global_store_dword v[2:3], v9, off offset:4
.L99:
  s_endpgm
.Largmax_part_end:
.size argmax_part, .Largmax_part_end-argmax_part
.rodata
.p2align 6
.amdhsa_kernel argmax_part
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 16
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
  - .name: argmax_part
    .symbol: argmax_part.kd
    .kernarg_segment_size: 16
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 36
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: logits
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: partial
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
...
.end_amdgpu_metadata
