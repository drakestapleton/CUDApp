/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* spirv-host: INTERIM C launcher that runs one SPIR-V module from backend/spirv/emit.om on a CPU
 * implementation: PoCL (OpenCL, format SPIRV_CL) or Mesa lavapipe (Vulkan, format SPIRV_VK), or on a
 * GPU through a vendor Vulkan runtime (MT_TARGET below: MoltenVK on Apple, NVIDIA's driver on the GB10).
 * It is a host-only conformance harness; it never generates or edits SPIR-V and never chooses results.
 *
 * Why C: seeded Omega has no checked shared-library call yet, so a .om program cannot call
 * libOpenCL or libvulkan (Omega extension proposal E2, ADR 0043, aien-dev/omega-mv#888). Migration-pending: this file only
 * builds with -DCUDAPP_MIGRATION_PENDING_SPIRV_HOST=1, which defaults OFF (mk/multitarget.mk).
 * Successor: backend/vulkan/host.om and backend/opencl/host.om over E2's checked calls.
 *
 * GPU isolation (MT_TARGET=0): the OpenCL loader is pinned to one absolute CPU library (MT_POCL_LIBRARY) and only
 * CL_DEVICE_TYPE_CPU is enumerated; the Vulkan loader is pinned to the lavapipe manifest
 * (MT_LVP_ICD) with layers disabled, and the device must report VK_PHYSICAL_DEVICE_TYPE_CPU.
 * Before exit the process scans /proc/self/maps and /proc/self/fd and fails (exit 9) on any mapping
 * or descriptor whose path contains MT_FORBID ("nvidia" by default; the gate builds a control copy
 * with a pattern that must trip).
 *
 * usage: spirv-host cl|vk MODULE.spv KERNEL COUNT IN.bin [IN.bin] OUT.bin
 *   COUNT is the output element count (n); every input file holds COUNT*IN_RATIO floats, where
 *   IN_RATIO = (input bytes) / (4*COUNT) must be a positive integer. Buffers are passed in order
 *   (inputs, then output); cl also gets uint n as the last argument. Output starts as the NaN
 *   pattern 0xFFC0FFEE so an unwritten element can never compare equal to a real result.
 * Exit: 0 ran; 2 usage; 3 environment; 4 input; 5 device; 6 program; 7 kernel; 8 run; 9 isolation.
 *
 * MT_TARGET selects the build (default 0):
 *   0  the CPU build above (Linux: PoCL for cl, lavapipe for vk).
 *   1  Apple GPU through MoltenVK on macOS (format SPIRV_VK only; ADR 0043 names MoltenVK + SPIR-V as
 *      the Apple oracle route, MSL_SOURCE stays PLANNED). The binary links MT_MVK_LIBRARY directly, with
 *      no Vulkan loader. Every inherited MVK_* variable is removed and MVK_CONFIG_FAST_MATH_ENABLED is
 *      set to MT_MVK_FAST_MATH ("0" by default: never compile Metal shaders with fast math, so binary32
 *      adds keep IEEE round-to-nearest and the frozen fsum16 order). The device must be the only one,
 *      VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, vendor 0x106b (Apple), driverID MOLTENVK, and its name
 *      must contain MT_EXPECT_DEVICE ("Apple M2" by default). Isolation reads the dyld image list: it
 *      fails (exit 9) unless MT_MVK_LIBRARY is loaded, and on any loaded image whose path contains
 *      MT_FORBID ("libvulkan" by default: a Khronos loader could select another driver). This is a
 *      vendor-runtime-backed route (Metal via MoltenVK), not an independent Apple GPU driver.
 *   2  NVIDIA GPU through NVIDIA's own Vulkan driver on Linux (format SPIRV_VK only; DRA-201: the same
 *      modules as on the Apple GPU, to compare one unchanged application across two GPU vendors). The
 *      Khronos loader is pinned to the one manifest MT_NV_ICD (NVIDIA's nvidia_icd.json), layers are
 *      disabled, every inherited __GL_* and __NV_* variable is removed and __GL_SHADER_DISK_CACHE is set
 *      to MT_NV_DISK_CACHE ("0" by default: no on-disk shader cache, every run compiles the module;
 *      MT_NV_CACHE_PATH names a private cache directory for the cache lifecycle test). The device must be
 *      the only one, a GPU (integrated or discrete), vendor 0x10de, driverID NVIDIA_PROPRIETARY, name
 *      containing MT_EXPECT_DEVICE ("NVIDIA GB10" by default). Isolation reads /proc/self/maps: it fails
 *      (exit 9) unless a mapping contains MT_REQUIRE ("libnvidia-glcore", the driver's core) and on any
 *      mapping or descriptor containing one of MT_FORBID ("vulkan_lvp:libpocl:libcuda.so:libnvcuda":
 *      no CPU fallback and no CUDA library). This is a vendor-runtime route (NVIDIA's proprietary
 *      Vulkan driver and its SPIR-V compiler), not CUDA++ execution: CUDA++'s own sm_121 path is
 *      examples/hello_gpu.
 *   3  GPU through a vendor OpenCL driver on Linux (format SPIRV_CL only; DRA-194: Intel's compute runtime
 *      on an Arc Pro B-Series / Xe2 GPU). No Vulkan code is built. GPU mode is opt-in twice: this build,
 *      and MT_CL_GPU=1 in the environment at run time; anything else exits 2 before the OpenCL loader is
 *      touched. The loader is pinned to the one vendor ICD library MT_CL_GPU_ICD (absolute; for Intel the
 *      path named in /etc/OpenCL/vendors/intel.icd). That path and its realpath must not contain an
 *      MT_FORBID entry ("libpocl:nvidia:libcuda:libnvcuda" by default: no CPU implementation, no NVIDIA
 *      driver), checked before the loader runs (exit 9). Inherited OCL_ICD_*, OPENCL_*, POCL_*, IGC_*, ZE_*
 *      and NEOReadDebugKeys variables are removed. Only CL_DEVICE_TYPE_GPU is enumerated: a platform with
 *      no GPU device exits 5 (never a CPU fallback); with more than one, MT_CL_GPU_INDEX picks one. The
 *      device must report vendor id MT_CL_GPU_VENDOR (0x8086, Intel, by default) and a name containing
 *      MT_EXPECT_DEVICE ("" by default: any). Platform, device, CL_DRIVER_VERSION, CL_DEVICE_VERSION and
 *      IL versions are printed. While the context exists /proc/self/maps must map MT_CL_GPU_REQUIRE (the
 *      ICD's realpath by default) and no mapping or descriptor may contain an MT_FORBID entry (exit 9).
 *      This is a vendor-runtime route (Intel's OpenCL driver and its SPIR-V compiler), not CUDA++
 *      execution on an Intel GPU.
 *
 * Every vk run also prints one observation line (never a pass condition):
 *   spirv-host: time setup_us=... pipeline_us=... run_us=... device_ns=... total_us=... maxrss_kib=... buffers_bytes=...
 * setup = instance, device, buffers and upload; pipeline = shader module + compute pipeline (the driver's
 * compile); run = record, submit and wait; device_ns = the timestamp-query span around the dispatch
 * (NOT_RUN when the queue has no timestamps); total = process start to output ready; maxrss = peak
 * resident set of this process.
 * Every MT_TARGET=3 run prints the same kind of line for OpenCL (observation only):
 *   spirv-host: time setup_us=... build_us=... run_us=... device_ns=... total_us=... maxrss_kib=... buffers_bytes=...
 * setup = loader, platform, device, context and queue; build = clCreateProgramWithIL + clBuildProgram (the
 * driver's compile); run = kernel, buffers (upload), dispatch and read back; device_ns = the NDRange
 * event's CL_PROFILING_COMMAND_START..END span. The line after it counts open /dev/dri/ descriptors
 * (dri_fds, observation only).
 */
#if !defined(CUDAPP_MIGRATION_PENDING_SPIRV_HOST) || CUDAPP_MIGRATION_PENDING_SPIRV_HOST != 1
#error "spirv-host is migration-pending C: build with -DCUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 (default OFF)"
#endif
#ifndef MT_TARGET
#define MT_TARGET 0
#endif
#if MT_TARGET != 0 && MT_TARGET != 1 && MT_TARGET != 2 && MT_TARGET != 3
#error "MT_TARGET must be 0 (CPU: PoCL/lavapipe), 1 (Apple GPU: MoltenVK), 2 (NVIDIA GPU: NVIDIA Vulkan driver) or 3 (GPU: vendor OpenCL driver)"
#endif
#define _GNU_SOURCE
#if MT_TARGET == 0 || MT_TARGET == 3
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#endif
#if MT_TARGET == 1 || MT_TARGET == 3
#include <limits.h>
#endif
#if MT_TARGET == 1
#include <mach-o/dyld.h>
#else
#include <dirent.h>
#endif
#if MT_TARGET != 3
#include <vulkan/vulkan.h>
#endif

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#ifndef MT_POCL_LIBRARY
#define MT_POCL_LIBRARY ""
#endif
#ifndef MT_LVP_ICD
#define MT_LVP_ICD "/usr/share/vulkan/icd.d/lvp_icd.json"
#endif
#if MT_TARGET == 1
#ifndef MT_MVK_LIBRARY
#define MT_MVK_LIBRARY ""
#endif
#ifndef MT_MVK_FAST_MATH
#define MT_MVK_FAST_MATH "0"
#endif
#ifndef MT_EXPECT_DEVICE
#define MT_EXPECT_DEVICE "Apple M2"
#endif
#ifndef MT_FORBID
#define MT_FORBID "libvulkan"
#endif
#elif MT_TARGET == 2
#ifndef MT_NV_ICD
#define MT_NV_ICD "/usr/share/vulkan/icd.d/nvidia_icd.json"
#endif
#ifndef MT_NV_DISK_CACHE
#define MT_NV_DISK_CACHE "0"
#endif
#ifndef MT_NV_CACHE_PATH
#define MT_NV_CACHE_PATH ""
#endif
#ifndef MT_EXPECT_DEVICE
#define MT_EXPECT_DEVICE "NVIDIA GB10"
#endif
#ifndef MT_REQUIRE
#define MT_REQUIRE "libnvidia-glcore"
#endif
#ifndef MT_FORBID
#define MT_FORBID "vulkan_lvp:libpocl:libcuda.so:libnvcuda"
#endif
#elif MT_TARGET == 3
#ifndef MT_CL_GPU_ICD
#define MT_CL_GPU_ICD ""
#endif
#ifndef MT_CL_GPU_VENDOR
#define MT_CL_GPU_VENDOR 0x8086u
#endif
#ifndef MT_CL_GPU_REQUIRE
#define MT_CL_GPU_REQUIRE ""
#endif
#ifndef MT_EXPECT_DEVICE
#define MT_EXPECT_DEVICE ""
#endif
#ifndef MT_FORBID
#define MT_FORBID "libpocl:nvidia:libcuda:libnvcuda"
#endif
#else
#ifndef MT_FORBID
#define MT_FORBID "nvidia"
#endif
#endif
#ifndef MT_REQUIRE
#define MT_REQUIRE ""
#endif
#define MAX_BUFS 3
#define POISON 0xFFC0FFEEu
#define LOCAL 64u

static int die(int code, const char *what) { fprintf(stderr, "spirv-host: %s\n", what); return code; }

static void *slurp(const char *path, size_t *size)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n <= 0 || fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    void *p = malloc((size_t)n);
    if (p && fread(p, 1, (size_t)n, f) != (size_t)n) { free(p); p = NULL; }
    fclose(f);
    *size = (size_t)n;
    return p;
}

#if MT_TARGET == 1
/* Fails unless the pinned MoltenVK image is loaded, or when any loaded image names MT_FORBID.
 * Paths are compared after realpath(): the MoltenVK release reaches dylib/ through a symlink. */
static int isolation(void)
{
    char want[PATH_MAX], have[PATH_MAX];
    if (!realpath(MT_MVK_LIBRARY, want)) return die(9, "isolation: cannot resolve MT_MVK_LIBRARY");
    uint32_t n = _dyld_image_count();
    int pinned = 0;
    for (uint32_t i = 0; i < n; i++) {
        const char *name = _dyld_get_image_name(i);
        if (!name) continue;
        if (!realpath(name, have)) have[0] = 0;
        if (have[0] && strcmp(have, want) == 0) pinned = 1;
        if (strstr(name, MT_FORBID) || strstr(have, MT_FORBID)) {
            fprintf(stderr, "spirv-host: isolation: loaded image %s\n", name);
            return die(9, "isolation FAILED: forbidden path \"" MT_FORBID "\" present");
        }
    }
    if (!pinned) return die(9, "isolation FAILED: pinned MoltenVK image " MT_MVK_LIBRARY " not loaded");
    fprintf(stderr, "spirv-host: isolation: %u images, pinned %s loaded, none contains \"%s\"\n", n, want, MT_FORBID);
    return 0;
}
#else
/* The mapping that must be present for isolation_check(1): MT_REQUIRE, or for MT_TARGET=3 the vendor ICD. */
static const char *g_require = MT_REQUIRE;

/* Name of the first MT_FORBID entry (colon-separated) contained in s goes to which; returns 1 if any. */
static int forbidden(const char *s, char *which, size_t cap)
{
    const char *p = MT_FORBID;
    while (*p) {
        size_t k = strcspn(p, ":");
        if (k > 0 && k < cap) {
            memcpy(which, p, k);
            which[k] = 0;
            if (strstr(s, which)) return 1;
        }
        p += k;
        if (*p == ':') p++;
    }
    return 0;
}

/* Fails when any mapping or open descriptor of this process names an MT_FORBID entry, or (when
 * MT_REQUIRE is set and need_required) when no mapping names MT_REQUIRE: the pinned driver must actually
 * be loaded. Target 2 checks the requirement inside run_vk while the driver's objects exist. */
static int isolation_check(int need_required)
{
    char line[4096], which[256];
    FILE *m = fopen("/proc/self/maps", "r");
    if (!m) return die(9, "isolation: cannot read /proc/self/maps");
    int bad = 0, required = !need_required || g_require[0] == 0;
    while (fgets(line, sizeof line, m)) {
        if (g_require[0] && strstr(line, g_require)) required = 1;
        if (forbidden(line, which, sizeof which)) { bad = 1; fprintf(stderr, "spirv-host: isolation: mapped %s", line); break; }
    }
    fclose(m);
    DIR *d = opendir("/proc/self/fd");
    if (!d) return die(9, "isolation: cannot read /proc/self/fd");
    struct dirent *e;
    char link[300], target[4096];
    while (!bad && (e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.') continue;
        snprintf(link, sizeof link, "/proc/self/fd/%s", e->d_name);
        ssize_t k = readlink(link, target, sizeof target - 1);
        if (k <= 0) continue;
        target[k] = 0;
        if (forbidden(target, which, sizeof which)) { bad = 1; fprintf(stderr, "spirv-host: isolation: open fd %s\n", target); }
    }
    closedir(d);
    if (bad) {
        fprintf(stderr, "spirv-host: isolation FAILED: forbidden path \"%s\" present\n", which);
        return 9;
    }
    if (!required) {
        fprintf(stderr, "spirv-host: isolation FAILED: required driver \"%s\" is not mapped\n", g_require);
        return 9;
    }
    fprintf(stderr, "spirv-host: isolation: no mapping or descriptor contains \"%s\"%s%s%s\n", MT_FORBID,
            need_required && g_require[0] ? "; \"" : "", need_required ? g_require : "",
            need_required && g_require[0] ? "\" mapped" : "");
    return 0;
}
static int isolation(void) { return isolation_check(MT_TARGET != 2 && MT_TARGET != 3); }
#endif

static double now_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e6 + (double)ts.tv_nsec / 1e3;
}
static double t_start;

typedef struct { const char *api, *module, *kernel; uint32_t count; int nin; const char *in[2], *out;
                 void *code; size_t code_size; float *buf[MAX_BUFS]; size_t bytes[MAX_BUFS]; } Job;

#if MT_TARGET != 0
extern char **environ;

/* Removes every inherited environment variable whose name starts with prefix. */
static int clear_prefix(const char *prefix)
{
    size_t pl = strlen(prefix);
    for (int again = 1; again;) {
        again = 0;
        for (char **e = environ; *e; e++) {
            if (strncmp(*e, prefix, pl) != 0) continue;
            char name[256];
            size_t k = strcspn(*e, "=");
            if (k >= sizeof name) return die(3, "inherited variable name too long");
            memcpy(name, *e, k);
            name[k] = 0;
            if (unsetenv(name)) return die(3, "cannot clear an inherited driver variable");
            again = 1;
            break;
        }
    }
    return 0;
}
#endif

#if MT_TARGET == 3
static char g_icd[PATH_MAX];

/* The run-time opt-in: MT_CL_GPU must be exactly "1". Checked first in main, before any file or loader. */
static int cl_gpu_opted_in(void)
{
    const char *flag = getenv("MT_CL_GPU");
    return flag && strcmp(flag, "1") == 0;
}

/* Pins the OpenCL ICD loader to the one vendor library and removes inherited loader and driver controls,
 * before the loader runs. A library path naming an MT_FORBID entry is refused without being loaded. */
static int pin_cl_gpu(void)
{
    char which[256];
    if (MT_CL_GPU_ICD[0] != '/') return die(3, "build needs an absolute MT_CL_GPU_ICD (the vendor OpenCL ICD library)");
    if (forbidden(MT_CL_GPU_ICD, which, sizeof which)) {
        fprintf(stderr, "spirv-host: isolation FAILED: MT_CL_GPU_ICD %s contains forbidden \"%s\" (not loaded)\n", MT_CL_GPU_ICD, which);
        return 9;
    }
    if (!realpath(MT_CL_GPU_ICD, g_icd)) return die(3, "cannot resolve MT_CL_GPU_ICD (not loaded)");
    if (forbidden(g_icd, which, sizeof which)) {
        fprintf(stderr, "spirv-host: isolation FAILED: MT_CL_GPU_ICD resolves to %s, which contains forbidden \"%s\" (not loaded)\n",
                g_icd, which);
        return 9;
    }
    if (clear_prefix("OCL_ICD_") || clear_prefix("OPENCL_") || clear_prefix("POCL_") || clear_prefix("IGC_") ||
        clear_prefix("ZE_") || clear_prefix("NEOReadDebugKeys"))
        return 3;
    if (setenv("OCL_ICD_VENDORS", g_icd, 1) || setenv("OPENCL_LAYER_PATH", "/dev/null", 1))
        return die(3, "cannot pin OpenCL loader environment");
    g_require = MT_CL_GPU_REQUIRE[0] ? MT_CL_GPU_REQUIRE : g_icd;
    fprintf(stderr, "spirv-host: cl target=gpu icd=%s realpath=%s MT_CL_GPU=1 forbid=\"%s\" require=\"%s\"\n",
            MT_CL_GPU_ICD, g_icd, MT_FORBID, g_require);
    return 0;
}

/* The one CL_DEVICE_TYPE_GPU device of the pinned platform (MT_CL_GPU_INDEX picks one of several). */
static int pick_gpu(cl_platform_id plat, cl_device_id *dev)
{
    cl_uint nd = 0;
    cl_int err = clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, 0, NULL, &nd);
    if (err == CL_DEVICE_NOT_FOUND || (err == CL_SUCCESS && nd == 0))
        return die(5, "no CL_DEVICE_TYPE_GPU device on the pinned platform (no CPU fallback)");
    if (err != CL_SUCCESS) return die(5, "clGetDeviceIDs(CL_DEVICE_TYPE_GPU)");
    cl_device_id devs[16];
    if (nd > 16) nd = 16;
    if (clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, nd, devs, NULL) != CL_SUCCESS) return die(5, "clGetDeviceIDs(CL_DEVICE_TYPE_GPU)");
    unsigned long idx = 0;
    const char *sel = getenv("MT_CL_GPU_INDEX");
    if (sel && sel[0]) {
        char *end;
        idx = strtoul(sel, &end, 10);
        if (*end || sel[0] < '0' || sel[0] > '9' || idx >= nd) return die(5, "MT_CL_GPU_INDEX is not a GPU index on the pinned platform");
    } else if (nd != 1) {
        fprintf(stderr, "spirv-host: cl %u GPU devices on the platform\n", nd);
        return die(5, "more than one GPU device: set MT_CL_GPU_INDEX");
    }
    *dev = devs[idx];
    fprintf(stderr, "spirv-host: cl gpu_devices=%u index=%lu\n", nd, idx);
    return 0;
}

static void devstr(cl_device_id dev, cl_device_info what, char *buf, size_t cap)
{
    if (clGetDeviceInfo(dev, what, cap - 1, buf, NULL) != CL_SUCCESS) snprintf(buf, cap, "unknown");
    buf[cap - 1] = 0;
}

/* Observation only: open descriptors on /dev/dri/ (the DRM render node a GPU driver holds). */
static int dri_fds(void)
{
    DIR *d = opendir("/proc/self/fd");
    if (!d) return -1;
    int n = 0;
    struct dirent *e;
    char link[300], target[4096];
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.') continue;
        snprintf(link, sizeof link, "/proc/self/fd/%s", e->d_name);
        ssize_t k = readlink(link, target, sizeof target - 1);
        if (k <= 0) continue;
        target[k] = 0;
        if (strncmp(target, "/dev/dri/", 9) == 0) n++;
    }
    closedir(d);
    return n;
}
#endif

#if MT_TARGET == 0 || MT_TARGET == 3
static int run_cl(Job *j)
{
#if MT_TARGET == 0
    const char *lib = MT_POCL_LIBRARY;
    if (lib[0] != '/') return die(3, "build needs an absolute MT_POCL_LIBRARY");
    if (setenv("OCL_ICD_VENDORS", lib, 1) || unsetenv("OCL_ICD_FILENAMES") || unsetenv("OPENCL_LAYERS") ||
        setenv("OPENCL_LAYER_PATH", "/dev/null", 1) || setenv("POCL_DEVICES", "cpu", 1))
        return die(3, "cannot pin OpenCL loader environment");
    cl_platform_id plat; cl_uint np = 0; cl_int err;
    if (clGetPlatformIDs(1, &plat, &np) != CL_SUCCESS || np != 1) return die(5, "expected exactly one OpenCL platform (pinned PoCL)");
    char text[512];
    if (clGetPlatformInfo(plat, CL_PLATFORM_NAME, sizeof text, text, NULL) != CL_SUCCESS ||
        strcmp(text, "Portable Computing Language") != 0) return die(5, "platform is not PoCL");
    clGetPlatformInfo(plat, CL_PLATFORM_VERSION, sizeof text, text, NULL);
    fprintf(stderr, "spirv-host: cl library=%s platform=\"%s\"\n", lib, text);
    cl_device_id dev;
    if (clGetDeviceIDs(plat, CL_DEVICE_TYPE_CPU, 1, &dev, NULL) != CL_SUCCESS) return die(5, "no CPU device");
    cl_device_type ty = 0;
    clGetDeviceInfo(dev, CL_DEVICE_TYPE, sizeof ty, &ty, NULL);
    if (ty != CL_DEVICE_TYPE_CPU) return die(5, "device is not CL_DEVICE_TYPE_CPU");
    clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof text, text, NULL);
    fprintf(stderr, "spirv-host: cl device=\"%s\" type=CPU\n", text);
    cl_context ctx = clCreateContext(NULL, 1, &dev, NULL, NULL, &err);
    if (err != CL_SUCCESS) return die(5, "clCreateContext");
    cl_command_queue q = clCreateCommandQueueWithProperties(ctx, dev, NULL, &err);
    if (err != CL_SUCCESS) return die(5, "clCreateCommandQueue");
#else
    const double t_setup0 = now_us();
    int prc = pin_cl_gpu();
    if (prc) return prc;
    cl_platform_id plat; cl_uint np = 0; cl_int err;
    if (clGetPlatformIDs(1, &plat, &np) != CL_SUCCESS || np != 1) return die(5, "expected exactly one OpenCL platform (pinned vendor ICD)");
    char pname[256] = "unknown", pvendor[256] = "unknown", pver[256] = "unknown";
    clGetPlatformInfo(plat, CL_PLATFORM_NAME, sizeof pname - 1, pname, NULL);
    clGetPlatformInfo(plat, CL_PLATFORM_VENDOR, sizeof pvendor - 1, pvendor, NULL);
    clGetPlatformInfo(plat, CL_PLATFORM_VERSION, sizeof pver - 1, pver, NULL);
    fprintf(stderr, "spirv-host: cl library=%s platform=\"%s\" platform_vendor=\"%s\" platform_version=\"%s\"\n",
            g_icd, pname, pvendor, pver);
    cl_device_id dev;
    int drc = pick_gpu(plat, &dev);
    if (drc) return drc;
    cl_device_type ty = 0;
    clGetDeviceInfo(dev, CL_DEVICE_TYPE, sizeof ty, &ty, NULL);
    if (!(ty & CL_DEVICE_TYPE_GPU) || (ty & CL_DEVICE_TYPE_CPU)) return die(5, "device is not CL_DEVICE_TYPE_GPU");
    cl_uint vid = 0, cu = 0, mhz = 0;
    cl_ulong gmem = 0;
    clGetDeviceInfo(dev, CL_DEVICE_VENDOR_ID, sizeof vid, &vid, NULL);
    clGetDeviceInfo(dev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof cu, &cu, NULL);
    clGetDeviceInfo(dev, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof mhz, &mhz, NULL);
    clGetDeviceInfo(dev, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof gmem, &gmem, NULL);
    char dname[256], dvendor[256], drv[256], dver[256], il[256];
    devstr(dev, CL_DEVICE_NAME, dname, sizeof dname);
    devstr(dev, CL_DEVICE_VENDOR, dvendor, sizeof dvendor);
    devstr(dev, CL_DRIVER_VERSION, drv, sizeof drv);
    devstr(dev, CL_DEVICE_VERSION, dver, sizeof dver);
    devstr(dev, CL_DEVICE_IL_VERSION, il, sizeof il);
    fprintf(stderr, "spirv-host: cl device=\"%s\" type=GPU vendor=\"%s\" vendor_id=0x%x driver_version=\"%s\" device_version=\"%s\""
            " il=\"%s\" compute_units=%u max_clock_mhz=%u global_mem_bytes=%llu\n", dname, dvendor, vid, drv, dver, il, cu, mhz,
            (unsigned long long)gmem);
    if (vid != (cl_uint)MT_CL_GPU_VENDOR) return die(5, "device vendor id differs from MT_CL_GPU_VENDOR");
    if (!strstr(dname, MT_EXPECT_DEVICE)) return die(5, "device name does not contain MT_EXPECT_DEVICE (" MT_EXPECT_DEVICE ")");
    cl_context ctx = clCreateContext(NULL, 1, &dev, NULL, NULL, &err);
    if (err != CL_SUCCESS) return die(5, "clCreateContext");
    const cl_queue_properties qprops[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
    cl_command_queue q = clCreateCommandQueueWithProperties(ctx, dev, qprops, &err);
    if (err != CL_SUCCESS) return die(5, "clCreateCommandQueue");
    const double t_build0 = now_us();
#endif
    cl_program prog = clCreateProgramWithIL(ctx, j->code, j->code_size, &err);
    if (err != CL_SUCCESS) return die(6, "clCreateProgramWithIL refused the module");
    if (clBuildProgram(prog, 1, &dev, "", NULL, NULL) != CL_SUCCESS) {
        char log[4096] = { 0 };
        clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_LOG, sizeof log - 1, log, NULL);
        fprintf(stderr, "%s\n", log);
        return die(6, "clBuildProgram failed");
    }
#if MT_TARGET == 3
    const double t_build1 = now_us();
#endif
    cl_kernel k = clCreateKernel(prog, j->kernel, &err);
    if (err != CL_SUCCESS) return die(7, "clCreateKernel: entry point not found");
    cl_mem mem[MAX_BUFS];
    int nb = j->nin + 1;
    for (int i = 0; i < nb; i++) {
        mem[i] = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, j->bytes[i], j->buf[i], &err);
        if (err != CL_SUCCESS || clSetKernelArg(k, (cl_uint)i, sizeof mem[i], &mem[i]) != CL_SUCCESS)
            return die(7, "buffer or argument");
    }
    cl_uint n = j->count;
    if (clSetKernelArg(k, (cl_uint)nb, sizeof n, &n) != CL_SUCCESS) return die(7, "argument n");
    size_t global = ((size_t)j->count + LOCAL - 1) / LOCAL * LOCAL, local = LOCAL;
#if MT_TARGET == 0
    if (clEnqueueNDRangeKernel(q, k, 1, NULL, &global, &local, 0, NULL, NULL) != CL_SUCCESS ||
#else
    cl_event ev;
    if (clEnqueueNDRangeKernel(q, k, 1, NULL, &global, &local, 0, NULL, &ev) != CL_SUCCESS ||
#endif
        clEnqueueReadBuffer(q, mem[nb - 1], CL_TRUE, 0, j->bytes[nb - 1], j->buf[nb - 1], 0, NULL, NULL) != CL_SUCCESS ||
        clFinish(q) != CL_SUCCESS)
        return die(8, "enqueue, read back or finish failed");
    fprintf(stderr, "spirv-host: cl dispatched global=%zu local=%zu\n", global, local);
#if MT_TARGET == 3
    const double t_run1 = now_us();
    char devns[32] = "NOT_RUN";
    cl_ulong ts0 = 0, ts1 = 0;
    if (clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof ts0, &ts0, NULL) == CL_SUCCESS &&
        clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof ts1, &ts1, NULL) == CL_SUCCESS && ts1 >= ts0)
        snprintf(devns, sizeof devns, "%llu", (unsigned long long)(ts1 - ts0));
    clReleaseEvent(ev);
    size_t total_bytes = 0;
    for (int i = 0; i < nb; i++) total_bytes += j->bytes[i];
    struct rusage ru;
    long maxrss_kib = getrusage(RUSAGE_SELF, &ru) == 0 ? ru.ru_maxrss : -1;
    fprintf(stderr, "spirv-host: time setup_us=%.0f build_us=%.0f run_us=%.0f device_ns=%s total_us=%.0f maxrss_kib=%ld buffers_bytes=%zu\n",
            t_build0 - t_setup0, t_build1 - t_build0, t_run1 - t_build1, devns, now_us() - t_start, maxrss_kib, total_bytes);
    fprintf(stderr, "spirv-host: cl dri_fds=%d (observation)\n", dri_fds());
    /* The vendor driver must be mapped while its objects exist; the loader may unload it at teardown. */
    int irc = isolation_check(1);
    if (irc) return irc;
