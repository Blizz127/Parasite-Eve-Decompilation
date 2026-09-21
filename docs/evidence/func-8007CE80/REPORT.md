# func_8007CE80 — 0x2C bytes, LINK_EXACT

Retail VRAM `0x8007CE80` (file offset `0x6D680`), in `asm/disc1/6CD60.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

`a2`-element word copy with a **pre-tested count** (`beqz $a2` before the loop,
no `slt` at the top). The load must be kept in a named local so retail's order
(`lw a1` / `addiu a1` / `addiu i` / `sw a0` / `sltu` / `bnez` / `addiu a0`)
survives — `*a0 = *a1` directly interleaves the store into the second slot and
puts `addiu i` third (3-word residual).

```c
void func_8007CE80(unsigned int *a0, unsigned int *a1, unsigned int a2) {
    unsigned int i = 0;
    if (a2 != 0) {
        do {
            unsigned int v = *a1;
            a1++;
            i++;
            *a0 = v;
            a0++;
        } while (i < a2);
    }
}
```

```
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu"
export PATH="$PWD/tools/mipsel-host/bin:$PATH"
tools/analysis/era_link_match.sh src/func_8007CE80.c 0x8007CE80 0x2C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8007CE80.c 0x8007CE80 0x2C -O2 -G0
```

Object: `ROM .text 44 bytes  C .text 64 bytes`; the surplus is the trailing gas
alignment pad. Link-level `word mismatches=0` → `LINK_EXACT`.
