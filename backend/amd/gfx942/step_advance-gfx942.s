// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl step_advance
.p2align 8
.type step_advance,@function
step_advance:
  s_sub_u32 s28, s2, 1
  s_cbranch_scc0 .L99
  s_sub_u32 s28, s3, 1
  s_cbranch_scc0 .L99
  s_load_dwordx2 s[4:5], s[0:1], 0
  s_load_dword s6, s[0:1], 8
  s_waitcnt lgkmcnt(0)
  v_mov_b32 v2, s4
  v_mov_b32 v3, s5
  global_load_dword v8, v[2:3], off
  s_waitcnt vmcnt(0)
  v_readfirstlane_b32 s8, v8
  s_nop 1
  s_sub_u32 s28, s8, s6
  s_cbranch_scc0 .L99
  s_mov_b32 s24, 1
  s_mov_b32 s25, 0
  s_and_saveexec_b64 s[26:27], s[24:25]
  v_add_u32 v8, 1, v8
  global_store_dword v[2:3], v8, off
  s_waitcnt vmcnt(0)
.L99:
  s_endpgm
.Lstep_advance_end:
.size step_advance, .Lstep_advance_end-step_advance
.rodata
.p2align 6
.amdhsa_kernel step_advance
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_sgpr_workgroup_id_y 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 12
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
  - .name: step_advance
    .symbol: step_advance.kd
    .kernarg_segment_size: 12
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 36
    .vgpr_count: 32
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - .name: position
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: capacity
        .offset: 8
        .size: 4
        .value_kind: by_value
...
.end_amdgpu_metadata
