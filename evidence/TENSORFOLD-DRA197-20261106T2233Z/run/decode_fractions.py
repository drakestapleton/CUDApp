#!/usr/bin/env python3
"""Per-kernel GPU time in the decode window of a TensorFold nsys profile.

Input: the SQLite export of prof.nsys-rep (`nsys export --type sqlite` or `nsys stats` creates it).
The decode window is the last run of kernels with no gap above 30 ms (load-time repacks come first).
Prints the window's wall and busy time and one row per (kernel, grid, block) sorted by total time.
Host analysis adapter for DRA-197 evidence; it reads timings only and stores no code listing.
"""
import collections
import sqlite3
import sys

GAP_NS = 30_000_000

db = sqlite3.connect(sys.argv[1])
rows = db.execute(
    "select k.start, k.end, s.value, k.gridX, k.gridY, k.gridZ, k.blockX "
    "from CUPTI_ACTIVITY_KIND_KERNEL k join StringIds s on s.id = k.demangledName order by k.start"
).fetchall()
window = [rows[0]]
for r in rows[1:]:
    if r[0] - window[-1][1] > GAP_NS:
        window = [r]
    else:
        window.append(r)
wall = window[-1][1] - window[0][0]
busy = sum(e - s for s, e, *_ in window)
agg = collections.defaultdict(lambda: [0, 0])
for s, e, name, gx, gy, gz, bx in window:
    a = agg[(name, gx, gy, gz, bx)]
    a[0] += e - s
    a[1] += 1
print(f"window_wall_ns\t{wall}\nwindow_busy_ns\t{busy}\nkernels\t{len(window)}")
print("total_ns\tcalls\tavg_ns\tgrid\tblock\tname")
for (name, gx, gy, gz, bx), (t, n) in sorted(agg.items(), key=lambda kv: -kv[1][0]):
    print(f"{t}\t{n}\t{t // n}\t{gx},{gy},{gz}\t{bx}\t{name}")
