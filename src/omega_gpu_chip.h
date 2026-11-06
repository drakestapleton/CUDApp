/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_chip.h: which GPUs CUDA++ has machine code for, and what it says about the others.
 *
 * Developer bars D2 and D6 (docs/PAIN_POINTS.md). Two layers:
 *   - the doctor (tools/doctor) reports the qualified-target table below and, for any other GPU,
 *     the long refusal text of omega_gpu_chip_refusal;
 *   - the library itself refuses at device open: nvrm_open calls omega_gpu_chip_admit_open before it
 *     opens any /dev/nvidia node, and omega_gpu_chip_admit_card on the driver-reported card before it
 *     opens /dev/nvidiaN. The decision is the seeded Omega unit om/admission/chip_admit.om; this C
 *     only reads sysfs, compares the CUDAPP_GPU_BDF string and formats the sentence.
 * The library admits the GB10 (sm_121) and the prepared, chip-NOT_RUN Ampere ids of
 * docs/PORT-AMPERE.md (sm_86); the doctor's qualified table holds only the GB10.
 *
 * A chip is named by its PCI vendor and device id, read from /sys/bus/pci/devices. That needs no
 * device open and no driver call. (The driver's SM-version field reads 0xa04 on the GB10, which does
 * not tell sm_120 from sm_121: evidence/m16-blackwell-native-path-requalification-receipt.json.)
 * New rows require durable raw evidence and a receipt. The existing GB10 row points to a
 * historical M16 summary whose raw logs are absent; it is not qualification of this checkout. */
#ifndef OMEGA_GPU_CHIP_H
#define OMEGA_GPU_CHIP_H

#include <stddef.h>
#include <stdint.h>

#define OMEGA_GPU_PCI_VENDOR_NVIDIA 0x10deu

typedef struct {
    uint16_t vendor, device;
    const char *name;      /* "NVIDIA GB10" */
    const char *sm;        /* "sm_121" */
    const char *receipt;   /* historical summary path, not current-commit qualification */
} OmegaGpuChipRow;

typedef struct {
    char bdf[64];          /* PCI address, e.g. "000f:01:00.0" */
    uint16_t vendor, device;
    uint32_t class_code;   /* 0x03xxxx: display or 3D controller */
} OmegaGpuPciDev;

/* The supported target row for vendor:device, or NULL. This is not an admission check. */
const OmegaGpuChipRow *omega_gpu_chip_find(uint16_t vendor, uint16_t device);

/* Every NVIDIA display/3D-class PCI function under <root>/sys/bus/pci/devices (root "" = the real
 * system). Fills up to max entries; returns how many were found (may exceed max), or -1 when the
 * folder cannot be read. */
int omega_gpu_chip_scan(const char *root, OmegaGpuPciDev *out, int max);

/* The doctor refusal text for a GPU with no target row: what it is, why the diagnostic refuses,
 * what is missing, and what to do. Returns the length written (snprintf rules). */
int omega_gpu_chip_refusal(const OmegaGpuPciDev *d, char *buf, size_t n);

/* Library admission. Status values are those of om/admission/chip_admit.om, plus two C-side ones. */
enum {
    OMEGA_GPU_ADMIT_OK = 0,
    OMEGA_GPU_ADMIT_NO_GPU = 1,     /* no NVIDIA display/3D-class PCI function */
    OMEGA_GPU_ADMIT_NO_BDF = 2,     /* CUDAPP_GPU_BDF names no NVIDIA display/3D-class function */
    OMEGA_GPU_ADMIT_REFUSED = 3,    /* an NVIDIA GPU that is not a CUDA++ target */
    OMEGA_GPU_ADMIT_BAD_INPUT = 4,  /* more than OMEGA_GPU_ADMIT_MAX GPUs, or the address matched twice */
    OMEGA_GPU_ADMIT_NO_SYSFS = 5,   /* <root>/sys/bus/pci/devices cannot be read */
    OMEGA_GPU_ADMIT_UNIT = 6,       /* the Omega unit returned a nonzero binding status */
};
#define OMEGA_GPU_ADMIT_MAX 16

typedef struct {
    int status;            /* OMEGA_GPU_ADMIT_* */
    int index;             /* entry the decision is about (refused or first admitted) */
    uint32_t target_sm;    /* 121 or 86 when admitted, else 0 */
    uint16_t device;       /* PCI device id of that entry */
} OmegaGpuChipAdmission;

/* Pure decision over n scanned functions (no I/O): bdf NULL = the driver's first card, else the
 * canonical dddd:bb:ss.f address. Fills *out and returns out->status. */
int omega_gpu_chip_admit(const OmegaGpuPciDev *devs, int n, const char *bdf, OmegaGpuChipAdmission *out);

/* Before any device open: scan sysfs (root set by omega_gpu_chip_set_root, default the real
 * system), decide, and on refusal write one sentence (under 240 bytes) to why. 0 admitted. */
int omega_gpu_chip_admit_open(const char *bdf, char *why, size_t n);

/* After the driver names the card it will open (NV_ESC_CARD_INFO): decide again on the driver's PCI
 * vendor:device; *target_sm gets the admitted target. On refusal writes the sentence. 0 admitted. */
int omega_gpu_chip_admit_card(uint32_t domain, uint32_t bus, uint32_t slot, uint32_t function, uint16_t vendor,
                              uint16_t device, uint32_t *target_sm, char *why, size_t n);

/* Tests only: read <root>/sys/bus/pci/devices instead of /sys/bus/pci/devices (NULL restores). */
void omega_gpu_chip_set_root(const char *root);

#endif
