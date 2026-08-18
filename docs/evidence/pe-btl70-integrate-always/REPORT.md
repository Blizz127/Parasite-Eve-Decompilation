# PE-BTL70 — 35C84/35E04 always apply +0x68

BTL11 treated `+0x98` bit 1 as the whole-motion gate.
**REJECTED.** ROM `35D28` / `35E78` are `beq` +13 into the
`+0x68 += +0x78; +0x28 += +0x68` body. Bit 1 only runs the
preceding `+0x88/+0x8C/+0x90` add.

Live type-0 after `0x3F` therefore moves when `710A4`/`7136C`
write `+0x68`, even with bit 1 clear. Do not invent pad;
`D26C=0` still writes zero velocity.
