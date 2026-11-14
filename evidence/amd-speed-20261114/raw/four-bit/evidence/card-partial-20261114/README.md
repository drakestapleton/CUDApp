# PARTIAL card evidence, 2026-11-14T01:21–01:26Z (session jupyter-user-261114011836-f591aa44)

State: PARTIAL. AmdLaunchGap killed the session by mistake at about 01:26Z, while card_q4.py was still running.
Nothing was downloaded from the card. The only record is the text of card_q4.log as read through the Jupyter
contents API at 01:24:40Z.

`card_q4.log.partial` holds the five KERNEL lines exactly as read. Each line was cut at 900 characters by the reader,
which lost the FP8 baseline fields. The other lines (ENV, WEIGHTS) are in the session transcript only.

`parsed.json` holds the q4gemv_d8 and q4gemv_d8r2 fields recovered from those lines: timings, bit verdicts, and
relative error against the float64 source.

Run identity:
- bundle sha256 `10c93d3d65c03e0f294075fc35c50479369c90f53cece416132c67e43c6a6574` (card/cells/BUNDLE.json);
- card ENV: gfx942, "AMD Instinct MI300X VF", 304 CUs, numpy 2.4.4;
- weights: 5 files sha256-verified at rev 017b9c7a.

Not covered:
- FP8/BF16 baseline timings;
- the shapes in_proj_qkv, attn q/k/o and lm_head;
- the container sha256 comparison;
- SHA256SUMS and the tarball.
