/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Offline vendor-layout oracle. No RM files, ioctls or GPU access. */
#define main nvrm_layout_probe_main
#include "nvrm_struct_layout.c"
#undef main
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cudapp_native.h"
struct field { size_t off, width; };
struct schema { size_t size, count; struct field f[259]; unsigned long cmd; };
#define D(t,f) {offsetof(t,f),sizeof(((t*)0)->f)}
#define B(t,f,n) {offsetof(t,f),n}
#define E(t,n,...) {sizeof(t),n,{__VA_ARGS__},0}
static struct schema s[27] = {
[1]=E(NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS,4,D(NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS,gpuId),D(NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS,deviceInstance),D(NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS,subDeviceInstance),D(NV0000_CTRL_GPU_GET_ID_INFO_V2_PARAMS,gpuFlags)),
[2]=E(NV0080_ALLOC_PARAMETERS,3,D(NV0080_ALLOC_PARAMETERS,deviceId),D(NV0080_ALLOC_PARAMETERS,hClientShare),D(NV0080_ALLOC_PARAMETERS,vaMode)),
[3]=E(NV2080_ALLOC_PARAMETERS,1,D(NV2080_ALLOC_PARAMETERS,subDeviceId)),
[4]=E(NV_MEMORY_VIRTUAL_ALLOCATION_PARAMS,3,D(NV_MEMORY_VIRTUAL_ALLOCATION_PARAMS,offset),D(NV_MEMORY_VIRTUAL_ALLOCATION_PARAMS,limit),D(NV_MEMORY_VIRTUAL_ALLOCATION_PARAMS,hVASpace)),
[5]=E(NV2080_CTRL_GPU_GET_INFO_V2_PARAMS,3,D(NV2080_CTRL_GPU_GET_INFO_V2_PARAMS,gpuInfoListSize),D(NV2080_CTRL_GPU_GET_INFO_V2_PARAMS,gpuInfoList[0].index),D(NV2080_CTRL_GPU_GET_INFO_V2_PARAMS,gpuInfoList[0].data)),
[6]=E(NV2080_CTRL_GR_GET_INFO_PARAMS,2,D(NV2080_CTRL_GR_GET_INFO_PARAMS,grInfoListSize),D(NV2080_CTRL_GR_GET_INFO_PARAMS,grInfoList)),
[7]=E(NV0080_CTRL_GPU_GET_CLASSLIST_PARAMS,2,D(NV0080_CTRL_GPU_GET_CLASSLIST_PARAMS,numClasses),D(NV0080_CTRL_GPU_GET_CLASSLIST_PARAMS,classList)),
[8]=E(NV_VASPACE_ALLOCATION_PARAMETERS,4,D(NV_VASPACE_ALLOCATION_PARAMETERS,index),D(NV_VASPACE_ALLOCATION_PARAMETERS,flags),D(NV_VASPACE_ALLOCATION_PARAMETERS,vaSize),D(NV_VASPACE_ALLOCATION_PARAMETERS,vaBase)),
[9]=E(NV2080_CTRL_GPU_GET_GID_INFO_PARAMS,259,D(NV2080_CTRL_GPU_GET_GID_INFO_PARAMS,index),D(NV2080_CTRL_GPU_GET_GID_INFO_PARAMS,flags),D(NV2080_CTRL_GPU_GET_GID_INFO_PARAMS,length)),
[10]=E(NV_MEMORY_ALLOCATION_PARAMS,9,D(NV_MEMORY_ALLOCATION_PARAMS,owner),D(NV_MEMORY_ALLOCATION_PARAMS,type),D(NV_MEMORY_ALLOCATION_PARAMS,flags),D(NV_MEMORY_ALLOCATION_PARAMS,attr),D(NV_MEMORY_ALLOCATION_PARAMS,attr2),D(NV_MEMORY_ALLOCATION_PARAMS,format),D(NV_MEMORY_ALLOCATION_PARAMS,size),D(NV_MEMORY_ALLOCATION_PARAMS,alignment),D(NV_MEMORY_ALLOCATION_PARAMS,limit)),
[11]=E(NV_CHANNEL_ALLOC_PARAMS,9,D(NV_CHANNEL_ALLOC_PARAMS,hObjectError),D(NV_CHANNEL_ALLOC_PARAMS,hObjectBuffer),D(NV_CHANNEL_ALLOC_PARAMS,gpFifoOffset),D(NV_CHANNEL_ALLOC_PARAMS,gpFifoEntries),D(NV_CHANNEL_ALLOC_PARAMS,hContextShare),D(NV_CHANNEL_ALLOC_PARAMS,hVASpace),D(NV_CHANNEL_ALLOC_PARAMS,hUserdMemory[0]),D(NV_CHANNEL_ALLOC_PARAMS,userdOffset[0]),D(NV_CHANNEL_ALLOC_PARAMS,engineType)),
[12]=E(NV_CHANNEL_GROUP_ALLOCATION_PARAMETERS,2,D(NV_CHANNEL_GROUP_ALLOCATION_PARAMETERS,hObjectError),D(NV_CHANNEL_GROUP_ALLOCATION_PARAMETERS,engineType)),
[13]=E(NV_CTXSHARE_ALLOCATION_PARAMETERS,3,D(NV_CTXSHARE_ALLOCATION_PARAMETERS,hVASpace),D(NV_CTXSHARE_ALLOCATION_PARAMETERS,flags),D(NV_CTXSHARE_ALLOCATION_PARAMETERS,subctxId)),
[14]=E(NVC36F_CTRL_CMD_GPFIFO_GET_WORK_SUBMIT_TOKEN_PARAMS,1,D(NVC36F_CTRL_CMD_GPFIFO_GET_WORK_SUBMIT_TOKEN_PARAMS,workSubmitToken)),
[15]=E(NVA06C_CTRL_GPFIFO_SCHEDULE_PARAMS,3,D(NVA06C_CTRL_GPFIFO_SCHEDULE_PARAMS,bEnable),D(NVA06C_CTRL_GPFIFO_SCHEDULE_PARAMS,bSkipSubmit),D(NVA06C_CTRL_GPFIFO_SCHEDULE_PARAMS,bSkipEnable)),
[16]={sizeof(nv_ioctl_card_info_t)*64,9,{D(nv_ioctl_card_info_t,valid),D(nv_ioctl_card_info_t,pci_info.domain),D(nv_ioctl_card_info_t,pci_info.bus),D(nv_ioctl_card_info_t,pci_info.slot),D(nv_ioctl_card_info_t,pci_info.function),D(nv_ioctl_card_info_t,gpu_id),D(nv_ioctl_card_info_t,minor_number),D(nv_ioctl_card_info_t,pci_info.vendor_id),D(nv_ioctl_card_info_t,pci_info.device_id)},0},
[17]=E(NV2080_CTRL_GR_INFO,2,D(NV2080_CTRL_GR_INFO,index),D(NV2080_CTRL_GR_INFO,data)),
[18]={sizeof(UVM_INITIALIZE_PARAMS),2,{D(UVM_INITIALIZE_PARAMS,flags),D(UVM_INITIALIZE_PARAMS,rmStatus)},UVM_INITIALIZE},
[19]={sizeof(UVM_MM_INITIALIZE_PARAMS),2,{D(UVM_MM_INITIALIZE_PARAMS,uvmFd),D(UVM_MM_INITIALIZE_PARAMS,rmStatus)},UVM_MM_INITIALIZE},
[20]={sizeof(UVM_FREE_PARAMS),3,{D(UVM_FREE_PARAMS,base),D(UVM_FREE_PARAMS,length),D(UVM_FREE_PARAMS,rmStatus)},UVM_FREE},
[21]={sizeof(UVM_REGISTER_GPU_PARAMS),6,{B(UVM_REGISTER_GPU_PARAMS,gpu_uuid.uuid[0],8),B(UVM_REGISTER_GPU_PARAMS,gpu_uuid.uuid[8],8),D(UVM_REGISTER_GPU_PARAMS,rmCtrlFd),D(UVM_REGISTER_GPU_PARAMS,hClient),D(UVM_REGISTER_GPU_PARAMS,hSmcPartRef),D(UVM_REGISTER_GPU_PARAMS,rmStatus)},UVM_REGISTER_GPU},
[22]={sizeof(UVM_UNREGISTER_GPU_PARAMS),3,{B(UVM_UNREGISTER_GPU_PARAMS,gpu_uuid.uuid[0],8),B(UVM_UNREGISTER_GPU_PARAMS,gpu_uuid.uuid[8],8),D(UVM_UNREGISTER_GPU_PARAMS,rmStatus)},UVM_UNREGISTER_GPU},
[23]={sizeof(UVM_REGISTER_GPU_VASPACE_PARAMS),6,{B(UVM_REGISTER_GPU_VASPACE_PARAMS,gpuUuid.uuid[0],8),B(UVM_REGISTER_GPU_VASPACE_PARAMS,gpuUuid.uuid[8],8),D(UVM_REGISTER_GPU_VASPACE_PARAMS,rmCtrlFd),D(UVM_REGISTER_GPU_VASPACE_PARAMS,hClient),D(UVM_REGISTER_GPU_VASPACE_PARAMS,hVaSpace),D(UVM_REGISTER_GPU_VASPACE_PARAMS,rmStatus)},UVM_REGISTER_GPU_VASPACE},
[24]={sizeof(UVM_REGISTER_CHANNEL_PARAMS),8,{B(UVM_REGISTER_CHANNEL_PARAMS,gpuUuid.uuid[0],8),B(UVM_REGISTER_CHANNEL_PARAMS,gpuUuid.uuid[8],8),D(UVM_REGISTER_CHANNEL_PARAMS,rmCtrlFd),D(UVM_REGISTER_CHANNEL_PARAMS,hClient),D(UVM_REGISTER_CHANNEL_PARAMS,hChannel),D(UVM_REGISTER_CHANNEL_PARAMS,base),D(UVM_REGISTER_CHANNEL_PARAMS,length),D(UVM_REGISTER_CHANNEL_PARAMS,rmStatus)},UVM_REGISTER_CHANNEL},
[25]={sizeof(UVM_UNREGISTER_CHANNEL_PARAMS),5,{B(UVM_UNREGISTER_CHANNEL_PARAMS,gpuUuid.uuid[0],8),B(UVM_UNREGISTER_CHANNEL_PARAMS,gpuUuid.uuid[8],8),D(UVM_UNREGISTER_CHANNEL_PARAMS,hClient),D(UVM_UNREGISTER_CHANNEL_PARAMS,hChannel),D(UVM_UNREGISTER_CHANNEL_PARAMS,rmStatus)},UVM_UNREGISTER_CHANNEL},
[26]={sizeof(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS),11,{D(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,base),D(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,length),D(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,offset),B(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,perGpuAttributes[0].gpuUuid.uuid[0],8),B(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,perGpuAttributes[0].gpuUuid.uuid[8],8),D(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,perGpuAttributes[0].gpuMappingType),D(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,gpuAttributesCount),D(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,rmCtrlFd),D(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,hClient),D(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,hMemory),D(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,rmStatus)},UVM_MAP_EXTERNAL_ALLOCATION}
};
static void die(const char *what, unsigned k, size_t f) { fprintf(stderr,"FAIL %s kind=%u field=%zu\n",what,k,f); exit(1); }
static uint64_t call(unsigned mode,unsigned k,const uint64_t *a,size_t n,unsigned char *b,size_t len) {
    uint64_t got=UINT64_MAX;
    if (omega_nvrm_struct(&got,mode,k,a,n,b,len)) die("bridge",k,mode);
    return got;
}
static void oracle(unsigned k,const uint64_t *a,unsigned char *b) {
    memset(b,0,s[k].size);
    for(size_t f=0;f<s[k].count;f++) memcpy(b+s[k].f[f].off,a+f,s[k].f[f].width);
}
int main(int argc,char **argv) {
    unsigned char got[9264],want[9264]; uint64_t a[259]={0},q[2]={0};
    int red=argc==2 && strcmp(argv[1],"--red")==0;
    uint64_t endian=1;
    if(*(unsigned char*)&endian!=1) die("oracle needs little endian",0,0);
    for(size_t i=0;i<sizeof(((NV2080_CTRL_GPU_GET_GID_INFO_PARAMS*)0)->data);i++)
        s[9].f[3+i]=(struct field){offsetof(NV2080_CTRL_GPU_GET_GID_INFO_PARAMS,data)+i,1};
    for(unsigned k=1;k<=26;k++) {
        if(call(2,k,q,2,got,0)!=s[k].size || call(5,k,q,2,got,0)!=s[k].count) die("layout size/count",k,0);
        for(size_t f=0;f<s[k].count;f++) {
            q[0]=f;
            if(call(3,k,q,2,got,0)!=s[k].f[f].off || call(4,k,q,2,got,0)!=s[k].f[f].width) die("layout offset/width",k,f);
            uint64_t max=UINT64_MAX >> ((8-s[k].f[f].width)*8);
            uint64_t values[]={0,1,max,max-1,(max>>1)+1,UINT64_C(0x123456789abcdef0)};
            for(size_t v=0;v<sizeof values/sizeof values[0];v++) {
                memset(a,0,sizeof a); a[f]=values[v];
                oracle(k,a,want); memset(got,0xa5,sizeof got);
                if(call(0,k,a,s[k].count,got,s[k].size)!=s[k].size) die("encode size",k,f);
                if(red) want[0]^=1;
                if(memcmp(got,want,s[k].size)) die("byte differential",k,f);
                q[1]=0;
                if(call(1,k,q,2,want,s[k].size)!=(values[v]&max)) die("decode differential",k,f);
                if(k>=18 && f==s[k].count-1) {
                    if(call(8,s[k].cmd,q,2,got,0)!=s[k].size || call(6,s[k].cmd,q,2,want,s[k].size)!=(values[v]&max)) die("UVM status",k,f);
                }
            }
        }
        /* All projected fields simultaneously, and every other byte zero. */
        for(size_t f=0;f<s[k].count;f++) a[f]=UINT64_C(0x9e3779b97f4a7c15)^f;
        oracle(k,a,want);
        call(0,k,a,s[k].count,got,s[k].size);
        if(memcmp(got,want,s[k].size)) die("combined fields",k,0);
        /* Exact-view refusal must not modify even one byte. */
        memset(got,0xa5,sizeof got); memcpy(want,got,sizeof want);
        if(call(0,k,a,s[k].count,got,s[k].size-1)!=0 || memcmp(got,want,sizeof got)) die("short view",k,0);
    }
    for(size_t idx=0;idx<64;idx++) {
        nv_ioctl_card_info_t cards[64]={0};
        cards[idx].valid=1; cards[idx].gpu_id=(uint32_t)idx+17; cards[idx].minor_number=(uint32_t)idx;
        cards[idx].pci_info.domain=0xabcd;cards[idx].pci_info.bus=0x91;cards[idx].pci_info.slot=0x1f;cards[idx].pci_info.function=7;
        cards[idx].pci_info.vendor_id=0x10de;cards[idx].pci_info.device_id=0x2901;
        for(size_t f=0;f<s[16].count;f++) {
            uint64_t expected=0; memcpy(&expected,(unsigned char*)&cards[idx]+s[16].f[f].off,s[16].f[f].width);
            q[0]=f;q[1]=idx;
            if(call(1,16,q,2,(unsigned char*)cards,sizeof cards)!=expected) die("card index",16,idx);
        }
    }
    NV2080_CTRL_GPU_GET_INFO_V2_PARAMS gpu={0};
    for(size_t idx=0;idx<sizeof gpu.gpuInfoList/sizeof gpu.gpuInfoList[0];idx++) {
        gpu.gpuInfoList[idx].index=(uint32_t)idx+1;gpu.gpuInfoList[idx].data=0xaabbccdd;
        q[0]=1;q[1]=idx;
        if(call(1,5,q,2,(unsigned char*)&gpu,sizeof gpu)!=idx+1) die("GPU info index",5,idx);
        q[0]=2;
        if(call(1,5,q,2,(unsigned char*)&gpu,sizeof gpu)!=0xaabbccdd) die("GPU info data",5,idx);
    }
    q[0]=1;q[1]=70;if(call(1,5,q,2,(unsigned char*)&gpu,sizeof gpu)) die("GPU bound",5,70);
    q[0]=0;q[1]=64;if(call(1,16,q,2,got,s[16].size)) die("card bound",16,64);
    NVOS32_PARAMETERS heap={0};heap.status=0x1234;heap.total=UINT64_C(0xfedcba9876543210);heap.free=UINT64_C(0x123456789abcdef0);
    uint64_t hv[]={heap.status,heap.total,heap.free};
    for(size_t i=0;i<3;i++){q[0]=i;if(call(7,0,q,2,(unsigned char*)&heap,sizeof heap)!=hv[i])die("heap decode",0,i);}
    puts("NVRM STRUCT LAYOUT + BYTE DIFFERENTIAL PASS (26 layouts; field boundaries; all card/GPU indexes; HOST_ONLY)");
    return 0;
}
