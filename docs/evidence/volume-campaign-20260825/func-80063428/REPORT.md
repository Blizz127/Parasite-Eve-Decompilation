# `func_80063428` — exact guarded value calculation

Leaf 333, matched on the first bounded phrasing.

## Function hood and boundary ownership

Retail span `[0x53C28,0x53C6C)`, VRAM `[0x80063428,0x8006346C)`, is
`0x44` bytes / seventeen words. It ends with canonical `jr ra; nop` at
`0x80063464/0x80063468`.

A raw executable scan finds 59 direct `jal 0x80063428` references:

```text
0x80043080 0x80043534 0x80043DD0 0x800442EC 0x800443B8 0x800444B8
0x800445A0 0x800445B8 0x80044608 0x80044B50 0x80044EBC 0x80045AD0
0x80045B20 0x80046018 0x80046034 0x800465A0 0x800465E0 0x80046704
0x80046730 0x80046C44 0x80047208 0x80047370 0x800474EC 0x80047A5C
0x800482C0 0x800482DC 0x8004885C 0x800492FC 0x80049384 0x800495D0
0x8004A100 0x8004A150 0x8004AE54 0x8004AEC4 0x8004AFE4 0x8004B0E4
0x8004B22C 0x8004B3B8 0x8004B474 0x8004C684 0x8004D550 0x8004E174
0x8004ECE4 0x8004F33C 0x8004F35C 0x8004F3B4 0x8004FA40 0x8004FA74
0x8004FA8C 0x8004FB70 0x8004FC94 0x80050358 0x8005062C 0x800507A8
0x80050BA8 0x80050F78 0x80050FCC 0x80051020 0x80057C2C
```

The preceding real `func_8006322C` owns `jr ra; nop` at
`0x80063420/0x80063424`. The following real `func_8006346C` begins
immediately with a null guard. There is no boundary padding.

`FUNCTION_HOOD=PROVEN_BY_59_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; true leaf |
| written-state Stage 0 | none; argument is read-only and no globals are written |
| coloring pressure | `$a1` retains field `+0x44`, `$v1` retains field `+0x48`, `$v0` transitions from `-1` accumulator to field `+0x34`, and `$a2` receives `mflo` |
| `$v0` liveness | default result `-1`; on success overwritten by multiplier and final sum |
| address retention | `$a0` remains the record base through three fixed-offset loads |
| optimization signal | era load-delay nops, branch-delay default result, and multiply scheduling select GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

The three field loads are all signed words. If the pointer is null or either
field `+0x44/+0x48` is negative, the function returns `-1`; otherwise it
returns `field_44 + field_34 * field_48`. No stronger record or field names
are asserted without caller-side type provenance.

## Final C and flags

```c
typedef struct {
    unsigned char pad_00[0x34];
    int multiplier;
    unsigned char pad_38[0x0C];
    int base;
    int factor;
} ValueRecord;

int func_80063428(ValueRecord *record) {
    int result = -1;

    if (record != 0) {
        int base = record->base;

        if (base >= 0) {
            int factor = record->factor;

            if (factor >= 0) {
                result = record->multiplier * factor + base;
            }
        }
    }

    return result;
}
```

Compiler: era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`. No pins,
inline assembly, special maspsx switch, or relocation normalization is needed.
The natural explicit-result phrasing matched immediately.

## Full seventeen-word comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `1080000E` | `1080000E` | `beqz a0,+14` |
| 1 | `2402FFFF` | `2402FFFF` | `addiu v0,zero,-1` |
| 2 | `8C850044` | `8C850044` | `lw a1,0x44(a0)` |
| 3 | `00000000` | `00000000` | `nop` |
| 4 | `04A0000A` | `04A0000A` | `bltz a1,+10` |
| 5 | `00000000` | `00000000` | `nop` |
| 6 | `8C830048` | `8C830048` | `lw v1,0x48(a0)` |
| 7 | `00000000` | `00000000` | `nop` |
| 8 | `04600006` | `04600006` | `bltz v1,+6` |
| 9 | `00000000` | `00000000` | `nop` |
| 10 | `8C820034` | `8C820034` | `lw v0,0x34(a0)` |
| 11 | `00000000` | `00000000` | `nop` |
| 12 | `00430018` | `00430018` | `mult v0,v1` |
| 13 | `00003012` | `00003012` | `mflo a2` |
| 14 | `00C51021` | `00C51021` | `addu v0,a2,a1` |
| 15 | `03E00008` | `03E00008` | `jr ra` |
| 16 | `00000000` | `00000000` | `nop` |

Single-leaf object:

```text
00000000 <func_80063428>:
   0: 1080000e  beqz   a0,3c
   4: 2402ffff  li      v0,-1
   8: 8c850044  lw      a1,68(a0)
   c: 00000000  nop
  10: 04a0000a  bltz    a1,3c
  14: 00000000  nop
  18: 8c830048  lw      v1,72(a0)
  1c: 00000000  nop
  20: 04600006  bltz    v1,3c
  24: 00000000  nop
  28: 8c820034  lw      v0,52(a0)
  2c: 00000000  nop
  30: 00430018  mult    v0,v1
  34: 00003012  mflo    a2
  38: 00c51021  addu    v0,a2,a1
  3c: 03e00008  jr      ra
  40: 00000000  nop
```

The object section is `0x50` bytes because GNU as aligns its end. The trim
guard verified that every byte after the real `0x44` body is zero before
removing that padding.

## Carve geometry and packed-span proof

The prior asm span was `[0x539DC,0x55248)`, size `0x186C`:

```text
prefix asm:  0x53C28 - 0x539DC = 0x024C
C leaf:      0x53C6C - 0x53C28 = 0x0044
resume asm:  0x55248 - 0x53C6C = 0x15DC
closure:     0x024C + 0x0044 + 0x15DC = 0x186C
```

Retail and packed candidate are identical through both real boundaries:

```text
retail:
00053c18: 1000b08f 2800bd27 0800e003 00000000
00053c28: 0e008010 ffff0224 4400858c 00000000
00053c38: 0a00a004 00000000 4800838c 00000000
00053c48: 06006004 00000000 3400828c 00000000
00053c58: 18004300 12300000 2110c500 0800e003
00053c68: 00000000 10008010 ffff0324 4400858c

candidate:
00053c18: 1000b08f 2800bd27 0800e003 00000000
00053c28: 0e008010 ffff0224 4400858c 00000000
00053c38: 0a00a004 00000000 4800838c 00000000
00053c48: 06006004 00000000 3400828c 00000000
00053c58: 18004300 12300000 2110c500 0800e003
00053c68: 00000000 10008010 ffff0324 4400858c
```

The packed target bytes equal the single-leaf object exactly:

```text
0e008010ffff02244400858c000000000a00a004000000004800838c00000000
06006004000000003400828c0000000018004300123000002110c5000800e003
00000000
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 333 leaves
333
```

Result: `MATCHED=17/17`, first phrasing; consecutive parks reset from one to
zero.
