/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/*
 * Host test of native/share/share_native.om, the OM-2 function-mode zero-copy unit, called
 * through a hosted native library that omega-bridge tools/native --host builds from that one
 * input. The library is a test artifact under the build directory; the product links the
 * combined hosted library that Integration generates.
 *
 * Fake RM/UVM: seccomp traps ioctl on two deliberately unopened descriptors and the SIGSYS
 * handler answers. It decodes every request with the NVIDIA 580.173.02 headers, so a literal in
 * the unit that disagrees with them fails here instead of agreeing with itself. Memory objects
 * are memfds; RM_MAP_MEMORY dup3()s the object's memfd over the fresh descriptor the unit
 * opened, so the unit's own mapbind maps the owner's real bytes. No device file is opened.
 * Exit 0 and "ZERO-COPY SHARE NATIVE PASS" on success; "native-share: FAIL line N" otherwise.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <ucontext.h>
#include <unistd.h>

#include "nv-ioctl-numbers.h"
#include "nv-unix-nvos-params-wrappers.h"
#include "nv_escape.h"
#include "nvos.h"
#include "nvstatuscodes.h"
#include "rs_access.h"
#include "uvm_ioctl.h"

#include "omega_api.h"

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "native-share: FAIL line %d: %s\n", __LINE__, #c); exit(1); } } while (0)
#define REQ(nr, type) ((unsigned long)_IOWR(NV_IOCTL_MAGIC, (nr), type))

enum { FK_CTL = 12345, FK_UVM = 12346 };
#define OC 0xc1d00001u /* owner client */
#define OD 0xd1000001u /* owner device */
#define OH 0xbeef0010u /* owner memory */
#define OH2 0xbeef0020u /* owner memory never shared */
#define IC 0xc1d00002u /* importer client */
#define ID 0xd1000002u /* importer device */
#define IV 0x7a000001u /* importer GPU VA space */
#define NEWH 0xa0010001u
#define SZ 8192u
/* WolframLanguageEvaluator 16^^5a00000000 -> 386547056640; < 2^40 - 2^16 -> True. */
#define A1 0x5a00000000ull
#define A2 0x5a00100000ull
#define A3 0x5a00200000ull

