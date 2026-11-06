/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Library GPU admission (developer bars D2, D6): om/admission/chip_admit.om through src/omega_gpu_chip.c, and
 * its two call sites in nvrm_open. HOST_ONLY: nvrm.c is compiled into this file with open/ioctl/close/mmap
 * replaced by a simulated driver, so no /dev/nvidia node is ever opened; every path nvrm_open tries to
 * open is recorded instead. Fake PCI trees are built under a temporary folder.
 *   1. the pure decision against a C host reference built from the doctor's chip table (omega_gpu_chip_find)
 *      and the prepared Ampere ids of tests/ampere/doctor.py, over every device id and four classes;
 *   2. nvrm_open on an unknown chip, an empty tree, an unreadable tree and an unmatched CUDAPP_GPU_BDF:
 *      refused with the sentence and zero device-node opens (the negative controls);
 *   3. nvrm_open on a GB10 tree: the guard passes and the driver's card is checked again, first with the
 *      matching card (reaches the usermode allocation, which the simulation fails), then a different card
 *      (refused before /dev/nvidia0), then a card admitted as sm_86 whose classes select sm_121 (refused). */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
static int sim_ioctl(int, unsigned long, ...);
static int sim_open(const char *, int, ...);
static int sim_close(int);
static void *sim_mmap(void *, size_t, int, int, int, off_t);
#define ioctl sim_ioctl
#define open sim_open
#define close sim_close
#define mmap sim_mmap
#ifndef NVRM_C
#define NVRM_C "../nvrm/nvrm.c"
#endif
#include NVRM_C
#undef ioctl
#undef open
#undef close
#undef mmap
#include "cudapp_native.h" /* omega_chip_admit, the generated entry of om/admission/chip_admit.om */

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); exit(1); } } while (0)

/* ---- simulated driver ---- */
static char opened[16][32];
static unsigned n_opened;
static nv_pci_info_t g_card;
static uint32_t g_sm_version;
static const uint32_t g_bw_classes[] = { BLACKWELL_COMPUTE_B, BLACKWELL_CHANNEL_GPFIFO_A, HOPPER_USERMODE_A };

static int sim_open(const char *path, int flags, ...) {
    (void)flags;
    if (n_opened < 16) snprintf(opened[n_opened], sizeof opened[0], "%s", path);
    n_opened++;
    /* Control nodes and the GPU node get a fake descriptor; nothing reaches the OS. */
    if (!strcmp(path, "/dev/nvidiactl") || !strcmp(path, "/dev/nvidia-uvm") || !strcmp(path, "/dev/nvidia0")) return 700;
    errno = ENOENT;
    return -1;
}
static int sim_close(int fd) { CHECK(fd == 700); return 0; }
static void *sim_mmap(void *va, size_t size, int prot, int flags, int fd, off_t offset) {
    (void)va; (void)size; (void)prot; (void)flags; (void)fd; (void)offset;
    CHECK(!"no mapping is reached in these tests");
    return MAP_FAILED;
}
static int sim_ioctl(int fd, unsigned long request, ...) {
    CHECK(fd == 700);
    va_list ap; va_start(ap, request); void *arg = va_arg(ap, void *); va_end(ap);
    if (request == UVM_INITIALIZE) { ((UVM_INITIALIZE_PARAMS *)arg)->rmStatus = NV_OK; return 0; }
    if (request == UVM_MM_INITIALIZE) return 0;
    CHECK(_IOC_TYPE(request) == NV_IOCTL_MAGIC);
    switch (_IOC_NR(request)) {
    case NV_ESC_CARD_INFO: {
        nv_ioctl_card_info_t *cards = arg;
        cards[0].valid = 1; cards[0].pci_info = g_card; cards[0].gpu_id = 0x100; cards[0].minor_number = 0;
        return 0;
    }
    case NV_ESC_REGISTER_FD: return 0;
    case NV_ESC_RM_ALLOC: {
        NVOS21_PARAMETERS *p = arg;
        if (p->hClass == NV01_ROOT_CLIENT) p->hObjectNew = 0xc1d00001u;
        p->status = p->hClass == HOPPER_USERMODE_A ? NV_ERR_INVALID_STATE : NV_OK; /* stop after admission */
        return 0;
    }
    case NV_ESC_RM_FREE: return 0;
    case NV_ESC_RM_CONTROL: {
        NVOS54_PARAMETERS *p = arg;
        void *params = (void *)(uintptr_t)p->params;
        p->status = NV_OK;
        if (p->cmd == NV2080_CTRL_CMD_GR_GET_INFO) {
            NV2080_CTRL_GR_GET_INFO_PARAMS *g = params;
            ((NV2080_CTRL_GR_INFO *)(uintptr_t)g->grInfoList)->data = g_sm_version;
        } else if (p->cmd == NV0080_CTRL_CMD_GPU_GET_CLASSLIST) {
            NV0080_CTRL_GPU_GET_CLASSLIST_PARAMS *c = params;
            if (c->classList) memcpy((void *)(uintptr_t)c->classList, g_bw_classes, sizeof g_bw_classes);
            c->numClasses = 3;
        }
        return 0;
    }
    default: CHECK(!"unexpected simulated driver operation"); return -1;
    }
}

