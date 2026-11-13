# launch2 relay encoding follow-up

Intent: fit the notebook relay's cell-size boundary without changing the launcher or any code object.

The generator and notebook decoder now use xz (LZMA extreme preset) and base85 for each asset. `receipt.json` records the source revision, commands, observed exits, cell dimensions and hashes. `asset-identity.json` compares every decoded asset with the previously accepted bundle; `notebook-check.json` records the rerun of the existing notebook CPU gate. The regenerated bundle is preserved under `bundle/` with its generated README and checksum manifest.

Earlier launcher CPU qualification remains in `../AMD-KFD-LAUNCH2/`; its historical evidence is unchanged. This follow-up changes encoding only, not launcher/device behavior. GPU execution, independent final-source review, merge and deployment remain NOT_RUN.
