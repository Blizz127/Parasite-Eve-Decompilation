# func_8002F7D8 — 0x6F body create matching C (102 words)

```text
MATCHING_C — func_8002F7D8 era -O2 -G0 + 3W store, 102/102 words
branch      phase5fm-main-barrier-revisit
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x8002F7D8..0x8002F970 exclusive
file        0x1FFD8 size 0x198
scripts/build_us.sh RESULT: EXACT MATCH
yaml C entries: 232
func_8002F7D8.c.o .text: 0x1A0→0x198
```

Claims the first free 220-byte `SlotRecord` at `D_800A5D58`, copies the
216-byte template `D_800109B0` through a stack `Body216`, publishes the
body pointer, increments `D_8009D2EC` into `body[7]`, stores `1<<i` at
`body+8`, optionally `func_8001A680(actor, 2)` if `*(actor+0x98)&0x2000`
is clear.

## 4-register copy shape

Retail both copies (template→stack, stack→body):

```
lw $v0,0($a2) / lw $v1,4 / lw $a0,8 / lw $a1,0xC
sw $v0,0($a3) / sw $v1,4 / sw $a0,8 / sw $a1,0xC
addiu $a2,0x10
bne $a2,$end
addiu $a3,0x10          # delay
# 2-word tail
```

A first-cut 4-word unroll (`dst[0]=src[0]; ... src+=4`) compiled to a
single-`$v0` walk with `-8/-4/0` offsets and a trip-guard `beq`. gcc
2.7.2 emits the retail cluster only for an aligned 216-byte block move
of `struct { unsigned int w[54]; }` (`tmp = *(Body216 *)D_800109B0` and
`*(Body216 *)(D_800A5D5C + 220*i) = tmp`). `memcpy` of `unsigned int*`
is the same expansion; `unsigned char*` memcpy becomes `lwl`/`lwr`.

Linked `0x8002F7F4` and `0x8002F884` match retail four-`lw` / four-`sw`
coloring exactly.

## Other levers

- `D_800A5D5C` as the body-base symbol (not `D_800A5D58+4`)
- `MASPSX_THREE_WORD_SYMBOL_STORE=1` for indexed `D_800A5D58` `lw`/`sw`
- After the second copy, recompute `off = 220 * i` then `id = D_8009D2EC`
  then `body = D_800A5D5C + off` so `lbu` fills the last-`sll` delay
  (`i` in `$a0`, `id` in `$v1`)

Tail of 11718: prefix `0xE8C0`, C `0x198`, then existing `func_8002F970`.
`0x20E40` / `0x24220` unchanged.

## Files

```text
src/func_8002F7D8.c
configs/USA/disc1.yaml          [0x1FFD8, c, func_8002F7D8]
```

## Commands

```text
scripts/split_us.sh
PATH=/tmp/pe-mipsel-wrap:$PATH scripts/build_us.sh
scripts/verify_us.sh
```
