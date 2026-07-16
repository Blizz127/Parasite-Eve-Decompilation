# Phase 5EE `$at` absolute-`sw` audit

Phase 5EE is intentionally separate from `lui;ori` constant synthesis. The
standalone probe proved only the absolute-store path: cc1 emits an
`sw $r,SYM` macro and GNU as expands it to `lui $at,%hi(SYM)` plus
`sw $r,%lo(SYM)($at)`. Maspsx 2.21 and 2.30 produced identical words for all
three probe shapes, so `addiu_at`, `nop_at_expansion`, and an
`ERA_ASPSX_VER` bump are irrelevant to this family. `lui;ori` remains
untested and unqueued.

The probe was word-level, not an integrated link test. Phase 5EE therefore
tested a three-function production pilot whose globals already have matching
declarations in accepted C getters:

| Setter | Global | Existing declaration evidence | Integrated result |
| --- | --- | --- | --- |
| `func_8003FFAC` | `D_800A1704` | `extern int` in `func_8003FFBC.c` | **Exact; integrated** |
| `func_8007DEA4` | `D_8009B4AC` | `extern int` in `func_8007DEB0.c` | Non-match; remains asm |
| `func_80080930` | `D_8009B554` | `extern int` in `func_80080940.c` | Non-match; remains asm |

The two negative members emit the correct `$at` relocation and store, but cc1
orders `sw` before `jr`; ROM places `sw` in the return delay slot. `-O1`,
`-O2`, `-fdelayed-branch`, and `-fno-delayed-branch` do not change that order.
The rejected three-member candidate differed in only those two functions (16
bytes total); both were removed from the integration.

## YAML-derived population

Source of truth: the 127 `[offset, asm]` entries in
`configs/USA/disc1.yaml`, mapped to their generated `asm/disc1/OFFSET.s`
units. For each live unit, functions were split on `glabel func_*` and
instruction lines were counted from the disassembly comments.

Selection rule: at most seven instructions, contains `lui $at,%hi(D_*)`, a
matching absolute store through `$at`, and `jr $ra`.

| Width | Live functions | Phase 5EE status |
| --- | ---: | --- |
| `sw` | **33** | 5ED baseline; one integrated, 32 remain |
| `sb` | 2 | Not covered by the `sw` probe; unqueued |
| `sh` | 3 | Not covered by the `sw` probe; unqueued |
| Total absolute-store shapes | 38 | Not a single evidence class |

The previously quoted “37 short live setters” and “~113 raw hits” are not
target counts. The first lacked a reproducible selection rule; the second
came from the ignored asm tree and can include stale or duplicate units.

Of the 33 baseline short `sw` functions, only four had declarations for every
stored global in accepted C leaves. The three simple setters above formed the
pilot. The fourth, `func_80074A14`, is a read/replace exchange rather than a
simple setter and stayed out. The other 29 functions remain blocked on
global-type evidence; matching store width alone does not prove signedness,
pointer-ness, or the intended declaration.

Batching beyond the one integrated member was blocked on both delay-slot shape
and global types. **Update (5EF-pilot):** the delay-slot shape is SOLVED — the
vendored maspsx LOCAL PATCH (`MASPSX_FILL_STORE_DELAY_SLOT=1`) fills the
`j $31` slot with the trailing absolute `sw`, and `func_8007FBC0` integrated
exact (leaf 192). The two negative members above (`func_8007DEA4`,
`func_80080930`) are now unblockable on the tool side; what remains per member
is declaration evidence and an exact integrated rebuild, as stated below.
No standalone probe result is to be called integrated.