enum { K_SHARE, K_DUP, K_FREE, K_MAP, K_DMA, K_UNDMA, K_RANGE, K_EXT, K_UFREE, K_N };
struct fkobj { uint32_t client, parent, handle; int fd; uint64_t size; int shared, used; };
struct fkrange { uint64_t base, size; uint32_t mem; int dma, ext, used; };
static struct fkobj fk_obj[16];
static struct fkrange fk_rng[8];
static volatile sig_atomic_t fk_bad;
static unsigned fk_calls[K_N];
static unsigned fk_fail_kind = K_N;
static NvU32 fk_fail_status;
static int fk_dupzero, fk_dmashift;
static uint32_t fk_mode; /* the access mode the next import must request: 0 RW, 1 RO */
static const uint8_t fk_uuid[16] = { 0x47, 0x42, 0x31, 0x30, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

#define BAD() (fk_bad = fk_bad ? fk_bad : __LINE__)

static int fk_find(uint32_t client, uint32_t handle)
{
	for (int i = 0; i < 16; i++)
		if (fk_obj[i].used && fk_obj[i].client == client && fk_obj[i].handle == handle)
			return i;
	return -1;
}
static int fk_new(uint32_t client, uint32_t parent, uint32_t handle, int fd, uint64_t size)
{
	for (int i = 0; i < 16; i++)
		if (!fk_obj[i].used) {
			fk_obj[i] = (struct fkobj){ client, parent, handle, fd, size, 0, 1 };
			return i;
		}
	return -1;
}
static int fk_range(uint64_t base)
{
	for (int i = 0; i < 8; i++)
		if (fk_rng[i].used && fk_rng[i].base == base)
			return i;
	return -1;
}
/* Count the call; answer an injected status once. */
static int fk_inject(unsigned kind, NvU32 *status)
{
	fk_calls[kind]++;
	if (fk_fail_kind != kind)
		return 0;
	fk_fail_kind = K_N;
	*status = fk_fail_status;
	return 1;
}

static void fk_rm(unsigned long req, void *arg)
{
	if (req == REQ(NV_ESC_RM_SHARE, NVOS57_PARAMETERS)) {
		NVOS57_PARAMETERS *p = arg;
		if (fk_inject(K_SHARE, &p->status))
			return;
		RS_SHARE_POLICY *sp = &p->sharePolicy;
		if (sp->target != 0 || sp->accessMask.limbs[0] != (1u << RS_ACCESS_DUP_OBJECT) ||
		    sp->type != RS_SHARE_TYPE_OS_SECURITY_TOKEN || sp->action != 0)
			BAD();
		for (unsigned i = 1; i < SDK_RS_ACCESS_MAX_LIMBS; i++)
			if (sp->accessMask.limbs[i])
				BAD();
		int i = fk_find(p->hClient, p->hObject);
		p->status = i < 0 ? NV_ERR_OBJECT_NOT_FOUND : NV_OK;
		if (i >= 0)
			fk_obj[i].shared = 1;
	} else if (req == REQ(NV_ESC_RM_DUP_OBJECT, NVOS55_PARAMETERS)) {
		NVOS55_PARAMETERS *p = arg;
		if (fk_inject(K_DUP, &p->status))
			return;
		if (p->flags != NV04_DUP_HANDLE_FLAGS_NONE || p->hClient != IC || p->hParent != ID)
			BAD();
		int s = fk_find(p->hClientSrc, p->hObjectSrc);
		if (s < 0)
			p->status = NV_ERR_OBJECT_NOT_FOUND;
		else if (!fk_obj[s].shared && p->hClientSrc != p->hClient)
			p->status = NV_ERR_INSUFFICIENT_PERMISSIONS;
		else if (fk_find(p->hClient, p->hObject) >= 0)
			p->status = NV_ERR_INSERT_DUPLICATE_NAME;
		else {
			int fd = fcntl(fk_obj[s].fd, F_DUPFD_CLOEXEC, 100);
			if (fd < 0 || fk_new(p->hClient, p->hParent, p->hObject, fd, fk_obj[s].size) < 0)
				BAD();
			p->status = NV_OK;
			if (fk_dupzero)
				p->hObject = 0;
		}
	} else if (req == REQ(NV_ESC_RM_FREE, NVOS00_PARAMETERS)) {
		NVOS00_PARAMETERS *p = arg;
		if (fk_inject(K_FREE, &p->status))
			return;
		int i = fk_find(p->hRoot, p->hObjectOld);
		if (i < 0 || fk_obj[i].parent != p->hObjectParent) {
			p->status = NV_ERR_OBJECT_NOT_FOUND;
			return;
		}
		/* nvmrel order: the GPU mappings of the object go before the object. */
		for (int r = 0; r < 8; r++)
			if (fk_rng[r].used && fk_rng[r].mem == p->hObjectOld && (fk_rng[r].dma || fk_rng[r].ext))
				BAD();
		close(fk_obj[i].fd);
		fk_obj[i].used = 0;
		p->status = NV_OK;
	} else if (req == REQ(NV_ESC_RM_MAP_MEMORY, nv_ioctl_nvos33_parameters_with_fd)) {
		nv_ioctl_nvos33_parameters_with_fd *w = arg;
		NVOS33_PARAMETERS *p = &w->params;
		if (fk_inject(K_MAP, &p->status))
			return;
		if (p->hClient != IC || p->hDevice != ID || w->fd < 3 || w->fd == FK_CTL ||
		    p->flags != (fk_mode ? NVOS33_FLAGS_ACCESS_READ_ONLY : 0) || p->pLinearAddress)
			BAD();
		int i = fk_find(p->hClient, p->hMemory);
		if (i < 0)
			p->status = NV_ERR_OBJECT_NOT_FOUND;
		else if (p->offset >= fk_obj[i].size)
			p->status = NV_ERR_INVALID_BASE;
		else if (p->length == 0 || p->length > fk_obj[i].size - p->offset)
			p->status = NV_ERR_INVALID_LIMIT;
		else {
			if (dup3(fk_obj[i].fd, w->fd, O_CLOEXEC) != w->fd)
				BAD();
			p->status = NV_OK;
		}
	} else if (req == REQ(NV_ESC_RM_MAP_MEMORY_DMA, NVOS46_PARAMETERS)) {
		NVOS46_PARAMETERS *p = arg;
		if (fk_inject(K_DMA, &p->status))
			return;
		int i = fk_find(p->hClient, p->hMemory), r = fk_range(p->dmaOffset);
		/* context.om nvalloc flags; WolframLanguageEvaluator BitOr[2^15,2^8,2^4] -> 33040. */
		if (p->hClient != IC || p->hDevice != ID || p->hDma != IV || p->offset != 0 ||
		    p->flags != (33040u | (fk_mode ? NVOS46_FLAGS_ACCESS_READ_ONLY : 0)) ||
		    p->flags2 != 0 || p->kindOverride != 0)
			BAD();
		if (i < 0 || r < 0 || fk_rng[r].dma || p->length != fk_obj[i].size || fk_rng[r].size != p->length) {
			p->status = NV_ERR_INVALID_ARGUMENT;
			return;
		}
		fk_rng[r].dma = 1;
		fk_rng[r].mem = p->hMemory;
		if (fk_dmashift)
			p->dmaOffset += 4096;
		p->status = NV_OK;
	} else if (req == REQ(NV_ESC_RM_UNMAP_MEMORY_DMA, NVOS47_PARAMETERS)) {
		NVOS47_PARAMETERS *p = arg;
		if (fk_inject(K_UNDMA, &p->status))
			return;
		int r = fk_range(p->dmaOffset);
		if (p->hClient != IC || p->hDevice != ID || p->hDma != IV || p->flags != 0)
			BAD();
		/* UVM_FREE of the range goes first in nvmrel order; the DMA record stays here. */
		if (r >= 0 || p->size != SZ) {
			p->status = NV_ERR_INVALID_ARGUMENT;
			return;
		}
		p->status = NV_OK;
	} else
		BAD();
}

static void fk_uvm(unsigned long req, void *arg)
{
	if (req == UVM_CREATE_EXTERNAL_RANGE) {
		UVM_CREATE_EXTERNAL_RANGE_PARAMS *p = arg;
		if (fk_inject(K_RANGE, &p->rmStatus))
			return;
		for (int i = 0; i < 8; i++)
			if (!fk_rng[i].used) {
				fk_rng[i] = (struct fkrange){ p->base, p->length, 0, 0, 0, 1 };
				p->rmStatus = NV_OK;
				return;
			}
		BAD();
	} else if (req == UVM_MAP_EXTERNAL_ALLOCATION) {
		UVM_MAP_EXTERNAL_ALLOCATION_PARAMS *p = arg;
		if (fk_inject(K_EXT, &p->rmStatus))
			return;
		int r = fk_range(p->base);
		if (p->offset != 0 || p->gpuAttributesCount != 1 || p->rmCtrlFd != FK_CTL || p->hClient != IC ||
		    memcmp(&p->perGpuAttributes[0].gpuUuid, fk_uuid, 16) != 0 ||
		    p->perGpuAttributes[0].gpuMappingType !=
			    (fk_mode ? UvmGpuMappingTypeReadOnly : UvmGpuMappingTypeReadWriteAtomic))
			BAD();
		if (r < 0 || !fk_rng[r].dma || fk_rng[r].ext || fk_rng[r].mem != p->hMemory || p->length != fk_rng[r].size) {
			p->rmStatus = NV_ERR_INVALID_ARGUMENT;
			return;
		}
		fk_rng[r].ext = 1;
		p->rmStatus = NV_OK;
	} else if (req == UVM_FREE) {
		UVM_FREE_PARAMS *p = arg;
		if (fk_inject(K_UFREE, &p->rmStatus))
			return;
		int r = fk_range(p->base);
		if (r < 0 || p->length != fk_rng[r].size) {
			p->rmStatus = NV_ERR_INVALID_ARGUMENT;
			return;
		}
		fk_rng[r].used = 0;
		p->rmStatus = NV_OK;
	} else
		BAD();
}

static void fk_sigsys(int signal, siginfo_t *info, void *opaque)
{
	(void)signal;
	ucontext_t *uc = opaque;
	int saved = errno;
#if defined(__aarch64__)
	long fd = (long)uc->uc_mcontext.regs[0];
	unsigned long req = uc->uc_mcontext.regs[1];
	void *arg = (void *)(uintptr_t)uc->uc_mcontext.regs[2];
	uc->uc_mcontext.regs[0] = 0;
#elif defined(__x86_64__)
	long fd = (long)uc->uc_mcontext.gregs[REG_RDI];
	unsigned long req = uc->uc_mcontext.gregs[REG_RSI];
	void *arg = (void *)(uintptr_t)uc->uc_mcontext.gregs[REG_RDX];
	uc->uc_mcontext.gregs[REG_RAX] = 0;
#else
#error "native-share-test requires Linux AArch64 or x86-64"
#endif
	if (info->si_syscall != SYS_ioctl)
		BAD();
	else if (fd == FK_CTL)
		fk_rm(req, arg);
	else if (fd == FK_UVM)
		fk_uvm(req, arg);
	else
		BAD();
	errno = saved;
}

static void fk_install(void)
{
	struct sigaction action = { 0 };
	action.sa_sigaction = fk_sigsys;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	CHECK(sigaction(SIGSYS, &action, NULL) == 0);
	struct sock_filter filters[] = {
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_ioctl, 0, 4),
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, args[0])),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, FK_CTL, 1, 0),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, FK_UVM, 0, 1),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRAP),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
	};
	struct sock_fprog program = { .len = sizeof filters / sizeof filters[0], .filter = filters };
	CHECK(prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == 0);
	CHECK(prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &program) == 0);
}

