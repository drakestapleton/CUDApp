/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_chip.c: the supported-target table, the doctor refusal sentence (developer bar D6), and the
 * library admission glue around the Omega decision om/admission/chip_admit.om (D2, D6). See omega_gpu_chip.h. */
#include "omega_gpu_chip.h"

#include "cudapp_native.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Target identities are unchanged. The GB10 row points to an imported historical summary;
 * its raw M16 logs are unavailable here, so it is not current-commit qualification.
 * Add a row only with durable raw evidence and a receipt; see README evidence limitation. */
static const OmegaGpuChipRow k_rows[] = {
    { OMEGA_GPU_PCI_VENDOR_NVIDIA, 0x2e12u, "NVIDIA GB10", "sm_121",
      "evidence/m16-blackwell-native-path-requalification-receipt.json" },
};

const OmegaGpuChipRow *omega_gpu_chip_find(uint16_t vendor, uint16_t device) {
    for (size_t i = 0; i < sizeof k_rows / sizeof k_rows[0]; i++)
        if (k_rows[i].vendor == vendor && k_rows[i].device == device) return &k_rows[i];
    return NULL;
}

static int read_hex(const char *path, unsigned long *out) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char line[64] = { 0 };
    int ok = fgets(line, sizeof line, f) != NULL;
    fclose(f);
    if (!ok) return -1;
    char *end = NULL;
    *out = strtoul(line, &end, 16);
    return end == line ? -1 : 0;
}

int omega_gpu_chip_scan(const char *root, OmegaGpuPciDev *out, int max) {
    char dir[4096];
    snprintf(dir, sizeof dir, "%s/sys/bus/pci/devices", root ? root : "");
    DIR *d = opendir(dir);
    if (!d) return -1;
    int found = 0;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.') continue;
        char p[4400];
        unsigned long vendor, device, cls;
        snprintf(p, sizeof p, "%s/%s/vendor", dir, e->d_name);
        if (read_hex(p, &vendor) != 0 || vendor != OMEGA_GPU_PCI_VENDOR_NVIDIA) continue;
        snprintf(p, sizeof p, "%s/%s/class", dir, e->d_name);
        if (read_hex(p, &cls) != 0 || (cls >> 16) != 0x03u) continue; /* display/3D only: skips the audio function */
        snprintf(p, sizeof p, "%s/%s/device", dir, e->d_name);
        if (read_hex(p, &device) != 0) continue;
        if (found < max && out) {
            OmegaGpuPciDev *g = &out[found];
            memset(g, 0, sizeof *g);
            snprintf(g->bdf, sizeof g->bdf, "%.63s", e->d_name);
            g->vendor = (uint16_t)vendor; g->device = (uint16_t)device; g->class_code = (uint32_t)cls;
        }
        found++;
    }
    closedir(d);
    return found;
}

int omega_gpu_chip_refusal(const OmegaGpuPciDev *d, char *buf, size_t n) {
    return snprintf(buf, n,
        "GPU %s: NVIDIA device 0x%04x is not one CUDA++ has qualified.\n"
        "Why: CUDA++ writes each chip's machine code itself and has run it only on the NVIDIA GB10 (sm_121),\n"
        "so it refuses this GPU now instead of failing later with \"no kernel image is available\".\n"
        "What is missing: a target row for this chip, a check that the encoder's instruction words are valid on it,\n"
        "its launch descriptor and class numbers, and the hello-gpu qualification run with its receipt.\n"
        "What to do: follow docs/runtime/PORTABILITY.md section 5 (\"What a new GPU needs\") and CONTRIBUTING.md (\"Port a chip\"),\n"
        "or report it with the output of `make doctor` attached.",
        d->bdf, d->device);
}

static const char *g_root; /* NULL: the real /sys */

void omega_gpu_chip_set_root(const char *root) { g_root = root; }

int omega_gpu_chip_admit(const OmegaGpuPciDev *devs, int n, const char *bdf, OmegaGpuChipAdmission *out) {
    uint64_t d[4 * OMEGA_GPU_ADMIT_MAX] = { 0 }, o[4] = { 0 }, status = 0;
    memset(out, 0, sizeof *out);
    if (n < 0 || (n > 0 && !devs)) return out->status = OMEGA_GPU_ADMIT_BAD_INPUT;
    /* More than 16 functions reaches the unit as n > 16, which it refuses as bad input. */
    for (int i = 0; i < n && i < OMEGA_GPU_ADMIT_MAX; i++) {
        d[4 * i] = devs[i].vendor;
        d[4 * i + 1] = devs[i].device;
        d[4 * i + 2] = devs[i].class_code;
        d[4 * i + 3] = bdf && strcmp(devs[i].bdf, bdf) == 0;
    }
    if (omega_chip_admit(&status, d, 4 * OMEGA_GPU_ADMIT_MAX, (uint64_t)n, bdf != NULL, o, 4) != OMEGA_OK)
        return out->status = OMEGA_GPU_ADMIT_UNIT;
    out->status = (int)status;
    out->index = (int)o[1];
    out->target_sm = (uint32_t)o[2];
    out->device = (uint16_t)o[3];
    return out->status;
}