#endif
    for (int i = 0; i < nb; i++) clReleaseMemObject(mem[i]);
    clReleaseKernel(k); clReleaseProgram(prog); clReleaseCommandQueue(q); clReleaseContext(ctx);
    return 0;
}
#endif

#if MT_TARGET != 3
#define VKOK(x, code, what) do { if ((x) != VK_SUCCESS) return die(code, what); } while (0)

static int memtype(VkPhysicalDevice pd, uint32_t bits)
{
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(pd, &mp);
    const VkMemoryPropertyFlags want = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
        if ((bits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & want) == want) return (int)i;
    return -1;
}

#if MT_TARGET == 1
/* Removes every inherited MVK_* variable, then pins the fast-math mode before MoltenVK initializes. */
static int pin_moltenvk(void)
{
    if (MT_MVK_LIBRARY[0] != '/') return die(3, "build needs an absolute MT_MVK_LIBRARY");
    if (clear_prefix("MVK_")) return 3;
    if (setenv("MVK_CONFIG_FAST_MATH_ENABLED", MT_MVK_FAST_MATH, 1)) return die(3, "cannot pin MoltenVK fast math");
    fprintf(stderr, "spirv-host: vk target=moltenvk library=%s linked=direct-no-loader MVK_CONFIG_FAST_MATH_ENABLED=%s\n",
            MT_MVK_LIBRARY, MT_MVK_FAST_MATH);
    return 0;
}
#endif

