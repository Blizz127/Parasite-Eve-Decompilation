# `func_8005F594` — exact GP-state forwarding twin

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G8`; matching-C
leaf 375 and Tier-2 continuation rung 34.

## Function hood and screens

- File `[0x4FD94,0x4FDB8)`, VA `[0x8005F594,0x8005F5B8)`: nine words,
  ending in canonical `jr ra; nop`.
- Seven direct callers target the exact start: `0x80044E48`, `0x8004BCD0`,
  `0x8004C53C`, `0x8004C55C`, `0x8004CE0C`, `0x8004CFF4`, and
  `0x8004D008`.
- Preceding real `func_8005F354` ends immediately at `0x4FD8C/0x4FD90`;
  following real `func_8005F5B8` starts immediately at `0x4FDB8`.
- One unresolved callee, `func_8005F354`; input remains in `$a0`, GP state
  is loaded into `$a1`, and the call result is dead. No loop or address
  retention exists.
- Stage-0 maps `_gp + 0x3C8` to `D_8009D138`. The writer is at
  `0x800630F4`; another reader exists at `0x80062B90`. This proves `-G8`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`.

## C and object

```c
extern int D_8009D138;

void func_8005F354(int value, int state);

void func_8005F594(int value) {
    func_8005F354(value, D_8009D138);
}
```

Flags: era `-O2 -G8`; no maspsx gate.

```text
4FD94: 8f8503c8  lw    a1,0x3C8(gp)
4FD98: 27bdffe8  addiu sp,sp,-24
4FD9C: afbf0010  sw    ra,16(sp)
4FDA0: 0c017cd5  jal   func_8005F354
4FDA4: 00000000  nop
4FDA8: 8fbf0010  lw    ra,16(sp)
4FDAC: 27bd0018  addiu sp,sp,24
4FDB0: 03e00008  jr    ra
4FDB4: 00000000  nop

object: 8f850000 27bdffe8 afbf0010 0c000000 00000000 8fbf0010 27bd0018 03e00008 00000000
ROM/C:  8f8503c8 27bdffe8 afbf0010 0c017cd5 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_GPREL16 D_8009D138 plus R_MIPS_26 func_8005F354
BYTE_EXACT=9/9
```

## Carve and packed gate

Former asm `[0x4F6D4,0x51CA0)` = `0x25CC`:

```text
prefix: 0x4FD94 - 0x4F6D4 = 0x06C0
leaf:   0x4FDB8 - 0x4FD94 = 0x0024
resume: 0x51CA0 - 0x4FDB8 = 0x1EE8
close:  0x06C0 + 0x0024 + 0x1EE8 = 0x25CC
```

Only zero standalone-object alignment is guarded-trimmed.

```text
4FD84: 8fb00010 = 8fb00010  preceding restore
4FD88: 27bd0028 = 27bd0028  preceding teardown
4FD8C: 03e00008 = 03e00008  preceding return
4FD90: 00000000 = 00000000  preceding delay
4FD94: 8f8503c8 = 8f8503c8  leaf 1
4FD98: 27bdffe8 = 27bdffe8  leaf 2
4FD9C: afbf0010 = afbf0010  leaf 3
4FDA0: 0c017cd5 = 0c017cd5  leaf 4
4FDA4: 00000000 = 00000000  leaf 5
4FDA8: 8fbf0010 = 8fbf0010  leaf 6
4FDAC: 27bd0018 = 27bd0018  leaf 7
4FDB0: 03e00008 = 03e00008  leaf 8
4FDB4: 00000000 = 00000000  leaf 9
4FDB8: 27bdffe0 = 27bdffe0  following real prologue
4FDBC: afbf0018 = afbf0018  following word 2
4FDC0: afb10014 = afb10014  following word 3
PACKED_SPAN=EXACT
```

```text
retail/candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 375
```
