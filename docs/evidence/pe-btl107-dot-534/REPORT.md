# PE-BTL107 — 27D14 DoT; 5C498 → gp+0x534

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

`27D14` `$s1` is `*actor` for the whole body. ATB is
`body+0x0C` / `+0x8E`. When `body&0x10` and
`(body>>5)&0x1F >= 0x1E`, `body+0x10 -= body+0x96`
and the shift field is cleared (`andi 0xFC1F`).
That is the in-function HP subtract; it is not
Aya `1F704`.

`299CC` @ `0x80029A5C` jals `5C498` and `sh`s `$v0`
to `gp+0x534` before the mode!=0 branch. `42ED0`
nonzero returns 0. Else `51504` zeros `D_8009D010`
and `514F8` returns it. `512AC` jtbl[10] writes
1000. The 5C498 jal chain that can reach that case
is not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl107_dot_534_oracle.py
PE_TEST_FILTER=BTL107 ./pc_port/build/pe-native-tests
```
