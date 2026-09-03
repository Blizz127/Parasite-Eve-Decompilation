# PE-EV1 — func_80042798 event-record cleanup walk translated

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C (native translation, not a decomp leaf; same
standard as OTC1).

`func_80042798` — 44 words `0x80042798..0x80042848`,
SHA-256 `ddf5ae1d…9570758b`. Walks two records at `D_800A0ED4+1` /
`+0x419` (stride `0x418`, bound `+0x831`, dead unsigned range guard
`+0x830`): a record whose tag byte is 8 or 10 fires `func_80072774`
with the word at `+0xB` (record `+0xC`, 4-aligned), then stamps word
`-1` / tag `12`.

Lifecycle fit: the translated `func_80042538` memsets this exact
`0x830`-byte block and writes the two `-1` sentinels at `+0xC`/`+0x424`
— the words this walk consumes. `func_80072774` is a 3-word BIOS-vector
trampoline (`jr 0xB0`, `$t1 = 0x36`) and remains the narrowed callee
boundary (recorded with trampoline identity + handle word). The 5C1EC
zero path now calls the real walk; its test was migrated from
`_is_named_boundary` to `_translated_walk` (quiet table: no stub, no
stop, no boundary record).

## Verify

```text
python3 pc_port/tools/pe_ev1_42798_oracle.py
PE_TEST_FILTER=EV1 ./pc_port/build/pe-native-tests
```

Full normal suite: 1018 run / 1001 passed / 1 pre-existing
environmental failure (`B54KY` missing `local/pe_disc1.path`, identical
on the base tree) / 16 skipped.
