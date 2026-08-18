# PE-BTL116 — rec+0x4C bit 0x80000 from 24250 case 19

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

The only TEXT OR of `rec+0x4C` bit `0x80000` is `24250` at
`24988` (`lui 8`) / `24994` (`sw`). That store is
`jtbl[19]` at `0x800107D4` (`0x80024974`). `jtbl[0]`
(tid 387) jumps to `24998` and skips the OR.

`22394` @ `22C9C` jals `24250(tid-387, actor)` when
`0x80000` is clear and `0x10000` is clear. It first ORs
`0x200000`. Case 19 then sets `0x80000` and clears
`0x200000`. The next `22394` sees `0x80000` and jals
`24A3C`. Do not plant the bit.

tid 406 = 387+19. `5112C` @ `51314` is `addiu 387`.
Do not plant tid 406 on the live menu path.

`6C1CC`: `lbu overlay+0xED`. State not in `[32,40)`
returns 0 at `6C4A8`. Case 2 honors a nonzero return
and does not increment. States 32–39 are not stubbed
to 0.

`Aya+0x252` still has no `sb $0` in EXE or PE.IMG.
Case 2 still sets 1; case 6 still waits.

## Verify

```text
python3 pc_port/tools/pe_btl114_ce54_1a_oracle.py
PE_TEST_FILTER=BTL116 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
