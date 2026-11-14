#!/bin/sh
# Re-run every recompute script, rebuild ../numbers.json and the session tables of ../REPORT.md, check the report against
# numbers.json, and prove the run is repeatable: a second complete run (every topic script, numbers.json, the session
# tables) must leave numbers.json, REPORT.md and every file under out/ byte-identical.
# With --verify-sums it also requires every file listed in ../SHA256SUMS to verify after the run (use it in a finished
# folder; while the report is being edited the list is stale by design).
# Exit 0 only if every step passes.
set -u
cd "$(dirname "$0")" || exit 2
rc=0
sum() { if command -v sha256sum >/dev/null 2>&1; then sha256sum "$@"; else shasum -a 256 "$@"; fi; }
build() {
    for s in baseline launcher kernels fourbit specdecode combined sessions; do
        python3 -B -E -s "$s.py" || rc=1
    done
    python3 -B -E -s build_numbers.py || rc=1
    python3 -B -E -s render_sessions.py || rc=1
}
snapshot() { sum ../numbers.json ../REPORT.md out/*; }
build
python3 -B -E -s check_report.py ../REPORT.md ../numbers.json claims.json || rc=1
python3 -B -E -s check_report.py ../REPORT.md ../numbers.json claims.json --selftest || rc=1
first=$(snapshot)
build >/dev/null
second=$(snapshot)
if [ "$first" = "$second" ]; then echo "REPEATABLE PASS a second complete run left numbers.json, REPORT.md and out/ ($(ls out | wc -l | tr -d ' ') files) byte-identical"
else echo "REPEATABLE FAIL a second complete run changed:"; printf '%s\n' "$first" > /tmp/runall-a.$$; printf '%s\n' "$second" > /tmp/runall-b.$$
     diff /tmp/runall-a.$$ /tmp/runall-b.$$ | sed -n 's/^> /  /p'; rm -f /tmp/runall-a.$$ /tmp/runall-b.$$; rc=1; fi
if [ "${1:-}" = "--verify-sums" ]; then
    if [ -f ../SHA256SUMS ]; then
        bad=$( (cd .. && sum -c SHA256SUMS 2>/dev/null) | grep -vc ': OK$')
        if [ "$bad" = "0" ]; then echo "SUMS PASS every file in SHA256SUMS verifies after the recompute"
        else echo "SUMS FAIL $bad files in SHA256SUMS do not verify after the recompute"; rc=1; fi
    else echo "SUMS FAIL ../SHA256SUMS not found"; rc=1; fi
fi
echo "RUN-ALL rc=$rc"
exit $rc
