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
| `D_800A1860` | `0x800A1860` | `sw` | `func_80042910` | READY-FROM-READER: `func_800428C4` → `int` |
| `D_800A1868` | `0x800A1868` | `sw` | `func_80042910` | DECISION-BLOCKED: no known reader (own phase) |
| `D_8009D28C` | `0x8009D28C` | `sw` | `func_80017FDC`, `func_80017FF0`, `func_800192B8`, `func_800192C8` | BLOCKED-ON-READER: `func_80019154` / `func_8001D340` |
| `D_800A1820` | `0x800A1820` | `sw` | `func_80042BD8` | INTEGRATED — `unsigned int` (opaque-word) |
| `D_800A1824` | `0x800A1824` | `sw` | `func_80042BEC`, `func_80042C00` | INTEGRATED — `unsigned int` (opaque-word) |
| `D_800A1828` | `0x800A1828` | `sw` | `func_80042C14` | INTEGRATED — `unsigned int` (opaque-word) |
| `D_800A182C` | `0x800A182C` | `sw` | `func_80042C28` | INTEGRATED — `unsigned int` (opaque-word; Stage 2 pilot) |
| `D_800A1830` | `0x800A1830` | `sw` | `func_80042C3C`, `func_80042C50` | INTEGRATED — `unsigned int` (opaque-word) |
| `D_800A1834` | `0x800A1834` | `sw` | `func_80042C64` | INTEGRATED — `unsigned int` (opaque-word) |
| `D_8009D270` | `0x8009D270` | `sw` | `func_80087198`, `func_80087414` | BLOCKED-ON-READER: `func_800871AC` / `func_80087428` |

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

`D_800A1868` and READY-FROM-READER setters (`D_800A1860` / `D_800A1870` /
`D_800A1874`) remain **out of scope**.

## `D_800A1874` / `D_800A1870` (narrowing readers — separate phase)

| Setter | Global(s) | Width | State |
|---|---|---|---|
| `func_80042B38` | `D_800A1870`, `D_800A1874` | `sw` | READY-FROM-READER via `func_80042B6C` |
| `func_80042B50` | `D_800A1870`, `D_800A1874` | `sw` | READY-FROM-READER via `func_80042B6C` |

## Search method

```bash
python3 tools/analysis/at_absolute_store_counter.py --shape pre-jr --verbose
# Stage 0: for each global, only lui/lw/sw mnemonics in live asm units
```

Last checked: Stage 0 opaque-word re-verify on tip `01a212c`; 6/6 pass.