#if MT_TARGET == 2
/* Pins the Khronos loader to NVIDIA's manifest, removes inherited __GL_* / __NV_* driver controls and
 * pins the on-disk shader cache before the driver loads. */
static int pin_nvidia(void)
{
    if (MT_NV_ICD[0] != '/') return die(3, "build needs an absolute MT_NV_ICD");
    if (clear_prefix("__GL_") || clear_prefix("__NV_") || clear_prefix("VK_")) return 3;
    if (setenv("VK_DRIVER_FILES", MT_NV_ICD, 1) || setenv("VK_ICD_FILENAMES", MT_NV_ICD, 1) ||
        setenv("VK_LOADER_LAYERS_DISABLE", "~all~", 1) || setenv("__GL_SHADER_DISK_CACHE", MT_NV_DISK_CACHE, 1))
        return die(3, "cannot pin Vulkan loader / NVIDIA driver environment");
    if (MT_NV_CACHE_PATH[0]) {
        if (MT_NV_CACHE_PATH[0] != '/') return die(3, "MT_NV_CACHE_PATH must be absolute");
        if (setenv("__GL_SHADER_DISK_CACHE_PATH", MT_NV_CACHE_PATH, 1) ||
            setenv("__GL_SHADER_DISK_CACHE_SKIP_CLEANUP", "1", 1))
            return die(3, "cannot pin the NVIDIA shader cache path");
    } else if (strcmp(MT_NV_DISK_CACHE, "0") != 0) {
        return die(3, "an enabled shader cache needs a private absolute MT_NV_CACHE_PATH");
    }
    fprintf(stderr, "spirv-host: vk target=nvidia icd=%s layers=disabled __GL_SHADER_DISK_CACHE=%s cache_path=%s\n",
            MT_NV_ICD, MT_NV_DISK_CACHE, MT_NV_CACHE_PATH[0] ? MT_NV_CACHE_PATH : "(driver default, unused)");
    return 0;
}
#endif

