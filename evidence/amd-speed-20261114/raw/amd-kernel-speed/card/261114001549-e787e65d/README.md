# AMD MI300X notebook session 2026-11-14T00:15:49Z (uxl-brief/AmdKernelSpeed, rung 2)

State: EXERCISED on hardware once. Unpublished, unreviewed, nothing landed.
- Session: jupyter-user-261114001549-e787e65d (AMD One-Click "Qwen/Qwen3.8-27B on AMD Instinct MI300X"), Chrome
  window AppleScript id 1612082477, driven by cell paste. Left running with the weights for AmdLaunchGap.
- Inputs: CUDApp dc536882 public clone. Published x86 launcher sha256 08a50491. Card-emitted real2.plan equals the
  published sha 2a292dad. Candidate objects rmsnorm5120 v2 7d63972f and gdnstate_step v2 c689890d (CPU gates in
  ../../gates/). Scripts: ../../measure/, card-measure.sh 5e36127b (attempt 2).
- Attempt 1 timed nothing: every run exited 74 because the weight paths were unexpanded. It is not downloaded.
- aks-results.tar.gz sha256 9894a97b9ddaa78c688aace5ce2d16814f435cadad0dbf43df6b77da3285baf2, identical on the card
  and the Mac. out/SHA256SUMS and out/runs/SHA256SUMS check.
- analysis.json: measure/analyze.py against the published AMD ids (card-evidence/uxl-native-run1.txt). ANALYZE PASS.
- Limits: one run per candidate; host wall clock per step; inserted dispatches run at the end of the step.
