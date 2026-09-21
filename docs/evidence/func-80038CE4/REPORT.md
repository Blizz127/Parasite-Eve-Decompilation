# func_80038CE4 — two-level indexed byte getter

VRAM `0x80038CE4`, size `0x28` (10 words), file offset `0x294E4`.
Carved from the head of the former `0x29154` asm span (prefix `0x390`),
closing exactly at the next span `0x2950C`:

```
- [0x294E4, c, func_80038CE4]
```

Retail: `base = *D_80091A28; p = base + (a0 & 0xFF); v = p[0x1D];
return base[v + 4];`

## Profile

Default `era_o2_g0` (`-O2 -G0`).

## Commands

```
env -u MASPSX_SYMBOL_AT_TEMP -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/check_leaf.sh func_80038CE4 0x80038CE4 0x28 -O2 -G0
```

Result: `LINK_EXACT`; deep span-size exact.

## Source

```c
extern unsigned char *D_80091A28;
int func_80038CE4(unsigned int a0) {
    unsigned char *base = D_80091A28;
    unsigned char *p = base + (a0 & 0xFF);
    unsigned int v = *(unsigned char *)(p + 0x1D);
    return *(unsigned char *)(base + v + 4);
}
```

The `nop` after retail's `lbu $v1,0x1D($a0)` is the load-delay hazard
before the second `addu`, reproduced naturally by the named `v` local.
