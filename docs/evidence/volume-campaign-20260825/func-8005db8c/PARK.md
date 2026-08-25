# `func_8005DB8C` — parked address-DAG/register allocation

No C leaf is claimed. Matching-C count remains 304. Candidate source is
preserved in stash `park volume func_8005DB8C address-DAG coloring`.

## Function hood and boundaries

Retail span `[0x4E38C,0x4E3AC)`, VRAM `0x8005DB8C`, eight words. It ends in
`jr ra` with the final add in the delay slot. The preceding word at `0x4E388`
is the real return delay slot of `func_8005DB44`; the following word at
`0x4E3AC` is the real first instruction of `func_8005DBAC`.

Four exact direct callers occur at `0x8005B938`, `0x8005BA94`, `0x8005BBF0`,
and `0x8005CCC8`. `FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail body and screens

```text
lui   v0,%hi(D_800A8038)
addiu v0,v0,%lo(D_800A8038)
sll   a0,a0,9
addiu v1,v0,-0x10
lw    v0,0(v0)
addu  a0,a0,v1
jr    ra
addu  v0,v0,a0
```

The helper returns the adjusted base at `D_800A8038 - 0x10`, plus the word
stored at `D_800A8038`, plus a `0x200`-byte index. It has no calls, loops, or
global writes. The blocker is the exact address/value DAG: retail initially
owns the symbol address in `$v0`, copies the adjusted base to `$v1`, and only
then overwrites `$v0` with the load.

## Two bounded phrasings

Attempt 1 applied the shared-address pointer lever proven by
`func_8005DADC`:

```c
unsigned int *address = &D_800A8038;
unsigned char *base = (unsigned char *)address - 0x10;
unsigned int offset = *address;
return base + offset + (a0 * 0x200);
```

cc1 shared the address, but selected the `5DADC` allocation/order:

```text
lui/addiu v1,D_800A8038
lw        v0,0(v1)
addiu     v1,v1,-0x10
sll       a0,a0,9
addu      v0,v0,v1
jr ra / addu v0,v0,a0
```

Attempt 2 made the adjusted base primary and recovered the load through
`base + 0x10`:

```c
unsigned char *base = (unsigned char *)&D_800A8038 - 0x10;
unsigned int offset = *(unsigned int *)(base + 0x10);
return base + offset + (a0 * 0x200);
```

cc1 instead duplicated the symbol materialization: one `lui/lw` for the
value and another `lui/addiu` for the base. Neither phrasing produces retail's
copy-before-load DAG. Both used era `-O2 -G0`; there is no relocation or
optimization-level ambiguity.

## Disposition

`PARKED-ADDRESS-DAG-COLORING`. A retry requires a new source or compiler
hypothesis that specifically makes `$v0` hold the address, creates `$v1` as
the adjusted copy before the load, then reuses `$a0` for the indexed base.
Pins and inline assembly remain forbidden. No YAML/build integration exists.
