# API shells chip runbook

Status: NOT_RUN. No GPU window is authorized for this port. Do not treat a host or simulator result as a chip result.

The decisions in `om/api/*.om` do not launch kernels. A chip run exercises the existing family binaries after the host differential has passed. Hold the GPU lock the operator uses (`flock -w 900 /tmp/aien-gb10.lock make hello-gpu`) and run each command from an empty environment except the variables that command documents.

```
flock -w 900 /tmp/aien-gb10.lock make hello-gpu
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_fp8_slots_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_fp8gemm_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_fp8_api_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_decode_attn256_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_prefill_attn_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_prefill_attn256_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_moe_window_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_tern_sched_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_tcgemm_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_pfgemm_test
  flock -w 900 /tmp/aien-gb10.lock ./build/gpu_pfhost_test
```

Host-only and `--sim` modes of those binaries are not this runbook. They never open the device. Record each chip command's exit code, the last line, and the lock wait. A missing device is NOT_RUN, not a failure of the Omega decisions.

Do not touch port 18091, the nvidia modules, vfio, or `/tmp/aien-gb10.lock` except through the operator's lock command above.
