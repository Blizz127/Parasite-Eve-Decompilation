# PE-BTL77 — 3F3C4 jals 661A4 then 661CC

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`661A4` — 10 words `0x800661A4..0x800661CC`,
SHA-256 `79eb7a67…de66`. Zero jal. `lhu BCF94/96`,
`sll 16`, `mtc2` OFX/OFY. Already ported.

`661CC` — 8 words. `SetGeomOffset(160, 112)`.
Already ported. Runs after overlay `E01BC`
(`3F578`). This cut does not jal `E01BC`.
After `661CC`, OFX/OFY are `160<<16` / `112<<16`.

`3F3C4` now jals `661A4` then `661CC` on the
same `B0CD8&0x100/0x200` gate as `68CE0`.
Next jal is `70E54` (`3F590`).

## Verify

```text
python3 pc_port/tools/pe_btl77_661a4_oracle.py
PE_TEST_FILTER=BTL77 ./pc_port/build/pe-native-tests
```
