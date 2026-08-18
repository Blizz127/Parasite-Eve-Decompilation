# PE-BTL62 — opcode 0x89 / 17FF0

## Handler

5 words `0x80017FF0..0x80018004`, SHA-256
`117504bf5c4fa6434b3703c4e26d350c077de797af2ab6dbbb56366c64e9115c`.
`D_800910A0[0x89]`. `addiu $v0,6` / `lui+sw D_8009D28C` / `jr` /
`addiu $v0,1`. Always `v0=1`. argc 0.

Matching `src/func_80017FF0.c` writes the host `extern int`. This
cut writes guest `0x8009D28C` so native `0x94` / 299CC / 2CF24 agree.

## Live type-6

After bit-clear `0x12` and equal `0x8B` misses: `persist[0xA]&1` is
clear, so the script takes `0x94`. Subop `0x0E` is `!=`. Live
`D28C==0` makes `local[3] != 0` false and skips to `+0x394` `0x89`.

After `0x89` the script polls `0x94` until `D28C==7`.

## Mode-7 producer

EXE-wide D28C stores: immediates 0/2/3/4/5/6/8/9/10/11/`-1`.
The only immediate-7 store is `0x8002CF24` (`addiu $v0,7` /
`sw $v0, 0x51C($gp)`), the named victory/exit cut inside the
299CC-family dispatcher. Do not invent that call from 3F3C4.

Related table setters: `0x8A`/`17FDC`=5, `0x95`/`192B8`=0,
`0x96`/`192C8`=8.
