# Weak-int setters — per-item decision context

This table supports the `weak-int-*` and `D_800A1874-policy` entries in
`docs/ai_context/parked_blockers.json`.

A width-only / write-only global written through `$at` is **not** auto-typed from
`sw` width alone. The entries below fall into three concrete states:

- **READY-FROM-READER** — a decompiled `src/*.c` reader already assigns a
  concrete type. Integrating the setter is unblocked now.
- **BLOCKED-ON-READER** — only undecompiled asm readers reference it. The global
  is blocked on decompiling the named reader function so it produces a typed
  src reader.
- **DECISION-BLOCKED** — no reader anywhere. Waits on an explicit per-item human
  typing decision, unless a later reader-phase finds a reader.

Search scope for reader evidence: all `src/*.c` leaves and yaml-live
`asm/disc1/*.s` units (orphan `.s` files are ignored).

## Width-only / write-only pre-jr `sw` setters

| Global | Address | Width | Setter(s) | State |
|---|---|---|---|---|
| `D_800A1860` | `0x800A1860` | `sw` | `func_80042910` | READY-FROM-READER: `func_800428C4` → `int` |
| `D_800A1868` | `0x800A1868` | `sw` | `func_80042910` | DECISION-BLOCKED: no known reader |
| `D_8009D28C` | `0x8009D28C` | `sw` | `func_80017FDC`, `func_80017FF0`, `func_800192B8`, `func_800192C8` | BLOCKED-ON-READER: `func_80019154` / `func_8001D340` |
| `D_800A1820` | `0x800A1820` | `sw` | `func_80042BD8` | BLOCKED-ON-READER: `func_800405A4` |
| `D_800A1824` | `0x800A1824` | `sw` | `func_80042BEC`, `func_80042C00` | BLOCKED-ON-READER: `func_800405A4` |
| `D_800A1828` | `0x800A1828` | `sw` | `func_80042C14` | BLOCKED-ON-READER: `func_800405A4` |
| `D_800A182C` | `0x800A182C` | `sw` | `func_80042C28` | BLOCKED-ON-READER: `func_800405A4` |
| `D_800A1830` | `0x800A1830` | `sw` | `func_80042C3C`, `func_80042C50` | BLOCKED-ON-READER: `func_800405A4` |
| `D_800A1834` | `0x800A1834` | `sw` | `func_80042C64` | BLOCKED-ON-READER: `func_800405A4` |
| `D_8009D270` | `0x8009D270` | `sw` | `func_80087198`, `func_80087414` | BLOCKED-ON-READER: `func_800871AC` / `func_80087428` |

## `D_800A1874` / `D_800A1870` (was a "policy" question)

| Setter | Global(s) | Width | State |
|---|---|---|---|
| `func_80042B38` | `D_800A1870` (`0x800A1870`), `D_800A1874` (`0x800A1874`) | `sw` | READY-FROM-READER: `func_80042B6C` → `D_800A1870`=`void (*)(void)`, `D_800A1874`=`int` |
| `func_80042B50` | `D_800A1870` (`0x800A1870`), `D_800A1874` (`0x800A1874`) | `sw` | READY-FROM-READER: same as above |

The original "policy" framing is obsolete: `func_80042B6C` already types both
globals. Integration is a separate leaf-integration decision, not a typing
question.

## Search method

```bash
# live asm unit list from configs/USA/disc1.yaml
python3 tools/analysis/at_absolute_store_counter.py --shape pre-jr --verbose
# reader search: grep each global in src/*.c and live asm/disc1/*.s units
```

Last checked: main `cc91719`, Phase 5EG-setter, 174 leaves.
