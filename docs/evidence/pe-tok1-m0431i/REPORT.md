# PE-TOK1 — New-Game token 0xA80830C8 is field map M0431I

`0xA80830C8` (published by the `func_8006E9A0` arg-1 dispatch and the
`1220C` `0xAA108448` arm) unpacks through the retail
`func_8006E2D0`/`func_8006E454` pair to name `M0431I`, index 431, package
table slot 430. The decode reuses the BTL150-proven 5-bit scheme
(shifts 27/22/17/12/7/2 over the `0x800930B4` charset, validated here
against both proven tokens: `0xA8066048`→m0360i, `0xA80663C8`→m0367i).
The arg-3 token `0xA80651C8` is `M0353I`, index 353, slot 352.

Route: `0xA80830C8 < 0xA9400048` (the 1220C state value) and
`!= 0xA8000048` (m0000i), so the 1220C three-way dispatch falls to its
else arm and calls the field tick `func_8003F3C4`, whose dest-change
latch (`D_8009D280` vs loaded dest, overlay KSEG chunks, `B0DD8`,
`dest_ready_cut`) enter the map through the translated
`func_8006B4F8_dest_load_cut`.

Field-entry ground truth for M0431I (static package table `D_80093378`,
authenticated by oracle): slot 430 `rel = 0x182CF`,
`packed = 0x01200911` — three PE.IMG chunks of 17 + 9 + 18 sectors from
`B0DD8 + 0x182CF` into overlay `+0x194` / `+0x168` / `+0x18C`, then the
`6E1C0` loops, `72714/726C4/72724`, and the 12574 publish. (BTL63 proved
the same machinery live for M0367I: slot 366 `0x15050`/`0x04E0AA21`,
33+170+78.)

## Verify

```text
python3 pc_port/tools/pe_tok1_m0431i_oracle.py
PE_TEST_FILTER=TOK1 ./pc_port/build/pe-native-tests
```