static int run_vk(Job *j)
{
    double t_setup0 = now_us();
#if MT_TARGET == 1
    int prc = pin_moltenvk();
    if (prc) return prc;
    const uint32_t api = VK_API_VERSION_1_2;
#elif MT_TARGET == 2
    int prc = pin_nvidia();
    if (prc) return prc;
    const uint32_t api = VK_API_VERSION_1_2;
#else
    if (setenv("VK_DRIVER_FILES", MT_LVP_ICD, 1) || setenv("VK_ICD_FILENAMES", MT_LVP_ICD, 1) ||
        unsetenv("VK_ADD_DRIVER_FILES") || unsetenv("VK_INSTANCE_LAYERS") || unsetenv("VK_LAYER_PATH") ||
        unsetenv("VK_ADD_LAYER_PATH") || setenv("VK_LOADER_LAYERS_DISABLE", "~all~", 1))
        return die(3, "cannot pin Vulkan loader environment");
    const uint32_t api = VK_API_VERSION_1_1;
#endif
    VkApplicationInfo app = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pApplicationName = "cudapp-spirv-host",
                              .apiVersion = api };
    VkInstanceCreateInfo ici = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &app };
    VkInstance inst;
    VKOK(vkCreateInstance(&ici, NULL, &inst), 5, "vkCreateInstance");
    uint32_t npd = 0;
    VKOK(vkEnumeratePhysicalDevices(inst, &npd, NULL), 5, "vkEnumeratePhysicalDevices");
    if (npd != 1) return die(5, "expected exactly one Vulkan device (pinned driver)");
    VkPhysicalDevice pd;
    VKOK(vkEnumeratePhysicalDevices(inst, &npd, &pd), 5, "vkEnumeratePhysicalDevices");
