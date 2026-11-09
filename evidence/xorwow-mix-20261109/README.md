# XORWOW: what the page-5 listing tunes, and an output mix (L441)

Host only, 2026-11-09, Spark `spark-b87b` (aarch64). No GPU, no libcurand call.

## The listing

Marsaglia, "Xorshift RNGs", Journal of Statistical Software 8(14), 2003, page 5. CUDA 13.0.3
`curand()` is the same step (earlier check on CUDApp branch `feat/fft-rng` at `f564d78b`,
`docs/LIBRARIES.md` line 18). State words x, y, z, w, v and the Weyl counter d, 32 bits each:

    t = x ^ (x >> 2); x = y; y = z; z = w; w = v;
    v = (v ^ (v << 4)) ^ (t ^ (t << 1));
    return (d += 362437) + v;

## What it tunes (Wolfram receipts)

1. The shift triple (a, b, c) = (2, 1, 4). The 160 by 160 GF(2) matrix of the xorshift part has a
   primitive characteristic polynomial, so the five words run through all 2^160 - 1 nonzero states.
   Neighbouring triples do not: (2, 1, 5), (1, 1, 1) and (1, 3, 10) are not primitive.
2. The Weyl increment 362437 is odd, so d runs through all 2^32 values.
3. 2^160 - 1 is odd, so gcd(2^160 - 1, 2^32) = 1 and the combined period is
   (2^160 - 1) * 2^32 = 2^192 - 2^32.

Query 1 (WolframLanguageEvaluator, 2026-11-09):

```
n=32; id=IdentityMatrix[n]; z0=ConstantArray[0,{n,n}]; L[k_]:=Table[If[j==i-k,1,0],{i,0,n-1},{j,0,n-1}]; R[k_]:=Table[If[j==i+k,1,0],{i,0,n-1},{j,0,n-1}]; T[a_,b_,c_]:=Mod[ArrayFlatten[{{z0,id,z0,z0,z0},{z0,z0,id,z0,z0},{z0,z0,z0,id,z0},{z0,z0,z0,z0,id},{Mod[(id+L[b]).(id+R[a]),2],z0,z0,z0,Mod[id+L[c],2]}}],2]; cp[a_,b_,c_]:=PolynomialMod[CharacteristicPolynomial[T[a,b,c],xx],2]; N0=2^160-1; qs=FactorInteger[N0][[All,1]]; pw[e_,p_]:=Fold[PolynomialMod[If[#2==1,xx*#1^2,#1^2],{p,2}]&,1,IntegerDigits[e,2]]; prim[p_]:=Exponent[p,xx]==160&&IrreduciblePolynomialQ[p,Modulus->2]&&pw[N0,p]===1&&AllTrue[qs,pw[N0/#,p]=!=1&];res=Table[{abc,prim[cp@@abc]},{abc,{{2,1,4},{2,1,5},{1,1,1},{1,3,10}}}];{FactorInteger[N0],res,Mod[362437,2],(2^160-1)*2^32==2^192-2^32,GCD[2^160-1,2^32]}
```

Result:

```
{{{3,1},{5,2},{11,1},{17,1},{31,1},{41,1},{257,1},{61681,1},{65537,1},{414721,1},{4278255361,1},{44479210368001,1}},
 {{{2,1,4},True},{{2,1,5},False},{{1,1,1},False},{{1,3,10},False}}, 1, True, 1}
```

(The kernel printed a lint warning that the single-letter names L, R, T were "undefined"; the
values are computed. Query 3 below rebuilds the same matrix with longer names and no warning.)

## The named metric

Linear complexity of each output bit, by Berlekamp-Massey over GF(2), over the first N = 1000
outputs from the published state 123456789, 362436069, 521288629, 88675123, 5783321, 6615241.
An unpredictable bit scores about N/2 = 500. A bit fails when it scores below 480. The score for
the whole generator is the number of failing bits out of 32; lower is better, 0 is ideal.

The low bits are weak by construction. Bit 0 of `d + v` is bit 0 of d XOR bit 0 of v: no carry
reaches it. Bit 0 of v is a 160-stage linear recurrence and bit 0 of d alternates, so bit 0 is a
linear recurrence of length 162. Bit 1 gets one carry, v0 AND d0, and has length 323. The carries
reach every higher bit through d, which is why bits 2 to 31 already score about 500.

