# Phase 5EF typing record

Phase 5EF integrates the 13 delay-slot `sw` members left after the
`func_8007FBC0` pilot. Together, all 14 family members are now matching C.

## Type evidence and leaves

| Global | Type | Evidence | Integrated leaves |
| --- | --- | --- | --- |
| `D_800956EC` | `int` | Existing matching getter `func_80074A28` declares and returns `int` | `func_80074A14` |
| `D_8009AFC0` | `int` | Live asm uses signed `blez` and `slti`; value is also exchanged/cleared | `func_8007A3EC`, `func_80080CC8` |
| `D_8009AFB4` | `int` | State value is saved, restored, and zero-tested | `func_8007A4A8` |
| `D_8009AFB8` | `int` | State value is saved, restored, and zero-tested | `func_8007A4BC` |
| `D_8009B260` | `unsigned int` | Write-only 32-bit word in the live yaml/asm population; no narrowing evidence | `func_8007C130` |
| `D_8009B4AC` | `int` | Existing matching getter `func_8007DEB0` declares and returns `int` | `func_8007DEA4` |
| `D_800A36A4` | `void (*)(int, void *)` | Null-tested then called with event/context in `704BC.s` | `func_8007FBCC` |
| `D_800A36A8` | `void (*)(int, void *)` | Null-tested then called with event/context in `704BC.s` | `func_8007FBD8` |
| `D_800A36AC` | `void (*)(int, void *)` | Null-tested then called with event/context in `704BC.s` | `func_8007FBE4` |
| `D_8009B554` | `int` | Existing matching getter `func_80080940` declares `int`; live uses are state/zero checks | `func_80080930` |
| `D_8009B6D0` | `void (*)(int, void *)` | Null-tested then called with event/context in `71150.s` | `func_80081254` |
| `D_8009B78C` | `int` | Written as a state flag, then returned and cleared | `func_80082CDC` |

## Gate

- `scripts/split_us.sh`: 205 C leaves; 140 yaml asm segments.
- `scripts/build_us.sh`: exact SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
- Per-leaf byte probes match for all 13 new functions.
- `MASPSX_FILL_STORE_DELAY_SLOT=1` remains opt-in on each family leaf; default
  maspsx behavior and the pre-`jr` `sb`/`sh` families are unchanged.
