# Matmul tolerance neighboring-float extension

The final differential also checks f32 output bits `0x3f800053`, `0x3f800054`, `0x3f800055` against the exact reference dot product 1.0. The first is below the source relative threshold 1e-5; the next two exceed it. Both default Omega and guarded C agree on exact error bits and mismatch count. This augments, and does not replace, the float-class differential.

WolframLanguageEvaluator query:

```wl
Table[{IntegerString[FromDigits["3f800000",16]+d,16],N[d/2^23,18],d/2^23>1/100000},{d,{83,84,85}}]
```

Result: `{{"3f800053",9.89437103271484375*^-6,False},{"3f800054",0.000010013580322265625,True},{"3f800055",0.00001013278961181640625,True}}`.

Repeated the same device-hidden `python3 tests/api_rest_check.py --mutants --out build/api-rest-check` command after adding this partition: rc 0, final `API-REST-CHECK PASS`, nine .om mutants KILLED and three strong-link omissions rejected. `threshold-commands.json` is this exact final run with home paths normalized. No implementation changed after the family-gates receipt.
