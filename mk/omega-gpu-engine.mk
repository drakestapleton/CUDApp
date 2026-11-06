# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Omega GPU Engine (docs/numeric/OMEGA_GPU_ENGINE.md). Picked up by
# `-include mk/*.mk`. Host only: opens no device; needs no physics headers except the two A3b1 targets, not
# part of `all` or `test`.
#   test-gpu-engine-compile  header + core compile, enum names, refusals, no-op backend (A2/A3a)
#   test-gpu-engine          the engine core against a fake backend (every state and failure)
#   test-gpu-engine-backend-compile  syntax-checks src/omega_blackwell_engine.c (A3b1): -c only, no link, no device
#   test-gpu-engine-prod-build       production build of GPU_PROD_TARGET (CUDA++: libomega_gpu.a; Omega: build/omegatool): links only, opens no device
#   test-gpu-engine-mutants  removes each protection from src/omega_gpu_engine.c in a scratch
#                            copy; the host test must fail each one (exit nonzero on a survivor)
#   test-gpu-engine-sim      (A3b2) the REAL engine core + Blackwell backend + vector wrapper against a simulated driver
#                            (tests/fake_m16_native.c); host only, no device, no physics C sources
#   test-gpu-engine-backend-mutants  (A3b2) one mutant per protection in the backend and the vector wrapper
#   test-gpu-engine-gb10-compile     (A3b2) compile-only check of tests/test_omega_gpu_engine_gb10.c (Phase B chip test, never run here)
#   gpu-engine-freestanding          (native v1 P07 cut A) the portable GPU files built -ffreestanding with
#                                    OMEGA_GPU_NO_POSIX: no Linux header, no errno, no clock, no libc beyond mem*/str*
#   test-codegen-freestanding        (cut A.2) codegen hosted vs freestanding byte-identical, bounded code buffers
#   test-gpu-engine-freestanding     the engine contract test and the platform contract test linked against
#                                    those freestanding objects (host runner, opens no device)
.PHONY: test-gpu-engine-compile test-gpu-engine test-gpu-engine-mutants test-gpu-engine-backend-compile test-gpu-engine-prod-build test-gpu-engine-sim test-gpu-engine-backend-mutants test-gpu-engine-gb10-compile gpu-engine-freestanding test-gpu-engine-freestanding
build/test_omega_gpu_engine_compile: tests/test_omega_gpu_engine_compile.c src/omega_gpu_engine.c src/omega_gpu_engine.h src/cudapp_om_eng.c src/omega_eng_dec.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -DOMEGA_NUMERIC_CPU_ONLY -o $@ tests/test_omega_gpu_engine_compile.c src/omega_gpu_engine.c src/cudapp_om_eng.c
test-gpu-engine-compile: build/test_omega_gpu_engine_compile
	./build/test_omega_gpu_engine_compile

build/test_omega_gpu_engine: tests/test_omega_gpu_engine.c src/omega_gpu_engine.c src/omega_gpu_engine.h src/cudapp_om_eng.c src/omega_eng_dec.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -DOMEGA_NUMERIC_CPU_ONLY -o $@ tests/test_omega_gpu_engine.c src/omega_gpu_engine.c src/cudapp_om_eng.c
test-gpu-engine: build/test_omega_gpu_engine
	./build/test_omega_gpu_engine

test-gpu-engine-mutants:
	sh tools/gpu_engine_mutations.sh

# A3b1: the Blackwell backend includes physics headers (m16_native.h, nvrm.h), so it
# uses the same physics -I paths as the top-level Makefile CFLAGS (Makefile lines 9-14).
GPU_ENGINE_PHYS_INC = -I$(PHYSICS_DIR)/m16 -I$(PHYSICS_DIR)/nvrm \
	-I$(PHYSICS_DIR)/third_party/nvidia-open-580.173.02/src/common/sdk/nvidia/inc \
	-I$(PHYSICS_DIR)/third_party/nvidia-open-580.173.02/kernel-open/common/inc \
	-I$(PHYSICS_DIR)/third_party/nvidia-open-580.173.02/kernel-open/nvidia-uvm \
	-I$(PHYSICS_DIR)/third_party/nvidia-open-580.173.02/src/nvidia/arch/nvalloc/unix/include

build/omega_blackwell_engine_syntax.o: src/omega_blackwell_engine.c src/omega_blackwell_engine.h src/omega_gpu_engine.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -Isrc $(GPU_ENGINE_PHYS_INC) -c -o $@ src/omega_blackwell_engine.c
test-gpu-engine-backend-compile: build/omega_blackwell_engine_syntax.o

# Links only, opens no device. Uses the real m16_native and nvrm sources at the physics pin.
test-gpu-engine-prod-build:
	$(MAKE) $(GPU_PROD_TARGET)

# A3b2: simulated-driver host test. Links the sources of the Makefile RX_COMPOSE_GPU_SRCS list (plus the
# three core files the vector builder needs) with tests/fake_m16_native.c in place of the physics
# m16_native.c and nvrm.c. Opens no device. OMEGA_BLACKWELL_HOST_SIMULATOR opts this
# binary into the fake driver. The production library build does not set it.
GPU_ENGINE_SIM_SRCS = src/omega_blackwell_submit.c src/omega_blackwell_engine.c src/omega_gpu_engine.c \
	src/omega_blackwell_matmul.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c $(CUDAPP_BINDING_SRCS) src/omega_blackwell_encoder.c \
	src/omega_blackwell_qmd.c src/omega_blackwell_realize.c src/omega_vector.c \
	src/omega_core.c src/omega_canonical.c src/sha256.c

build/test_omega_blackwell_engine: tests/test_omega_blackwell_engine.c tests/fake_m16_native.c tests/fake_m16_native.h \
	$(GPU_ENGINE_SIM_SRCS) src/omega_gpu_engine.h src/omega_blackwell_engine.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -DOMEGA_BLACKWELL_HOST_SIMULATOR -pthread -ffunction-sections -fdata-sections \
		-Isrc -Itests $(GPU_ENGINE_PHYS_INC) -o $@ tests/test_omega_blackwell_engine.c tests/fake_m16_native.c \
		$(GPU_ENGINE_SIM_SRCS) -Wl,--gc-sections -lm
