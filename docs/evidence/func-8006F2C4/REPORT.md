# `func_8006F2C4` — reset one command record by id

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as
matching-C leaf. Linked at its retail VMA with its four referenced symbols
defined, the object `.text` is **byte-identical** to retail (`LINK_EXACT`).

## Function hood and retail span

- File span `[0x5FAC4,0x5FB9C)` = `0xD8` bytes = 54 words.
- VRAM span `[0x8006F2C4,0x8006F39C)`.
- Carved from the `0x5F4EC` asm span; immediately precedes `func_8006F39C`.
- Uses the same `D_800942E4`/`D_800942E8` command-id arenas as
  `func_8006F224`/`func_8006F820`, plus the `D_800942E0` handler table and
  the `D_800E10A0` voice table.

## Semantics (from retail bytes)

```text
8006f2c4  2c820016  sltiu v0,a0,0x16      ; id < 0x16 ?
8006f2c8  14400003  bnez  v0,0x8006F2D8
8006f2cc  2c82000b  sltiu v0,a0,0xB       ; delay: id < 0xB ?
8006f2d0  0801bce5  j     0x8006F394
8006f2d4  2402ffff  li    v0,-1            ; delay: return -1
8006f2d8  1440000A  bnez  v0,0x8006F304    ; id<0xB -> D_800942E4 arm
   (id>=0xB)  addiu v0,a0,-0xB / sll v1,v0,4 / addu / sll 2 / subu
              lui v0,0x8009 / lw v0,D_800942E8        ; base (stride 0x10C)
8006f304   (id<0xB)  addu v0,v0,a0 / sll 5 / addu / sll 2 / subu
              lui v1,0x8009 / lw v1,D_800942E4        ; base (stride 0xA0C)
8006f324  addu a1,v1,v0                     ; a1 = record
8006f328  lbu v1,1(a1) / li v0,0x72 / bne  -> .L8006F378
   ; if (record[1] == 0x72)
8006f338  i=0x6C / lui v1,D_800E10A0
.L8006F344: sw zero,0(v1) / i++ / sltiu i,0x73 / bnez delay addiu v1,4
8006f358  lui a0,0xFFFE ; ori a0,0xFFFF ; lui v1,D_800B0CD8
          lw v0,0(v1) ; and v0,a0 ; sw v0,0(v1)   ; clear bit16 of flags
.L8006F378: v1=0xFF ; sb zero,0(a1) / sb v1,1 / sb v1,2 / sb v1,3
            sw zero,4(a1) / sw zero,8(a1)
8006f394  jr ra / nop
```

Clears one record: bytes 0..3 become `00 FF FF FF` and the two words at
`+4`/`+8` are zeroed. For a record whose handler byte (`+1`) is `0x72`, it
additionally zeroes the seven `D_800E10A0` voice-table words (indices
`0x6C..0x72`) and clears bit 16 of `D_800B0CD8`. Out-of-range ids return
`-1`, success returns `0`.

C shape (`src/func_8006F2C4.c`):

```c
int func_8006F2C4(int idx) {
    unsigned char *p;
    if ((unsigned int)idx >= 0x16)
        return -1;
    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;
    if (p[1] == 0x72) {
        unsigned int i;
        unsigned int *flagsPtr = &D_800B0CD8;
        for (i = 0x6C; i < 0x73; i++)
            D_800E10A0[i - 0x6C] = 0;
        *flagsPtr &= 0xFFFEFFFF;
    }
    p[0] = 0; p[1] = 0xFF; p[2] = 0xFF; p[3] = 0xFF;
    *(int *)(p + 4) = 0;
    *(int *)(p + 8) = 0;
    return 0;
}
```

## Levers

1. **Arena bases are pointer globals.** `lui`/`lw` in retail means the
   `D_800942E4`/`D_800942E8` symbols hold pointers, so declare
   `extern unsigned char *` and compute `base + i*stride`. An array
   declaration would emit `lui`/`addiu` (address-of) and mismatch.
2. **Branch polarity / block layout.** Writing the arena selector as
   `if ((unsigned)idx >= 0xB) <E8 arm> else <E4 arm>` makes cc1 place the
   `D_800942E4` arm as the `bnez` target (retail `bnez v0,0x8006F304`).
   The mirrored `idx < 0xB` form flips the layout and mismatches 17 words.
3. **One `&D_800B0CD8` pointer local** so the load/and/store share a single
   base register (`lui v1` + `lw 0(v1)` + `sw 0(v1)`); a flat global
   `D_800B0CD8 &= …` makes cc1 use two independent absolute accesses.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
  tools/analysis/era_leaf_match.sh src/func_8006F2C4.c 0x8006F2C4 0xD8 -O2 -G0
```

Result: `MISMATCHES=10`, size `0xE0` (4-word link pad) vs ROM `0xD8`; every
mismatch is a link-time relocation field:

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x8006F2D0` | `0801bce5` | `08000034` | R_MIPS_26 local `.L8006F394` |
| `0x8006F2F4` | `3c028009` | `3c020000` | R_MIPS_HI16 `D_800942E8` |
| `0x8006F2F8` | `8c4242e8` | `8c420000` | R_MIPS_LO16 `D_800942E8` |
| `0x8006F2FC` | `0801bcc9` | `08000018` | R_MIPS_26 local `.L8006F324` |
| `0x8006F318` | `3c038009` | `3c030000` | R_MIPS_HI16 `D_800942E4` |
| `0x8006F31C` | `8c6342e4` | `8c630000` | R_MIPS_LO16 `D_800942E4` |
| `0x8006F33C` | `3c03800e` | `3c030000` | R_MIPS_HI16 `D_800E10A0` |
| `0x8006F340` | `246310a0` | `24630000` | R_MIPS_LO16 `D_800E10A0` |
| `0x8006F35C` | `3c03800b` | `3c030000` | R_MIPS_HI16 `D_800B0CD8` |
| `0x8006F360` | `24630cd8` | `24630000` | R_MIPS_LO16 `D_800B0CD8` |

Link-level proof (assemble, link at `0x8006F2C4` with
`--defsym D_800942E4=0x800942E4 --defsym D_800942E8=0x800942E8
--defsym D_800E10A0=0x800E10A0 --defsym D_800B0CD8=0x800B0CD8`, compare the
linked `.text` word-for-word with the ROM):

```text
linked .text 224 bytes, target 0xd8, word mismatches=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006F2C4.c`; YAML carve
  `- [0x5FAC4, c, func_8006F2C4]` inside the `0x5F4EC` asm span (resume asm
  at `0x5FB9C`). Default `era_o2_g0` profile.
- `python3 tools/build/disc1_plan.py --check` → 859 spans
  (569 c, 288 asm, 2 rodata), geometry `0x1EE000`.
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
