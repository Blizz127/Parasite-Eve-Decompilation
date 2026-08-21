# PE-BTL72 — playable-loop TRACE through actors/HP capture

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

This rung is the first native composition of the field→encounter
pipeline, not a new leaf. It starts on the proven m0004i mailbox
path (not title boot / `30894`) and stops before any invented
attack, hit, rec=4, 4D4, or mode 7.

## Pipeline

```text
3E974                         cold pad table (A76F0)
0x1C mailbox 3                field start
0x31 0xA80002C8               m0005i dest
0x6F slot claim               2F7D8
0x89 / 299CC consume          mode 6 → 0
293F4 HP cut                  +0x0C → +0x0E (copy, not damage)
BE9A2=0xFFEF → 3EB04          Up; D26C bit 3 (not planted D26C)
66CE8 + 35C84                 pose Z += 0x50000
live 0x0B pose + 409 Right    X enters rect1; 1CAB0 hit
0x85 / 18EB4                  CFEE=2 after authentic hit
```

TRACE rows:
`field` → `mailbox_3` → `m0005i_enter` → `mode6_consumed` →
`hp_copied` → `input_held` → `actors_captured` →
`attack_available`.

`attack_available` here is type-3 `0x85` after a 1CAB0 hit from
the retail 0x0B pose + 409 Right ticks (BTL73). It is not ATB,
menu Attack, or HP damage. Live ops after that `0x85` are
`0x9C` → `0x0A` persist[1]=5 → `0x31` `0xA8000248` (M0004I).
That hop is a door exit, not playable-loop `field_return`.
Not emitted: `hp_mutated`, `encounter_complete`, `post_return`,
`field_return`. HP `+0x0C/+0x0E` stay 0x28. See
`docs/evidence/pe-btl82-hp-writers/`.

## Verify

```text
python3 pc_port/tools/pe_btl72_playable_loop_oracle.py
PE_TEST_FILTER=BTL72 ./pc_port/build/pe-native-tests
```