# The matmul wrappers' dimension and code-size refusals (they also need the fixed-deadline wait).
build/test_omega_blackwell_matmul_bounds: tests/test_omega_blackwell_matmul_bounds.c tests/fake_m16_native.c \
	tests/fake_m16_native.h $(GPU_ENGINE_SIM_SRCS) src/omega_gpu_wait.c src/omega_blackwell_submit.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -DOMEGA_BLACKWELL_HOST_SIMULATOR -pthread -ffunction-sections -fdata-sections \
		-Isrc -Itests $(GPU_ENGINE_PHYS_INC) -o $@ tests/test_omega_blackwell_matmul_bounds.c tests/fake_m16_native.c \
		$(GPU_ENGINE_SIM_SRCS) src/omega_gpu_wait.c -Wl,--gc-sections -lm
test-gpu-engine-sim: build/test_omega_blackwell_engine build/test_omega_blackwell_matmul_bounds
	./build/test_omega_blackwell_engine
	./build/test_omega_blackwell_matmul_bounds

# One mutant per backend and vector-wrapper protection; the sim test must fail each one.
test-gpu-engine-backend-mutants:
	PHYSICS_DIR="$(PHYSICS_DIR)" sh tools/gpu_engine_backend_mutations.sh

# Phase B chip test, compile only here. Never run in Phase A.
build/test_omega_gpu_engine_gb10.o: tests/test_omega_gpu_engine_gb10.c src/omega_gpu_engine.h src/omega_blackwell_engine.h src/omega_blackwell_submit.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math -pthread -Isrc $(GPU_ENGINE_PHYS_INC) -c -o $@ tests/test_omega_gpu_engine_gb10.c
test-gpu-engine-gb10-compile: build/test_omega_gpu_engine_gb10.o

# Measurement (sc#277): driver allocations and frees made by the REAL session + matmul + attention
# APIs while serving a Qwen3-4B-shaped sequence, against the simulated driver. Host only, no device.
# Prints MEASURE lines; asserts only bookkeeping. Links no physics C sources, only its headers.
GPU_SERVE_ALLOC_SRCS = src/omega_gpu_session.c src/omega_gpu_platform.c src/omega_gpu_platform_linux.c om/platform-linux/gen/link/plat_fx_effects.c om/platform-linux/gen/link/plat_fx_wait.c src/omega_gpu_mem_reserve.c src/omega_gpu_fmt.c src/omega_gpu_matmul_api.c src/omega_gpu_attention_api.c src/omega_gpu_serving.c \
	src/omega_gpu_wait.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c $(CUDAPP_BINDING_SRCS) src/omega_blackwell_encoder.c src/omega_blackwell_qmd.c \
	src/omega_blackwell_matmul.c src/omega_ampere_qmd.c src/sha256.c
build/gpu_serving_alloc_count_test: tests/gpu_serving_alloc_count_test.c tests/fake_m16_native.c tests/fake_m16_native.h $(GPU_SERVE_ALLOC_SRCS)
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -pthread -ffp-contract=off -Isrc -Itests $(GPU_ENGINE_PHYS_INC) \
		-o $@ tests/gpu_serving_alloc_count_test.c tests/fake_m16_native.c tests/omega_gpu_live_marker_fixture.c $(GPU_SERVE_ALLOC_SRCS) -lm
.PHONY: test-gpu-serving-alloc
test-gpu-serving-alloc: build/gpu_serving_alloc_count_test
	./build/gpu_serving_alloc_count_test plain
	./build/gpu_serving_alloc_count_test reserve
	./build/gpu_serving_alloc_count_test bounds
	./build/gpu_serving_alloc_count_test bounds-block
	./build/gpu_serving_alloc_count_test rollback
	./build/gpu_serving_alloc_count_test sweep
	./build/gpu_serving_alloc_count_test pin
	./build/gpu_serving_alloc_count_test seal-bare
	./build/gpu_serving_alloc_count_test update
	@if ./build/gpu_serving_alloc_count_test sweep-skip >build/gpu_serving_alloc_count_sweep_skip.log 2>&1; then echo "RED RUN PASSED: zero allocations over the row sweep do not need prepare + seal"; exit 1; fi
	@if [ "$$(grep -c "^FAIL" build/gpu_serving_alloc_count_sweep_skip.log)" -lt 1 ]; then echo "sweep-skip exited nonzero without a FAIL check (crash, not the red assertion)"; exit 1; fi
	@grep -E "^MEASURE sweep" build/gpu_serving_alloc_count_sweep_skip.log | sed "s/^/sweep-skip (control): /"
	@if ./build/gpu_serving_alloc_count_test reserve-skip >build/gpu_serving_alloc_count_skip.log 2>&1; then echo "RED RUN PASSED: the zero-allocation assertion does not need the reservation call"; exit 1; fi
	@if [ "$$(grep -c "^FAIL" build/gpu_serving_alloc_count_skip.log)" -lt 1 ]; then echo "reserve-skip exited nonzero without a FAIL check (crash, not the red assertion)"; exit 1; fi
	@echo "reserve-skip (no reservation call): fails as required ($$(grep -c "^FAIL" build/gpu_serving_alloc_count_skip.log) failing checks)"

# native v1 P07 cut A (aienos#286 cut A, aien-architecture#190): the freestanding link target.
# Each file below is compiled with no hosted headers available (-ffreestanding, -fno-builtin,
# OMEGA_GPU_NO_POSIX) and the partial link of all of them may leave only the mem*/str* symbols
# undefined. Any other undefined symbol (errno, clock_gettime, printf, malloc, pthread_*, nvrm_*,
# m16_*) fails the target. This proves the first-compute PATH links without Linux headers or
# runtime symbols; it proves nothing about device ownership (NOT_RUN until cut B runs on the Spark).
GPU_FS_DIR = build/gpu_fs
GPU_FS_CFLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-stack-protector \
	-U_FORTIFY_SOURCE -DOMEGA_GPU_NO_POSIX -DOMEGA_NUMERIC_CPU_ONLY -Isrc
# P07 cut C (session part): the persistent session, the matmul and attention APIs, the serving reservation and the
# bounded wait join the link. Their machine comes from an installed OmegaGpuPlatform; the Linux reference
# (src/omega_gpu_platform_linux.c) is deliberately NOT in this list. Still outside it: the elementwise API, the
# numeric launchers, the vector executor, the resident runtime (docs/numeric/OMEGA_GPU_NATIVE_BOUNDARY.md).
GPU_FS_SRCS = src/omega_gpu_engine.c src/omega_gpu_platform.c src/omega_blackwell_encoder.c src/omega_blackwell_codegen.c \
	src/omega_blackwell_matmul.c src/omega_blackwell_qmd.c src/omega_blackwell_realize.c src/sha256.c \
	src/omega_gpu_session.c src/omega_gpu_fmt.c src/omega_gpu_matmul_api.c src/omega_gpu_attention_api.c \
	src/omega_gpu_serving.c src/omega_gpu_wait.c src/cudapp_om_av_bf16.c $(CUDAPP_FS_BINDING)