#define NO_NODE "the library opened no /dev/nvidia node"

static int sentence(const OmegaGpuChipAdmission *a, const OmegaGpuPciDev *dev, const char *bdf, const char *root,
                    char *why, size_t n) {
    switch (a->status) {
    case OMEGA_GPU_ADMIT_NO_SYSFS:
        snprintf(why, n, "cannot read %.120s/sys/bus/pci/devices to name the GPU, so " NO_NODE ". Run `make doctor`.",
                 root ? root : "");
        break;
    case OMEGA_GPU_ADMIT_NO_GPU:
        snprintf(why, n, "no NVIDIA GPU under /sys/bus/pci/devices, so " NO_NODE ". Run `make doctor`.");
        break;
    case OMEGA_GPU_ADMIT_NO_BDF:
        snprintf(why, n, "CUDAPP_GPU_BDF=%.32s names no NVIDIA GPU under /sys/bus/pci/devices, so " NO_NODE
                 ". Run `make doctor` for the addresses.", bdf ? bdf : "");
        break;
    case OMEGA_GPU_ADMIT_REFUSED:
        snprintf(why, n, "GPU %.32s: NVIDIA device 0x%04x is not one CUDA++ has qualified, so " NO_NODE
                 ". Run `make doctor` for why, what is missing and what to do.", dev ? dev->bdf : "?", a->device);
        break;
    case OMEGA_GPU_ADMIT_BAD_INPUT:
        snprintf(why, n, "GPU admission refused: more than %d NVIDIA GPUs, or CUDAPP_GPU_BDF matched twice, so " NO_NODE
                 ".", OMEGA_GPU_ADMIT_MAX);
        break;
    default:
        snprintf(why, n, "GPU admission unit om/admission/chip_admit.om failed, so " NO_NODE ".");
        break;
    }
    return -1;
}

int omega_gpu_chip_admit_open(const char *bdf, char *why, size_t n) {
    OmegaGpuPciDev devs[OMEGA_GPU_ADMIT_MAX];
    OmegaGpuChipAdmission a = { 0 };
    int found = omega_gpu_chip_scan(g_root, devs, OMEGA_GPU_ADMIT_MAX);
    if (found < 0) a.status = OMEGA_GPU_ADMIT_NO_SYSFS;
    else if (omega_gpu_chip_admit(devs, found, bdf, &a) == OMEGA_GPU_ADMIT_OK) return 0;
    const OmegaGpuPciDev *dev = found > 0 && a.index < found && a.index < OMEGA_GPU_ADMIT_MAX ? &devs[a.index] : NULL;
    return sentence(&a, dev, bdf, g_root, why, n);
}

int omega_gpu_chip_admit_card(uint32_t domain, uint32_t bus, uint32_t slot, uint32_t function, uint16_t vendor,
                              uint16_t device, uint32_t *target_sm, char *why, size_t n) {
    /* NV_ESC_CARD_INFO lists GPUs only, so the card is given the display class 0x030000. */
    OmegaGpuPciDev card = { .vendor = vendor, .device = device, .class_code = 0x030000u };
    snprintf(card.bdf, sizeof card.bdf, "%04x:%02x:%02x.%x", domain & 0xffffu, bus & 0xffu, slot & 0x1fu, function & 7u);
    OmegaGpuChipAdmission a = { 0 };
    *target_sm = 0;
    if (omega_gpu_chip_admit(&card, 1, card.bdf, &a) == OMEGA_GPU_ADMIT_OK) {
        *target_sm = a.target_sm;
        return 0;
    }
    if (a.status == OMEGA_GPU_ADMIT_NO_BDF) { /* the driver named a non-NVIDIA vendor */
        snprintf(why, n, "GPU %s: the driver reports vendor 0x%04x device 0x%04x, not an NVIDIA GPU CUDA++ has qualified;"
                 " refused before opening the device node.", card.bdf, vendor, device);
        return -1;
    }
    if (a.status == OMEGA_GPU_ADMIT_REFUSED) {
        snprintf(why, n, "GPU %s: the driver reports NVIDIA device 0x%04x, which is not one CUDA++ has qualified;"
                 " refused before opening the device node. Run `make doctor`.", card.bdf, device);
        return -1;
    }
    return sentence(&a, &card, card.bdf, NULL, why, n);
}
