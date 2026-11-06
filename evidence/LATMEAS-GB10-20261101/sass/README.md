`sm121dis --pairs` on the machine-code bins that the 2026-11-01 launch ran. Each name is `<case>_<stall>.pairs`. The stall is the smallest one that matched, and the stall under it.

The tool exits 1 because the two CS2R clock sites are unknown to the disassembler. Both sites are in the listing as `??`. The second hex column's high 32 bits are the control word. The stall is bits 9..12 of that word.
