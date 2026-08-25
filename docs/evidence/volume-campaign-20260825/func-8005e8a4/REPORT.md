# `func_8005E8A4` — exact matching-C two-component accumulator

Leaf 301, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x4F0A4,0x4F0C4)`, VRAM `0x8005E8A4`, eight words. It ends in
`jr ra` plus `nop`. The preceding word at `0x4F0A0` is the real return delay
slot of `func_8005E894`; the following word at `0x4F0C4` is
`addiu sp,sp,-0x18`, the real first instruction of `func_8005E8C4`.

A raw executable census finds 247 instances of the exact `jal 0x8005E8A4`
word (`0C017A29`). `FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; leaf updater |
| written globals | `D_8009D124` and `D_8009D128`, the gp-relative pair at offsets `0x3B4` and `0x3B8` |
| Stage-0 readers | 32 other functions read one or both words; census listed below |
| coloring pressure | low but paired: retail keeps the first value in `$v0` and the second in `$v1` across both adds |
| `$v0` liveness | first updated component only; function has no return value |
| address retention | both globals stay gp-relative; no symbolic address register is retained |
| optimization signal | load-load/add-add/store-store scheduling and gp-relative accesses select era `-O2 -G8` |
| loop/back-edge owner | none |

Stage-0 reader census, excluding this updater:

```text
func_8005E8C4  func_8005EB64  func_8005EED4  func_8005F27C
func_8005F354  func_8005F5B8  func_8005F698  func_8005FA3C
func_8005FB74  func_8005FCAC  func_8005FDF0  func_8005FF28
func_8006006C  func_800602D0  func_80060528  func_8006055C
func_80060590  func_800605C4  func_800605F8  func_8006062C
func_80061A3C  func_80061B80  func_80061C34  func_80062090
func_800622BC  func_80062830  func_80062A7C  func_80063158
func_800634D4  func_800638D8  func_80064C80  func_80064EB4
```

## C and flags

```c
extern int D_8009D124;
extern int D_8009D128;

void func_8005E8A4(int a0, int a1) {
    D_8009D124 += a0;
    D_8009D128 += a1;
}
```

Compiled with `era_compile ... -O2 -G8`. `_gp = 0x8009CD70`, so the
object's `R_MIPS_GPREL16` relocations normalize as follows:

```text
D_8009D124 - _gp = 0x3B4
D_8009D128 - _gp = 0x3B8
```

No pin, inline assembly, or special maspsx switch is used.

## Full single-leaf comparison

| word | retail | candidate after relocation | instruction |
|---:|---:|---:|---|
| 0 | `8F8203B4` | `8F8203B4` | `lw v0,0x3B4(gp)` |
| 1 | `8F8303B8` | `8F8303B8` | `lw v1,0x3B8(gp)` |
| 2 | `00441021` | `00441021` | `addu v0,v0,a0` |
| 3 | `00651821` | `00651821` | `addu v1,v1,a1` |
| 4 | `AF8203B4` | `AF8203B4` | `sw v0,0x3B4(gp)` |
| 5 | `AF8303B8` | `AF8303B8` | `sw v1,0x3B8(gp)` |
| 6 | `03E00008` | `03E00008` | `jr ra` |
| 7 | `00000000` | `00000000` | `nop` |

## Carve and packed-span proof

The prior asm span is `[0x4F0A4,0x4F358)`, size `0x2B4`:

```text
C leaf:     0x4F0C4 - 0x4F0A4 = 0x020
resume asm: 0x4F358 - 0x4F0C4 = 0x294
closure:    0x020 + 0x294 = 0x2B4
```

Full packed span, file `0x4F0A4..0x4F0C3`:

```text
retail:    b403828fb803838f2110440021186500b40382afb80383af0800e00300000000
candidate: b403828fb803838f2110440021186500b40382afb80383af0800e00300000000
```

The packed executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. `scripts/verify_us.sh`
passed and reports 301 matching-C leaves.
