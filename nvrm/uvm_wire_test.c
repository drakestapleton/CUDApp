/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* HOST_ONLY: vendored UVM ABI oracle, no device calls. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cudapp_native.h"
#include "uvm_linux_ioctl.h"
#include "uvm_ioctl.h"

static int check(uint64_t cmd,const uint64_t a[16],const unsigned char uuid[16],const void *expected,size_t size) {
    unsigned char wire[56], full[56]={0}; uint64_t n=0;
    memcpy(full,expected,size);
    if (omega_nvrm_uvm_wire(&n,cmd,a,16,uuid,16,wire,56) || n!=size || memcmp(wire,full,56)) return 1;
    for (size_t i=0;i<size;i++) { wire[i]^=1; if (!memcmp(wire,full,size)) return 1; wire[i]^=1; }
    return 0;
}
#define CHECK(cmd,p) do { if(check(cmd,a,uuid,&p,sizeof p)) { fprintf(stderr,"UVM wire mismatch: %s\n",#p);return 1;} } while(0)
int main(void) {
    for (unsigned k=0;k<256;k++) {
        uint64_t a[16]; unsigned char uuid[16];
        for (unsigned i=0;i<16;i++) { a[i]=UINT64_C(0xfedcba9876543210)^((uint64_t)k<<32)^i; uuid[i]=(unsigned char)(k+i); }
        UVM_INITIALIZE_PARAMS init={0};init.flags=a[0];CHECK(UVM_INITIALIZE,init);
        UVM_MM_INITIALIZE_PARAMS mm={0};mm.uvmFd=(int32_t)a[0];CHECK(UVM_MM_INITIALIZE,mm);
        UVM_REGISTER_GPU_PARAMS gpu={0};memcpy(gpu.gpu_uuid.uuid,uuid,16);gpu.rmCtrlFd=(int32_t)a[0];gpu.hClient=a[1];gpu.hSmcPartRef=a[2];CHECK(UVM_REGISTER_GPU,gpu);
        UVM_UNREGISTER_GPU_PARAMS ungpu={0};memcpy(ungpu.gpu_uuid.uuid,uuid,16);CHECK(UVM_UNREGISTER_GPU,ungpu);
        UVM_REGISTER_GPU_VASPACE_PARAMS vas={0};memcpy(vas.gpuUuid.uuid,uuid,16);vas.rmCtrlFd=(int32_t)a[0];vas.hClient=a[1];vas.hVaSpace=a[2];CHECK(UVM_REGISTER_GPU_VASPACE,vas);
        UVM_UNREGISTER_GPU_VASPACE_PARAMS unvas={0};memcpy(unvas.gpuUuid.uuid,uuid,16);CHECK(UVM_UNREGISTER_GPU_VASPACE,unvas);
        UVM_REGISTER_CHANNEL_PARAMS ch={0};memcpy(ch.gpuUuid.uuid,uuid,16);ch.rmCtrlFd=(int32_t)a[0];ch.hClient=a[1];ch.hChannel=a[2];ch.base=a[3];ch.length=a[4];CHECK(UVM_REGISTER_CHANNEL,ch);
        UVM_UNREGISTER_CHANNEL_PARAMS unch={0};memcpy(unch.gpuUuid.uuid,uuid,16);unch.hClient=a[0];unch.hChannel=a[1];CHECK(UVM_UNREGISTER_CHANNEL,unch);
        UVM_FREE_PARAMS release={0};release.base=a[0];release.length=a[1];CHECK(UVM_FREE,release);
        UVM_CREATE_EXTERNAL_RANGE_PARAMS cr={0};cr.base=a[0];cr.length=a[1];CHECK(UVM_CREATE_EXTERNAL_RANGE,cr);
        UVM_MAP_EXTERNAL_ALLOCATION_PARAMS ma={0};unsigned char mapped[9264];uint64_t bytes=0;
        ma.base=a[0];ma.length=a[1];ma.offset=a[2];ma.gpuAttributesCount=a[3];ma.rmCtrlFd=(int32_t)a[4];ma.hClient=a[5];ma.hMemory=a[6];
        memcpy(ma.perGpuAttributes[0].gpuUuid.uuid,uuid,16);ma.perGpuAttributes[0].gpuMappingType=a[7];
        _Static_assert(sizeof ma==sizeof mapped,"vendored UVM allocation ABI changed");
        if (omega_nvrm_uvm_map_wire(&bytes,a,16,uuid,16,mapped,sizeof mapped) || bytes!=sizeof ma || memcmp(mapped,&ma,sizeof ma)) return 1;
        for (size_t i=0;i<sizeof mapped;i++) { mapped[i]^=1;if(!memcmp(mapped,&ma,sizeof ma))return 1;mapped[i]^=1; }
    }
    uint64_t a[16]={0},n=99;unsigned char u[16]={0},b[56];
    if(omega_nvrm_uvm_wire(&n,0,a,16,u,16,b,56)||n) return 1;
    puts("NVRM UVM WIRE PASS (all client requests, vendor bytes, every-byte mutants, invalid command)");
    return 0;
}
