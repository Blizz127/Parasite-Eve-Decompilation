# PE-BTL63 — 6B4F8 dest load for M0367I

## Token

Watch-arm `0x31` stores `D_8009D280 = 0xA80663C8`. `6E2D0` / `6E454`
decode that to `M0367I` / 367. `D_80093378[366]` is
`rel=0x15050` `packed=0x04E0AA21` (33+170+78 sectors).

Chunk2 SHA-256
`ab9af4f446a6f9a1f8f79f1f80b862c4d516beddbede1229afab2dfc30b44b1e`.
Words 0/1 = `0x26BD8` / `0x2696C`.

## 6B4F8 load prefix

3F074 @ `3F088` jals `6B4F8(D280)` every tick, then `34FC4`.
This cut is dest-change only: three `6E6A8` into overlay
`+0x194/+0x168/+0x18C`, the two `6E1C0` counted loops, then
`72714/726C4/72724` and the existing 12574 publish. `6CDA4`
and `34FC4/125E0` are not this cut.

3F3C4 runs the cut when `D280` changes after the CE90-once
publish and the three dest pointers are already KSEG.
