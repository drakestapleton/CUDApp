# Chip qualification

NOT_RUN. No GPU authorization was granted to this lane. No device execution,
performance measurement, speedup, hardware accuracy or chip qualification is
claimed. Host simulation and generated-code checks are separate evidence tiers.

The host acceptance executables use link-time abort guards on session open,
lock, unlock and free (also allocation). Their `--sim` mode cannot reach a real
session. The explicit `--shell-contract` mode uses a memory-only fake backend
solely to prove pointer-success and failed-open/failed-allocation control flow.
A compiled direct-open source mutant must terminate with SIGABRT under `--sim`.

Hardware qualification remains the integrator/operator's separately authorized
responsibility. This lane does not run a full suite, merge or deploy.