/* ---- fake PCI trees ---- */
static char g_tmp[256];
static void put(const char *root, const char *bdf, const char *file, const char *value) {
    char p[512];
    snprintf(p, sizeof p, "%s/sys/bus/pci/devices/%s", root, bdf);
    char cmd[640];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s'", p);
    CHECK(system(cmd) == 0);
    snprintf(p, sizeof p, "%s/sys/bus/pci/devices/%s/%s", root, bdf, file);
    FILE *f = fopen(p, "w");
    CHECK(f != NULL);
    fprintf(f, "%s\n", value);
    fclose(f);
}
static void gpu(const char *root, const char *bdf, const char *vendor, const char *device, const char *cls) {
    put(root, bdf, "vendor", vendor); put(root, bdf, "device", device); put(root, bdf, "class", cls);
}
static const char *tree(const char *name) {
    static char p[320];
    snprintf(p, sizeof p, "%s/%s", g_tmp, name);
    char cmd[400];
    snprintf(cmd, sizeof cmd, "mkdir -p '%s/sys/bus/pci/devices'", p);
    CHECK(system(cmd) == 0);
    return p;
}

/* ---- host reference ---- */
static const uint16_t k_ampere[] = { 0x2204u, 0x2486u, 0x2489u }; /* tests/ampere/doctor.py CARDS */
static uint32_t ref_target(uint16_t vendor, uint16_t device, uint32_t cls) {
    if (vendor != OMEGA_GPU_PCI_VENDOR_NVIDIA || (cls >> 16) != 0x03u) return 0;
    const OmegaGpuChipRow *r = omega_gpu_chip_find(vendor, device);
    if (r) return (uint32_t)strtoul(r->sm + 3, NULL, 10); /* "sm_121" */
    for (size_t i = 0; i < sizeof k_ampere / sizeof k_ampere[0]; i++) if (k_ampere[i] == device) return 86;
    return 0;
}

static void differential(void) {
    static const uint32_t classes[] = { 0x030000u, 0x030200u, 0x040300u, 0x060400u };
    static const uint16_t vendors[] = { 0x10deu, 0x1002u };
    unsigned same = 0, admitted = 0;
    for (size_t v = 0; v < 2; v++)
        for (size_t c = 0; c < 4; c++)
            for (uint32_t dev = 0; dev <= 0xffffu; dev++) {
                OmegaGpuPciDev d = { .vendor = vendors[v], .device = (uint16_t)dev, .class_code = classes[c] };
                snprintf(d.bdf, sizeof d.bdf, "0000:01:00.0");
                OmegaGpuChipAdmission a;
                int st = omega_gpu_chip_admit(&d, 1, NULL, &a);
                uint32_t want = ref_target(d.vendor, d.device, d.class_code);
                int want_st = want ? OMEGA_GPU_ADMIT_OK
                            : (d.vendor == 0x10deu && (d.class_code >> 16) == 3u) ? OMEGA_GPU_ADMIT_REFUSED
                            : OMEGA_GPU_ADMIT_NO_GPU;
                if (st != want_st || a.target_sm != want || (st == OMEGA_GPU_ADMIT_REFUSED && a.device != dev)) {
                    fprintf(stderr, "CHIP ADMIT DIFF vendor 0x%04x device 0x%04x class 0x%06x: omega status %d sm %u,"
                            " reference status %d sm %u\n", d.vendor, dev, d.class_code, st, a.target_sm, want_st, want);
                    exit(1);
                }
                same++;
                admitted += st == OMEGA_GPU_ADMIT_OK;
            }
    CHECK(admitted == 2 * 4); /* GB10 and three Ampere ids, each in two display classes */
    printf("chip admission: .om decision equals the C host reference on %u single-GPU inputs (%u admitted)\n", same,
           admitted);
}

