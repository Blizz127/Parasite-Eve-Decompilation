# Decompilation coverage ceiling (Disc 1)

Measured 2026-09-20 (refreshed after the 813-leaf merge) with `tools/analysis/triage_m2c.py`, `tools/analysis/port_backed_worklist.py`
and `tools/analysis/asm_function_worklist.py` against the retail SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Regenerate with:

```sh
bash scripts/split_us.sh
python3 tools/analysis/asm_function_worklist.py
python3 tools/analysis/port_backed_worklist.py
python3 tools/analysis/triage_m2c.py --jobs 8
```

## The denominator

`asm/disc1/*.s` contains exactly **1523** `nonmatching <name>, <size>` function
boundaries (592,168 bytes) and 447 sizeless `nonmatching <name>` data
symbols. With the **870** functions already registered as `c` spans in
`configs/USA/disc1.yaml`, the code inventory is:

| | functions | bytes |
|---|---:|---:|
| matched C leaves | 870 | 53,004 |
| remaining | 1,523 | 592,168 |
| **total code** | **2,393** | **645,172** |

So current coverage is **36.4% of functions** but only **8.2% of code bytes** —
the leaves landed so far are small (avg 57 B) while the remaining queue is
dominated by large routines (avg 389 B). Byte coverage is the honest progress
metric for "100% decompiled"; the `4459` figure quoted in older handoff entries
is not a function count and should not be used.

## How much of the remainder can ever be a C leaf

`triage_m2c.py` runs m2c over all 1622 remaining functions and classifies each
body. A `handwritten` verdict means the body contains MIPS that no C compiler
can be made to emit through `cc1`+`maspsx` (bare `ctc2`/`cop2` GTE moves,
`syscall` trampolines, `cache`/`rfe`/`mtc0`, ...):

| verdict | functions | bytes | share of remaining bytes |
|---|---:|---:|---:|
| `draftable` (m2c produces C) | 1,335 | 460,180 | 76.8% |
| `handwritten` (not C-matchable) | 119 | 79,948 | 13.3% |
| `m2c-error` (jump-table heavy, m2c gave up) | 112 | 58,156 | 9.7% |
| `unknown-instr` (other unmodelled opcodes) | 17 | 1,176 | 0.2% |

**Ceiling:** at most ~1,335 further matching C leaves exist. The ~119
handwritten functions must stay `asm` in the yaml; they are glue (GTE
load/store sequences, BIOS syscall stubs) and their byte-exactness is already
guaranteed by splat, not by C matching.

## Cheapest leaves: functions that already have a port body

`pc_port/` holds 579 hand-written `*_port.c` files transcribed from retail so
the native port can run. **738 of the 1523 remaining functions (330,928 bytes,
55.9% of remaining bytes) already have such a body**, i.e. their semantics are
written down and only the era-cc1 re-expression is missing. Ranked list:

```sh
python3 tools/analysis/port_backed_worklist.py --top 40
# -> build/port_backed_names.txt, build/port_backed_worklist.json
```

These are the highest-yield targets and are what the `agent/pb-*` slices work.

## Drafting pipeline

`m2c` (matt-kempster/m2c, pinned `708d2d2c`) is installed under the git-ignored
`tools/era/` by `scripts/setup_m2c.sh`. It is a **triage aid, never an
authority** — `scripts/build_us.sh` is the only matching authority.

```sh
python3 tools/analysis/m2c_leaf.py --print func_80020D50    # candidate C
python3 tools/analysis/auto_leaf.py --from-file build/port_backed_names.txt --limit 40
```

`auto_leaf.py` runs m2c → normalise → sweep all era profiles and reports any
function that matches with zero hand-editing (it also auto-wraps the
comparison in `distrobox enter pe-mipsel`, without which `try_leaf.py` fails
on the host with a missing `mipsel-linux-gnu-as`).

Sanity check that the triage harness itself works — a known leaf must report a
match:

```sh
distrobox enter pe-mipsel -- bash -lc \
  'cd <worktree> && python3 tools/analysis/try_leaf.py src/func_8006DBE0.c 0x5E3E0 0x38'
# retail 56 bytes / candidate 64 bytes / WORDS MATCH (+8 pad bytes, trimmed by the build)
```
