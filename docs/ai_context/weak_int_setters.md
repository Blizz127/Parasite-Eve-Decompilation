# Weak-int setters — per-item decision context

This table supports the `weak-int-*`, `opaque-word-A182x`, and
`D_800A1874-policy` entries in `docs/ai_context/parked_blockers.json`.

## States

- **READY-FROM-READER** — a decompiled `src/*.c` reader assigns a concrete
  *narrowing* type. Integrating the setter is unblocked.
- **READY-OPAQUE-WORD** — every writer/reader treats the global as a bare
  32-bit word (consistent `sw`/`lw`; no arith/deref/bitwise/typed-callee).
  Lead ruling: type as `unsigned int` (`u32`) and integrate. Provenance is
  opaque-word; refine if a narrowing reader appears later.
- **BLOCKED-ON-READER** — undecompiled asm reader not yet proven to be a mere
  use-site. Blocked on decompiling that reader.
- **DECISION-BLOCKED** — write-only (no reader). Separate from opaque-word
  batch until its own phase.

## Not the rejected weak-int cheat

| | Rejected "weak-int" cheat | Opaque-word ruling (accepted) |
| --- | --- | --- |
| What it does | Widen a `sh`/`sb` store to `int` | Name a consistently 32-bit word as `u32` |
| Width | **Invented** | **Observed** |
| Status | **NO** | **Accepted** — type `u32` and integrate |

## Width-only / write-only pre-jr `sw` setters

| Global | Address | Width | Setter(s) | State |
|---|---|---|---|---|
| `D_800A1860` | `0x800A1860` | `sw` | `func_80042910` | INTEGRATED 5EI — `int` (func_80042910 dual) |
| `D_800A1868` | `0x800A1868` | `sw` | `func_80042910` | READY-OPAQUE-WORD → `u32`; decl+clear via integrated func_80042910; other writers still asm |
| `D_8009D28C` | `0x8009D28C` | `sw` | `func_80017FDC`, `func_80017FF0`, `func_800192B8`, `func_800192C8` | **INTEGRATED** (5EJ) — `int` state (READY-FROM-READER; not opaque-word) |
| `D_800A1820`…`D_800A1834` | | `sw` | eight setters | **INTEGRATED** (5EH) — `unsigned int` opaque-word |
| `D_8009D270` | `0x8009D270` | `sw` | `func_80087198`, `func_80087414` | READY-FROM-BITWISE → `unsigned int` flags (`andi` 1/2) — **not** opaque |

## Opaque-word typing policy

### Lead ruling (accepted)

A global whose every writer and every reader treats it as a bare 32-bit word
— no arithmetic, no deref, no bit-masking, no typed-callee arg, consistent
`sw`/`lw` width — is typed **`unsigned int` (`u32`)** and integrated.
Signedness is undetermined by `beqz`/`bnez`; `u32` is the conservative default.

This is **not** the rejected weak-int cheat (inventing width a narrower store
contradicts). It names the width retail uses.

### Stage 0 confirmation (live asm + src)

All six symbols appear only as `lui` / `lw` / `sw` in yaml-live `asm/disc1`
units and not at all in `src/`. No arithmetic, pointer base, bitwise, or
callee-arg use of the loaded value. `func_800405A4` is a **use-site only**
(`lw; beqz/bnez; sw $zero` test-and-clear). Former `blocker_reader:
func_800405A4` edge **removed**.

### Integration result

1. Stage 2 pilot `func_80042C28` matched exact (ROM word-identical).
2. Stage 3 batch: remaining seven setters matched exact.
3. Leaf arithmetic: **174 + 8 setter functions = 182** (not +6 globals).

## Stage 0 sweep (post-5EH) — what the ruling unlocks

Read-only re-search of remaining weak-int / decision-blocked members on tip
`2234794`. Floor: *no* arith / pointer-base / bitwise / typed-callee use of the
loaded value, **searched not assumed**. READY-FROM-READER with real types stays
in its own bucket.

| Global | Stage 0 result | Unlocks |
| --- | --- | --- |
| `D_800A1868` | **PASS** write-only `lui`/`sw`; stores 0 or 1 | `func_80042910` (dual-store w/ `D_800A1860`) |
| `D_8009D28C` | **PASS** stores 0/3/4/5/6/8; load = word-copy or `bne` vs 1 | 4 setters (`17FDC`…`192C8`) |
| `D_8009D270` | **FAIL** `andi`/`and` bit ops | 2 setters via **bitwise** path, not opaque |
| `D_800A1860` | **FAIL** `addiu -1` + src `int` | stays READY-FROM-READER |
| `D_800A1870` | **FAIL** `jalr` function pointer | stays READY-FROM-READER |
| `D_800A1874` | **FAIL** `+1` counter `int` | stays READY-FROM-READER |

**Opaque-word unlock this sweep:** 2 globals → **5 setter functions**  
(`func_80042910` + four `D_8009D28C` setters).  
**Bitwise unlock (separate):** `D_8009D270` → 2 setters as `unsigned int` flags.  
**No integration in this pass.**

### `D_8009D28C` detail (why int state, not opaque-word)

- `func_80019154`: `lw` global → `sw` to `*a0` — **copies the word**, does not
  use it as a pointer base or do arith/bits.
- `A404` load: `bne` against constant 1 — equality test, not narrowing.
- Distinct state codes **0/3/4/5/6/8** (not bare test-and-clear flags like A182x).
- Signedness undetermined → **`int`** (sign-neutral). Classified READY-FROM-READER.
- **INTEGRATED 5EJ:** four setters (`17FDC`/`17FF0`/`192B8`/`192C8`); leaf count
  185 + 4 = **189**.

### `D_8009D270` detail (why not opaque-word)

Loads do `andi $v0, 0x1` / `andi $v0, 0x2` and `and` with `-2`/`-3` (clear
bits). That **is** the Stage 0 floor being hit. Type as `unsigned int` flags
from bitwise evidence; do not claim “bare opaque word.”

## `D_800A1874` / `D_800A1870` (narrowing readers — separate phase)

| Setter | Global(s) | Width | State |
|---|---|---|---|
| `func_80042B38` | `D_800A1870`, `D_800A1874` | `sw` | **INTEGRATED 5EI** — `void(*)(void)` + `int` via `func_80042B6C` |
| `func_80042B50` | `D_800A1870`, `D_800A1874` | `sw` | **INTEGRATED 5EI** — same types; `sw $a0` fn-ptr matched |

## Search method

```bash
python3 tools/analysis/at_absolute_store_counter.py --shape pre-jr --verbose
# Stage 0: for each global, only lui/lw/sw mnemonics in live asm units
```

Last checked: Stage 0 opaque-word re-verify on tip `01a212c`; 6/6 pass.
