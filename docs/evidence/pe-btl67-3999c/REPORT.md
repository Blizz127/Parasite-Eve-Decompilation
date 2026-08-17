# PE-BTL67 — 0x3F enables 3999C on type-0

After Writer A / 1A4AC, type-0 persist==0x27 can finish
mailbox `0x1A` → `scratch[0x12]` → `0x30` → `persist=0x28` →
`0x3F` / `0xAB` / `0x20`. `0x3F` is `D2E8 &= ~1`, the 35C84
gate that **enables** `jal 3999C`.

## `func_8003999C`

118 words `0x8003999C..0x80039B74`, SHA-256 `9a5267b0…3f6f`.
Sole TEXT caller `35C84` @ `35D14`. Zero `jal`.
`a0`=actor, `a1`=`D_800943C0`, `a2`=&code (`actor+0x0E` or
`0x11` when `*(*D254)+0x4C & 0xC0 == 0x80`).

ROM indexes `table[actor+0]`. `2F76C` stores `0x800B8A20`
there. Nonzero rows are 4/5/16/17/21/22/23; row 5 and 21
point at `7136C`. Host returns when the index is not 0..63
so the live pointer does not fault. Do not invent
`actor+0 = command`. `7136C` / `710A4` / `71754` / `716A4`
jalr is not this cut.
