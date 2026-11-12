// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Independent LLVM 18.1.3 oracle only: never input to hsaco.om or the executor.
// fsum16: frozen left-to-right binary32 reduction of 16 inputs per row, g<n.
// Kernarg x @0, o @8, n @16 (20 bytes); wave64, local size 64. Hardware NOT_RUN.
.amdgcn_target "amdgcn-amd-amdhsa--gfx942"
.text
.globl fsum16
.p2align 8
.type fsum16,@function
fsum16:
  s_load_dwordx4 s[4:7], s[0:1], 0
  s_load_dword s8, s[0:1], 16
  v_and_b32 v0, 0x3ff, v0
  s_lshl_b32 s9, s2, 6
  v_add_u32 v1, s9, v0
  s_waitcnt lgkmcnt(0)
  v_cmp_gt_u32 vcc, s8, v1
  s_and_saveexec_b64 s[10:11], vcc
  s_cbranch_execz .Lend
  v_lshlrev_b32 v2, 6, v1
  v_lshrrev_b32 v3, 26, v1
  v_mov_b32 v5, s5
  v_add_co_u32 v6, vcc, s4, v2
  v_addc_co_u32 v7, vcc, v5, v3, vcc
  global_load_dword v4, v[6:7], off
  s_waitcnt vmcnt(0)
.irp offset,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60
  global_load_dword v8, v[6:7], off offset:\offset
  s_waitcnt vmcnt(0)
  v_add_f32 v4, v4, v8
.endr
  v_lshlrev_b32 v2, 2, v1
  v_lshrrev_b32 v3, 30, v1
  v_mov_b32 v5, s7
  v_add_co_u32 v6, vcc, s6, v2
  v_addc_co_u32 v7, vcc, v5, v3, vcc
  global_store_dword v[6:7], v4, off
.Lend:
  s_endpgm
.Lfsum16_end:
.size fsum16, .Lfsum16_end-fsum16
.rodata
.p2align 6
.amdhsa_kernel fsum16
  .amdhsa_user_sgpr_kernarg_segment_ptr 1
  .amdhsa_system_sgpr_workgroup_id_x 1
  .amdhsa_system_vgpr_workitem_id 0
  .amdhsa_kernarg_size 20
  .amdhsa_next_free_vgpr 9
  .amdhsa_next_free_sgpr 12
  .amdhsa_accum_offset 12
  .amdhsa_float_denorm_mode_32 3
.end_amdhsa_kernel
.amdgpu_metadata
---
amdhsa.version: [ 1, 2 ]
amdhsa.target: amdgcn-amd-amdhsa--gfx942
amdhsa.kernels:
  - .name: fsum16
    .symbol: fsum16.kd
    .kernarg_segment_size: 20
    .kernarg_segment_align: 8
    .group_segment_fixed_size: 0
    .private_segment_fixed_size: 0
    .wavefront_size: 64
    .sgpr_count: 18
    .vgpr_count: 9
    .agpr_count: 0
    .max_flat_workgroup_size: 64
    .args:
      - { .name: x, .offset: 0, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: o, .offset: 8, .size: 8, .value_kind: global_buffer, .address_space: global }
      - { .name: !str n, .offset: 16, .size: 4, .value_kind: by_value }
...
.end_amdgpu_metadata
