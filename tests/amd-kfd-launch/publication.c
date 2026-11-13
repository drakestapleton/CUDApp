/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * CPU-only AArch64 instruction-step observer of real Omega packet stores. */
#define _GNU_SOURCE
#include <asm/ptrace.h>
#include <elf.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>
static pid_t child;
static void fail(const char *s) { fprintf(stderr,"PUBLICATION FAIL %s\n",s); exit(1); }
#define CHECK(x) do { if(!(x)) fail(#x); } while(0)
static void readmem(uint64_t a,void *p,size_t n) {
    for(size_t i=0;i<n;i+=8) {
        errno=0; long v=ptrace(PTRACE_PEEKDATA,child,(void *)(uintptr_t)(a+i),0);
        CHECK(!errno); memcpy((char *)p+i,&v,n-i<8?n-i:8);
    }
}
static uint64_t u64(uint64_t a) { uint64_t v; readmem(a,&v,8); return v; }
int main(int argc,char **argv) {
    CHECK(argc==2); child=fork(); CHECK(child>=0);
    if(!child) { CHECK(!ptrace(PTRACE_TRACEME,0,0,0)); raise(SIGSTOP); execl(argv[1],argv[1],(char *)0); _exit(92); }
    int st; CHECK(waitpid(child,&st,0)==child && WIFSTOPPED(st));
    CHECK(!ptrace(PTRACE_SETOPTIONS,child,0,PTRACE_O_TRACESYSGOOD|PTRACE_O_EXITKILL));
    int entry=1,armed=0,stepping=0,published=0,rang=0; uint64_t a[6]={0};
    for(;;) {
        CHECK(!ptrace(stepping?PTRACE_SINGLESTEP:PTRACE_SYSCALL,child,0,0));
        CHECK(waitpid(child,&st,0)==child);
        if(WIFEXITED(st)) { CHECK(WEXITSTATUS(st)==0 && published && rang); fprintf(stderr,"PUBLICATION PASS body-before-header-before-wptr-before-doorbell\n"); return 0; }
        CHECK(WIFSTOPPED(st));
        if(!stepping) {
            if(WSTOPSIG(st)==SIGTRAP) continue;
            CHECK(WSTOPSIG(st)==(SIGTRAP|0x80));
            struct user_pt_regs r; struct iovec v={&r,sizeof r};
            CHECK(!ptrace(PTRACE_GETREGSET,child,(void *)NT_PRSTATUS,&v));
            if(entry && r.regs[8]==SYS_write && r.regs[0]==1) {
                CHECK(!armed && r.regs[2]==sizeof a); readmem(r.regs[1],a,sizeof a); armed=1;
            }
            if(!entry && armed) stepping=1;
            entry=!entry;
        } else {
            CHECK(WSTOPSIG(st)==SIGTRAP);
            uint32_t h=(uint32_t)u64(a[0]);
            if((h&65535)==0x1502) {
                CHECK(h==0x21502);
                uint16_t wg[3]; uint32_t grid[3];
                readmem(a[0]+4,wg,sizeof wg); readmem(a[0]+12,grid,sizeof grid);
                CHECK(wg[0]==64 && wg[1]==1 && wg[2]==1);
                CHECK(grid[0]==256 && grid[1]==1 && grid[2]==1);
                CHECK(u64(a[0]+24)==0 && u64(a[0]+32)==a[1]+64);
                CHECK(u64(a[0]+40)==a[2] && !u64(a[0]+48) && u64(a[0]+56)==a[3]);
                published=1;
            } else CHECK(h==1);
            if(u64(a[4])) CHECK(published && u64(a[4])==1);
            if(u64(a[5])!=UINT64_MAX) { CHECK(published && u64(a[4])==1 && !u64(a[5])); rang=1; }
        }
    }
}
