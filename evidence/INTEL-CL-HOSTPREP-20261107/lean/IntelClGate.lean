/-
DRA-194 (kanban t_3bbd5f48): model of the MT_TARGET=3 exit decision of tools/multitarget/spirv-host.c and of one
line of `tools/access/intel-cl.sh compare`, on CUDApp branch hive/t_3bbd5f48-intel-cl-gpu.
Model-to-system link: hand-copied from main() (opt-in check), pin_cl_gpu(), pick_gpu(), run_cl() and the final
isolation() call, one Bool per check, in the C's order; and from do_compare()'s verdict branch. The theorems speak
for this model only, not for the compiled launcher (that is what tests/multitarget/run.sh exercises).
Checked with the Spark `lean` MCP (Lean v4.35.0, Mathlib v4.35.0; no Mathlib import needed).
-/

def spirvHost3 (optin icdAbs icdForbidden icdResolves realForbidden icdIsElfLib onePlatform gpuFound devIsGpu
    vendorOk nameOk runOk isoInside isoFinal canWrite : Bool) : Nat :=
  if !optin then 2
  else if !icdAbs then 3
  else if icdForbidden then 9
  else if !icdResolves then 3
  else if realForbidden then 9
  else if !icdIsElfLib then 3
  else if !onePlatform then 5
  else if !gpuFound then 5
  else if !devIsGpu then 5
  else if !vendorOk then 5
  else if !nameOk then 5
  else if !runOk then 6
  else if !isoInside then 9
  else if !isoFinal then 9
  else if !canWrite then 4
  else 0

/- An output is written (exit 0) only with the opt-in, a GPU-typed device (no CPU fallback), an ICD path and
realpath free of forbidden entries, an ICD that is a regular ELF library (not a directory or .icd descriptor),
and both isolation scans passing. -/
set_option synthInstance.maxSize 16384 in
set_option synthInstance.maxHeartbeats 2000000 in
set_option maxRecDepth 200000 in
theorem output_needs_optin_gpu_isolation :
    ∀ o a f r rf el p g d v n ru ii fi w : Bool,
      spirvHost3 o a f r rf el p g d v n ru ii fi w = 0 →
      o = true ∧ g = true ∧ d = true ∧ f = false ∧ rf = false ∧ el = true ∧ ii = true ∧ fi = true := by
  decide

/- Without MT_CL_GPU=1 the exit is 2 whatever else holds (checked first; nothing is opened). -/
set_option synthInstance.maxSize 16384 in
set_option synthInstance.maxHeartbeats 2000000 in
set_option maxRecDepth 200000 in
theorem no_optin_exit2 :
    ∀ a f r rf el p g d v n ru ii fi w : Bool, spirvHost3 false a f r rf el p g d v n ru ii fi w = 2 := by
  decide

/-- Negative control 1: the same launcher with the two GPU checks removed (a CPU fallback). -/
def spirvHost3NoGpuCheck (optin icdAbs icdForbidden icdResolves realForbidden icdIsElfLib onePlatform _gpuFound
    _devIsGpu vendorOk nameOk runOk isoInside isoFinal canWrite : Bool) : Nat :=
  if !optin then 2
  else if !icdAbs then 3
  else if icdForbidden then 9
  else if !icdResolves then 3
  else if realForbidden then 9
  else if !icdIsElfLib then 3
  else if !onePlatform then 5
  else if !vendorOk then 5
  else if !nameOk then 5
  else if !runOk then 6
  else if !isoInside then 9
  else if !isoFinal then 9
  else if !canWrite then 4
  else 0

set_option synthInstance.maxSize 16384 in
set_option synthInstance.maxHeartbeats 2000000 in
set_option maxRecDepth 200000 in
theorem no_gpu_check_violates :
    ¬ (∀ o a f r rf el p g d v n ru ii fi w : Bool,
      spirvHost3NoGpuCheck o a f r rf el p g d v n ru ii fi w = 0 → d = true) := by
  decide

/-- intel-cl.sh compare, one line: 0 pass, 1 fail, 2 not run. -/
def cmpLine (hasRow hasOut vmPass vmNotRun conformEqWant : Bool) : Nat :=
  if !hasRow then 2
  else if !hasOut then (if vmNotRun then 2 else 1)
  else if conformEqWant && vmPass then 0
  else 1

/-- A compare PASS needs the output, conform.om's exit equal to the row's want, and the VM row PASS. -/
theorem compare_pass_means_agreement :
    ∀ r o vp vn c : Bool, cmpLine r o vp vn c = 0 → c = true ∧ vp = true ∧ o = true := by
  decide

/-- Negative control 2: trusting the VM verdict alone lets bytes conform.om rejects pass. -/
def cmpVmOnly (vmPass _conformEqWant : Bool) : Nat := if vmPass then 0 else 1
theorem vm_only_violates : ¬ (∀ vp c : Bool, cmpVmOnly vp c = 0 → c = true) := by decide

#print axioms output_needs_optin_gpu_isolation
#print axioms no_optin_exit2
#print axioms no_gpu_check_violates
#print axioms compare_pass_means_agreement
#print axioms vm_only_violates
