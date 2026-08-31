# `func_8007A8EC` — exact slot-3 forwarding twin

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 380 and Tier-2 continuation rung 39.

## Function hood and screens

- File `[0x6B0EC,0x6B110)`, VA `[0x8007A8EC,0x8007A910)`: nine words,
  ending in canonical `jr ra; nop`.
- Exact direct caller `0x8007A2E0` targets the start.
- Preceding real `func_8007A8CC` ends at `0x6B0E4/0x6B0E8`; following real
  `func_8007A910` starts immediately at `0x6B110`.
- One unresolved callee, `func_80073CF4`; no globals, loop, address retention,
  or result liveness. Input moves to `$a1`, fixed slot 3 fills the call delay.
- The body is word-identical to independently accepted `func_800824F0`, but
  function hood and compilation were checked independently. Established era
  `-O2 -G0` call scheduling needs no maspsx gate.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL`.

## C and object

```c
void func_80073CF4(int slot, unsigned int value);

void func_8007A8EC(unsigned int value) {
    func_80073CF4(3, value);
}
```

```text
6B0EC: 27bdffe8  addiu sp,sp,-24
6B0F0: afbf0010  sw    ra,16(sp)
6B0F4: 00802821  addu  a1,a0,zero
6B0F8: 0c01cf3d  jal   func_80073CF4
6B0FC: 24040003  addiu a0,zero,3
6B100: 8fbf0010  lw    ra,16(sp)
6B104: 27bd0018  addiu sp,sp,24
6B108: 03e00008  jr    ra
6B10C: 00000000  nop

object: 27bdffe8 afbf0010 00802821 0c000000 24040003 8fbf0010 27bd0018 03e00008 00000000
ROM/C:  27bdffe8 afbf0010 00802821 0c01cf3d 24040003 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_26 func_80073CF4
BYTE_EXACT=9/9
```

## Carve and packed gate

Former asm `[0x6B0AC,0x6C930)` = `0x1884`:

```text
prefix: 0x6B0EC - 0x6B0AC = 0x0040
leaf:   0x6B110 - 0x6B0EC = 0x0024
resume: 0x6C930 - 0x6B110 = 0x1820
close:  0x0040 + 0x0024 + 0x1820 = 0x1884
```

```text
6B0DC: 8fbf0010 = 8fbf0010  preceding epilogue
6B0E0: 2c420001 = 2c420001
6B0E4: 03e00008 = 03e00008
6B0E8: 27bd0018 = 27bd0018  live return delay
6B0EC: 27bdffe8 = 27bdffe8  leaf 1
6B0F0: afbf0010 = afbf0010  leaf 2
6B0F4: 00802821 = 00802821  leaf 3
6B0F8: 0c01cf3d = 0c01cf3d  leaf 4
6B0FC: 24040003 = 24040003  leaf 5
6B100: 8fbf0010 = 8fbf0010  leaf 6
6B104: 27bd0018 = 27bd0018  leaf 7
6B108: 03e00008 = 03e00008  leaf 8
6B10C: 00000000 = 00000000  leaf 9
6B110: 27bdffe8 = 27bdffe8  following real prologue
6B114: afbf0010 = afbf0010
6B118: 0c01ef77 = 0c01ef77
PACKED_SPAN=EXACT
```

```text
retail/candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/build_us.sh: exit 0; Compare: EXACT SHA-1 MATCH
matching-C count: 380
```