GPU_FS_HDRS = src/omega_gpu_engine.h src/omega_gpu_platform.h src/omega_blackwell_codegen.h src/omega_blackwell_matmul.h \
	src/omega_gpu_session.h src/omega_gpu_fmt.h src/omega_gpu_matmul_api.h src/omega_gpu_attention_api.h \
	src/omega_gpu_serving.h src/omega_gpu_wait.h src/cudapp_native.h src/omega_api_plan.h src/omega_host_policy.h src/omega_gpu_apiviews.h
GPU_FS_OBJS = $(patsubst src/%.c,$(GPU_FS_DIR)/%.o,$(GPU_FS_SRCS))
GPU_FS_LINK = $(GPU_FS_DIR)/omega_gpu_freestanding.o
GPU_FS_ALLOWED = memcpy|memmove|memset|memcmp|strlen|strcmp

$(GPU_FS_DIR)/%.o: src/%.c $(GPU_FS_HDRS)
	@mkdir -p $(GPU_FS_DIR)
	gcc $(GPU_FS_CFLAGS) -c -o $@ $<

$(GPU_FS_LINK): $(GPU_FS_OBJS)
	gcc -r -nostdlib -o $@ $(GPU_FS_OBJS)
	@bad=$$(nm -u $@ | awk '{print $$NF}' | grep -Ev '^(_GLOBAL_OFFSET_TABLE_|($(GPU_FS_ALLOWED)))$$' || true); \
	if [ -n "$$bad" ]; then echo "freestanding GPU link still needs: $$bad"; rm -f $@; exit 1; fi
	@echo "freestanding GPU link ($(words $(GPU_FS_SRCS)) files): only $$(nm -u $@ | awk '{print $$NF}' | grep -Ev '^_GLOBAL_OFFSET_TABLE_$$' | sort -u | tr '\n' ' ')undefined"
gpu-engine-freestanding: $(GPU_FS_LINK)

build/test_omega_gpu_engine_freestanding: tests/test_omega_gpu_engine.c $(GPU_FS_DIR)/omega_gpu_engine.o src/omega_gpu_engine.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -DOMEGA_NUMERIC_CPU_ONLY -DOMEGA_GPU_NO_POSIX \
		-o $@ tests/test_omega_gpu_engine.c $(GPU_FS_DIR)/omega_gpu_engine.o
build/test_omega_gpu_platform: tests/test_omega_gpu_platform.c $(GPU_FS_DIR)/omega_gpu_platform.o src/omega_gpu_platform.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -o $@ tests/test_omega_gpu_platform.c $(GPU_FS_DIR)/omega_gpu_platform.o
# The same platform test on the hosted build, where the refusal decisions come from om/engine/eng.om.
build/test_omega_gpu_platform_om: tests/test_omega_gpu_platform.c src/omega_gpu_platform.c src/omega_gpu_platform.h \
		src/cudapp_om_eng.c src/omega_eng_dec.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -o $@ tests/test_omega_gpu_platform.c src/omega_gpu_platform.c src/cudapp_om_eng.c
test-gpu-engine-freestanding: $(GPU_FS_LINK) build/test_omega_gpu_engine_freestanding build/test_omega_gpu_platform \
		build/test_omega_gpu_platform_om
	./build/test_omega_gpu_engine_freestanding
	./build/test_omega_gpu_platform
	./build/test_omega_gpu_platform_om

# native v1 P07 cut A.2 (aienos#286): code generation joins the freestanding link above with a caller-owned
# code buffer (omega_blackwell_kernel_encode_into, no allocator). The same test source is built hosted (the
# allocating generators) and freestanding (the *_prog generators into a caller buffer); their instruction
# stream dumps of every fixture must be byte-identical (cmp). The freestanding run then checks the bounded
# buffer contract, the golden-word fixture verifiers on the freestanding object and the bf16 rounding path on
# all 2^32 inputs. The allowlist control links a malloc caller into the freestanding object and requires the
# nm -u check to refuse it (a check that can never fail proves nothing).
.PHONY: test-codegen-freestanding test-gpu-fs-allowlist-control
CODEGEN_FS_TEST_DEPS = tests/test_omega_codegen_freestanding.c src/omega_blackwell_codegen.h src/omega_blackwell_matmul.h
# The hosted side encodes through the .om encoder (om/encoder/bw_enc_sm121.om via portable generated C), the
# freestanding side through the migration-pending C encoder, so the cmp below is also an .om-vs-C differential.
build/test_codegen_hosted: $(CODEGEN_FS_TEST_DEPS) src/omega_blackwell_codegen.c $(CUDAPP_BINDING_SRCS) src/cudapp_native.h \
		src/omega_blackwell_matmul.c src/sha256.c src/omega_gpu_om_ir.c
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -pthread -Isrc -o $@ tests/test_omega_codegen_freestanding.c \
		src/omega_blackwell_codegen.c $(CUDAPP_BINDING_SRCS) src/omega_blackwell_matmul.c src/sha256.c src/omega_gpu_om_ir.c
build/test_codegen_freestanding: $(CODEGEN_FS_TEST_DEPS) $(GPU_FS_LINK)
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -DOMEGA_GPU_NO_POSIX -o $@ tests/test_omega_codegen_freestanding.c \
		$(GPU_FS_DIR)/omega_blackwell_codegen.o $(GPU_FS_DIR)/omega_blackwell_matmul.o $(GPU_FS_DIR)/sha256.o $(GPU_FS_DIR)/cudapp_freest_f16.o
test-codegen-freestanding: build/test_codegen_hosted build/test_codegen_freestanding test-gpu-fs-allowlist-control
	./build/test_codegen_hosted --dump > build/codegen_dump_hosted.txt
	./build/test_codegen_freestanding --dump > build/codegen_dump_freestanding.txt
	cmp build/codegen_dump_hosted.txt build/codegen_dump_freestanding.txt
	@echo "codegen dumps byte-identical: $$(grep -c '^fixture ' build/codegen_dump_hosted.txt) fixtures, $$(grep -v -E '^(fixture|digest) ' build/codegen_dump_hosted.txt | tr -d '\n' | wc -c | awk '{print $$1/2}') code bytes"
	./build/test_codegen_freestanding

