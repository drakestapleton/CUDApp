# DGX Spark, same request, 13 November 2026 19:13 to 19:19 UTC

The same lighthouse request that ran on the AMD card (`run/decode1.request.json`, 252 bytes, sha256 `14a9bcc11fe43d146b532770b60d67af6703bc161132d5ec8142ad988b2c1090`), sent to my engine on the Spark: one warmup, five timed requests, one streamed request, one at a time.

- Engine: `aien-serve-http`, sha256 `df9360112fdc815539f3e289fccfd8070845129ef4ccb466bb90c729ec71d0e5`, the same binary as the 1 November measurement.
- Checkpoint: `Qwen/Qwen3.8-27B-FP8` revision `017b9c7af6b5689d5dd426a76e0bc077eb5ca20a`, all files hash-checked before the run.
- Raw values and their definitions: `measurements.json`. Server log: `run/server.log`. Every request and response: `run/`. The script that drove it: `paired_runner.py`.

| Request | Whole request, s (client) | Graph time per token, ms (server) | Decode, ms for 256 tokens (server) | Prefill, ms (server) |
|---|---|---|---|---|
| warmup | 35.6159 | 135.7 | 35002 | 612 |
| decode1 | 35.6233 | 135.7 | 35018 | 603 |
| decode2 | 35.5472 | 135.5 | 34942 | 602 |
| decode3 | 35.6083 | 135.6 | 34973 | 633 |
| decode4 | 35.4961 | 135.2 | 34883 | 610 |
| decode5 | 35.6701 | 135.9 | 35058 | 607 |

Streamed request: first visible content at 0.6114 s, complete at 35.6284 s. Engine ready 49.8 s by its own log.

All generations returned the same 256 token ids as the 1 November response.

Limit: the machine was shared with other CPU work during this run (one-minute load average 10.50 to 16.34, against 3.43 to 4.16 during the 1 November measurement). The cause of the slower decode was not isolated.

The log files carry local folder paths from the machine that produced them.