#if MT_TARGET != 0
    VkPhysicalDeviceDriverProperties drv = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES };
    VkPhysicalDeviceProperties2 p2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &drv };
    vkGetPhysicalDeviceProperties2(pd, &p2);
    const VkPhysicalDeviceProperties pp = p2.properties;
    fprintf(stderr, "spirv-host: vk device=\"%s\" type=%d vendor=0x%x device_id=0x%x api=%u.%u.%u driver=\"%s\" info=\"%s\""
            " conformance=%u.%u.%u.%u\n", pp.deviceName, (int)pp.deviceType, pp.vendorID, pp.deviceID,
            VK_API_VERSION_MAJOR(pp.apiVersion), VK_API_VERSION_MINOR(pp.apiVersion), VK_API_VERSION_PATCH(pp.apiVersion),
            drv.driverName, drv.driverInfo, drv.conformanceVersion.major, drv.conformanceVersion.minor,
            drv.conformanceVersion.subminor, drv.conformanceVersion.patch);
#if MT_TARGET == 1
    if (pp.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        return die(5, "device is not VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU");
    if (pp.vendorID != 0x106b) return die(5, "device vendor is not Apple (0x106b)");
    if (drv.driverID != VK_DRIVER_ID_MOLTENVK) return die(5, "driver is not MoltenVK");
#else
    if (pp.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && pp.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        return die(5, "device is not a GPU (integrated or discrete): no CPU fallback");
    if (pp.vendorID != 0x10de) return die(5, "device vendor is not NVIDIA (0x10de)");
    if (drv.driverID != VK_DRIVER_ID_NVIDIA_PROPRIETARY) return die(5, "driver is not NVIDIA_PROPRIETARY");
#endif
    if (!strstr(pp.deviceName, MT_EXPECT_DEVICE))
        return die(5, "device name does not contain MT_EXPECT_DEVICE (" MT_EXPECT_DEVICE ")");
    fprintf(stderr, "spirv-host: vk limits max_group_count_x=%u max_invocations=%u max_shared=%u timestamp_period_ns=%g\n",
            pp.limits.maxComputeWorkGroupCount[0], pp.limits.maxComputeWorkGroupInvocations,
            pp.limits.maxComputeSharedMemorySize, (double)pp.limits.timestampPeriod);
#else
    VkPhysicalDeviceProperties pp;
    vkGetPhysicalDeviceProperties(pd, &pp);
    if (pp.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU) return die(5, "device is not VK_PHYSICAL_DEVICE_TYPE_CPU");
    fprintf(stderr, "spirv-host: vk icd=%s device=\"%s\" type=CPU api=%u.%u.%u driver=0x%x\n", MT_LVP_ICD, pp.deviceName,
            VK_API_VERSION_MAJOR(pp.apiVersion), VK_API_VERSION_MINOR(pp.apiVersion), VK_API_VERSION_PATCH(pp.apiVersion),
            pp.driverVersion);
#endif
    uint32_t nq = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &nq, NULL);
    VkQueueFamilyProperties qf[16];
    if (nq > 16) nq = 16;
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &nq, qf);
    uint32_t fam = UINT32_MAX;
    for (uint32_t i = 0; i < nq; i++) if (qf[i].queueFlags & VK_QUEUE_COMPUTE_BIT) { fam = i; break; }
    if (fam == UINT32_MAX) return die(5, "no compute queue");
    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = fam,
                                    .queueCount = 1, .pQueuePriorities = &prio };
    VkDeviceCreateInfo dci = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .queueCreateInfoCount = 1, .pQueueCreateInfos = &qci };
