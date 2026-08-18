# PE-BTL47 — type-2 0x12 / 131E8 script-task fork

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_800131E8` — 70 words `0x800131E8..0x80013300`,
SHA-256 `ae11241b…3e1a`. `D_800910A0[0x12]`.
Zero jal. v0=1.

```text
pc = (D2F0+0x9C) + (*arg0 << 1)
pop D_8009CDFC and init like 12700
if (D300+8) & 3 == 0:
    insert new at current+0x24
else:
    prepend new onto actor+0xA8
```

Live type-2 `scratch[0]&4==0` skips to three
`0x12` (imms `0x75E`, `0x6FE`, `0x6B0`) then
`0x6A`. Live mailbox tasks have bit 2 set, not
bits 0–1, so the +0x24 insert arm is the
authentic one. Type-6 still waits on
scratch[0]&4 before its `0x12`. Do not force
that bit.

Fork `0x75E` is an already-ported 0x59/0x20
wait loop. Forks `0x6FE` / `0x6B0` next hit
unported `0x54` / `0x4B` after their `0x02`.
The continuing task hits `0x6A` → `6F39C(0x75)`.

## Verify

```text
python3 pc_port/tools/pe_btl47_131e8_oracle.py
PE_TEST_FILTER=BTL47 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
