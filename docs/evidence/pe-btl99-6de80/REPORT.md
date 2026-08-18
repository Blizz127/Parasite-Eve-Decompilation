# PE-BTL99 — 6DE80 after 1A680

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. `1F814` / `6DE80` are NONMATCHING_C
native ports. MATCHED yaml C leaves stay 227.

## Live path

```
1F4D4 → 1F704 HP-=s0 → HP!=0 → 1F814 → 305C8 → 1A680
→ 6DE80(0x46A, 0, lh Aya+0x2A/+0x2E/+0x32)
→ +0x98 bit 0x100 clear (gated)
→ D1D0 publication (gated)
```

Death is not at `1F4D4`. HP==0 skips `1F814` (and
therefore this `6DE80`). Mode stays 0. `4D4` stays 1.

## 6DE80

21 words `0x8006DE80..0x8006DED4`, SHA-256
`860d94bc…26529d9`.

```
6DED4(lw(D_800B0E08), a0, a1, (int16)a2, (int16)a3, (int16)a4)
```

Hit-path jal `@ 0x8001F970`. Death-arm jal `@ 0x8001F430`
uses `a0=0x46B` (set `@ 0x8001F3E8`) and is not this cut.

## Parked

`6DED4` packs an SVECTOR and jals `6DFA8` (GTE / `79244`)
then `6DF50` (`6E514` lookup + `86608`). Those callees are
not invented. `D_800B0E08==0` is the live BTL99 fixture.

Authentic death remains `1F078` `lh +0x0C` / `1F080` `bgtz`
after the `1D340` prefix (HUD `sb` storm, `21D4C`, `6F6D4`,
then death `6DE80(0x46B)`).

## Verify

```text
python3 pc_port/tools/pe_btl99_6de80_oracle.py
python3 pc_port/tools/pe_btl99_1f814_death_oracle.py
PE_TEST_FILTER=BTL99 ./pc_port/build/pe-native-tests
```