#if MT_TARGET == 1
    /* A portability device (MoltenVK) must have VK_KHR_portability_subset enabled when it offers it. */
    uint32_t nx = 0;
    VKOK(vkEnumerateDeviceExtensionProperties(pd, NULL, &nx, NULL), 5, "vkEnumerateDeviceExtensionProperties");
    VkExtensionProperties *xp = calloc(nx ? nx : 1, sizeof *xp);
    if (!xp) return die(5, "out of memory");
    VKOK(vkEnumerateDeviceExtensionProperties(pd, NULL, &nx, xp), 5, "vkEnumerateDeviceExtensionProperties");
    static const char *portability = "VK_KHR_portability_subset";
    for (uint32_t i = 0; i < nx; i++)
        if (strcmp(xp[i].extensionName, portability) == 0) {
            dci.enabledExtensionCount = 1;
            dci.ppEnabledExtensionNames = &portability;
        }
    free(xp);
    fprintf(stderr, "spirv-host: vk portability_subset=%s\n", dci.enabledExtensionCount ? "enabled" : "absent");
#endif
    VkDevice dev;
    VKOK(vkCreateDevice(pd, &dci, NULL, &dev), 5, "vkCreateDevice");
    VkQueue q;
    vkGetDeviceQueue(dev, fam, 0, &q);
    int nb = j->nin + 1;
    VkBuffer b[MAX_BUFS]; VkDeviceMemory m[MAX_BUFS]; void *map[MAX_BUFS];
    for (int i = 0; i < nb; i++) {
        VkBufferCreateInfo bci = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = j->bytes[i],
                                   .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, .sharingMode = VK_SHARING_MODE_EXCLUSIVE };
        VKOK(vkCreateBuffer(dev, &bci, NULL, &b[i]), 5, "vkCreateBuffer");
        VkMemoryRequirements mr;
        vkGetBufferMemoryRequirements(dev, b[i], &mr);
        int t = memtype(pd, mr.memoryTypeBits);
        if (t < 0) return die(5, "no host-visible coherent memory type");
        VkMemoryAllocateInfo mai = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = mr.size, .memoryTypeIndex = (uint32_t)t };
        VKOK(vkAllocateMemory(dev, &mai, NULL, &m[i]), 5, "vkAllocateMemory");
        VKOK(vkBindBufferMemory(dev, b[i], m[i], 0), 5, "vkBindBufferMemory");
        VKOK(vkMapMemory(dev, m[i], 0, j->bytes[i], 0, &map[i]), 5, "vkMapMemory");
        memcpy(map[i], j->buf[i], j->bytes[i]);
    }
    double t_pipe0 = now_us();
    VkShaderModuleCreateInfo smci = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = j->code_size, .pCode = j->code };
    VkShaderModule sm;
    VKOK(vkCreateShaderModule(dev, &smci, NULL, &sm), 6, "vkCreateShaderModule refused the module");
    VkDescriptorSetLayoutBinding lb[MAX_BUFS];
    for (int i = 0; i < nb; i++)
        lb[i] = (VkDescriptorSetLayoutBinding){ .binding = (uint32_t)i, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT };
    VkDescriptorSetLayoutCreateInfo dlci = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = (uint32_t)nb, .pBindings = lb };
    VkDescriptorSetLayout dsl;
    VKOK(vkCreateDescriptorSetLayout(dev, &dlci, NULL, &dsl), 6, "vkCreateDescriptorSetLayout");
    VkPipelineLayoutCreateInfo plci = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1, .pSetLayouts = &dsl };
    VkPipelineLayout pl;
    VKOK(vkCreatePipelineLayout(dev, &plci, NULL, &pl), 6, "vkCreatePipelineLayout");
    VkComputePipelineCreateInfo cpci = { .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, .layout = pl,
        .stage = { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                   .module = sm, .pName = j->kernel } };
    VkPipeline pipe;
    VKOK(vkCreateComputePipelines(dev, VK_NULL_HANDLE, 1, &cpci, NULL, &pipe), 7, "vkCreateComputePipelines (entry point?)");
    double t_pipe1 = now_us();
    /* Device-side span of the dispatch (observation only); skipped when the queue has no timestamps. */
    VkQueryPool qp = VK_NULL_HANDLE;
    const uint32_t tsbits = qf[fam].timestampValidBits;
    if (tsbits) {
        VkQueryPoolCreateInfo qpci = { .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, .queryType = VK_QUERY_TYPE_TIMESTAMP, .queryCount = 2 };
        VKOK(vkCreateQueryPool(dev, &qpci, NULL, &qp), 8, "vkCreateQueryPool");
    }
    VkDescriptorPoolSize ps = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, (uint32_t)nb };
    VkDescriptorPoolCreateInfo dpci = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .maxSets = 1, .poolSizeCount = 1, .pPoolSizes = &ps };
    VkDescriptorPool dp;
    VKOK(vkCreateDescriptorPool(dev, &dpci, NULL, &dp), 7, "vkCreateDescriptorPool");
    VkDescriptorSetAllocateInfo dsai = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .descriptorPool = dp, .descriptorSetCount = 1, .pSetLayouts = &dsl };
    VkDescriptorSet ds;
    VKOK(vkAllocateDescriptorSets(dev, &dsai, &ds), 7, "vkAllocateDescriptorSets");
    VkDescriptorBufferInfo dbi[MAX_BUFS];
    VkWriteDescriptorSet wds[MAX_BUFS];
    for (int i = 0; i < nb; i++) {
        dbi[i] = (VkDescriptorBufferInfo){ b[i], 0, j->bytes[i] };
        wds[i] = (VkWriteDescriptorSet){ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = ds, .dstBinding = (uint32_t)i,
                                         .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .pBufferInfo = &dbi[i] };
    }
    vkUpdateDescriptorSets(dev, (uint32_t)nb, wds, 0, NULL);
    VkCommandPoolCreateInfo cpi = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .queueFamilyIndex = fam };
    VkCommandPool cp;
    VKOK(vkCreateCommandPool(dev, &cpi, NULL, &cp), 8, "vkCreateCommandPool");
    VkCommandBufferAllocateInfo cbai = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = cp,
                                         .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1 };
    VkCommandBuffer cb;
    VKOK(vkAllocateCommandBuffers(dev, &cbai, &cb), 8, "vkAllocateCommandBuffers");
    VkCommandBufferBeginInfo cbbi = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
    VKOK(vkBeginCommandBuffer(cb, &cbbi), 8, "vkBeginCommandBuffer");
    if (qp) vkCmdResetQueryPool(cb, qp, 0, 2);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_COMPUTE, pl, 0, 1, &ds, 0, NULL);
    uint32_t groups = (j->count + LOCAL - 1) / LOCAL;
    if (qp) vkCmdWriteTimestamp(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, qp, 0);
    vkCmdDispatch(cb, groups, 1, 1);
    if (qp) vkCmdWriteTimestamp(cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, qp, 1);
    VkMemoryBarrier mb = { .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER, .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT, .dstAccessMask = VK_ACCESS_HOST_READ_BIT };
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &mb, 0, NULL, 0, NULL);
    VKOK(vkEndCommandBuffer(cb), 8, "vkEndCommandBuffer");
    VkSubmitInfo si = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cb };
    VKOK(vkQueueSubmit(q, 1, &si, VK_NULL_HANDLE), 8, "vkQueueSubmit");
    VKOK(vkQueueWaitIdle(q), 8, "vkQueueWaitIdle");
    double t_run1 = now_us();
    fprintf(stderr, "spirv-host: vk dispatched groups=%u local=%u\n", groups, LOCAL);
    memcpy(j->buf[nb - 1], map[nb - 1], j->bytes[nb - 1]);
    char devns[32] = "NOT_RUN";
    if (qp) {
        uint64_t ts[2] = { 0, 0 };
        VKOK(vkGetQueryPoolResults(dev, qp, 0, 2, sizeof ts, ts, sizeof ts[0], VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT),
             8, "vkGetQueryPoolResults");
        const uint64_t mask = tsbits >= 64 ? UINT64_MAX : ((1ull << tsbits) - 1);
        const uint64_t ticks = ((ts[1] & mask) - (ts[0] & mask)) & mask;
        snprintf(devns, sizeof devns, "%.0f", (double)ticks * (double)pp.limits.timestampPeriod);
        vkDestroyQueryPool(dev, qp, NULL);
    }
    size_t total_bytes = 0;
    for (int i = 0; i < nb; i++) total_bytes += j->bytes[i];
    struct rusage ru;
    long maxrss_kib = getrusage(RUSAGE_SELF, &ru) == 0 ? ru.ru_maxrss : -1;
