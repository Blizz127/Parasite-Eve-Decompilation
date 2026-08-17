# PE-BTL64 — M0367I type-1 spawn and opcode 0x03

## Dest-change rebuild

After 6B4F8 loads M0367I, 3F074's pool tail is `34FC4` /
`1266C` / `125E0`. The 125E0 desc count is 1: type 1 id 0.
Do not publish `B0E70` (still gated on the 3D050 tail).

## 0x03 / `0x80017C54`

14 words, SHA-256 `4d53aeb5…fa24`. `jal 0x800661EC` with
`lh`/`lh`/`lhu` and `a3=0`. Always `v0=1`.

## `func_800661EC`

31 words `0x800661EC..0x80066268`, SHA-256 `095ed047…ba28`.
`BCF88&0x40` clear → return -19, no stores. Set →
`BCFA0=1`, `BCF9C/9E/A2=a0/a1/a2`, `BCF98=*BCF8C`,
`BCF88=(flags&0xFFF0)|(a3==8?1:9)`, return 0.

Live M0367I `+0x1C8` after persist `0x4A==0x26`:
`(0x12B, 0x7C, 1)`. Twin `0x46` / 17C8C passes `a3=8`.