static void decisions(void) {
    OmegaGpuPciDev d[18];
    memset(d, 0, sizeof d);
    OmegaGpuChipAdmission a;
    CHECK(omega_gpu_chip_admit(d, 0, NULL, &a) == OMEGA_GPU_ADMIT_NO_GPU);
    CHECK(omega_gpu_chip_admit(d, 0, "000f:01:00.0", &a) == OMEGA_GPU_ADMIT_NO_BDF);
    snprintf(d[0].bdf, 64, "000f:00:00.0"); d[0].vendor = 0x10de; d[0].device = 0x22d1; d[0].class_code = 0x060400;
    snprintf(d[1].bdf, 64, "000f:01:00.0"); d[1].vendor = 0x10de; d[1].device = 0x2e12; d[1].class_code = 0x030000;
    snprintf(d[2].bdf, 64, "000f:01:00.1"); d[2].vendor = 0x10de; d[2].device = 0x22ce; d[2].class_code = 0x040300;
    CHECK(omega_gpu_chip_admit(d, 3, NULL, &a) == OMEGA_GPU_ADMIT_OK && a.target_sm == 121 && a.index == 1);
    CHECK(omega_gpu_chip_admit(d, 3, "000f:01:00.0", &a) == OMEGA_GPU_ADMIT_OK && a.target_sm == 121);
    CHECK(omega_gpu_chip_admit(d, 3, "000f:01:00.1", &a) == OMEGA_GPU_ADMIT_NO_BDF && a.target_sm == 0);
    /* A second, unknown GPU: refused without an address, ignored with the GB10's address. */
    snprintf(d[3].bdf, 64, "0001:01:00.0"); d[3].vendor = 0x10de; d[3].device = 0x2b85; d[3].class_code = 0x030000;
    CHECK(omega_gpu_chip_admit(d, 4, NULL, &a) == OMEGA_GPU_ADMIT_REFUSED && a.index == 3 && a.device == 0x2b85 &&
          a.target_sm == 0);
    CHECK(omega_gpu_chip_admit(d, 4, "000f:01:00.0", &a) == OMEGA_GPU_ADMIT_OK && a.target_sm == 121);
    CHECK(omega_gpu_chip_admit(d, 4, "0001:01:00.0", &a) == OMEGA_GPU_ADMIT_REFUSED && a.device == 0x2b85);
    /* An address matching two entries and more than 16 GPUs are bad input. */
    snprintf(d[3].bdf, 64, "000f:01:00.0");
    CHECK(omega_gpu_chip_admit(d, 4, "000f:01:00.0", &a) == OMEGA_GPU_ADMIT_BAD_INPUT && a.target_sm == 0);
    CHECK(omega_gpu_chip_admit(d, 17, NULL, &a) == OMEGA_GPU_ADMIT_BAD_INPUT);
    CHECK(omega_gpu_chip_admit(d, -1, NULL, &a) == OMEGA_GPU_ADMIT_BAD_INPUT);
    {   /* The unit's own input contract, called directly (the C wrapper only ever passes 0 or 1 as the match word):
         * a match word above 1 or a want above 1 is bad input; the same entry with match word 1 is admitted. */
        uint64_t w[4 * OMEGA_GPU_ADMIT_MAX] = { 0x10de, 0x2e12, 0x030000, 2 }, o[4] = { 0 }, st = 99;
        CHECK(omega_chip_admit(&st, w, 4 * OMEGA_GPU_ADMIT_MAX, 1, 1, o, 4) == OMEGA_OK && st == 4 && o[0] == 4 && o[2] == 0);
        w[3] = 1;
        CHECK(omega_chip_admit(&st, w, 4 * OMEGA_GPU_ADMIT_MAX, 1, 1, o, 4) == OMEGA_OK && st == 0 && o[2] == 121);
        CHECK(omega_chip_admit(&st, w, 4 * OMEGA_GPU_ADMIT_MAX, 1, 2, o, 4) == OMEGA_OK && st == 4 && o[2] == 0);
    }
    /* Ampere ids admitted as sm_86. */
    for (size_t i = 0; i < 3; i++) {
        d[1].device = k_ampere[i];
        CHECK(omega_gpu_chip_admit(d, 3, NULL, &a) == OMEGA_GPU_ADMIT_OK && a.target_sm == 86);
    }
    puts("chip admission: decision cases (bridge, audio, two GPUs, address, duplicates, more than 16, Ampere) pass");
}

static void open_refused(const char *root, const char *bdf, const char *expect) {
    static Nvrm rm;
    n_opened = 0;
    omega_gpu_chip_set_root(root);
    if (bdf) setenv("CUDAPP_GPU_BDF", bdf, 1); else unsetenv("CUDAPP_GPU_BDF");
    CHECK(nvrm_open(&rm) == -1);
    if (!strstr(rm.err, expect) || n_opened != 0 || rm.va_slot_base != 0 || strlen(rm.err) >= 240) {
        fprintf(stderr, "FAIL refusal: err \"%s\" (%zu bytes), %u device opens, expected \"%s\"\n", rm.err,
                strlen(rm.err), n_opened, expect);
        exit(1);
    }
    (void)nvrm_close(&rm);
    printf("refused, 0 device opens: %s\n", rm.err);
}