#if MT_TARGET == 1
    if (maxrss_kib > 0) maxrss_kib /= 1024; /* macOS reports bytes, Linux KiB */
#endif
    fprintf(stderr, "spirv-host: time setup_us=%.0f pipeline_us=%.0f run_us=%.0f device_ns=%s total_us=%.0f maxrss_kib=%ld buffers_bytes=%zu\n",
            t_pipe0 - t_setup0, t_pipe1 - t_pipe0, t_run1 - t_pipe1, devns, now_us() - t_start, maxrss_kib, total_bytes);
#if MT_TARGET == 2
    /* The driver must still be mapped while its objects exist; the loader may unload it at teardown. */
    int irc = isolation_check(1);
    if (irc) return irc;
#endif
    vkDestroyCommandPool(dev, cp, NULL); vkDestroyDescriptorPool(dev, dp, NULL); vkDestroyPipeline(dev, pipe, NULL);
    vkDestroyPipelineLayout(dev, pl, NULL); vkDestroyDescriptorSetLayout(dev, dsl, NULL); vkDestroyShaderModule(dev, sm, NULL);
    for (int i = 0; i < nb; i++) { vkUnmapMemory(dev, m[i]); vkDestroyBuffer(dev, b[i], NULL); vkFreeMemory(dev, m[i], NULL); }
    vkDestroyDevice(dev, NULL);
    vkDestroyInstance(inst, NULL);
    return 0;
}
#endif

