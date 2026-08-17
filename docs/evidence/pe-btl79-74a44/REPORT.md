# PE-BTL79 — 70E54 jals ResetGraph(1)

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`70EC0` jals `74A44` with `a0=1` after VSync(2).
`74A44` is already translated ResetGraph.
`mode&7==1` is the light path: no guest-RAM
stores, `v0=0`. Next `70E54` jal is `755F0`
(disp env at `BCE80 + 20*(gp+108)`).

## Verify

```text
python3 pc_port/tools/pe_btl79_74a44_oracle.py
PE_TEST_FILTER=BTL79 ./pc_port/build/pe-native-tests
```
