// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// LLVM18 independent encoding oracle only.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl readfirst_probe
.p2align 8
.type readfirst_probe,@function
readfirst_probe:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dwordx2 s[8:9], s[0:1], 16
  s_waitcnt lgkmcnt(0)
  v_lshlrev_b32 v2, 2, v0
  v_mov_b32 v3, s5
  v_add_co_u32 v2, vcc, s4, v2
  v_addc_co_u32 v3, vcc, 0, v3, vcc
  global_load_dword v8, v[2:3], off
  s_waitcnt vmcnt(0)
  s_and_saveexec_b64 s[24:25], s[8:9]
  v_readfirstlane_b32 s10, v8
  s_mov_b64 exec, s[24:25]
  s_nop 1
  v_mov_b32 v8, s10
  v_mov_b32 v2, s6
  v_mov_b32 v3, s7
  s_mov_b32 s28, 1
  s_mov_b32 s29, 0
  s_and_saveexec_b64 s[24:25], s[28:29]
  global_store_dword v[2:3], v8, off
  s_waitcnt vmcnt(0)
  s_endpgm
.Lreadfirst_probe_end:
.size readfirst_probe, .Lreadfirst_probe_end-readfirst_probe
.rodata
.p2align 6
.amdhsa_kernel readfirst_probe
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
  - .name: readfirst_probe
    .symbol: readfirst_probe.kd
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
      - .name: input
        .offset: 0
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: output
        .offset: 8
        .size: 8
        .value_kind: global_buffer
        .address_space: global
      - .name: mask_lo
        .offset: 16
        .size: 4
        .value_kind: by_value
      - .name: mask_hi
        .offset: 20
        .size: 4
        .value_kind: by_value
...
.end_amdgpu_metadata
