# func_80019410 — mode guard on D_800BCFEE

VRAM `0x80019410`, size `0x40` (16 words), file offset `0x9C10`.
Carved from the tail of the former `0x9BD8` asm span (prefix `0x38`),
closing exactly at the next span `0x9C50`.

Retail: `if ((D_800BCFEE & 3) < 2) return 1;` else push
`D_8009CE00 -= 8` and set `D_8009D300->f+0x10 = 1`, return 0.
The `sltiu` proves the masked byte is compared as unsigned.

## Profile

`era_o2_g8_force_d800bcfee_absolute` — era `-O2 -G8` with
`MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800BCFEE` (the `.data` byte is out of
±32K of `$gp`, so plain `-G8` fails to link); the two int destinations
stay gp-relative (`0x90($gp)`, `0x590($gp)`).

## Commands

```
env -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  bash -c 'export MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800BCFEE; \
    tools/analysis/check_leaf.sh func_80019410 0x80019410 0x40 -O2 -G8'
```

Result: `LINK_EXACT`; deep span-size exact. `-O1 -G8` mismatches 11.

## Source

```c
extern unsigned char D_800BCFEE;
extern int D_8009CE00;
extern unsigned char *D_8009D300;
int func_80019410(void) {
    if ((D_800BCFEE & 3) < 2) return 1;
    D_8009CE00 -= 8;
    *(int *)(D_8009D300 + 0x10) = 1;
    return 0;
}
```
