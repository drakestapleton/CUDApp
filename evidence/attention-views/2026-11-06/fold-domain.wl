(* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception *)
(* Finite normalized fold gaps are nonpositive; the oracle's positive clamp
   is retained from the scalar helper but is unreachable in this call domain. *)
Reduce[x > 127 && x <= 0, x, Reals]
(* WolframLanguageEvaluator observed result: False *)