static omega_native_context *ctx;
static uint8_t par[9264];
static const char devpath[] = "/dev/null";

static unsigned calls(void)
{
	unsigned n = 0;
	for (int k = 0; k < K_N; k++)
		n += fk_calls[k];
	return n;
}
static int64_t ex(zcshare *s, zcrec *r)
{
	uint64_t res = 0xdead;
	CHECK(omega2_ctx_nvshare_export(ctx, &res, (uint8_t *)s, (uint8_t *)r, par, sizeof par) == 0);
	CHECK((int64_t)res == s->err);
	return (int64_t)res;
}
static int64_t im_ctx(omega_native_context *c, zcshare *s, const zcrec *r)
{
	uint64_t res = 0xdead;
	CHECK(omega2_ctx_nvshare_import(c, &res, (uint8_t *)s, (const uint8_t *)r, par, sizeof par,
					(const uint8_t *)devpath, strlen(devpath)) == 0);
	CHECK((int64_t)res == s->err);
	return (int64_t)res;
}
static int64_t im(zcshare *s, const zcrec *r) { return im_ctx(ctx, s, r); }
static int64_t rel(zcshare *s)
{
	uint64_t res = 0xdead;
	CHECK(omega2_ctx_nvshare_release(ctx, &res, (uint8_t *)s, par, sizeof par) == 0);
	CHECK((int64_t)res == s->err);
	return (int64_t)res;
}
static void block(zcshare *s, uint32_t newh, uint32_t mode, uint64_t size, uint64_t addr)
{
	memset(s, 0, sizeof *s);
	s->ctlfd = FK_CTL;
	s->uvmfd = FK_UVM;
	s->client = IC;
	s->device = ID;
	s->vaspace = IV;
	s->newh = newh;
	s->mode = mode;
	s->size = size;
	s->addr = addr;
	memcpy(s->uuid, fk_uuid, 16);
	fk_mode = mode;
}
static int unmapped(uint64_t addr)
{
	unsigned char v;
	return mincore((void *)(uintptr_t)addr, 4096, &v) != 0 && errno == ENOMEM;
}
/* Write one byte at addr in a child: 0 if the write landed, else the terminating signal. */
static int child_write(uint64_t addr)
{
	pid_t c = fork();
	CHECK(c >= 0);
	if (!c) {
		*(volatile uint8_t *)(uintptr_t)addr = 0x99;
		_exit(0);
	}
	int st = 0;
	CHECK(waitpid(c, &st, 0) == c);
	return WIFSIGNALED(st) ? WTERMSIG(st) : WEXITSTATUS(st) ? -1 : 0;
}
/* A clean refusal: the expected error, no RM or UVM call, nothing live, no fault. */
static void refused(zcshare *s, const zcrec *r, int64_t want, int line)
{
	unsigned before = calls();
	int64_t got = im(s, r);
	if (got != want || calls() != before || s->live || s->fault || s->obj) {
		fprintf(stderr, "native-share: FAIL line %d: refusal got %lld want %lld calls %u->%u live %u fault %u\n",
			line, (long long)got, (long long)want, before, calls(), s->live, s->fault);
		exit(1);
	}
}

