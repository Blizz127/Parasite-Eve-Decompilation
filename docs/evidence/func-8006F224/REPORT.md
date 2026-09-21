# `func_8006F224` — free-slot search over the two command-id arenas

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as
matching-C leaf. Linked at its retail VMA with its two referenced symbols
defined, the object `.text` is **byte-identical** to retail (`LINK_EXACT`).

## Function hood and retail span

- File span `[0x5FA24,0x5FAC4)` = `0xA0` bytes = 40 words.
- VRAM span `[0x8006F224,0x8006F2C4)`.
- Carved from the `0x5F4EC` asm span; immediately precedes matched
  `func_8006F2C4` (`0x8006F2C4`).
- Last instruction pairs with `func_8006F2C4`/`func_8006F820`/`func_8006F8EC`
  and the id-record family; the `D_800942E4`/`D_800942E8` arenas are the
  command-state tables also read by `func_8006F39C`/`func_8006FC18`.

## Semantics (from retail bytes)

```text
8006f224  2c8200c0  sltiu v0,a0,0xC0      ; id < 0xC0 ?
8006f228  14400003  bnez  v0,0x8006F238
8006f22c  2405ffff  li    a1,-1            ; delay: result = -1
8006f230  0801bcaf  j     0x8006F2BC
8006f234  2402ffff  li    v0,-1            ; delay: return -1
8006f238  2482ffba  addiu v0,a0,-0x46
8006f23c  2c42000f  sltiu v0,v0,0xF        ; (unsigned)(id-0x46) < 0xF ?
8006f240  10400012  beqz  v0,0x8006F28C
8006f244  00001821  move  v1,zero          ; delay: i = 0
8006f248  3c048009  lui   a0,0x8009
8006f24c  8c8442e8  lw    a0,-15640(a0)    ; p = D_800942E8  (stride 0x10C)
...
.L8006F250: lbu v0,0(a0) / beqz -> .L8006F27C (result = i+0xB)
            addiu v1,v1,1 / slti v0,v1,0xB / bnez .L8006F250
            addiu a0,a0,0x10C (delay)
...
.L8006F28C: lui a0,0x8009 / lw a0,D_800942E4 ; p = D_800942E4 (stride 0xA0C)
.L8006F294: lbu v0,0(a0) / beqz -> .L8006F284 (result = i)
            addiu v1,v1,1 / slti v0,v1,0xB / bnez .L8006F294
            addiu a0,a0,0xA0C (delay)
.L8006F2B8: move v0,a1   ; return result
```

Returns the first free slot: ids `0x46..0x54` map to the `D_800942E8` arena
with slot number `i+0xB`; all other accepted ids map to the `D_800942E4`
arena with slot number `i`. Both loops probe the in-use byte at record
offset 0, and a full arena yields the initial `-1`.

C shape (`src/func_8006F224.c`):

```c
int func_8006F224(int id) {
    int i;
    int result = -1;
    unsigned char *p;
    if ((unsigned int)id >= 0xC0)
        return -1;
    if ((unsigned int)(id - 0x46) < 0xF) {
        p = D_800942E8;
        for (i = 0; i < 0xB; i++) {
            if (p[0] == 0) { result = i + 0xB; break; }
            p += 0x10C;
        }
    } else {
        p = D_800942E4;
        for (i = 0; i < 0xB; i++) {
            if (p[0] == 0) { result = i; break; }
            p += 0xA0C;
        }
    }
    return result;
}
```

## Levers

1. **Arena bases are pointer globals.** Retail loads each base with
   `lui`/`lw` (`lw a0,D_800942E8`), not with `lui`/`addiu`. Declaring the
   arenas `unsigned char *` and indexing by pointer bump reproduces the
   `lui`/`lw` base load; declaring them as arrays emits `lui`/`addiu` and
   fails.
2. **One shared `result` variable** (so cc1 keeps `-1` in `$a1` and both
   `break`s materialize the slot number into the same register) matches the
   retail `move v0,a1` tail shared by both loops.
3. **Threshold test written positive** (`(unsigned)(id - 0x46) < 0xF`) keeps
   the `beqz`-to-second-loop layout retail uses.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
  tools/analysis/era_leaf_match.sh src/func_8006F224.c 0x8006F224 0xA0 -O2 -G0
```

Result: `MISMATCHES=8`, size `0xA0` = ROM. Every mismatch is a link-time
relocation field:

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x8006F230` | `0801bcaf` | `08000026` | R_MIPS_26 local `.L8006F2BC` |
| `0x8006F248` | `3c048009` | `3c040000` | R_MIPS_HI16 `D_800942E8` |
| `0x8006F24C` | `8c8442e8` | `8c840000` | R_MIPS_LO16 `D_800942E8` |
| `0x8006F274` | `0801bcaf` | `08000026` | R_MIPS_26 local |
| `0x8006F27C` | `0801bcae` | `08000025` | R_MIPS_26 local |
| `0x8006F284` | `0801bcae` | `08000025` | R_MIPS_26 local |
| `0x8006F28C` | `3c048009` | `3c040000` | R_MIPS_HI16 `D_800942E4` |
| `0x8006F290` | `8c8442e4` | `8c840000` | R_MIPS_LO16 `D_800942E4` |

Link-level proof (assemble, link at `0x8006F224` with
`--defsym D_800942E4=0x800942E4 --defsym D_800942E8=0x800942E8`, compare the
linked `.text` word-for-word with the ROM):

```text
linked .text 160 bytes, target 0xa0, word mismatches=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006F224.c`; YAML carve
  `- [0x5FA24, c, func_8006F224]` (asm span `0x5F4EC` now ends at `0x5FA24`;
  resume `func_8006F2C4` at `0x5FAC4`). Default `era_o2_g0` profile.
- `python3 tools/build/disc1_plan.py --check` → 859 spans
  (569 c, 288 asm, 2 rodata), geometry `0x1EE000`.
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
