/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Test oracle only: trap ioctls to one deliberately unopened fd in a child.
 * Ordinary Linux syscall execution and Omega's request construction are real;
 * the SIGSYS handler stands in for the RM/UVM kernel and checks every byte. */
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <stddef.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <ucontext.h>
static unsigned char fixture_expected[9264];
static size_t fixture_size,fixture_status;
static unsigned long fixture_request;
static volatile sig_atomic_t fixture_seen,fixture_bad;
static int fixture_mutant;
static unsigned char fixture_params[16];
static void fixture_sigsys(int signal,siginfo_t *info,void *opaque) {
    (void)signal;
    ucontext_t *uc=opaque;
    unsigned long request; unsigned char *bytes;
#if defined(__aarch64__)
    request=uc->uc_mcontext.regs[1];bytes=(void *)(uintptr_t)uc->uc_mcontext.regs[2];
    uc->uc_mcontext.regs[0]=0;
#elif defined(__x86_64__)
    request=uc->uc_mcontext.gregs[REG_RSI];bytes=(void *)(uintptr_t)uc->uc_mcontext.gregs[REG_RDX];
    uc->uc_mcontext.gregs[REG_RAX]=0;
#else
#error "NVRM native syscall fixture requires Linux AArch64 or x86-64"
#endif
    fixture_seen++;
    if(info->si_syscall!=SYS_ioctl || request!=(fixture_request^(unsigned)fixture_mutant))fixture_bad=1;
    for(size_t i=0;i<fixture_size;i++)if(bytes[i]!=fixture_expected[i])fixture_bad=1;
    if(fixture_status<fixture_size) {
        uint32_t reply=0x0badc0de;memcpy(bytes+fixture_status,&reply,sizeof reply);
    }
    if(request==NV_IOWR(NV_ESC_RM_ALLOC,sizeof(NVOS21_PARAMETERS)) ||
       request==NV_IOWR(NV_ESC_RM_CONTROL,sizeof(NVOS54_PARAMETERS))) {
        uintptr_t pointer=0;memcpy(&pointer,bytes+16,sizeof pointer);
        if(pointer!=(uintptr_t)fixture_params)fixture_bad=1;
        else ((unsigned char *)pointer)[0]=0x42;
    }
}
static void fixture_prepare(unsigned long request,const void *bytes,size_t size,size_t status) {
    memcpy(fixture_expected,bytes,size);fixture_request=request;fixture_size=size;fixture_status=status;fixture_seen=0;
}
#define FIXTURE_RM(type,nr,status_field) do { \
    type p;memset(&p,0,sizeof p); \
    fixture_prepare(NV_IOWR(nr,sizeof p),&p,sizeof p,offsetof(type,status_field)); \
    assert(nvrm_rm_ioctl(12345,nr,&p,sizeof p)==0); \
    assert(fixture_seen==1 && p.status_field==0x0badc0de); \
} while(0)
#define FIXTURE_UVM(type,command) do { \
    type p;memset(&p,0,sizeof p); \
    fixture_prepare(command,&p,sizeof p,offsetof(type,rmStatus)); \
    assert(nvrm_uvm_ioctl(12345,command,&p)==0); \
    assert(fixture_seen==1 && p.rmStatus==0x0badc0de); \
} while(0)
static int fixture_child(void) {
    struct sigaction action={0};action.sa_sigaction=fixture_sigsys;action.sa_flags=SA_SIGINFO;
    sigemptyset(&action.sa_mask);assert(sigaction(SIGSYS,&action,NULL)==0);
    struct sock_filter filters[]={
        BPF_STMT(BPF_LD|BPF_W|BPF_ABS,offsetof(struct seccomp_data,nr)),
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_ioctl,0,3),
        BPF_STMT(BPF_LD|BPF_W|BPF_ABS,offsetof(struct seccomp_data,args[0])),
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,12345,0,1),
        BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_TRAP),
        BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ALLOW)
    };
    struct sock_fprog program={.len=sizeof filters/sizeof filters[0],.filter=filters};
    assert(prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0)==0);
    assert(prctl(PR_SET_SECCOMP,SECCOMP_MODE_FILTER,&program)==0);
    FIXTURE_RM(NVOS00_PARAMETERS,NV_ESC_RM_FREE,status);
    FIXTURE_RM(nv_ioctl_nvos33_parameters_with_fd,NV_ESC_RM_MAP_MEMORY,params.status);
    FIXTURE_RM(NVOS46_PARAMETERS,NV_ESC_RM_MAP_MEMORY_DMA,status);
    FIXTURE_RM(NVOS47_PARAMETERS,NV_ESC_RM_UNMAP_MEMORY_DMA,status);
    FIXTURE_RM(NVOS32_PARAMETERS,NV_ESC_RM_VID_HEAP_CONTROL,status);
    nv_ioctl_register_fd_t registration={.ctl_fd=71};
    fixture_prepare(NV_IOWR(NV_ESC_REGISTER_FD,sizeof registration),&registration,sizeof registration,SIZE_MAX);
    assert(nvrm_rm_ioctl(12345,NV_ESC_REGISTER_FD,&registration,sizeof registration)==0 && fixture_seen==1);
    nv_ioctl_card_info_t cards[64];memset(cards,0,sizeof cards);
    fixture_prepare(NV_IOWR(NV_ESC_CARD_INFO,sizeof cards),cards,sizeof cards,SIZE_MAX);
    assert(nvrm_rm_ioctl(12345,NV_ESC_CARD_INFO,cards,sizeof cards)==0 && fixture_seen==1);
    NVOS21_PARAMETERS allocation={.hRoot=11,.hObjectParent=22,.hObjectNew=33,.hClass=44,
        .pAllocParms=fixture_params,.paramsSize=sizeof fixture_params};
    fixture_prepare(NV_IOWR(NV_ESC_RM_ALLOC,sizeof allocation),&allocation,sizeof allocation,offsetof(NVOS21_PARAMETERS,status));
    assert(nvrm_rm_ioctl(12345,NV_ESC_RM_ALLOC,&allocation,sizeof allocation)==0 && fixture_seen==1);
    assert(allocation.status==0x0badc0de && fixture_params[0]==0x42 && allocation.pAllocParms==0);
    NVOS54_PARAMETERS control={.hClient=11,.hObject=22,.cmd=33,.flags=44,
        .params=fixture_params,.paramsSize=sizeof fixture_params};
    fixture_params[0]=0;
    fixture_prepare(NV_IOWR(NV_ESC_RM_CONTROL,sizeof control),&control,sizeof control,offsetof(NVOS54_PARAMETERS,status));
    assert(nvrm_rm_ioctl(12345,NV_ESC_RM_CONTROL,&control,sizeof control)==0 && fixture_seen==1);
    assert(control.status==0x0badc0de && fixture_params[0]==0x42 && control.params==0);
    FIXTURE_UVM(UVM_INITIALIZE_PARAMS,UVM_INITIALIZE);
    FIXTURE_UVM(UVM_MM_INITIALIZE_PARAMS,UVM_MM_INITIALIZE);
    FIXTURE_UVM(UVM_REGISTER_GPU_PARAMS,UVM_REGISTER_GPU);
    FIXTURE_UVM(UVM_UNREGISTER_GPU_PARAMS,UVM_UNREGISTER_GPU);
    FIXTURE_UVM(UVM_REGISTER_GPU_VASPACE_PARAMS,UVM_REGISTER_GPU_VASPACE);
    FIXTURE_UVM(UVM_UNREGISTER_GPU_VASPACE_PARAMS,UVM_UNREGISTER_GPU_VASPACE);
    FIXTURE_UVM(UVM_REGISTER_CHANNEL_PARAMS,UVM_REGISTER_CHANNEL);
    FIXTURE_UVM(UVM_UNREGISTER_CHANNEL_PARAMS,UVM_UNREGISTER_CHANNEL);
    FIXTURE_UVM(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS,UVM_MAP_EXTERNAL_ALLOCATION);
    FIXTURE_UVM(UVM_FREE_PARAMS,UVM_FREE);
    FIXTURE_UVM(UVM_CREATE_EXTERNAL_RANGE_PARAMS,UVM_CREATE_EXTERNAL_RANGE);
    return fixture_bad ? 1 : 0;
}
static void fixture_run(void) {
    for(int mutant=0;mutant<=1;mutant++) {
        pid_t child=fork();assert(child>=0);
        if(!child){fixture_mutant=mutant;_exit(fixture_child());}
        int status=0;assert(waitpid(child,&status,0)==child && WIFEXITED(status));
        assert(WEXITSTATUS(status)==mutant);
    }
    puts("NVRM NATIVE IOCTL PASS (all RM/UVM request numbers and bytes, fake nonzero replies, parameter copyback, syscall request mutant caught; HOST_ONLY)");
}
#undef FIXTURE_RM
#undef FIXTURE_UVM

