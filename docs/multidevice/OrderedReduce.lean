/- DRA-203 two-device ordered reduction (CUDApp docs/multidevice/PLAN.md section 4).
  Model: the aien-train v1 step gradient GRD = block 0, then GRD := f GRD block_j for j = 1..G-1
  in ascending j (docs/omegallm/CONTRACT-V1.md section 5), with f an ARBITRARY binary operation
  (IEEE addition is not associative, so nothing below assumes associativity or commutativity).
  Rank 0 holds blocks 0..G/2-1 (head x0, tail xt), rank 1 holds blocks G/2..G-1 (ys).
  Protocol: rank 0 folds its blocks to S0 and sends S0; rank 1 folds its blocks onto S0.
  Checked 2026-11-07 with the Spark `lean` MCP (lean_run_code), Lean v4.35.0, core only (no imports):
    'ordered2_eq_single' depends on axioms: [propext, Quot.sound]
    'naive2_differs' does not depend on any axioms
    'swapped2_differs' does not depend on any axioms
    'ordered2_same_data' does not depend on any axioms
    #eval -> (26, 18, 44)
  Model-to-system link: the fold order is copied by hand from aien-train main 0518c915
  docs/omegallm/CONTRACT-V1.md section 5 ("GRD = block 0; then for j = 1..G-1, GRD[i] = GRD[i] +
  block_j[i]"); no implementation of the two-device protocol exists yet, so this proves the design only. -/

/-- single-device step gradient over all blocks -/
def single {α : Type} (f : α → α → α) (x0 : α) (rest : List α) : α := rest.foldl f x0

/-- the two-device ordered reduction: rank 1 continues rank 0's fold -/
def ordered2 {α : Type} (f : α → α → α) (x0 : α) (xt ys : List α) : α :=
  ys.foldl f (xt.foldl f x0)

/-- SAFETY: for every operation f and every split of the blocks, the two-device result equals the
    single-device result exactly (no tolerance needed). -/
theorem ordered2_eq_single {α : Type} (f : α → α → α) (x0 : α) (xt ys : List α) :
    ordered2 f x0 xt ys = single f x0 (xt ++ ys) := by
  simp [ordered2, single, List.foldl_append]

/-- the naive combination: each rank folds its own half, then the two partial sums are added -/
def naive2 {α : Type} (f : α → α → α) (x0 : α) (xt : List α) (y0 : α) (yt : List α) : α :=
  f (xt.foldl f x0) (yt.foldl f y0)

/-- rank order swapped: rank 1's blocks first -/
def swapped2 {α : Type} (f : α → α → α) (x0 : α) (xt : List α) (y0 : α) (yt : List α) : α :=
  (yt ++ x0 :: xt).foldl f y0

/-- a non-associative, non-commutative operation standing in for rounded addition -/
def g (a b : Nat) : Nat := 2 * a + b

/-- NEGATIVE CONTROL 1: the naive partial-sum combination is NOT the single-device result in general -/
theorem naive2_differs : naive2 g 1 [2] 3 [4] ≠ single g 1 ([2] ++ 3 :: [4]) := by decide

/-- NEGATIVE CONTROL 2: swapping the rank order changes the result, so a rank-swap mutant is detectable -/
theorem swapped2_differs : swapped2 g 1 [2] 3 [4] ≠ single g 1 ([2] ++ 3 :: [4]) := by decide

/-- the protocol is not vacuous on the same data: it does equal the single-device fold -/
theorem ordered2_same_data : ordered2 g 1 [2] (3 :: [4]) = single g 1 ([2] ++ 3 :: [4]) := by decide

#print axioms ordered2_eq_single
#print axioms naive2_differs
#print axioms swapped2_differs
#print axioms ordered2_same_data
#eval (single g 1 [2,3,4], naive2 g 1 [2] 3 [4], swapped2 g 1 [2] 3 [4])