Query 2 (the scores, N = 1000), result:

| Output | bit 0 | bit 1 | bits 2..31 | failing bits |
|---|---|---|---|---|
| raw XORWOW `d + v` | 162 | 323 | 499..502 | 2 |
| fold `o ^ (o >> 16)` | 500 | 501 | 499..502 | 0 |
| murmur3 fmix32 of `o` | 500 | 500 | 497..503 | 0 |
| Wolfram `RandomInteger` words | 500 | 501 | 497..501 | 0 |

```
msk=2^32-1; stp[{xa_,ya_,za_,wa_,va_,da_}]:=Module[{t=BitXor[xa,BitShiftRight[xa,2]],nv,nd},nv=BitAnd[BitXor[BitXor[va,BitShiftLeft[va,4]],BitXor[t,BitShiftLeft[t,1]]],msk];nd=Mod[da+362437,2^32];{ya,za,wa,va,nv,nd}]; num=1000; sts=Rest[NestList[stp,{123456789,362436069,521288629,88675123,5783321,6615241},num]]; raw=Mod[#[[5]]+#[[6]],2^32]&/@sts; fold[o_]:=BitXor[o,BitShiftRight[o,16]]; fmix[h0_]:=Module[{h=h0},h=BitXor[h,BitShiftRight[h,16]];h=Mod[h*16^^85ebca6b,2^32];h=BitXor[h,BitShiftRight[h,13]];h=Mod[h*16^^c2b2ae35,2^32];BitXor[h,BitShiftRight[h,16]]]; bm[s_]:=Module[{nn=Length[s],c,b,lc=0,m=-1,dd,tt,sh},c=ConstantArray[0,nn+1];c[[1]]=1;b=c;Do[dd=Mod[c[[1;;lc+1]].Reverse[s[[i-lc+1;;i+1]]],2];If[dd==1,tt=c;sh=i-m;c=Mod[c+Join[ConstantArray[0,sh],b[[1;;nn+1-sh]]],2];If[2 lc<=i,lc=i+1-lc;m=i;b=tt]],{i,0,nn-1}];lc]; lcs[out_]:=Table[bm[BitAnd[BitShiftRight[out,k],1]],{k,0,31}]; lr=lcs[raw]; lf=lcs[fold/@raw]; lm=lcs[fmix/@raw]; rnd=lcs[RandomInteger[msk,num]]; {Take[raw,8], lr, lf, lm, rnd, Count[#,v_/;v<480]&/@{lr,lf,lm,rnd}}
```

The first eight raw outputs it printed are 246875399 3690007200 1264581005 3906711041 1866187943
2481925219 2464530826 1604040631, the same eight as the earlier C and Wolfram listing check. The
full 32-score rows for raw and fold are `expected.txt` in this folder. Failing-bit counts:
`{2, 0, 0, 0}`.

Query 3 proves the two low-bit scores exactly: the Berlekamp-Massey minimal polynomial of bit 0
equals p(x) (x + 1)^2 and that of bit 1 equals p(x)^2 (x + 1)^3, where p is the degree-160
characteristic polynomial of the (2, 1, 4) matrix. Result `{162, 323, True, True, 160}`.

