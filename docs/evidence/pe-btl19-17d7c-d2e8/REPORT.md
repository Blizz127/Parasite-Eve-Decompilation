# PE-BTL19 — VM 0x40 sets D_8009D2E8 bit 0

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_80017D7C

8 words `0x80017D7C..0x80017D9C`, SHA-256 `ce839661…9b09`.
`D_800910A0[0x40]`. Zero jal. Always v0=1.

```text
D_8009D2E8 |= 1
```

Live type-1 `+0x040` is argc-0 opcode `0x40`. It runs on the
first type-1 17018 visit, after `0xEA` `0x190` and before
`0xED` / the later `0x08` type-0 spawn.

## Relation to 3999C

`35C84` calls `3999C` only when `D_8009D2E8 bit 0` is clear.
This live VM op **sets** that bit. Type 0 is spawned later
by type-1 `0x08`, so on this encounter `3999C` stays skipped
unless a later clearer runs. Do not force the bit. Do not
name `3999C` BattleInput. Do not name `0x40` BattleInput.

VM `0x3F` = `func_80017D5C` (8w) is `D_8009D2E8 &= ~1`.
Not on the live type-1 prefix. `34F10` zeros the whole word
at new-game. After-poll clears bits 2+3 only (`&= ~0xC`).

## Verify

```text
python3 pc_port/tools/pe_btl19_17d7c_oracle.py
PE_TEST_FILTER=BTL19 ./pc_port/build/pe-native-tests
```
