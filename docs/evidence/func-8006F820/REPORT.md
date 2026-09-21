# `func_8006F820` — get/set one command-record state byte

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as
matching-C leaf. Linked at its retail VMA with its three referenced symbols
defined, the object `.text` is **byte-identical** to retail (`LINK_EXACT`).

## Function hood and retail span

- File span `[0x60020,0x600EC)` = `0xCC` bytes = 51 words.
- VRAM span `[0x8006F820,0x8006F8EC)`.
- Carved from the `0x5F4EC` asm span (that span resumes after the
  `func_8006F2C4` carve); immediately precedes `func_8006F8EC`.
- Uses the `D_800942E4`/`D_800942E8` arenas and the `D_800942E0` handler
  pointer table.

## Semantics (from retail bytes)

```text
8006f820  2c820016  sltiu v0,a0,0x16      ; id < 0x16 ?
8006f824  14400003  bnez  v0,0x8006F834
8006f828  2c82000b  sltiu v0,a0,0xB       ; delay
8006f82c  0801be39  j     0x8006F8E4
8006f830  2402fff3  li    v0,-0xD          ; delay: return -0xD
8006f834  1440000A  bnez  v0,0x8006F860    ; id<0xB -> D_800942E4 arm
   (id>=0xB)  ... / lw v0,D_800942E8       ; stride 0x10C
8006f860   (id<0xB)  ... / lw v1,D_800942E4 ; stride 0xA0C
8006f880  addu a0,v1,v0                    ; a0 = record
8006f884  lbu v1,1(a0) / sltiu v0,v1,0x55
8006f890  bnez v0,0x8006F89C / delay nop
8006f898  li v1,0x55                       ; clamp handler index to 0x55
8006f89c  lw v0,D_800942E0 / sll v1,2 / addu / lw v0,0(v1)
8006f8b4  beqz v0,-> return -0xF
8006f8bc  bnez a1,0x8006F8D4               ; mode!=0 -> store through arg
8006f8c0  sltiu v0,a2,0x6                  ; delay
8006f8c4  beqz v0,0x8006F8E0
8006f8cc  j 0x8006F8E0 / delay sb a2,0(a0) ; p[0] = arg if arg<6
.L8006F8D4: lbu v0,0(a0) / sw v0,0(a2)      ; *arg = p[0]
.L8006F8E0: lbu v0,0(a0)                    ; return p[0]
8006f8e4  jr ra / nop
```

Reads/writes the state byte of the record whose handler (record offset 1,
clamped to `0x55`) indexes `D_800942E0`. A null handler fails with `-0xF`,
an out-of-range id with `-0xD`. When `mode == 0` the third argument is an
integer `0..5` written to the state byte; otherwise it is an out-pointer
receiving the current byte. Always returns the current byte.

C shape (`src/func_8006F820.c`):

```c
int func_8006F820(int idx, int mode, int arg) {
    unsigned char *p;
    int h;
    if ((unsigned int)idx >= 0x16)
        return -0xD;
    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;
    h = p[1];
    if ((unsigned int)h >= 0x55)
        h = 0x55;
    if (D_800942E0[h] == 0)
        return -0xF;
    if (mode == 0) {
        if ((unsigned int)arg < 6)
            p[0] = arg;
    } else {
        *(int *)arg = p[0];
    }
    return p[0];
}
```

## Levers

1. **Arena bases are pointer globals** (`lui`/`lw`), as in
   `func_8006F224`/`func_8006F2C4`; declare `unsigned char *`.
2. **`D_800942E0` is a pointer to an array of pointers** (`lui`/`lw` base,
   then `sll`/`addu`/`lw`), declared `extern void **`.
3. **Dual-use third argument.** Retail uses `$a2` as an integer in the
   `mode==0` path (`sltiu $v0,$a2,6` then `sb $a2,0($a0)`) and as an address
   in the `mode!=0` path (`sw $v0,0($a2)`). The parameter is typed `int` and
   cast to `int *` for the store.
4. **Branch polarity / block layout.** Writing the test as `mode == 0` with
   the get-arm as the `else` makes cc1 lay the store block out as the
   `bnez $a1` target. The `mode != 0`-first form instead emits
   `beq $5,$0,<set>` and fails at `0x8006F8BC`.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
  tools/analysis/era_leaf_match.sh src/func_8006F820.c 0x8006F820 0xCC -O2 -G0
```

Result: `MISMATCHES=9`, size `0xD0` (4-word link pad) vs ROM `0xCC`; every
mismatch is a link-time relocation field:

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x8006F82C` | `0801be39` | `08000031` | R_MIPS_26 local `.L8006F8E4` |
| `0x8006F850` | `3c028009` | `3c020000` | R_MIPS_HI16 `D_800942E8` |
| `0x8006F854` | `8c4242e8` | `8c420000` | R_MIPS_LO16 `D_800942E8` |
| `0x8006F858` | `0801be20` | `08000018` | R_MIPS_26 local `.L8006F880` |
| `0x8006F874` | `3c038009` | `3c030000` | R_MIPS_HI16 `D_800942E4` |
| `0x8006F878` | `8c6342e4` | `8c630000` | R_MIPS_LO16 `D_800942E4` |
| `0x8006F89C` | `3c028009` | `3c020000` | R_MIPS_HI16 `D_800942E0` |
| `0x8006F8A0` | `8c4242e0` | `8c420000` | R_MIPS_LO16 `D_800942E0` |
| `0x8006F8CC` | `0801be38` | `08000030` | R_MIPS_26 local `.L8006F8E0` |

Link-level proof (assemble, link at `0x8006F820` with
`--defsym D_800942E4=0x800942E4 --defsym D_800942E8=0x800942E8
--defsym D_800942E0=0x800942E0`, compare the linked `.text` word-for-word
with the ROM):

```text
linked .text 208 bytes, target 0xcc, word mismatches=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006F820.c`; YAML carve
  `- [0x60020, c, func_8006F820]` (asm resumes `0x5FB9C`→`0x60020`, then
  `0x600EC`). Default `era_o2_g0` profile.
- `python3 tools/build/disc1_plan.py --check` → 859 spans
  (569 c, 288 asm, 2 rodata), geometry `0x1EE000`.
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
