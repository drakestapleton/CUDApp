/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-only vendor-struct oracle. Never opens a device. */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "cudapp_native.h"
#include "nvos.h"
#include "nv-ioctl.h"
#include "nv-unix-nvos-params-wrappers.h"

static int check(uint64_t op, const uint64_t a[16], const void *oracle, size_t size,
                 size_t status_at, size_t value_at, size_t value_size) {
    unsigned char wire[64], expected[64] = {0};
    uint64_t n = 0, result = 0, reply[2] = {0};
    memcpy(expected, oracle, size);
    if (omega_nvrm_wire_pack(&n, op, a, 16, wire, 64) || n != size) return 1;
#ifdef NVRM_WIRE_MUTANT
    wire[0] ^= 1; /* Negative control: an incorrect serializer must fail the oracle. */
#endif
    if (memcmp(wire, expected, 64)) return 1;
    /* Every byte participates: the deliberately broken request must differ. */
    for (size_t i = 0; i < size; i++) {
        wire[i] ^= 1;
        if (!memcmp(wire, expected, size)) return 1;
        wire[i] ^= 1;
    }
    if (op == 7) return 0;
    /* Fake reply includes nonzero RM status; parser reports it, never hides it. */
    uint32_t status = 0x12345678;
    uint64_t value = value_size == 4 ? UINT64_C(0x89abcdef) : UINT64_C(0x1234567889abcdef);
    memcpy(wire + status_at, &status, sizeof status);
    if (value_size) memcpy(wire + value_at, &value, value_size);
    if (omega_nvrm_wire_reply(&result, op, wire, 64, reply, 2) || result || reply[0] != status ||
        reply[1] != (value_size ? value : 0)) return 1;
    return 0;
}
int main(void) {
    uint64_t a[16];
    for (unsigned k = 0; k < 256; k++) {
        for (unsigned i = 0; i < 16; i++) a[i] = (UINT64_C(0xfedcba9876543210) ^ ((uint64_t)k << 32) ^ i);
        NVOS21_PARAMETERS alloc = {0};
        alloc.hRoot=a[0]; alloc.hObjectParent=a[1]; alloc.hObjectNew=a[2]; alloc.hClass=a[3];
        alloc.pAllocParms=(void *)(uintptr_t)a[4]; alloc.paramsSize=a[5];
        if (check(1,a,&alloc,sizeof alloc,offsetof(NVOS21_PARAMETERS,status),offsetof(NVOS21_PARAMETERS,hObjectNew),4)) return 1;
        NVOS54_PARAMETERS ctrl = {0};
        ctrl.hClient=a[0]; ctrl.hObject=a[1]; ctrl.cmd=a[2]; ctrl.flags=a[3]; ctrl.params=(void *)(uintptr_t)a[4]; ctrl.paramsSize=a[5];
        if (check(2,a,&ctrl,sizeof ctrl,offsetof(NVOS54_PARAMETERS,status),0,0)) return 2;
        NVOS00_PARAMETERS release = {0};
        release.hRoot=a[0]; release.hObjectParent=a[1]; release.hObjectOld=a[2];
        if (check(3,a,&release,sizeof release,offsetof(NVOS00_PARAMETERS,status),0,0)) return 3;
        nv_ioctl_nvos33_parameters_with_fd map = {0};
        map.params.hClient=a[0]; map.params.hDevice=a[1]; map.params.hMemory=a[2];
        map.params.offset=a[3]; map.params.length=a[4]; map.params.pLinearAddress=(void *)(uintptr_t)a[5]; map.params.flags=a[6]; map.fd=(int)(uint32_t)a[7];
        if (check(4,a,&map,sizeof map,offsetof(NVOS33_PARAMETERS,status),offsetof(NVOS33_PARAMETERS,pLinearAddress),8)) return 4;
        NVOS46_PARAMETERS dma = {0};
        dma.hClient=a[0]; dma.hDevice=a[1]; dma.hDma=a[2]; dma.hMemory=a[3]; dma.offset=a[4]; dma.length=a[5];
        dma.flags=a[6]; dma.flags2=a[7]; dma.kindOverride=a[8]; dma.dmaOffset=a[9];
        if (check(5,a,&dma,sizeof dma,offsetof(NVOS46_PARAMETERS,status),offsetof(NVOS46_PARAMETERS,dmaOffset),8)) return 5;
        NVOS47_PARAMETERS unmap = {0};
        unmap.hClient=a[0]; unmap.hDevice=a[1]; unmap.hDma=a[2]; unmap.hMemory=a[3]; unmap.flags=a[4]; unmap.dmaOffset=a[5]; unmap.size=a[6];
        if (check(6,a,&unmap,sizeof unmap,offsetof(NVOS47_PARAMETERS,status),0,0)) return 6;
        nv_ioctl_register_fd_t reg = { .ctl_fd=(int)(uint32_t)a[0] };
        if (check(7,a,&reg,sizeof reg,0,0,0)) return 7;
        NVOS32_PARAMETERS heap = {0};
        unsigned char heap_wire[184];
        uint64_t heap_size = 0;
        heap.hRoot=a[0]; heap.hObjectParent=a[1]; heap.function=a[2]; heap.data.Info.attr=a[3];
        _Static_assert(sizeof heap == sizeof heap_wire, "vendored NVOS32 layout changed");
        if (omega_nvrm_heap_wire(&heap_size,a,16,heap_wire,sizeof heap_wire) ||
            heap_size != sizeof heap || memcmp(heap_wire,&heap,sizeof heap)) return 9;
    }
    unsigned char b[64]; uint64_t n=99, a0[16]={0}, r[2]={0};
    if (omega_nvrm_wire_pack(&n,0,a0,16,b,64) || n || omega_nvrm_wire_reply(&n,0,b,64,r,2) || n != 1) return 8;
    puts("NVRM WIRE PASS (vendor byte differential, fake replies, every-byte mutants, invalid op)");
    return 0;
}
