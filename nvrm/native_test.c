/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* HOST_ONLY. Ordinary files/anonymous reservations only; never nvrm_open(). */
#include "nvrm.c"
#include <assert.h>
#include "native_ioctl_fixture.h"

static int64_t signed_result(uint64_t n) { return (int64_t)n; }
int main(void) {
    fixture_run();
    fixture_cleanup_run();
    int fd = nvrm_file_open("/dev/null"); assert(fd >= 0);
    assert(nvrm_file_close(fd) == 0);
    errno = 0; assert(nvrm_file_close(-1) == -1 && errno == EBADF);
    assert(nvrm_file_open("/nonexistent-cudapp-nvrm-native-fixture") == -1 && errno == ENOENT);
    NVOS00_PARAMETERS release = {.hRoot=1,.hObjectParent=2,.hObjectOld=3,.status=0x12345678};
    assert(nvrm_rm_ioctl(-1,NV_ESC_RM_FREE,&release,sizeof release) == -1 && errno == EBADF);
    assert(release.status == 0x12345678);
    NVOS21_PARAMETERS alloc = {.hRoot=1,.hObjectParent=2,.hObjectNew=3,.hClass=4,.status=0x87654321};
    assert(nvrm_rm_ioctl(-1,NV_ESC_RM_ALLOC,&alloc,sizeof alloc) == -1 && errno == EBADF);
    assert(alloc.status == 0x87654321);
    UVM_FREE_PARAMS uf = {.base=0x7000000000,.length=4096,.rmStatus=0x12345678};
    assert(nvrm_uvm_ioctl(-1,UVM_FREE,&uf) == -1 && errno == EBADF);
    assert(uf.rmStatus == 0x12345678);
    uint64_t value=0;
    assert(omega2_nvrm_ioctl(&value,UINT64_MAX,0,(unsigned char *)&release,sizeof release,NULL,0)==0);
    assert(signed_result(value)==-EINVAL);
    assert(omega2_nvrm_uvm(&value,UINT64_MAX,UVM_FREE,(unsigned char *)&uf,sizeof uf-1)==0);
    assert(signed_result(value)==-EINVAL);

    fd=memfd_create("cudapp-nvrm-host-fixture",MFD_CLOEXEC);assert(fd>=0);
    assert(ftruncate(fd,0x10000)==0);
    Nvrm rm={0};
    void *address=(void *)(uintptr_t)UINT64_C(0x7000000000);
    assert(nvrm_cpu_reserve(&rm,(uintptr_t)address,4096)==address);
    uint64_t retired=rm.omega_pending_mapping;
    assert(nvrm_cpu_map(&rm,-1,4096,address)==MAP_FAILED && errno==EBADF);
    assert(rm.omega_pending_mapping==0);
    assert(omega2_ctx_nvrm_drop(rm.omega_context,&value,retired)!=0);
    assert(nvrm_cpu_reserve(&rm,UINT64_C(0x10000000000),4096)==MAP_FAILED && errno==EFBIG);
    assert(nvrm_cpu_reserve(&rm,UINT64_C(0x7000000000),UINT64_C(0x1000000001))==MAP_FAILED && errno==EFBIG);
    assert(nvrm_cpu_reserve(&rm,UINT64_C(0xfffffff000),8192)==MAP_FAILED && errno==EFBIG);
    assert(nvrm_cpu_map(&rm,fd,UINT64_C(0x1000000001),NULL)==MAP_FAILED && errno==EFBIG);
    assert(!rm.omega_pending_mapping && !rm.omega_usermode_mapping);
    /* WolframLanguageEvaluator: {2^36,2^40-4096,2^36+2^36<2^40}
       -> {68719476736,1099511623680,True}. PROT_NONE only, never backed memory. */
    void *extent_start=(void *)(uintptr_t)UINT64_C(68719476736);
    assert(nvrm_cpu_reserve(&rm,(uintptr_t)extent_start,UINT64_C(68719476736))==extent_start);
    assert(nvrm_cpu_drop(&rm,extent_start,UINT64_C(68719476736))==0);
    void *span_end=(void *)(uintptr_t)UINT64_C(1099511623680);
    assert(nvrm_cpu_reserve(&rm,(uintptr_t)span_end,4096)==span_end);
    assert(nvrm_cpu_drop(&rm,span_end,4096)==0);
    assert(nvrm_cpu_reserve(&rm,(uintptr_t)address,4096)==address);
    assert(nvrm_cpu_map(&rm,fd,4096,address)==address);
    *(volatile unsigned char *)address=0x5a;
    unsigned char byte=0;assert(pread(fd,&byte,1,0)==1 && byte==0x5a);
    rm.live[0]=(NvrmLiveAlloc){.va=(uintptr_t)address,.size=4096,.cpu=address,.cpu_live=1};
    nvrm_mapping_transfer(&rm,&rm.live[0]);rm.live_count=1;
    assert(rm.live[0].omega_mapping && !rm.omega_pending_mapping);
    assert(omega_native_context_destroy(rm.omega_context)==36);
    assert(nvrm_cpu_drop(&rm,(unsigned char *)address+4096,4096)==-1 && errno==EINVAL);
    assert(*(volatile unsigned char *)address==0x5a);
    assert(nvrm_cpu_drop(&rm,address,4096)==0 && !rm.live[0].cpu_live && !rm.live[0].omega_mapping);
    rm.live_count=0;
    rm.usermode_cpu=nvrm_cpu_map(&rm,fd,0x10000,NULL);assert(rm.usermode_cpu!=MAP_FAILED);
    assert(nvrm_cpu_drop(&rm,rm.usermode_cpu,0x10000)==0);rm.usermode_cpu=NULL;
    assert(nvrm_mapping_finish(&rm)==0 && !rm.omega_context);

    omega_native_context *a=NULL,*b=NULL;
    assert(omega_native_context_create(4098,&a)==0);
    assert(omega_native_context_create(4098,&b)==0);
    uint64_t handles[4098],foreign=0;
    for(unsigned i=0;i<4098;i++) {
        assert(omega2_ctx_nvrm_map_file(a,&handles[i],fd,4096)==0 && signed_result(handles[i])>0);
    }
    assert(omega2_ctx_nvrm_map_file(a,&value,fd,4096)==0 && signed_result(value)==-EMFILE);
    assert(omega2_ctx_nvrm_map_file(b,&foreign,fd,4096)==0 && signed_result(foreign)>0);
    assert(foreign!=handles[0]);
    assert(omega2_ctx_nvrm_drop(b,&value,handles[0])!=0);
    assert(omega2_ctx_nvrm_drop(a,&value,foreign)!=0);
    assert(omega2_ctx_nvrm_bind(a,&value,handles[0],fd)!=0); /* data mapping is not a reservation */
    for(unsigned i=0;i<4098;i++) assert(omega2_ctx_nvrm_drop(a,&value,handles[i])==0 && value==0);
    assert(omega2_ctx_nvrm_drop(b,&value,foreign)==0 && value==0);
    assert(omega_native_context_destroy(a)==0 && omega_native_context_destroy(b)==0);
    assert(close(fd)==0);
    puts("NVRM NATIVE TRANSPORT PASS (file/error/fake reply, reserve/bind/free, 4098 slots, independent contexts, stale/foreign/invalid controls; HOST_ONLY)");
    return 0;
}
