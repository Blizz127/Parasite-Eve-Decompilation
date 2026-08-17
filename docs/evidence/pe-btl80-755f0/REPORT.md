# PE-BTL80 — 70E54 jals PutDispEnv

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`70EE4` jals `755F0` with
`a0 = BCE80 + 20*(gp+108)`. `gp+108` is
`D_8009CDDC`. `755F0` is the existing
PutDispEnv host shim (`HostFB_Present`).
Next `70E54` jal is `6EC08`.

## Verify

```text
python3 pc_port/tools/pe_btl80_755f0_oracle.py
PE_TEST_FILTER=BTL80 ./pc_port/build/pe-native-tests
```
