#!/bin/sh
# Test-only, per-source-site input-demand alarms. No device or product cap.
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
exec python3 -I "$root/tests/amd/loop-sites.py" files "$root" "$@"
