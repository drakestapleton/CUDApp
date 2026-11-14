# Combined MI300X run, session jupyter-user-261114020544-6e585e21 (launched ~02:05:44Z), uxl-brief/AmdKernelSpeed

State: EXERCISED on hardware once (3 interleaved repetitions per arm). Unreviewed (reviews deferred to the end by
Drake), nothing landed or published. Route A: omp-managed Chromium, Jupyter API; Drake's Chrome not touched.
- Archive akc-results.tar.gz sha256 493d6c29b79c7474659a9f69f877dcd8b597ffbb95a03b8230c5d21d57da9cee (card = Mac);
  out/SHA256SUMS all OK; every run file mirrored during the run equals the archive copy.
- Inputs (objects.sha256): launch-x86 08a50491 (published), launch-new de67f27e (AmdLaunchGap plan3, 9076c537 /
  139bb6a9), rmsnorm5120 v2 7d63972f, gdnstate_step v2 c689890d, fp8gemv_b128f v2b1 00686ea5 / v2b2 21f4265d
  (GpuBenchSync, taken over). Card-emitted real2.plan = published 2a292dad. Every launch under timeout 150 s: no hang.
- analysis.json: analyze-combined.py; all 21 timed runs print the 256 published AMD ids; the altered-weights control
  (plan3 + all v2 objects) differs.