```
n=32; id=IdentityMatrix[n]; z0=ConstantArray[0,{n,n}]; shl[k_]:=Table[If[j==i-k,1,0],{i,0,n-1},{j,0,n-1}]; shr[k_]:=Table[If[j==i+k,1,0],{i,0,n-1},{j,0,n-1}]; tm=Mod[ArrayFlatten[{{z0,id,z0,z0,z0},{z0,z0,id,z0,z0},{z0,z0,z0,id,z0},{z0,z0,z0,z0,id},{Mod[(id+shl[1]).(id+shr[2]),2],z0,z0,z0,Mod[id+shl[4],2]}}],2]; p=PolynomialMod[CharacteristicPolynomial[tm,xx],2]; msk=2^32-1; stp[{xa_,ya_,za_,wa_,va_,da_}]:=Module[{t=BitXor[xa,BitShiftRight[xa,2]],nv},nv=BitAnd[BitXor[BitXor[va,BitShiftLeft[va,4]],BitXor[t,BitShiftLeft[t,1]]],msk];{ya,za,wa,va,nv,Mod[da+362437,2^32]}]; sts=Rest[NestList[stp,{123456789,362436069,521288629,88675123,5783321,6615241},1000]]; raw=Mod[#[[5]]+#[[6]],2^32]&/@sts; bmc[s_]:=Module[{nn=Length[s],c,b,lc=0,m=-1,dd,tt,sh},c=ConstantArray[0,nn+1];c[[1]]=1;b=c;Do[dd=Mod[c[[1;;lc+1]].Reverse[s[[i-lc+1;;i+1]]],2];If[dd==1,tt=c;sh=i-m;c=Mod[c+Join[ConstantArray[0,sh],b[[1;;nn+1-sh]]],2];If[2 lc<=i,lc=i+1-lc;m=i;b=tt]],{i,0,nn-1}];{lc,c[[1;;lc+1]]}]; minp[{lc_,c_}]:=Sum[c[[k+1]] xx^(lc-k),{k,0,lc}]; r0=bmc[BitAnd[raw,1]]; r1=bmc[BitAnd[BitShiftRight[raw,1],1]]; {r0[[1]],r1[[1]],PolynomialMod[minp[r0]-p*(xx+1)^2,2]===0,PolynomialMod[minp[r1]-p^2*(xx+1)^3,2]===0, Exponent[p,xx]}
```

Query 4 (N = 3000, bits 0..7): raw `{162, 323, 1500, 1501, 1499, 1500, 1501, 1501}`, fold
`{1501, 1501, 1501, 1499, 1501, 1500, 1501, 1500}`. Bits 0 and 1 do not grow with N: 162 and 323
are their full linear complexity. The fold's 32 by 32 GF(2) matrix has determinant 1 mod 2
(`Det[foldM, Modulus->2]` -> 1), so the fold is a bijection on 32-bit words: it keeps the period
and the uniform distribution of each output word.

Query 4 is query 2 with `num=3000`, `lcs` over bits `{k,0,7}`, only `raw` and `fold`, and
`foldM=Table[If[j==i||j==i+16,1,0],{i,0,31},{j,0,31}]`; it returned `{lr, lf, Det[foldM,Modulus->2]}`.

## Answer

The fold `o ^ (o >> 16)` wins on the named metric: 0 failing bits against 2 for raw XORWOW, at the
cost of one shift and one XOR per output. murmur3 fmix32 also scores 0 but costs two more
shift-XOR pairs and two multiplies, so the fold is the cheaper winner. Scope of the claim: this
one metric, this seed, N = 1000 and 3000. It is not a TestU01 BigCrush result. The folded
generator no longer reproduces
`curand()`, so it can only be an opt-in variant; the curand-matching step stays unmixed.

## Omega check

`omega/libraries/xorwow_mix.om` computes the same 64 scores in Omega and exits 0 only when raw
bit 0 is 162, raw bit 1 is 323, raw has exactly 2 failing bits and the fold has 0.
`run.sh` builds it with the pinned compiler, compares stdout byte for byte with `expected.txt`
(the Wolfram scores) and builds three mutants that must fail:

| Mutant | Change | Required exit |
|---|---|---|
| weyl | increment 362438 | 10 (first eight outputs) |
| nofold | fold returns `o` | 30 (fold has failing bits) |
| bmlen | Berlekamp-Massey never grows its length | 20 (raw bit 0 not 162) |

Receipt `run.log` (source commit `7863601676150a18241e39d75e364f9805270f1c`): omc
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`, omega-mv `e5b32567` rebuilt
from seed by `sh seed/omega-from-seed.sh` (RC=0), the pin in `AGENTS.md`. Binary sha256
`57a7667907f491e1d576a672b205f7937734b035b717a0550f468c11bf4cc2cc`. All rows PASS, exit 0.
With a different compiler (omc `3ad9f10f…`) `run.sh` prints NOT_RUN and exits 3. Compiled
directly by that omc (omega-mv `f4df8239`) the program is byte-identical (same binary sha256)
and its stdout equals `expected.txt`.
