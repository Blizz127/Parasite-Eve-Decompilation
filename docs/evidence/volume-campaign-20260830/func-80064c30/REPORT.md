# `func_80064C30` — exact GP-state forwarding wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G8`; matching-C
leaf 374 and Tier-2 continuation rung 33.

## Function hood and screens

- File `[0x55430,0x55454)`, VA `[0x80064C30,0x80064C54)`: nine words,
  ending in canonical `jr ra; nop`.
- Exact direct caller `0x80050980` targets the start.
- Preceding already-C `func_80064C20` ends immediately at `0x5542C`;
  following real `func_80064C54` starts immediately at `0x55454`.
- One unresolved callee, `func_8005F354`; the wrapper passes its input in
  `$a0` and GP state in `$a1`. Its call result is dead; there is no loop,
  direct write, address retention, or `$v0` pressure.
- Stage-0 maps `_gp + 0x3F4` to `D_8009D164`; the executable writer is at
  `0x80063974`, and readers are this leaf plus the nearby `64C54/64C80`
  routines. `-G8` is therefore retail-proven.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL`.

## C and object

```c
extern int D_8009D164;

void func_8005F354(int value, int state);

void func_80064C30(int value) {
    func_8005F354(value, D_8009D164);
}
```

Flags: era `-O2 -G8`; no maspsx gate.

```text
55430: 8f8503f4  lw    a1,0x3F4(gp)
55434: 27bdffe8  addiu sp,sp,-24
55438: afbf0010  sw    ra,16(sp)
5543C: 0c017cd5  jal   func_8005F354
55440: 00000000  nop
55444: 8fbf0010  lw    ra,16(sp)
55448: 27bd0018  addiu sp,sp,24
5544C: 03e00008  jr    ra
55450: 00000000  nop

object: 8f850000 27bdffe8 afbf0010 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
ROM/C:  8f8503f4 27bdffe8 afbf0010 0c017cd5 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_GPREL16 D_8009D164 plus R_MIPS_26 func_8005F354
BYTE_EXACT=9/9
```

## Carve and packed gate

Former asm `[0x55430,0x55BB8)` = `0x0788`:

```text
leaf:   0x55454 - 0x55430 = 0x0024
resume: 0x55BB8 - 0x55454 = 0x0764
close:  0x0024 + 0x0764 = 0x0788
```

Boundary-derived sizes only; the object's zero alignment is guarded-trimmed.

```text
55420: 2402ffff = 2402ffff  preceding real leaf
55424: ac820070 = ac820070
55428: 03e00008 = 03e00008  preceding return
5542C: ac820044 = ac820044  preceding live delay store
55430: 8f8503f4 = 8f8503f4  leaf 1
55434: 27bdffe8 = 27bdffe8  leaf 2
55438: afbf0010 = afbf0010  leaf 3
5543C: 0c017cd5 = 0c017cd5  leaf 4
55440: 00000000 = 00000000  leaf 5
55444: 8fbf0010 = 8fbf0010  leaf 6
55448: 27bd0018 = 27bd0018  leaf 7
5544C: 03e00008 = 03e00008  leaf 8
55450: 00000000 = 00000000  leaf 9
55454: 27bdffe8 = 27bdffe8  following real prologue
55458: afbf0010 = afbf0010  following word 2
5545C: 0c017713 = 0c017713  following call
PACKED_SPAN=EXACT
```

```text
retail/candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 374
```
