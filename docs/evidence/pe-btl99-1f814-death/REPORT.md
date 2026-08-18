# PE-BTL99 — 1F814 after 1F704; death is not at 1F4D4

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Retail BTL83 capture: `1F704` 40→39 then 39→34.
No matching `src/` C. Do not poke `4D4`, mode 7,
scratch bits, or HP.

## Live path

After `1F704` stores `HP-=s0`, `1F708` reloads
`lh +0x0C`. HP==0 skips to `1F7D8`. HP!=0 jals
`1F814(actor)` at `0x8001F724`.

`1F814` (SHA-256 `f4060ee9…6f2c`):
- Aya `D254+0x0E` in 6..15 takes jtbl `0x800106E4`
  (D29A / D29B / `gp+0x528=1` / D29C).
- If record+0x4C bit `0x12000` is clear: `305C8`
  then `1A680(D254, facing cmd)` then
  `6DE80(0x46A, 0, Aya+0x2A/+0x2E/+0x32)`.
- `6DED4` → `6DFA8` / `6DF50` stay fail-closed.

`305C8` is also jal'd from `28D28` (not this cut).

## Damage loop

A later `1D340` tick repeats `1F4D4`. Retail second
delta is 39→34 (`s0=5`). Native reproduces that by
a second live subtract with weapon `+0x0C=5`.
HP is not planted between ticks.

## Death (ROM-located, not this cut)

Death is **not** at `1F4D4`. `1D340` `1F078`
`lh +0x0C` / `1F080` `bgtz` is after the 1D340
prefix (HUD `sb` storm, `77CF4`, `1F9C4`, `32B0C`,
`21054`, `6F6D4`, `21D4C`). HP<=0 then stores:

- Aya `+0x68/6C/70=0`
- `D1AC &= ~0x300`
- mode 3 at `1F41C`
- `D1CE=0`
- `6DE80(0x46B, …)` at `1F430`
- `4D4=0` at `1F43C`
- `D1A0 &= ~4`, `D2E8 |= 1`
- `1A680(D254, 19)`
- `1F4B0` `sh $zero, +0x0C`

Do not invent those stores immediately after
`1F4D4`. `encounter_complete` / teardown / field
return are not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl99_6de80_oracle.py
python3 pc_port/tools/pe_btl99_1f814_death_oracle.py
python3 pc_port/tools/pe_btl98_1d340_hp_oracle.py
PE_TEST_FILTER=BTL99 ./pc_port/build/pe-native-tests
PE_TEST_FILTER=BTL98 ./pc_port/build/pe-native-tests
```
