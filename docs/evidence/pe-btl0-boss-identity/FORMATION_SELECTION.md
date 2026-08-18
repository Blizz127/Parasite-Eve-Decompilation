# FORMATION_SELECTION — 49 vs 50 and slot fields 1332 / 1333 / 1334

## 49 vs 50

m0005i module 2, current actor locals (binder mode 1 →
`D_8009D2F0 + 0xAC + 4*index`):

```text
+0x1DCC  0x1A  dest=local[24]  lo=0  hi=100     ; func_800176FC -> func_80070DD0
+0x1DE0  0x09  slt  local[24] < 19              ; ALU sub 9, dest cond[0]
+0x1DF8  0x05  skip_if_false cond[0] -> +0x1E24
+0x1E08  0x0A  local[24] = 49
+0x1E18  0x00  goto +0x1E34
+0x1E24  0x0A  local[24] = 50
+0x1E34  0x6F  always
```

Both arms fall into the same `0x6F` / `0x5A` / `0x70` / `0xB7` block.
The 1332/1333/1334 writes are immediates, not `local[24]`.

## Consumers of local[24] on m0005i

Mode-1 uses of index 24, entire script:

| site | op | effect |
|---|---|---|
| mod2 +0x1DCC | `0x1A` | write rng |
| mod2 +0x1DE0 | `0x09` | read for `< 19` |
| mod2 +0x1E08 | `0x0A` | write 49 |
| mod2 +0x1E24 | `0x0A` | write 50 |
| mod6 +0x380C | `0x0A` | **overwrite 0** (after both early `0x89`) |
| mod6 +0x381C | `0x94` | dest for mode-word poll |
| mod6 +0x3828 / +0x38F4 | `0x09` | compare to 10 / 9 |

No `0x6F` / `0x5A` / `0x70` / `0xB7` operand is `local[24]`.
`0x70` at `+0x21A0` is

```text
args 0,0,8,9,1,5,-1,-1,-1,3,15
modes 0,0,0,0,3,0,0,0,0,0,0     ; only arg 4 is cond, value 1
```

`0xB7` at `+0x2154` is `3,0,6,7,1` with last mode cond.

So 49 vs 50 is a **script-local variant that setup does not consume**.
Enemy resource IDs and opened names are the same on both arms.

## Slot fields 50 / 51 / 52

`0x5A` handler `0x80018164`: if `lbu (D_8009D2F0)+0x0C == 0` then
`func_8002FF78` (Aya tags 0..34 / 255). Else `func_80030220`.
After `0x6F` the slot path is live.

`func_80030220`: `tag = (tag & 0xFF) - 40`; reject if `tag >= 85`;
`jr` through `0x80010C90[tag]`.

```text
tag 50  ->  0x800303B4   sh  value, 0xB0(slot)     ; 1333
tag 51  ->  0x800303BC   sh  value, 0xB2(slot)     ; 1332
tag 52  ->  0x800303C4   sh  value, 0xB4(slot)     ; 1334
```

First-encounter writes (`+0x223C` / `+0x224C` / `+0x225C`) are
unconditional immediates.

Meaning, from EXE readers, not from names:

| field | value | proven use |
|---|---|---|
| +0xB2 | 1332 | `0x8002FC14` `lhu $a0, 0xB2` then `jal func_8006DCE4` (forwards to `func_8006DED4` with archive base `D_800B0E64`) |
| +0xB0 | 1333 | `0x80087EC4` `lhu` compare to 1; 1333 ≠ 1 so that arm is not taken |
| +0xB4 | 1334 | `0x8003058C` `lh` shifted 16 and subtracted (heading); `0x80087EB0` increment |

They are **resource / pose halfwords**. They are not stream-1 message
IDs.

## Later 0x89

Module 6 has three `0x89` and **zero** `0x6F` / `0x5A` / `0x70` /
`0xB7`. The second and third requests do not install a new 1332/1333/1334
set. Persist `[10]` bits gate which request runs; they do not retarget
the slot.

```text
first_day1_formation_id   = 1332,1333,1334
script_local_variant      = 49_or_50
variant_selects_resources = no
```
