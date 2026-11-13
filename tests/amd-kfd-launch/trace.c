/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Native AArch64 TEST instrument. No GPU pathname is ever opened: device opens
 * become memfd_create before entry, ioctls become getpid, and only this oracle
 * supplies KFD replies. Production contains none of this simulation. */
#define _GNU_SOURCE
#include <asm/ptrace.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/kfd_ioctl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>
#ifndef NT_ARM_SYSTEM_CALL
#define NT_ARM_SYSTEM_CALL 0x404
#endif
static pid_t child;
static void fail(const char *s) { fprintf(stderr,"TEST_SIM FAIL %s errno=%d\n",s,errno); exit(1); }
#define CHECK(x) do { if (!(x)) fail(#x); } while(0)
static void rd(uint64_t a, void *dst, size_t n) {
    unsigned char *d=dst;
    for (size_t i=0;i<n;i+=sizeof(long)) {
        errno=0; long v=ptrace(PTRACE_PEEKDATA,child,(void *)(uintptr_t)(a+i),0);
        if (errno) fail("peek");
        memcpy(d+i,&v,n-i<sizeof v?n-i:sizeof v);
    }
}
static void wr(uint64_t a, const void *src, size_t n) {
    const unsigned char *s=src;
    for (size_t i=0;i<n;i+=sizeof(long)) {
        long v;
        rd(a+i,&v,sizeof v);
        memcpy(&v,s+i,n-i<sizeof v?n-i:sizeof v);
        CHECK(!ptrace(PTRACE_POKEDATA,child,(void *)(uintptr_t)(a+i),(void *)v));
    }
}
static uint64_t word(uint64_t a) { uint64_t v; rd(a,&v,8); return v; }
static void setword(uint64_t a,uint64_t v) { wr(a,&v,8); }
static void string(uint64_t a,char *s,size_t n) {
    for(size_t i=0;i<n;i++) { rd(a+i,s+i,1); if(!s[i]) return; }
    fail("unterminated pathname");
}
static void setregs(struct user_pt_regs *r) {
    struct iovec v={r,sizeof *r};
    CHECK(!ptrace(PTRACE_SETREGSET,child,(void *)NT_PRSTATUS,&v));
}
static void replace_call(struct user_pt_regs *r,int nr) {
    r->regs[8]=(uint64_t)nr; setregs(r);
    struct iovec v={&nr,sizeof nr};
    CHECK(!ptrace(PTRACE_SETREGSET,child,(void *)NT_ARM_SYSTEM_CALL,&v));
}
struct alloc { uint64_t va,size,off,handle; int mapped,freed; };
static struct alloc *allocs;
static size_t count;
static uint64_t endoff;
static int kfd=-1,drm=-1,kback=-1,dback=-1;
static struct kfd_ioctl_create_queue_args queue;
static int queued,packets;
static unsigned clocks;
static unsigned expected_gpu=1,expected_minor=128;
static int denied;
static int topology_fd=-1;
static int entry_order(const void *a,const void *b) {
    return strcmp(*(const char *const *)a+19,*(const char *const *)b+19);
}
static void ordered_entries(uint64_t address,size_t bytes) {
    unsigned char *input=malloc(bytes),*output=malloc(bytes);
    char **entries=malloc((bytes/20+1)*sizeof *entries);
    CHECK(input && output && entries); rd(address,input,bytes);
    size_t count_entries=0;
    for(size_t p=0;p<bytes;) {
        uint16_t size; CHECK(bytes-p>=20); memcpy(&size,input+p+16,2);
        CHECK(size>=20 && size<=bytes-p && memchr(input+p+19,0,size-19));
        entries[count_entries++]=(char *)input+p; p+=size;
    }
    qsort(entries,count_entries,sizeof *entries,entry_order);
    size_t offset=0;
    for(size_t i=0;i<count_entries;i++) {
        uint16_t size; memcpy(&size,entries[i]+16,2);
        memcpy(output+offset,entries[i],size); offset+=size;
    }
    wr(address,output,bytes); free(entries); free(output); free(input);
}
static int backing(int fd) {
    char p[128]; snprintf(p,sizeof p,"/proc/%d/fd/%d",child,fd);
    int x=open(p,O_RDWR); CHECK(x>=0); return x;
}
static struct alloc *findalloc(uint64_t h) {
    for(size_t i=0;i<count;i++) if(allocs[i].handle==h && !allocs[i].freed) return allocs+i;
    fail("unknown allocation handle"); return 0;
}
static void ioctl_reply(uint64_t request,uint64_t p) {
    fprintf(stderr,"TEST_SIM ioctl request=0x%llx bytes=%lu\n",(unsigned long long)request,(unsigned long)_IOC_SIZE(request));
    switch(request) {
    case AMDKFD_IOC_GET_VERSION: {
        struct kfd_ioctl_get_version_args a={1,14}; wr(p,&a,sizeof a); break;
    }
    case AMDKFD_IOC_ACQUIRE_VM: {
        struct kfd_ioctl_acquire_vm_args a; rd(p,&a,sizeof a);
        CHECK(a.drm_fd==(unsigned)drm && a.gpu_id==expected_gpu); break;
    }
    case AMDKFD_IOC_ALLOC_MEMORY_OF_GPU: {
        struct kfd_ioctl_alloc_memory_of_gpu_args a; rd(p,&a,sizeof a);
        CHECK(a.gpu_id==expected_gpu && a.va_addr && !(a.va_addr%2097152) && a.size && !(a.size%4096));
        CHECK(a.flags & KFD_IOC_ALLOC_MEM_FLAGS_WRITABLE);
        CHECK(a.flags & KFD_IOC_ALLOC_MEM_FLAGS_PUBLIC);
        allocs=realloc(allocs,(count+1)*sizeof *allocs); CHECK(allocs);
        a.handle=count+1; a.mmap_offset=endoff;
        allocs[count++]=(struct alloc){a.va_addr,a.size,endoff,a.handle,0,0};
        CHECK(a.size<=INT64_MAX-endoff); endoff+=a.size;
        CHECK(!ftruncate(dback,(off_t)endoff)); wr(p,&a,sizeof a); break;
    }
    case AMDKFD_IOC_MAP_MEMORY_TO_GPU:
    case AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU: {
        struct kfd_ioctl_map_memory_to_gpu_args a; rd(p,&a,sizeof a);
        uint32_t gpu; rd(a.device_ids_array_ptr,&gpu,sizeof gpu);
        CHECK(a.n_devices==1 && gpu==expected_gpu);
        struct alloc *b=findalloc(a.handle);
        CHECK(b->mapped==(request==AMDKFD_IOC_UNMAP_MEMORY_FROM_GPU));
        b->mapped=request==AMDKFD_IOC_MAP_MEMORY_TO_GPU;
        a.n_success=1; wr(p,&a,sizeof a); break;
    }
    case AMDKFD_IOC_FREE_MEMORY_OF_GPU: {
        struct kfd_ioctl_free_memory_of_gpu_args a; rd(p,&a,sizeof a);
        struct alloc *b=findalloc(a.handle); CHECK(!b->mapped); b->freed=1; break;
    }
    case AMDKFD_IOC_CREATE_QUEUE:
        rd(p,&queue,sizeof queue);
        for(size_t i=0;i<sizeof queue/8;i++) {
            uint64_t value; memcpy(&value,(char *)&queue+i*8,8);
            fprintf(stderr,"TEST_SIM queue_input_qword_%zu=%llu\n",i,(unsigned long long)value);
        }
        CHECK(count==10 && !queued && queue.gpu_id==expected_gpu && queue.queue_type==KFD_IOC_QUEUE_TYPE_COMPUTE_AQL);
        CHECK(queue.ring_base_address==allocs[5].va && queue.ring_size==1024);
        CHECK(queue.write_pointer_address==allocs[6].va && queue.read_pointer_address==allocs[7].va);
        /* AMD CP ABI: queue_properties is 52 bytes after read_dispatch_id.
         * ROCr AMD_QUEUE_PROPERTIES_IS_PTR64 is bit 1 for ELF64 kernels. */
        CHECK((word(queue.read_pointer_address+48)>>32)==2);
        CHECK(!queue.eop_buffer_address && !queue.eop_buffer_size);
        CHECK(queue.ctx_save_restore_address==allocs[8].va && queue.ctx_save_restore_size<=allocs[8].size);
        CHECK(queue.ctl_stack_size<=queue.ctx_save_restore_size);
        queue.queue_id=7; queue.doorbell_offset=8; wr(p,&queue,sizeof queue); queued=1; break;
    case AMDKFD_IOC_DESTROY_QUEUE: {
        struct kfd_ioctl_destroy_queue_args a; rd(p,&a,sizeof a);
        CHECK(queued && a.queue_id==7 && packets); queued=0; break;
    }
    default: fail("unexpected KFD request");
    }
}
static void complete_packets(void) {
    uint64_t nw=word(queue.write_pointer_address);
    while ((uint64_t)packets<nw) {
        uint64_t p=queue.ring_base_address+(uint64_t)(packets%16)*64;
        uint32_t h; rd(p,&h,4); CHECK(h==0x21502);
        uint16_t wg[3]; rd(p+4,wg,sizeof wg); CHECK(wg[0]==64 && wg[1]==1 && wg[2]==1);
        uint64_t kd=word(p+32),args=word(p+40),sig=word(p+56);
        CHECK(kd>=allocs[0].va && kd+64<=allocs[0].va+allocs[0].size);
        CHECK(args==allocs[1].va && sig==allocs[9].va);
        uint64_t a=word(args),b=word(args+8),out=word(args+16);
        uint32_t n; rd(args+24,&n,4); CHECK(n==256);
        CHECK(a==allocs[3].va && b==allocs[4].va && out==allocs[2].va);
        if(!getenv("KFD_TEST_NO_OUTPUT"))
            for(uint32_t i=0;i<n;i++) { float x,y,z; rd(a+4*i,&x,4); rd(b+4*i,&y,4); z=x+y; wr(out+4*i,&z,4); }
        CHECK(word(sig)==1 && word(sig+8));
        if(!getenv("KFD_TEST_NO_SIGNAL")) setword(sig+8,word(sig+8)-1);
        packets++; setword(queue.read_pointer_address,(uint64_t)packets);
    }
}
int main(int argc,char **argv) {
    CHECK(argc>=4);
    if(getenv("KFD_TEST_GPU")) expected_gpu=(unsigned)strtoul(getenv("KFD_TEST_GPU"),0,10);
    if(getenv("KFD_TEST_MINOR")) expected_minor=(unsigned)strtoul(getenv("KFD_TEST_MINOR"),0,10);
    child=fork(); CHECK(child>=0);
    if(!child) { CHECK(!ptrace(PTRACE_TRACEME,0,0,0)); raise(SIGSTOP); execv(argv[2],argv+2); _exit(92); }
    int st; CHECK(waitpid(child,&st,0)==child && WIFSTOPPED(st));
    CHECK(!ptrace(PTRACE_SETOPTIONS,child,0,PTRACE_O_TRACESYSGOOD|PTRACE_O_EXITKILL));
    int enter=1,kind=0,changed=0; uint64_t call=0,a[6]={0}; char old[4096];
    const char *prefix="/sys/class/kfd/kfd/topology/nodes/";
    for(;;) {
        CHECK(!ptrace(PTRACE_SYSCALL,child,0,0)); CHECK(waitpid(child,&st,0)==child);
        if(WIFEXITED(st)) {
            fprintf(stderr,"TEST_SIM child_exit=%d allocations=%zu packets=%d queued=%d\n",WEXITSTATUS(st),count,packets,queued);
            CHECK(WEXITSTATUS(st)==0 && count==10 && packets && !queued);
            for(size_t i=0;i<count;i++) CHECK(allocs[i].freed);
            fprintf(stderr,"TEST_SIM PASS allocations=%zu packets=%d freed=all device=NOT_RUN\n",count,packets);
            free(allocs); close(kback); close(dback); return 0;
        }
        CHECK(WIFSTOPPED(st)); if(WSTOPSIG(st)==SIGTRAP) continue;
        CHECK(WSTOPSIG(st)==(SIGTRAP|0x80));
        struct user_pt_regs r; struct iovec v={&r,sizeof r};
        CHECK(!ptrace(PTRACE_GETREGSET,child,(void *)NT_PRSTATUS,&v));
        if(enter) {
            call=r.regs[8]; memcpy(a,r.regs,sizeof a); kind=0; changed=0; denied=0;
            if(call==SYS_openat) {
                string(a[1],old,sizeof old);
                if(!strcmp(old,"/dev/kfd") || !strncmp(old,"/dev/dri/renderD",sizeof("/dev/dri/renderD")-1)) {
                    kind=!strcmp(old,"/dev/kfd")?1:2;
                    if(kind==2) {
                        CHECK(strtoul(old+strlen("/dev/dri/renderD"),0,10)==expected_minor);
                        if(getenv("KFD_TEST_RENDER_ABSENT")) {
                            denied=ENOENT; replace_call(&r,SYS_getpid); kind=0;
                        }
                    }
                    if(!denied) { r.regs[0]=a[1]; r.regs[1]=0; replace_call(&r,SYS_memfd_create); }
                } else if(!strncmp(old,prefix,strlen(prefix))) {
                    const char *deny=getenv("KFD_TEST_DENY");
                    if(deny && !strcmp(old+strlen(prefix),deny)) {
                        denied=EPERM; replace_call(&r,SYS_getpid);
                    } else {
                        char path[4096]; int n=snprintf(path,sizeof path,"%s/%s",argv[1],old+strlen(prefix));
                        CHECK(n>0 && (size_t)n<=strlen(old)); wr(a[1],path,(size_t)n+1); changed=1;
                    }
                } else if(!strncmp(old,"/dev/",5)) fail("unexpected device open");
            } else if(call==SYS_ioctl) { CHECK((int)a[0]==kfd); replace_call(&r,SYS_getpid); }
            else if(call==SYS_newfstatat) { string(a[1],old,sizeof old); CHECK(!strcmp(old,"/dev/kfd")); replace_call(&r,SYS_getpid); }
            else if(call==SYS_mmap && ((int)a[4]==drm || (int)a[4]==kfd)) {
                if((int)a[4]==drm) {
                    size_t i; for(i=0;i<count;i++) if(a[5]==allocs[i].off) break;
                    CHECK(i<count && a[0]==allocs[i].va && a[1]==allocs[i].size && a[3]==(MAP_SHARED|MAP_FIXED));
                } else CHECK(a[1]==8192 && a[5]==0 && a[3]==MAP_SHARED);
                fprintf(stderr,"TEST_SIM mmap fd_kind=%s fd=%llu offset=%llu bytes=%llu va=%llx\n",(int)a[4]==drm?"render":"kfd",(unsigned long long)a[4],(unsigned long long)a[5],(unsigned long long)a[1],(unsigned long long)a[0]);
            }
        } else {
            if(call==SYS_openat) {
                if(denied) { r.regs[0]=(uint64_t)-(int64_t)denied; setregs(&r); }
                if(changed) wr(a[1],old,strlen(old)+1);
                if(changed && !strcmp(old,prefix)) topology_fd=(int)r.regs[0];
                if(kind) { CHECK((int64_t)r.regs[0]>=0); int fd=(int)r.regs[0];
                    if(kind==1) { kfd=fd; kback=backing(fd); CHECK(!ftruncate(kback,8192)); }
                    else { drm=fd; dback=backing(fd); }
                }
            } else if(call==SYS_ioctl) { ioctl_reply(a[1],a[2]); r.regs[0]=0; setregs(&r); }
            else if(call==SYS_fstat && ((int)a[0]==kfd || (int)a[0]==drm)) {
                struct stat s; rd(a[1],&s,sizeof s); s.st_mode=S_IFCHR|0600;
                s.st_rdev=(int)a[0]==drm?makedev(226,expected_minor):makedev(234,0); wr(a[1],&s,sizeof s);
            } else if(call==SYS_newfstatat) {
                struct stat s={0}; s.st_mode=S_IFCHR|0600; s.st_rdev=makedev(234,0); wr(a[2],&s,sizeof s); r.regs[0]=0; setregs(&r);
            } else if(call==SYS_getdents64 && (int)a[0]==topology_fd && (int64_t)r.regs[0]>0 && getenv("KFD_TEST_SORT_NODES")) {
                ordered_entries(a[1],(size_t)r.regs[0]);
            } else if(call==SYS_clock_gettime && queued && ++clocks>=2) {
                if(clocks==2) {
                    fprintf(stderr,"TEST_SIM packet_kernel_object=%llu\n",(unsigned long long)word(queue.ring_base_address+32));
                    fprintf(stderr,"TEST_SIM packet_kernarg_address=%llu\n",(unsigned long long)word(queue.ring_base_address+40));
                }
                if(!getenv("KFD_TEST_NO_COMPLETE")) complete_packets();
                if(getenv("KFD_TEST_EXPIRE")) {
                    struct timespec now; rd(a[1],&now,sizeof now); now.tv_sec+=11; wr(a[1],&now,sizeof now);
                }
            }
        }
        enter=!enter;
    }
}