/* One denied cleanup followed by a real retry. The test-only third syscall
 * argument lets the SIGSYS handler forward munmap without recursively trapping;
 * Linux munmap ignores it. No production syscall or bridge gate is changed. */
static volatile sig_atomic_t fixture_cleanup_seen;
static void fixture_cleanup_sigsys(int signal,siginfo_t *info,void *opaque) {
    (void)signal;
    ucontext_t *uc=opaque;
    uintptr_t address;size_t size;long result;
#if defined(__aarch64__)
    address=uc->uc_mcontext.regs[0];size=uc->uc_mcontext.regs[1];
#elif defined(__x86_64__)
    address=uc->uc_mcontext.gregs[REG_RDI];size=uc->uc_mcontext.gregs[REG_RSI];
#endif
    if(info->si_syscall!=SYS_munmap)_exit(80);
    fixture_cleanup_seen++;
    int saved_errno=errno;
    result=fixture_cleanup_seen==1?-EPERM:syscall(SYS_munmap,address,size,0x43554441);
    errno=saved_errno;
#if defined(__aarch64__)
    uc->uc_mcontext.regs[0]=(uint64_t)result;
#elif defined(__x86_64__)
    uc->uc_mcontext.gregs[REG_RAX]=result;
#endif
}
static void fixture_cleanup_child(void) {
    Nvrm rm={0};
    void *address=(void *)(uintptr_t)UINT64_C(0x7000000000);
    assert(nvrm_cpu_reserve(&rm,(uintptr_t)address,4096)==address);
    uint64_t owner=rm.omega_pending_mapping;
    int fd=memfd_create("cudapp-nvrm-cleanup-fixture",MFD_CLOEXEC);
    assert(fd>=0 && ftruncate(fd,4096)==0);
    struct sigaction action={0};action.sa_sigaction=fixture_cleanup_sigsys;action.sa_flags=SA_SIGINFO;
    sigemptyset(&action.sa_mask);assert(sigaction(SIGSYS,&action,NULL)==0);
    struct sock_filter filters[]={
        BPF_STMT(BPF_LD|BPF_W|BPF_ABS,offsetof(struct seccomp_data,nr)),
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,SYS_munmap,0,3),
        BPF_STMT(BPF_LD|BPF_W|BPF_ABS,offsetof(struct seccomp_data,args[2])),
        BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K,0x43554441,1,0),
        BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_TRAP),
        BPF_STMT(BPF_RET|BPF_K,SECCOMP_RET_ALLOW)
    };
    struct sock_fprog program={.len=sizeof filters/sizeof filters[0],.filter=filters};
    assert(prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0)==0);
    assert(prctl(PR_SET_SECCOMP,SECCOMP_MODE_FILTER,&program)==0);
    assert(nvrm_cpu_map(&rm,-1,4096,address)==MAP_FAILED && errno==EPROTO);
    assert(fixture_cleanup_seen==1 && rm.omega_pending_mapping==owner);
    assert(rm.omega_pending_address==(uintptr_t)address && rm.omega_pending_size==4096);
    uint64_t result=0x12345678;
    assert(omega2_ctx_nvrm_bind(rm.omega_context,&result,owner,(uint64_t)fd)!=0);
    assert(result==0x12345678 && fixture_cleanup_seen==1);
    assert(omega_native_context_destroy(rm.omega_context)==36);
    assert(nvrm_cpu_drop(&rm,address,4096)==0 && !rm.omega_pending_mapping);
    assert(fixture_cleanup_seen==2);
    assert(nvrm_mapping_finish(&rm)==0 && !rm.omega_context);
    assert(close(fd)==0);
}
static void fixture_cleanup_run(void) {
    pid_t child=fork();assert(child>=0);
    if(!child){fixture_cleanup_child();_exit(0);}
    int status=0;assert(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
    puts("NVRM CLEANUP FAILURE PASS (failed bind plus denied munmap retains quarantined owner, refuses rebind/destroy, retry drops; HOST_ONLY)");
}
