# agent/decompile-100 baseline — rebuild harness reproduces retail

Date: 2026-09-19
Branch: `agent/decompile-100` (worktree `/tmp/pe-agent-decomp`, base `3b302ed`)

## Purpose

Confirm, before any new decompilation work, that the Disc 1 rebuild/verify
harness reproduces the retail executable byte-for-byte from the current YAML.
This is the prerequisite for every "matched" claim that follows.

## Environment

A `git worktree` does not carry git-ignored prerequisites, so the worktree was
made self-contained by copying the retail-derived and toolchain inputs:

- `build/extracted/disc1/SLUS_006.62` (input, git-ignored)
- `asm/disc1/` and `include/*.inc` (split output, git-ignored)
- `tools/era/gcc-2.7.2-psx` (era compiler, git-ignored)

`tools/era/maspsx` needed repair: `scripts/setup_era.sh` clones floating
upstream, whose `maspsx.py` no longer matches the repo-tracked patched
`maspsx/__init__.py`. Installed upstream `025620f^` (the last revision before
`--passthrough`, matching the tracked module) and preserved the tracked patched
`maspsx/__init__.py` and its two local tests.

Build container: `localhost/pe-mipsel-img:latest` built from
`dev/mipsel/Dockerfile` (Debian trixie, binutils 2.44, gcc-mipsel 14.2.0).

## Commands and results

Rebuild:

```sh
podman run --rm --userns=keep-id -v /tmp/pe-agent-decomp:/workspace:Z \
  -w /workspace localhost/pe-mipsel-img:latest bash scripts/build_us.sh
```

Observed (tail):

```
  OK original EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  OK YAML plan: 847 spans, 560 C leaves, geometry 0x1EE000
  ...
  orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  RESULT: EXACT MATCH
  Matching claim: YES (560 registered C leaves)
BUILD EXIT: 0
```

Verify (run inside the `pe-mipsel` distrobox, which shares the home filesystem
so the worktree `.git` file resolves; git and python3 were installed there):

```sh
distrobox enter pe-mipsel -- bash -lc \
  'cd /tmp/pe-agent-decomp && bash scripts/verify_us.sh'
```

Observed:

```
[1/7] PASS 847 spans = 560 c + 285 asm + 2 rodata
[2/7] PASS 560 YAML C spans -> source -> object -> verify span
[3/7] PASS every YAML C source is tracked; published matching count equals YAML
[4/7] PASS plan SHA-256 9372a0966a947a20e2c5bfc469acab1ece101c30ba7bd9f96396990255e69434
[5/7] PASS all YAML-derived split sources exist and are ignored
[6/7] PASS scripts/split_us.sh --check
[7/7] PASS retail SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
      PASS candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
      PASS all 560 packed C spans equal retail
VERIFY_US=PASS
matching-C count: 560 (from YAML)
```

Result: **harness confirmed; retail SHA-1 reproduced exactly.** Baseline is
560 matching C leaves, 285 asm spans, 2 rodata.

## Remaining-coverage survey (from the split)

- Total asm bytes: `0x1B4AE8` (1,788,648).
- Recognized function text inside those spans: `0x976D8` (620,248 bytes,
  1,826 `nonmatching func_*` symbols including `nop`-only artifacts).
- The single YAML span `0xC5060` is 1,216,416 bytes, but only its first
  `0xBFA8` bytes (84 functions, `func_800D4860`..`func_800E051C`) are code;
  the rest (`0xD1008`..`0x1EE000`) is a large `dlabel` data/rodata region.
  Raw span size therefore overstates the code work by ~25x.
- Other large all-code spans: `120D8` (31,408), `AB74` (27,528), `19DE4`
  (24,968), `BF0F0` (24,416), `55C00` (20,924), `B3390` (17,200),
  `3420` (16,856), `37CD0` (16,484), `486D8` (14,776), `340EC` (14,544).
