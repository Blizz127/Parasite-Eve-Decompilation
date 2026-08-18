# PE-BTL14 — m0005i spawn-list publish; opcode 0xCE

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

1A918 / E0060 / 371B0 / 125E0 / 2F76C / 35C84 are already
ported. This rung does not reopen them.

## Live 125E0 list is not type 0

`3F074` jals `6B4F8(D_8009D280)` at `0x8003F088` before the
poll. Token `0xA80002C8` decodes (`6E2D0`/`6E454`) to
`M0005I`, table index 4, PE.IMG rel `0x266A`, packed
`0x0600A921` (33+169+96 sectors). Chunk 2 SHA-256
`01a64ba3…7e3b`.

12574 list at chunk2+`0x202A4` has 7 type entries. The
`*header` desc at +`0x4D70` is:

```text
count=2
actor[0] type=1 idB=0
actor[1] type=6 idB=0
```

a1=0 insert puts type 6 at `D_8009D20C` head. Both use
vtable `35E04`. Type 0 / `35C84` / `3999C` are not on this
125E0 spawn. Type 0 is later opcode `0x08` (`1735C` jal
`35038`) on the type-1 stream. Do not call that type-0.

## First live 17018 word

Type-6 stream at chunk2+`0x2341C` starts `0x000080CE`
(op `0xCE`, argc 4, kinds 0, imms `0,0,0xFF,0xFFFFFBA9`).
`D_800910A0[0xCE]=0x800181CC` (53w, SHA-256 `79f58896…`).
`*arg0==0` → already-ported `2FF78(255, 0xFFFFFBA9)` →
`D_800942EC=0xFBA9`. Returns 1.

ROM `17018@0x80017248` `bne v0,0 → 0x800170F0` re-fetches
from `gp+0x90`, not `*task`. Next word is op `0xEA`
(`15DAC`, 729w, 56 jals) — not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl14_m0005i_publish_oracle.py
python3 pc_port/tools/pe_btl8_1a918_oracle.py
PE_TEST_FILTER=BTL14 ./pc_port/build/pe-native-tests
```
