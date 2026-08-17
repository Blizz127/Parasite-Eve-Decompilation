# PE-BTL87 — 3F3C4 dest-change D1A0/B0CD8 stores

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

After `696F0` the dest-change exit stores:

- `D1A0 = (D1A0 | 0x40) & ~0x3800` (`lw/sw gp+0x430`)
- `B0CD8 = (B0CD8 | 2) & ~0x800`
- if `(B0CD8|2) & 0x200`: `B0CD8 = ((B0CD8|2)&~0x800 | 2) & 0xFFFF7DFF`

Live steady ticks keep `D280` stable, so these
stores do not run. Host `D_8009D1A0` is written
with the guest word on the dest-change path.

## Verify

```text
python3 pc_port/tools/pe_btl87_3f3c4_exit_stores_oracle.py
PE_TEST_FILTER=BTL87 ./pc_port/build/pe-native-tests
```