test-gpu-fs-allowlist-control: $(GPU_FS_LINK)
	@mkdir -p $(GPU_FS_DIR)/control
	printf 'void *malloc(unsigned long);\nvoid *omega_gpu_fs_control(void) { return malloc(16); }\n' > $(GPU_FS_DIR)/control/canary.c
	gcc $(GPU_FS_CFLAGS) -c -o $(GPU_FS_DIR)/control/canary.o $(GPU_FS_DIR)/control/canary.c
	gcc -r -nostdlib -o $(GPU_FS_DIR)/control/with_canary.o $(GPU_FS_OBJS) $(GPU_FS_DIR)/control/canary.o
	@bad=$$(nm -u $(GPU_FS_DIR)/control/with_canary.o | awk '{print $$NF}' | grep -Ev '^(_GLOBAL_OFFSET_TABLE_|($(GPU_FS_ALLOWED)))$$' || true); \
	if [ "$$bad" != "malloc" ]; then echo "allowlist control: expected exactly malloc refused, got '$$bad'"; exit 1; fi; \
	echo "allowlist control: a malloc caller is refused by the nm -u check (as required)"

# native v1 P07 cut C, session part (aienos#286): the portable session over OmegaGpuPlatform.
#  - test_omega_gpu_session_platform links the FREESTANDING partial-link object and drives it through a fake
#    platform: install refusals, no default platform, typed GPU-uncached buffers, polling only, cancel before and
#    after submit, timeout (latch + park, nothing released), stale handle after close/reopen, reservation refusal.
#  - test_omega_gpu_platform_linux drives the Linux REFERENCE platform (reference only) on the simulated driver:
#    same driver calls in the same order as before cut C, its refusals, and a marker that never lands.
#  - the mutants prove the session test can fail: each one breaks one rule in a copy of the session and the test
#    must report FAIL for it.
.PHONY: test-gpu-session-platform test-gpu-session-mutants
build/test_omega_gpu_session_platform: tests/test_omega_gpu_session_platform.c $(GPU_FS_LINK)
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -DOMEGA_GPU_NO_POSIX -o $@ tests/test_omega_gpu_session_platform.c $(GPU_FS_LINK)

# Same typed synthetic platform and real session, with hosted native bridge/cache
# consumers. No Linux platform is linked, so this target cannot open a GPU.
GPU_NATIVE_RUNTIME_SRCS = src/omega_gpu_cmdlist.c src/omega_gpu_fp8_api.c \
	src/omega_gpu_fp8gemm_api.c src/omega_blackwell_codegen_moe.c src/omega_blackwell_codegen_moe2.c \
	src/omega_blackwell_codegen_gemm.c src/omega_gpu_fp8_pack.c src/omega_gpu_mem_cached.c \
	src/omega_gpu_hostmath.c src/omega_native_artifact.c src/omega_cubin.c $(CUDAPP_HOST_BINDING) \
	src/omega_gpu_q36_graph.c src/omega_gpu_decode_graph.c src/omega_gpu_router_api.c \
	src/omega_gpu_gemm_ex.c src/omega_gpu_gemm.c src/omega_blackwell_codegen_qmma.c src/omega_gpu_om_ir.c
.PHONY: test-gpu-native-runtime
build/test_omega_gpu_native_runtime: tests/test_omega_gpu_session_platform.c tests/native_runtime_controls.h \
		$(GPU_FS_LINK) $(GPU_NATIVE_RUNTIME_SRCS) src/omega_gpu_native_runtime.h src/omega_gpu_q36_native.h \
		src/omega_gpu_cmdlist_native.h src/omega_native_artifact_snapshot.h
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -DOMEGA_GPU_NO_POSIX -DOMEGA_NATIVE_RUNTIME_TEST \
		-DCUDAPP_NATIVE_RUNTIME_TEST -ffunction-sections -fdata-sections -Wl,--gc-sections \
		-o $@ tests/test_omega_gpu_session_platform.c $(GPU_FS_LINK) $(GPU_NATIVE_RUNTIME_SRCS) -lpthread -lm
test-gpu-native-runtime: build/test_omega_gpu_native_runtime
	./build/test_omega_gpu_native_runtime
GPU_LINUX_REF_SRCS = src/omega_gpu_session.c src/omega_gpu_platform.c src/omega_gpu_platform_linux.c om/platform-linux/gen/link/plat_fx_effects.c om/platform-linux/gen/link/plat_fx_wait.c src/omega_gpu_mem_reserve.c src/omega_gpu_fmt.c src/omega_blackwell_qmd.c src/omega_ampere_qmd.c $(CUDAPP_FS_BINDING)
# omega #459: the command list over the reference (near-full ring with the #457 L2 invalidate prefix pending) needs the
# cached allocation, the command list and the codegen behind its nop kernel.
GPU_LINUX_REF_CMDLIST_SRCS = src/omega_gpu_mem_cached.c src/omega_gpu_cmdlist.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c $(CUDAPP_HOST_BINDING) src/omega_blackwell_encoder.c \
	src/omega_blackwell_matmul.c src/omega_blackwell_realize.c src/sha256.c
build/test_omega_gpu_platform_linux: tests/test_omega_gpu_platform_linux.c tests/fake_m16_native.c tests/fake_m16_native.h $(GPU_LINUX_REF_SRCS) $(GPU_FS_HDRS) src/omega_gpu_platform_linux.h \
		src/omega_gpu_cmdlist.h src/omega_gpu_mem_cached.h $(GPU_LINUX_REF_CMDLIST_SRCS)
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -pthread -Isrc -Itests $(GPU_ENGINE_PHYS_INC) \
		-o $@ tests/test_omega_gpu_platform_linux.c tests/fake_m16_native.c tests/omega_gpu_live_marker_fixture.c $(GPU_LINUX_REF_SRCS) $(GPU_LINUX_REF_CMDLIST_SRCS) -lm
