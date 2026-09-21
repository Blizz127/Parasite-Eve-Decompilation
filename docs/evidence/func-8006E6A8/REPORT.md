# `func_8006E6A8` — sector-read issue wrapper

Outcome: **MATCHED** on the first natural C phrasing under era `-O2 -G0`
(YAML default profile). Integrated as matching-C leaf 561.

## Function hood and retail span

- File span: `[0x5EEA8,0x5EED4)` = `0x2C` bytes = 11 words.
- VRAM span: `[0x8006E6A8,0x8006E6D4)`.
- Canonical `jr ra; nop` at `0x8006E6CC/0x8006E6D0`.
- Preceding word pair is the real `jr ra; nop` ending the previous function
  at `0x8006E6A0/0x8006E6A4`.
- Following word is the real `addiu sp,sp,-0x30` entry of `func_8006E6D4`
  at `0x8006E6D4`.
- 26 direct `jal` sites encode `0C01B9AA` (target `0x8006E6A8`):

```text
80014EA4 80014F14 80038A20 80038DC4 80069414 80069B84
80069BE4 8006AA5C 8006AAB0 8006ABEC 8006AC4C 8006ADA4
8006ADF4 8006AF88 8006B080 8006B0F0 8006B188 8006B594
8006B608 8006B6EC 8006BDDC 8006BF78 8006C090 8006C6A8
8006C7C4 8006F424
```

`FUNCTION_HOOD=PROVEN`. Callable function, not padding or a mis-split tail.

## Retail body (from `SLUS_006.62` SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`)

```text
8006e6a8  27bdffe8  addiu sp,sp,-24
8006e6ac  afbf0010  sw    ra,16(sp)
8006e6b0  00a01021  move  v0,a1
8006e6b4  00c03821  move  a3,a2
8006e6b8  00002821  move  a1,zero
8006e6bc  0c01b9b5  jal   func_8006E6D4   # 0x8006E6D4
8006e6c0  00403021  move  a2,v0           # delay: dest
8006e6c4  8fbf0010  lw    ra,16(sp)
8006e6c8  27bd0018  addiu sp,sp,24
8006e6cc  03e00008  jr    ra
8006e6d0  00000000  nop
```

Semantics: `func_8006E6A8(lba, dest, sectors)` →
`func_8006E6D4(lba, 0, dest, sectors)`, return value forwarded. `a0` is
passed through. Frame is 24 bytes (`ra` + 16-byte outgoing arg area).
The 4th argument to `func_8006E6D4` is a sector count (callers in
`func_8006A9E4` / native `func_8006E6A8_port.c`).

## Screens

| Screen | Result |
|---|---|
| Frame | args 16 + ra 8 = `0x18`; one saved register |
| Callee | one `jal func_8006E6D4` |
| Globals | none |
| `-O` signal | arg shuffle into `v0`/`a3`/`a1=0` with dest moved in the jal delay slot |
| Relocations | one `R_MIPS_26` for the jal |

Flags: era `-O2 -G0`. No GP, pins, inline assembly, or maspsx knobs.

## Minimal C

```c
extern int func_8006E6D4(int lba, int mode, unsigned char *dest, int sectors);

int func_8006E6A8(int lba, unsigned char *dest, int sectors)
{
    return func_8006E6D4(lba, 0, dest, sectors);
}
```

## Single-leaf object

Command:

```text
AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  OBJCOPY=mipsel-linux-gnu-objcopy \
  tools/analysis/era_leaf_match.sh src/func_8006E6A8.c 0x8006E6A8 0x2C -O2 -G0
```

All 11 instruction words match retail except the unlinked `jal` placeholder
`0c000000` vs ROM `0c01b9b5` (R_MIPS_26 against `func_8006E6D4`). Extra
`.text` byte is GNU as alignment `nop` pad (`C=0x30` vs `ROM=0x2C`).

YAML mid-`5E39C` carve: prefix `0xB0C`, C `0x2C`, resume `[0x5EED4, asm]`
to existing `func_8006E7E8` at `0x5EFE8`.