int main(int argc, char **argv)
{
    t_start = now_us();
#if MT_TARGET == 3
    if (!cl_gpu_opted_in())
        return die(2, "this build is MT_TARGET=3 and runs on a GPU: set MT_CL_GPU=1 to opt in (refused before any file or the OpenCL loader is opened)");
#endif
    if (argc != 7 && argc != 8) return die(2, "usage: spirv-host cl|vk MODULE.spv KERNEL COUNT IN.bin [IN.bin] OUT.bin");
    Job j = { .api = argv[1], .module = argv[2], .kernel = argv[3], .nin = argc - 6, .out = argv[argc - 1] };
    if (strcmp(j.api, "cl") != 0 && strcmp(j.api, "vk") != 0) return die(2, "api must be cl or vk");
#if MT_TARGET == 1
    if (strcmp(j.api, "vk") != 0) return die(2, "this build is MT_TARGET=1 (MoltenVK): only vk");
#elif MT_TARGET == 2
    if (strcmp(j.api, "vk") != 0) return die(2, "this build is MT_TARGET=2 (NVIDIA Vulkan driver): only vk");
#elif MT_TARGET == 3
    if (strcmp(j.api, "cl") != 0) return die(2, "this build is MT_TARGET=3 (vendor OpenCL GPU driver): only cl");
#endif
    char *end;
    unsigned long c = strtoul(argv[4], &end, 10);
    if (*end || c == 0 || c > (1ul << 26)) return die(2, "COUNT must be 1..67108864");
    j.count = (uint32_t)c;
    for (int i = 0; i < j.nin; i++) j.in[i] = argv[5 + i];
    j.code = slurp(j.module, &j.code_size);
    if (!j.code || j.code_size % 4) return die(4, "module unreadable or not whole 32-bit words");
    for (int i = 0; i < j.nin; i++) {
        j.buf[i] = slurp(j.in[i], &j.bytes[i]);
        if (!j.buf[i] || j.bytes[i] % ((size_t)4 * j.count)) return die(4, "input unreadable or not a multiple of COUNT floats");
    }
    j.bytes[j.nin] = (size_t)4 * j.count;
    uint32_t *o = malloc(j.bytes[j.nin]);
    if (!o) return die(4, "out of memory");
    for (uint32_t i = 0; i < j.count; i++) o[i] = POISON;
    j.buf[j.nin] = (float *)o;
#if MT_TARGET == 3
    int rc = run_cl(&j);
#elif MT_TARGET != 0
    int rc = run_vk(&j);
#else
    int rc = strcmp(j.api, "cl") == 0 ? run_cl(&j) : run_vk(&j);
#endif
    if (rc) return rc;
    rc = isolation();
    if (rc) return rc;
    /* 0600, not fopen's 0666 & ~umask (CodeQL cpp/world-writable-file-creation) */
    int fd = open(j.out, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    FILE *f = fd >= 0 ? fdopen(fd, "wb") : NULL;
    if (!f && fd >= 0) close(fd);
    if (!f || fwrite(o, 1, j.bytes[j.nin], f) != j.bytes[j.nin] || fclose(f) != 0) return die(4, "cannot write output");
    return 0;
}