# The driver client's VA slots and close path on the host (no device opened, nvrm_open never called).
build/test_nvrm_va_slots: tests/test_nvrm_va_slots.c $(PHYSICS_DIR)/nvrm/nvrm.c $(PHYSICS_DIR)/nvrm/nvrm.h src/omega_ampere_qmd.c \
		src/omega_gpu_chip.c $(CUDAPP_BINDING_SRCS)
	@mkdir -p build
	gcc -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -Isrc $(GPU_ENGINE_PHYS_INC) -o $@ tests/test_nvrm_va_slots.c src/omega_ampere_qmd.c \
		src/omega_gpu_chip.c $(CUDAPP_BINDING_SRCS)
test-gpu-session-platform: build/test_omega_gpu_session_platform build/test_omega_gpu_platform_linux build/test_nvrm_va_slots \
		test-gpu-session-mutants
	./build/test_omega_gpu_session_platform
	./build/test_omega_gpu_platform_linux
	./build/test_nvrm_va_slots

# The aien-gpu platform (src/omega_gpu_platform_aien.c, second implementation of the contract over aien-gpu.ko) on a
# simulated /dev/aien-gpu (tests/test_omega_gpu_platform_aien.c): no device opened. It links the Linux reference,
# m16_native.c and nvrm.c only to compare caps, the GP entry and the USERD layout with them; nothing in it calls the
# NVIDIA driver. Decisions and request bytes come from native/aiengpu/context.om (CUDApp#128) through the generated
# src/cudapp_om_aiengpu.c; the C they replaced is the oracle under CUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN=1.
#   make test-gpu-platform-aien            (in `make test`, no Omega tools needed)
#       1. the test through the .om decisions, and through the C oracle;
#       2. the differential: AIEN_TRACE of both (every ioctl's bytes in and out, map/unmap arguments, the final
#          ring, USERD GPPut, pushbuffer and doorbell) must be byte-identical;
#       3. the C-glue mutants: each breaks one effect in a copy of the platform; the test must fail AND its trace
#          must differ from the oracle's (the red check of step 2).
#   make test-gpu-platform-aien-om-mutants mutant context.om files, each compiled by the pinned omc, must fail the
#       test or change the trace (needs OMEGA_OMC and OMEGA_BRIDGE as in om/units.sh; without them exit 3, NOT_RUN).
GPU_AIEN_INC = -I$(PHYSICS_DIR)/third_party/aien-gpu-abi1
# AArch64: the effects through native/aiengpu/effects.om (src/cudapp_aiengpu_effects.c, regenerated by
# native/aiengpu/units.sh); elsewhere the C system calls (CUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN_EFFECTS=1 by default).
GPU_AIEN_EFFECTS_SRCS = $(if $(filter aarch64,$(CUDAPP_HOST_ARCH)),src/cudapp_aiengpu_effects.c)
GPU_AIEN_OTHER_SRCS = src/omega_gpu_platform.c src/omega_gpu_mem_reserve.c src/omega_gpu_session.c \
	src/omega_gpu_platform_linux.c om/platform-linux/gen/link/plat_fx_effects.c om/platform-linux/gen/link/plat_fx_wait.c src/omega_gpu_fmt.c src/omega_blackwell_qmd.c src/omega_ampere_qmd.c src/omega_gpu_chip.c \
	$(PHYSICS_DIR)/m16/m16_native.c $(PHYSICS_DIR)/nvrm/nvrm.c $(GPU_AIEN_EFFECTS_SRCS)
# The platform uses nvrm.c only for its pure encoders (nvrm_mthd, nvrm_gp_entry) and never opens the RM, so nvrm.c takes
# the C test transport and the NVRM native unit (src/cudapp_om_nvrm_transport.c, NVRM #281) stays out: each native unit
# carries its own omega_native_context_create/destroy, and on AArch64 cudapp_aiengpu_effects.c is already linked.
GPU_AIEN_BINDING = $(filter-out src/cudapp_om_nvrm_transport.c,$(CUDAPP_BINDING_SRCS))
GPU_AIEN_SRCS = src/omega_gpu_platform_aien.c $(GPU_AIEN_OTHER_SRCS) $(GPU_AIEN_BINDING)
# MIGRATION-PENDING C: the platform's code exists only with CUDAPP_MIGRATION_PENDING_AIEN_GPU=1, set here for the test and
# its mutants and OFF in libomega_gpu.a and every other build (the effects wait on devctl/maphold for AGPU requests).
GPU_AIEN_CFLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -pthread -DCUDAPP_MIGRATION_PENDING_AIEN_GPU=1 \
	-DCUDAPP_NVRM_TEST_TRANSPORT=1 -Isrc $(GPU_ENGINE_PHYS_INC) $(GPU_AIEN_INC)
GPU_AIEN_DEPS = tests/test_omega_gpu_platform_aien.c $(GPU_AIEN_SRCS) src/omega_gpu_platform_aien.h \
	src/omega_gpu_mem_reserve.h src/cudapp_native.h third_party/aien-gpu-abi1/aien_gpu_uapi.h \
	$(if $(GPU_AIEN_EFFECTS_SRCS),src/cudapp_aiengpu_native.h)
build/test_omega_gpu_platform_aien: $(GPU_AIEN_DEPS)
	@mkdir -p build
	gcc $(GPU_AIEN_CFLAGS) -o $@ tests/test_omega_gpu_platform_aien.c $(GPU_AIEN_SRCS)
