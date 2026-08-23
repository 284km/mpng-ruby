# PAIN.md — what this consumer forced

This gem is the first host-language consumer of Mere's `--lib` boundary: the
dogfood that made "a Mere function inside a foreign process" real. Everything
below fed the boundary's design; the version numbers are where each landed.

## P1 — there was no library output at all (v0.1.308)

Every Mere backend emitted one shape: a standalone executable with its own
`main`. The C backend's top-level functions were already externally visible
under predictable names — but `main`, a SIGSEGV handler, stdout rebuffering
and an atexit hook came with them, all process-global actions a loaded
library must never take in someone else's process. `mere -c --lib` emits the
boundary instead: init/shutdown/free plus one uncurried wrapper per
exportable entry-file function, everything else static.

## P2 — str and bytes could not cross (v0.1.309)

The first boundary spoke int/bool/float. A PNG codec speaks bytes. In and
out deliberately differ: inputs are borrowed (copied into the region during
the call), outputs are malloc'd copies the host owns — a region pointer never
crosses, because a later call may reclaim the region. Ruby Strings carry
NULs, so the gate round-trips a buffer with embedded NULs.

## P3 — a Mere `fail` called exit(1) in the host (v0.1.310)

Correct for a program, lethal for a library. Wrappers arm the same setjmp
guard `try_or` uses and hand the failure across as a status plus the
failure's own words; this gem turns that into a RuntimeError. Making the
guard per-thread exposed a latent bug that predates `--lib`: the jmp_buf was
one process-wide global, so a fail in one thread could longjmp into a
jmp_buf some other thread's try_or had armed.

## P4 — N calls used to cost N times what one costs (v0.1.311)

Containers whose region variable erased to `__heap` were pinned to the
default region — sound while "the process dies with the program", a leak the
moment the process outlives every call. Measured on small exports: 512 B per
call for a Vec, 1.5 KB for a Map, 226 MB over 100k calls. In lib mode they
follow the current region: per-call region during a call, default region
during module init (module state persists; copy-on-store covers values
stored across the seam). Decoding the same PNG 50 times and 2000 times now
reports byte-identical default-region stats.

## P5 — the consumer side was ~60 lines of Mere and ~80 of C

`libmpng.mere` adapts mpng's vec-based pipeline to bytes-in/bytes-out; the
whole codec came along via a package import, including the CLI's own file
(imports splice declarations, not the program body, so the CLI's `main`
rides along inert). The Ruby side is three thin functions. Nothing in mpng
itself changed — the adaptation cost lives entirely at the edge, which is
where it should live.