int main(void)
{
	/* Owner object: two pages with a known pattern, also mapped here to watch aliasing. */
	int ofd = memfd_create("zero-copy-owner", MFD_CLOEXEC);
	CHECK(ofd >= 0 && ftruncate(ofd, SZ) == 0);
	uint8_t *ov = mmap(NULL, SZ, PROT_READ | PROT_WRITE, MAP_SHARED, ofd, 0);
	CHECK(ov != MAP_FAILED);
	for (unsigned i = 0; i < SZ; i++)
		ov[i] = (uint8_t)(i * 7 + 1);
	int ofd2 = memfd_create("zero-copy-owner2", MFD_CLOEXEC);
	CHECK(ofd2 >= 0 && ftruncate(ofd2, SZ) == 0);
	CHECK(fk_new(OC, OD, OH, ofd, SZ) >= 0 && fk_new(OC, OD, OH2, ofd2, SZ) >= 0);
	CHECK(omega_native_context_create(4, &ctx) == 0);
	fk_install();

	/* Export. */
	zcshare own, s;
	zcrec rec, junk;
	memset(&own, 0, sizeof own);
	own.ctlfd = FK_CTL;
	own.client = OC;
	own.device = OD;
	own.obj = OH;
	own.size = SZ;
	memset(&rec, 0xaa, sizeof rec);
	CHECK(ex(&own, &rec) == 0 && fk_calls[K_SHARE] == 1 && own.stage == 51 && !fk_bad);
	CHECK(rec.magic == 1129991503u && rec.version == 1 && rec.client == OC && rec.handle == OH && rec.size == SZ);
	CHECK(fk_obj[fk_find(OC, OH)].shared && !fk_obj[fk_find(OC, OH2)].shared);
	/* Export refusals: no RM call, record untouched; an RM refusal is its positive status. */
	unsigned before = calls();
	zcshare bad = own;
	memset(&junk, 0xaa, sizeof junk);
	zcrec junk0 = junk;
	bad.obj = 0;
	CHECK(ex(&bad, &junk) == -22);
	bad = own;
	bad.size = 4097;
	CHECK(ex(&bad, &junk) == -22);
	bad.size = 0;
	CHECK(ex(&bad, &junk) == -22);
	bad.size = 68719476736ull + 4096; /* WolframLanguageEvaluator 2^36 -> 68719476736 */
	CHECK(ex(&bad, &junk) == -22);
	bad = own;
	bad.fault = 1;
	CHECK(ex(&bad, &junk) == -5);
	CHECK(calls() == before && memcmp(&junk, &junk0, sizeof junk) == 0);
	bad = own;
	bad.obj = OH + 99;
	CHECK(ex(&bad, &junk) == NV_ERR_OBJECT_NOT_FOUND && fk_calls[K_SHARE] == 2);
	CHECK(memcmp(&junk, &junk0, sizeof junk) == 0);

	/* Read/write import at A1: one RM duplicate, size proof, CPU map, GPU map. */
	block(&s, NEWH, 0, SZ, A1);
	uint64_t res = 0xdead;
	before = calls();
	CHECK(omega2_ctx_nvshare_import(ctx, &res, (uint8_t *)&s, (const uint8_t *)&rec, par, sizeof par - 1,
					(const uint8_t *)devpath, strlen(devpath)) == OMEGA_BAD_ARG);
	CHECK(res == 0xdead && calls() == before && s.live == 0);
	CHECK(im(&s, &rec) == 0 && !fk_bad);
	CHECK(s.live == 31 && s.stage == 57 && s.obj == NEWH && s.map > 0 && s.fault == 0);
	CHECK(fk_calls[K_DUP] == 1 && fk_calls[K_MAP] == 2 && fk_calls[K_RANGE] == 1 && fk_calls[K_DMA] == 1 &&
	      fk_calls[K_EXT] == 1);
	uint8_t *a = (uint8_t *)(uintptr_t)A1;
	CHECK(memcmp(a, ov, SZ) == 0);
	a[100] = 0x5c;
	ov[SZ - 1] = 0x77;
	CHECK(ov[100] == 0x5c && a[SZ - 1] == 0x77);
	CHECK(child_write(A1 + 64) == 0);
	/* The owner's object goes away; the duplicate keeps its own reference. */
	int oi = fk_find(OC, OH);
	int keep = dup(fk_obj[oi].fd);
	close(fk_obj[oi].fd);
	fk_obj[oi].used = 0;
	CHECK(a[100] == 0x5c && fk_find(IC, NEWH) >= 0);
	/* An import cannot be re-exported, and the block holds one import at a time. */
	before = calls();
	CHECK(ex(&s, &junk) == -1 && im(&s, &rec) == -16 && calls() == before && s.live == 31);

	/* Release through another context: the RM and UVM steps run, then mapdrop finds the token
	 * foreign and the call traps (bridge status 3, result unwritten). The block, written in
	 * place, still records the CPU mapping (live 16|2); the right context finishes the release. */
	omega_native_context *other = NULL;
	CHECK(omega_native_context_create(2, &other) == 0);
	CHECK(omega_native_context_destroy(ctx) == OMEGA_CONTEXT_BUSY);
	res = 0xdead;
	uint32_t st = omega2_ctx_nvshare_release(other, &res, (uint8_t *)&s, par, sizeof par);
	CHECK(st == 3 /* Omega trap 3: mapdrop of a foreign token */ && res == 0xdead && s.live == 18 && s.fault == 0);
	CHECK(s.map > 0 && a[100] == 0x5c);
	CHECK(fk_calls[K_UFREE] == 1 && fk_calls[K_UNDMA] == 1 && fk_calls[K_FREE] == 1 && fk_find(IC, NEWH) < 0);
	CHECK(omega_native_context_destroy(other) == 0);
	CHECK(rel(&s) == 0 && s.live == 0 && s.obj == 0 && s.map == 0 && !fk_bad);
	CHECK(fk_calls[K_UFREE] == 1 && fk_calls[K_UNDMA] == 1 && fk_calls[K_FREE] == 1);
	CHECK(unmapped(A1));
	CHECK(rel(&s) == 0 && calls() == before + 3);
	/* Restore the owner's object for the remaining imports. */
	CHECK(fk_new(OC, OD, OH, keep, SZ) >= 0);
	fk_obj[fk_find(OC, OH)].shared = 1;

	/* A complete release order: UVM free, DMA unmap, RM free, CPU unmap; then the context is empty. */
	block(&s, NEWH, 0, SZ, A1);
	CHECK(im(&s, &rec) == 0 && s.live == 31);
	CHECK(rel(&s) == 0 && s.live == 0 && unmapped(A1) && !fk_bad);
	CHECK(fk_calls[K_UFREE] == 2 && fk_calls[K_UNDMA] == 2 && fk_calls[K_FREE] == 2);

	/* Read-only import: every access flag read-only; a CPU write faults. */
	block(&s, NEWH, 1, SZ, A2);
	CHECK(im(&s, &rec) == 0 && s.live == 31 && !fk_bad);
	CHECK(memcmp((void *)(uintptr_t)A2, ov, SZ) == 0);
	CHECK(child_write(A2 + 64) == SIGSEGV);
	CHECK(rel(&s) == 0 && unmapped(A2));
	fk_mode = 0;

	/* Clean refusals before any RM call. */
	zcrec r2;
	block(&s, NEWH, 0, SZ, A1);
	r2 = rec;
	r2.magic++;
	refused(&s, &r2, -22, __LINE__);
	r2 = rec;
	r2.version = 2;
	refused(&s, &r2, -22, __LINE__);
	r2 = rec;
	r2.client = 0;
	refused(&s, &r2, -22, __LINE__);
	r2 = rec;
	r2.handle = 0;
	refused(&s, &r2, -22, __LINE__);
	r2 = rec;
	r2.size = SZ + 1;
	refused(&s, &r2, -22, __LINE__);
	block(&s, NEWH, 0, SZ * 2, A1);
	refused(&s, &rec, -34, __LINE__);
	block(&s, NEWH, 2, SZ, A1);
	refused(&s, &rec, -22, __LINE__);
	block(&s, 0, 0, SZ, A1);
	refused(&s, &rec, -22, __LINE__);
	block(&s, NEWH, 0, SZ, A1 + 16);
	refused(&s, &rec, -75, __LINE__);
	block(&s, NEWH, 0, SZ, 4096);
	refused(&s, &rec, -75, __LINE__);
	/* WolframLanguageEvaluator 2^40 - 8192 -> 1099511619584: the first page past the bound. */
	block(&s, NEWH, 0, SZ, 1099511619584ull + 4096);
	refused(&s, &rec, -75, __LINE__);
	block(&s, NEWH, 0, SZ, A1);
	s.fault = 1;
	before = calls();
	CHECK(im(&s, &rec) == -5 && calls() == before);

	/* RM refusals: an unshared object, an unknown one; positive status, nothing live. */
	block(&s, NEWH, 0, SZ, A1);
	r2 = rec;
	r2.handle = OH2;
	CHECK(im(&s, &r2) == NV_ERR_INSUFFICIENT_PERMISSIONS && s.live == 0 && s.fault == 0 && s.obj == 0);
	r2.handle = OH + 5;
	CHECK(im(&s, &r2) == NV_ERR_OBJECT_NOT_FOUND && s.live == 0 && s.fault == 0);

	/* Size lies: the record claims more or less than the object holds. The duplicate is freed. */
	unsigned frees = fk_calls[K_FREE];
	block(&s, NEWH, 0, SZ * 2, A1);
	r2 = rec;
	r2.size = SZ * 2;
	CHECK(im(&s, &r2) == -34 && s.live == 0 && s.fault == 0 && s.obj == 0 && s.stage == 53);
	CHECK(fk_calls[K_FREE] == frees + 1 && fk_find(IC, NEWH) < 0 && unmapped(A1));
	block(&s, NEWH, 0, 4096, A1);
	r2.size = 4096;
	CHECK(im(&s, &r2) == -34 && s.live == 0 && s.fault == 0 && fk_calls[K_FREE] == frees + 2);
	/* An RM refusal of the CPU map is that status, also cleanly undone. */
	block(&s, NEWH, 0, SZ, A1);
	fk_fail_kind = K_MAP;
	fk_fail_status = NV_ERR_NO_MEMORY;
	CHECK(im(&s, &rec) == NV_ERR_NO_MEMORY && s.live == 0 && s.fault == 0 && fk_calls[K_FREE] == frees + 3);

	/* Slots: a context with one free slot takes one import; the next is refused cleanly. */
	omega_native_context *small = NULL;
	CHECK(omega_native_context_create(1, &small) == 0);
	block(&s, NEWH, 0, SZ, A1);
	CHECK(im_ctx(small, &s, &rec) == 0 && s.live == 31);
	zcshare s2;
	block(&s2, NEWH + 1, 0, SZ, A3);
	CHECK(im_ctx(small, &s2, &rec) == -24 && s2.live == 0 && s2.fault == 0 && fk_find(IC, NEWH + 1) < 0);
	CHECK(omega_native_context_destroy(small) == OMEGA_CONTEXT_BUSY);
	res = 0xdead;
	CHECK(omega2_ctx_nvshare_release(small, &res, (uint8_t *)&s, par, sizeof par) == 0 && res == 0);
	CHECK(s.live == 0 && omega_native_context_destroy(small) == 0);

	/* Faults. A duplicate returned as handle 0 faults the block; later imports are refused. */
	block(&s, NEWH, 0, SZ, A1);
	fk_dupzero = 1;
	CHECK(im(&s, &rec) == -22 && s.fault == 1 && s.live == 0);
	fk_dupzero = 0;
	CHECK(im(&s, &rec) == -5);
	CHECK(fk_find(IC, NEWH) >= 0);
	fk_obj[fk_find(IC, NEWH)].used = 0; /* the fake's leaked duplicate */
	/* A refused DMA map faults with the CPU map and UVM range live; release undoes both. */
	block(&s, NEWH, 0, SZ, A1);
	fk_fail_kind = K_DMA;
	fk_fail_status = NV_ERR_INVALID_ARGUMENT;
	CHECK(im(&s, &rec) == NV_ERR_INVALID_ARGUMENT && s.fault == 1 && s.live == 7 && s.stage == 55);
	CHECK(rel(&s) == 0 && s.live == 0 && s.fault == 1 && unmapped(A1) && fk_find(IC, NEWH) < 0 && !fk_bad);
	CHECK(im(&s, &rec) == -5);
	/* A DMA map at another address than asked faults with everything recorded for release. */
	block(&s, NEWH, 0, SZ, A1);
	fk_dmashift = 1;
	CHECK(im(&s, &rec) == -22 && s.fault == 1 && s.live == 15);
	fk_dmashift = 0;
	CHECK(rel(&s) == 0 && s.live == 0 && unmapped(A1) && !fk_bad);
	/* A refused UVM external mapping faults; release still undoes the rest. */
	block(&s, NEWH, 0, SZ, A1);
	fk_fail_kind = K_EXT;
	fk_fail_status = NV_ERR_INVALID_ARGUMENT;
	CHECK(im(&s, &rec) == NV_ERR_INVALID_ARGUMENT && s.fault == 1 && s.live == 15 && s.stage == 56);
	CHECK(rel(&s) == 0 && s.live == 0 && unmapped(A1) && !fk_bad);
	/* A refused RM free keeps the duplicate and the CPU map live for a retry. */
	block(&s, NEWH, 0, SZ, A1);
	CHECK(im(&s, &rec) == 0);
	fk_fail_kind = K_FREE;
	fk_fail_status = NV_ERR_INVALID_STATE;
	CHECK(rel(&s) == NV_ERR_INVALID_STATE && s.live == 19 && s.fault == 1 && !unmapped(A1));
	CHECK(rel(&s) == 0 && s.live == 0 && unmapped(A1) && fk_find(IC, NEWH) < 0);

	CHECK(!fk_bad);
	CHECK(omega_native_context_destroy(ctx) == 0);
	printf("ZERO-COPY SHARE NATIVE PASS (export, read/write and read-only import, aliasing, release order, "
	       "refusals, faults, slots; fake RM via seccomp, HOST_ONLY)\n");
	return 0;
}