build/test_omega_gpu_platform_aien_c: $(GPU_AIEN_DEPS)
	@mkdir -p build
	gcc $(GPU_AIEN_CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN=1 -o $@ tests/test_omega_gpu_platform_aien.c \
		$(GPU_AIEN_SRCS)
GPU_AIEN_MUT_DIR = build/gpu_aien_mut
.PHONY: test-gpu-platform-aien test-gpu-platform-aien-mutants test-gpu-platform-aien-om-mutants
test-gpu-platform-aien: build/test_omega_gpu_platform_aien build/test_omega_gpu_platform_aien_c test-gpu-platform-aien-mutants \
	$(if $(GPU_AIEN_EFFECTS_SRCS),test-gpu-platform-aien-effects)
	@mkdir -p $(GPU_AIEN_MUT_DIR)
	@rm -f $(GPU_AIEN_MUT_DIR)/om.trace $(GPU_AIEN_MUT_DIR)/c.trace
	AIEN_TRACE=$(GPU_AIEN_MUT_DIR)/om.trace ./build/test_omega_gpu_platform_aien
	AIEN_TRACE=$(GPU_AIEN_MUT_DIR)/c.trace ./build/test_omega_gpu_platform_aien_c
	@cmp $(GPU_AIEN_MUT_DIR)/om.trace $(GPU_AIEN_MUT_DIR)/c.trace || { echo "AIEN DIFF FAIL: .om and C oracle traces differ"; exit 1; }
	@echo "AIEN DIFF PASS: .om and C oracle traces identical ($$(grep -c '^case ' $(GPU_AIEN_MUT_DIR)/om.trace) cases, $$(grep -c '^ioctl 0x' $(GPU_AIEN_MUT_DIR)/om.trace) ioctls, $$(wc -c < $(GPU_AIEN_MUT_DIR)/om.trace) bytes)"
test-gpu-platform-aien-mutants: build/test_omega_gpu_platform_aien_c
	@mkdir -p $(GPU_AIEN_MUT_DIR)
	@rm -f $(GPU_AIEN_MUT_DIR)/oracle.trace; AIEN_TRACE=$(GPU_AIEN_MUT_DIR)/oracle.trace ./build/test_omega_gpu_platform_aien_c > /dev/null
	@set -e; killed=0; \
	amut() { name=$$1; script=$$2; \
	  sed -e "$$script" src/omega_gpu_platform_aien.c > $(GPU_AIEN_MUT_DIR)/$$name.c; \
	  if cmp -s src/omega_gpu_platform_aien.c $(GPU_AIEN_MUT_DIR)/$$name.c; then echo "mutant $$name: pattern not found"; exit 1; fi; \
	  gcc $(GPU_AIEN_CFLAGS) -o $(GPU_AIEN_MUT_DIR)/$$name tests/test_omega_gpu_platform_aien.c $(GPU_AIEN_MUT_DIR)/$$name.c \
	    $(GPU_AIEN_OTHER_SRCS) $(GPU_AIEN_BINDING); \
	  rm -f $(GPU_AIEN_MUT_DIR)/$$name.trace; \
	  if AIEN_TRACE=$(GPU_AIEN_MUT_DIR)/$$name.trace ./$(GPU_AIEN_MUT_DIR)/$$name > $(GPU_AIEN_MUT_DIR)/$$name.log 2>&1; then \
	    echo "mutant $$name SURVIVED"; exit 1; fi; \
	  if cmp -s $(GPU_AIEN_MUT_DIR)/$$name.trace $(GPU_AIEN_MUT_DIR)/oracle.trace; then \
	    echo "mutant $$name: trace equals the oracle's, the differential cannot see it"; exit 1; fi; \
	  echo "mutant $$name killed: $$(grep -c '^FAIL' $(GPU_AIEN_MUT_DIR)/$$name.log) case(s) failed, trace differs"; killed=$$((killed+1)); }; \
	amut no-doorbell 's|= r->token;|= 0;|'; \
	amut no-l2-prefix 's|r->l2_inval_pending = 1; /\* omega #457 \*/|{ }|'; \
	echo "test-gpu-platform-aien-mutants: $$killed of 2 mutants killed"
# Each sed must match exactly one line of context.om (refused otherwise). A mutant is killed when the test fails or
# its trace differs from the C oracle's.
GPU_AIEN_OM_MUTANTS = \
	'no-gpput|s/^ag_put b 36 rem and add get m 5 1 4294967295 AGRING 4$$/ag_put b 36 0 4/' \
	'state-unchecked|s/^if ne ag_load b 4 4 4$$/if ne ag_load b 4 4 ag_load b 4 4/' \
	'always-cached|s/^const AGUNC ulong 1$$/const AGUNC ulong 0/' \
	'ring-never-full|s/ sub AGRING 1$$/ 4294967295/' \
	'l2-word|s/^ag_put b 0 537133066 4$$/ag_put b 0 0 4/' \
	'gp-bit|s/ shl ulong 1 41$$/ shl ulong 1 40/' \
	'alloc-va|s/^ag_put b 8 get m 2 8$$/ag_put b 8 0 8/' \
	'userd-off|s/^ag_put b 24 AGUSERD 8$$/ag_put b 24 0 8/'
test-gpu-platform-aien-om-mutants: build/test_omega_gpu_platform_aien_c
	@[ -n "$(OMEGA_OMC)" ] && [ -n "$(OMEGA_BRIDGE)" ] || { echo "NOT_RUN: set OMEGA_OMC and OMEGA_BRIDGE"; exit 3; }
	@mkdir -p $(GPU_AIEN_MUT_DIR)
	@rm -f $(GPU_AIEN_MUT_DIR)/oracle.trace; AIEN_TRACE=$(GPU_AIEN_MUT_DIR)/oracle.trace ./build/test_omega_gpu_platform_aien_c > /dev/null
	@n=0; k=0; for m in $(GPU_AIEN_OM_MUTANTS); do n=$$((n+1)); name=$${m%%|*}; s=$${m#*|}; d=$(GPU_AIEN_MUT_DIR)/om-$$name; \
	rm -rf $$d; mkdir -p $$d; sed "$$s" native/aiengpu/context.om > $$d/context.om; \
	c=$$(diff native/aiengpu/context.om $$d/context.om | grep -c '^>'); \
	if [ "$$c" != 1 ]; then echo "MUTANT $$name BROKEN: '$$s' changed $$c lines, not 1"; exit 1; fi; \
	OMEGA_UNITS="aiengpu=$$(pwd)/$$d/context.om" OMEGA_UNITS_DEST=$$d OMEGA_OMC="$(OMEGA_OMC)" \
		OMEGA_BRIDGE="$(OMEGA_BRIDGE)" sh om/units.sh > $$d/units.log 2>&1 || { echo "MUTANT $$name: omc refused it"; cat $$d/units.log; exit 1; }; \
	gcc $(GPU_AIEN_CFLAGS) -o $$d/test tests/test_omega_gpu_platform_aien.c src/omega_gpu_platform_aien.c $(GPU_AIEN_OTHER_SRCS) \
		$(filter-out src/cudapp_om_aiengpu.c,$(GPU_AIEN_BINDING)) $$d/cudapp_om_aiengpu.c || exit 1; \
	t=0; AIEN_TRACE=$$d/trace $$d/test > $$d/test.log 2>&1 || t=1; \
	r=0; cmp -s $$d/trace $(GPU_AIEN_MUT_DIR)/oracle.trace || r=1; \
	if [ $$t = 0 ] && [ $$r = 0 ]; then echo "MUTANT $$name SURVIVED"; exit 1; fi; \
	k=$$((k+1)); echo "mutant $$name killed: test $$( [ $$t = 1 ] && echo failed || echo passed ), trace $$( [ $$r = 1 ] && echo differs || echo equal )"; done; \
	echo "test-gpu-platform-aien-om-mutants: $$k of $$n mutants killed"

# aien-gpu effects through device words (native/aiengpu/effects.om, AArch64 only, CUDApp #128):
#   make test-gpu-platform-aien-effects  (AArch64: part of test-gpu-platform-aien, so in `make test`; no Omega tools)
#       tests/test_omega_gpu_platform_aien_fx.c runs the platform's real effects against a memfd fake device whose
#       'A' ioctls a seccomp trap answers, once through effects.om (the committed src/cudapp_aiengpu_effects.c) and once
#       through the C system calls (CUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN_EFFECTS=1); AIEN_FX_TRACE of both (every
#       ioctl's bytes in and out, every nonzero line of the fake device) must be byte-identical. One C-glue mutant
#       (the unmap that never drops the unit's mapping) must fail the test or change the trace.
#   make test-gpu-platform-aien-effects-mutants  needs OMEGA_AIENGPU_BRIDGE (the bridge checkout named in
#       src/cudapp_aiengpu_effects.ident), OMEGA_NATIVE_OMC (generator omc 734f0a39) and OMEGA_NATIVE_OMC2 (device-word
#       omc b8f8f137); without them exit 3, NOT_RUN. Regenerates the unit into build/ and requires the committed C and
#       header to be byte-identical, then mutant effects.om files, each changing exactly one line, must fail the test
#       or change the trace.
GPU_AIEN_FX_DIR = build/gpu_aien_fx
GPU_AIEN_FX_OM_SRCS = $(filter-out src/cudapp_aiengpu_effects.c,$(GPU_AIEN_SRCS))
GPU_AIEN_FX_MUTANTS = \
	'doorbell-off|s/mapset db 144 4/mapset db 148 4/' \
	'gpput-off|s/fifo 8332 4/fifo 8336 4/' \
	'prefix-half|s/a = mapwrite pb 0 plan 0 mul pre 4/a = mapwrite pb 0 plan 0 mul pre 2/' \
	'destroy-len|s/eq req 3222815012 eq len b 24/eq req 3222815012 eq len b 25/'
GPU_AIEN_FX_DEPS = tests/test_omega_gpu_platform_aien_fx.c $(GPU_AIEN_DEPS)
build/test_omega_gpu_platform_aien_fx: $(GPU_AIEN_FX_DEPS)
	@mkdir -p build
	gcc $(GPU_AIEN_CFLAGS) -o $@ tests/test_omega_gpu_platform_aien_fx.c $(GPU_AIEN_SRCS)
build/test_omega_gpu_platform_aien_fx_c: $(GPU_AIEN_FX_DEPS)
	@mkdir -p build
	gcc $(GPU_AIEN_CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN_EFFECTS=1 -o $@ tests/test_omega_gpu_platform_aien_fx.c \
		$(GPU_AIEN_FX_OM_SRCS)
# judge NAME DIR: DIR/test must fail, or its trace must differ from the C system calls' ($(GPU_AIEN_FX_DIR)/c.trace).
GPU_AIEN_FX_JUDGE = judge() { name=$$1; t=0; AIEN_FX_TRACE=$$2/trace $$2/test > $$2/test.log 2>&1 || t=1; \
	  r=0; cmp -s $$2/trace $(GPU_AIEN_FX_DIR)/c.trace || r=1; \
	  if [ $$t = 0 ] && [ $$r = 0 ]; then echo "MUTANT $$name SURVIVED"; exit 1; fi; \
	  k=$$((k+1)); echo "mutant $$name killed: test $$( [ $$t = 1 ] && echo failed || echo passed ), trace $$( [ $$r = 1 ] && echo differs || echo equal )"; }
.PHONY: test-gpu-platform-aien-effects test-gpu-platform-aien-effects-mutants
test-gpu-platform-aien-effects: $(if $(GPU_AIEN_EFFECTS_SRCS),build/test_omega_gpu_platform_aien_fx build/test_omega_gpu_platform_aien_fx_c)
	@[ -n "$(GPU_AIEN_EFFECTS_SRCS)" ] || { echo "NOT_RUN: the aien-gpu effects unit is AArch64 only"; exit 3; }
	@set -e; d=$(GPU_AIEN_FX_DIR); mkdir -p $$d; rm -f $$d/c.trace $$d/om.trace; \
	AIEN_FX_TRACE=$$d/c.trace ./build/test_omega_gpu_platform_aien_fx_c; \
	AIEN_FX_TRACE=$$d/om.trace ./build/test_omega_gpu_platform_aien_fx; \
	cmp $$d/om.trace $$d/c.trace || { echo "AIEN EFFECTS DIFF FAIL: effects.om and C system-call traces differ"; exit 1; }; \
	echo "AIEN EFFECTS DIFF PASS: effects.om and C system-call traces identical ($$(grep -c '^ioctl ' $$d/om.trace) ioctls, $$(grep -c '^dev ' $$d/om.trace) device lines, $$(wc -c < $$d/om.trace) bytes)"; \
	k=0; $(GPU_AIEN_FX_JUDGE); md=$$d/c-glue-drop; rm -rf $$md; mkdir -p $$md; \
	sed 's/drop(g_om.ctx, &res, m->handle)/address(g_om.ctx, \&res, m->handle)/' src/omega_gpu_platform_aien.c > $$md/platform.c; \
	c=$$(diff src/omega_gpu_platform_aien.c $$md/platform.c | grep -c '^>' || true); \
	if [ "$$c" != 1 ]; then echo "MUTANT c-glue-drop BROKEN: changed $$c lines, not 1"; exit 1; fi; \
	gcc $(GPU_AIEN_CFLAGS) -o $$md/test tests/test_omega_gpu_platform_aien_fx.c $$md/platform.c $(GPU_AIEN_OTHER_SRCS) \
		$(GPU_AIEN_BINDING); \
	judge c-glue-drop $$md; \
	echo "test-gpu-platform-aien-effects: $$k of 1 C-glue mutants killed"
test-gpu-platform-aien-effects-mutants: test-gpu-platform-aien-effects
	@[ -n "$(OMEGA_AIENGPU_BRIDGE)" ] && [ -n "$(OMEGA_NATIVE_OMC)" ] && [ -n "$(OMEGA_NATIVE_OMC2)" ] || \
		{ echo "NOT_RUN: set OMEGA_AIENGPU_BRIDGE, OMEGA_NATIVE_OMC and OMEGA_NATIVE_OMC2"; exit 3; }
	@set -e; d=$(GPU_AIEN_FX_DIR); rm -rf $$d/gen; \
	pin=$$(tr ' ' '\n' < src/cudapp_aiengpu_effects.ident | sed -n 's/^bridge=//p'); \
	[ "$$(git -C "$(OMEGA_AIENGPU_BRIDGE)" rev-parse HEAD)" = "$$pin" ] || { echo "NOT_RUN: OMEGA_AIENGPU_BRIDGE is not at the pinned bridge $$pin"; exit 3; }; \
	sh native/aiengpu/units.sh --candidate $$d/gen; \
	cmp $$d/gen/cudapp_aiengpu_effects.c src/cudapp_aiengpu_effects.c && cmp $$d/gen/cudapp_aiengpu_native.h src/cudapp_aiengpu_native.h || \
		{ echo "AIEN EFFECTS REGEN FAIL: src/ differs from native/aiengpu/units.sh output"; exit 1; }; \
	echo "AIEN EFFECTS REGEN PASS: src/cudapp_aiengpu_effects.c and cudapp_aiengpu_native.h reproduce at bridge $$pin"; \
	n=0; k=0; $(GPU_AIEN_FX_JUDGE); \
	for m in $(GPU_AIEN_FX_MUTANTS); do n=$$((n+1)); name=$${m%%|*}; s=$${m#*|}; md=$$d/om-$$name; rm -rf $$md; mkdir -p $$md; \
	  sed "$$s" native/aiengpu/effects.om > $$md/effects.om; \
	  c=$$(diff native/aiengpu/effects.om $$md/effects.om | grep -c '^>' || true); \
	  if [ "$$c" != 1 ]; then echo "MUTANT $$name BROKEN: '$$s' changed $$c lines, not 1"; exit 1; fi; \
	  sh native/aiengpu/units.sh --candidate $$md/gen $$md/effects.om > $$md/units.log 2>&1 || { echo "MUTANT $$name: generation refused it"; cat $$md/units.log; exit 1; }; \
	  gcc -I$$md/gen $(GPU_AIEN_CFLAGS) -o $$md/test tests/test_omega_gpu_platform_aien_fx.c $(GPU_AIEN_FX_OM_SRCS) \
		$$md/gen/cudapp_aiengpu_effects.c; \
	  judge $$name $$md; done; \
	echo "test-gpu-platform-aien-effects-mutants: $$k of $$n mutants killed"

GPU_SESSION_MUT_DIR = build/gpu_session_mut
test-gpu-session-mutants: $(GPU_FS_LINK)
	@mkdir -p $(GPU_SESSION_MUT_DIR)
	@set -e; killed=0; \
	smut() { name=$$1; script=$$2; \
	  sed -e "$$script" src/omega_gpu_session.c > $(GPU_SESSION_MUT_DIR)/$$name.c; \
	  if cmp -s src/omega_gpu_session.c $(GPU_SESSION_MUT_DIR)/$$name.c; then echo "mutant $$name: pattern not found"; exit 1; fi; \
	  gcc $(GPU_FS_CFLAGS) -c -o $(GPU_SESSION_MUT_DIR)/$$name.o $(GPU_SESSION_MUT_DIR)/$$name.c; \
	  objs=$$(echo $(GPU_FS_OBJS) | tr ' ' '\n' | grep -v '/omega_gpu_session.o$$' | tr '\n' ' '); \
	  gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -DOMEGA_GPU_NO_POSIX -o $(GPU_SESSION_MUT_DIR)/$$name \
	    tests/test_omega_gpu_session_platform.c $$objs $(GPU_SESSION_MUT_DIR)/$$name.o; \
	  if ./$(GPU_SESSION_MUT_DIR)/$$name > $(GPU_SESSION_MUT_DIR)/$$name.log 2>&1; then echo "mutant $$name SURVIVED"; exit 1; fi; \
	  echo "mutant $$name killed: $$(grep -c '^FAIL' $(GPU_SESSION_MUT_DIR)/$$name.log) case(s) failed"; killed=$$((killed+1)); }; \
	smut release-when-latched 's|if (LOAD(g_blocked)) {$$|if (0) {|'; \
	smut ignore-cancel 's|if (LOAD(g_cancel)) return WAIT_CANCELLED;|;|g'; \
	smut no-latch-on-timeout 's|latch(w == WAIT_TIMED_OUT|omega_gpu_session_set_error(w == WAIT_TIMED_OUT|'; \
	smut stale-not-counted 's|else { BUMP(g_stats.stale_refusals, 1u); fail_plat("dma_free", rc); }|else {}|'; \
	smut marker-not-polled 's|RIGHTS_MARKER, PAGE|RIGHTS_DATA, PAGE|'; \
	smut close-while-latched 's|if (g_open \&\& !LOAD(g_blocked) \&\& p) {|if (g_open \&\& p) {|; s|if (!LOAD(g_blocked)) { /\* a hook|if (1) { /* a hook|'; \
	smut recover-keeps-latch 's|STORE(g_blocked, 0u);|;|'; \
	smut recover-without-close 's|const int prc = p->device_close(p->ctx, g_s.dev);|const int prc = OMEGA_GPU_PLAT_OK;|'; \
	smut recover-when-not-latched 's|const int latched = LOAD(g_blocked) \|\| LOAD(g_module_latched);|const int latched = 1;|'; \
	smut recover-ignores-teardown-fault 's|if (prc != OMEGA_GPU_PLAT_OK) {|if (0) {|'; \
	smut recover-ignores-module-latch 's|const int latched = LOAD(g_blocked) \|\| LOAD(g_module_latched);|const int latched = LOAD(g_blocked);|'; \
	smut legacy-per-marker-deadline 's|#define CUDAPP_MIGRATION_PENDING_C_SESSION_TIMEOUT 0|#define CUDAPP_MIGRATION_PENDING_C_SESSION_TIMEOUT 1|'; \
	grep -q '^FAIL SUBMISSION_DEADLINE_SHARED_BY_BOTH_MARKERS$$' $(GPU_SESSION_MUT_DIR)/legacy-per-marker-deadline.log; \
	echo "test-gpu-session-mutants: $$killed of 12 mutants killed"