static void open_admitted(uint16_t card_device, uint32_t sm_version, const char *expect, const char *last_open) {
    static Nvrm rm;
    n_opened = 0;
    g_card = (nv_pci_info_t){ .domain = 0xf, .bus = 1, .slot = 0, .function = 0, .vendor_id = 0x10de,
                              .device_id = card_device };
    g_sm_version = sm_version;
    CHECK(nvrm_open(&rm) == -1);
    if (!strstr(rm.err, expect) || n_opened == 0 || n_opened > 16 || strcmp(opened[n_opened - 1], last_open) != 0) {
        fprintf(stderr, "FAIL admitted path: err \"%s\", %u opens, last \"%s\", expected \"%s\" after \"%s\"\n",
                rm.err, n_opened, n_opened ? opened[n_opened - 1] : "", expect, last_open);
        exit(1);
    }
    CHECK(strcmp(opened[0], "/dev/nvidiactl") == 0);
    (void)nvrm_close(&rm);
    printf("guard passed, %u simulated opens (last %s): %s\n", n_opened, last_open, rm.err);
}

int main(void) {
    snprintf(g_tmp, sizeof g_tmp, "%s/cudapp-chip-admission-XXXXXX", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
    CHECK(mkdtemp(g_tmp) != NULL);
    decisions();
    differential();

    /* Negative controls: refused with the sentence before any device node. */
    const char *unknown = tree("unknown");
    gpu(unknown, "0000:01:00.0", "0x10de", "0x2b85", "0x030000");
    gpu(unknown, "0000:01:00.1", "0x10de", "0x22e8", "0x040300");
    open_refused(unknown, NULL, "GPU 0000:01:00.0: NVIDIA device 0x2b85 is not one CUDA++ has qualified, so the library "
                 "opened no /dev/nvidia node. Run `make doctor`");
    open_refused(unknown, "0000:01:00.0", "NVIDIA device 0x2b85 is not one CUDA++ has qualified");
    open_refused(tree("empty"), NULL, "no NVIDIA GPU under /sys/bus/pci/devices, so the library opened no /dev/nvidia");
    char missing[320];
    snprintf(missing, sizeof missing, "%s/absent", g_tmp);
    open_refused(missing, NULL, "to name the GPU, so the library opened no /dev/nvidia node");
    const char *gb10 = tree("gb10");
    gpu(gb10, "000f:00:00.0", "0x10de", "0x22d1", "0x060400");
    gpu(gb10, "000f:01:00.0", "0x10de", "0x2e12", "0x030000");
    open_refused(gb10, "0000:02:00.0", "CUDAPP_GPU_BDF=0000:02:00.0 names no NVIDIA GPU");
    open_refused(gb10, "000f:00:00.0", "CUDAPP_GPU_BDF=000f:00:00.0 names no NVIDIA GPU");

    /* Admitted: the guard passes; the driver's card is checked again. */
    unsetenv("CUDAPP_GPU_BDF");
    omega_gpu_chip_set_root(gb10);
    /* The physical Spark reports raw RM SM 10.04 for 10de:2e12 (tests/test_ampere_runtime.c). */
    open_admitted(0x2e12, NV2080_CTRL_GR_INFO_SM_VERSION_10_04, "RM_ALLOC class 0xc661 status", "/dev/nvidia0");
    open_admitted(0x2e13, NV2080_CTRL_GR_INFO_SM_VERSION_12_01,
                  "GPU 000f:01:00.0: the driver reports NVIDIA device 0x2e13, which is not one CUDA++ has qualified",
                  "/dev/nvidia-uvm");
    open_admitted(0x2204, NV2080_CTRL_GR_INFO_SM_VERSION_12_01,
                  "GPU admission admitted sm_86 but the driver classes select sm_121; refused", "/dev/nvidia0");
    setenv("CUDAPP_GPU_BDF", "000f:01:00.0", 1);
    open_admitted(0x2e12, NV2080_CTRL_GR_INFO_SM_VERSION_10_04, "RM_ALLOC class 0xc661 status", "/dev/nvidia0");
    omega_gpu_chip_set_root(NULL);

    char cmd[320];
    snprintf(cmd, sizeof cmd, "rm -rf '%s'", g_tmp);
    CHECK(system(cmd) == 0);
    puts("chip admission tests passed (HOST_ONLY: simulated driver, fake PCI trees; chip NOT_RUN)");
    return 0;
}
