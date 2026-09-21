# func_80036E7C — matched (709th C leaf)

## Identity

```text
VRAM        0x80036E7C
file        0x2767C
size        0x100 / 64 words
asan unit   (was) 0x2767C, asm
```

Straight-line, no `jal`. Packs three `(a0[1] % 60)` fields into `a1[0]`:

```text
a1[0] bits 0..19  = (x%60) * 216000 & 0xFFFFF
a1[0] bits 20..25 = (x%60) * 3600 & 0x3F  << 20
a1[0] bits 28..31 = (x%60) * 15           << 28
```

## Source (matches byte-exact)

```c
void func_80036E7C(unsigned int *a0, unsigned int *a1)
{
    a1[0] = (a1[0] & 0xFFF00000u) | ((a0[1] % 60u) * 216000u & 0xFFFFFu);
    a1[0] = (a1[0] & 0xFC0FFFFFu) | (((a0[1] % 60u) * 3600u & 0x3Fu) << 20);
    a1[0] = (a1[0] & 0x03FFFFFFu) | ((a0[1] % 60u) * 15u << 28);
}
```

## The lever

The operand type of `%` is the whole match. Retail emits the unsigned
constant-division sequence `multu $a3,0x88888889` / `mfhi $t2` / `srl $v1,$t2,5`
and keeps the magic constant live in `$t0` across all three fields. An `int`
operand makes cc1 add the signed sign-correction sequence (12 extra words); an
`unsigned int` operand reproduces retail exactly, and the three read-modify-write
statements keep the running `a1[0]` in one register (retail loads it once).

Prior attempts on this leaf: `int` best 31/64 words differing; an `int v`
accumulator form 70/74 differing. `unsigned` -> 0 differing.

## Verification

```text
bash scripts/split_us.sh                     -> 1032 spans (709 c, 321 asm, 2 rodata)
distrobox enter pe-mipsel -- bash -lc \
  'cd /tmp/pe-agent-36e7c && bash scripts/build_us.sh'
  -> RESULT: EXACT MATCH
     Matching claim: YES (709 registered C leaves)
     retail/candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
distrobox enter pe-mipsel -- bash -lc \
  'cd /tmp/pe-agent-36e7c && bash scripts/verify_us.sh'
  -> VERIFY_US=PASS
     PASS all 709 packed C spans equal retail
```
