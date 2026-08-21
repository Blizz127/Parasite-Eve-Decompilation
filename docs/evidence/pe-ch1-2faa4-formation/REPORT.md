# PE-CH1 — func_8002FAA4 opcode 0xB7 formation write

Native translation of the m0005i formation store. Matching `src/` C
was not added: this worktree has no `asm/`, no era `cc1`, and no
extracted SLUS.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x8002FAA4..0x8002FAD4  (13 words, 0x34)
file        0x202A4
yaml        [0x20210, asm] after 2FA10
table       D_800910A0[0xB7] @ 0x8009137C = wrapper 0x80018A48
jal         sole site 0x80018A84; wrapper a0 = *(D_8009D2F0)
```

## Contract

`a0` is the current actor; `*a0` is the slot body from `0x6F`.

```text
i = a1 & 0xFF
p16 = *actor + i*16 + 0x1C
  +0=0  +1=a2  +2=a3  +3=st10(lbu)
  +0xC=st14(lhu)   # jr delay slot
```

Subset of `0x70` (`func_8002FA10`): no `+0xE`/`+0xF` and no
`+0x7C` quartet.

m0005i `+0x2154` operands: `3,0,6,7,1`
(`docs/evidence/pe-btl0-field-battle-handoff/FIRST_DAY1_ENCOUNTER.md`).

## Verify

```text
python3 pc_port/tools/pe_ch1_2faa4_oracle.py
# from pc_port/build: PE_TEST_FILTER=2FAA4 ./pe-native-tests
```

Oracle: 13/13 ROM words + 0xB7 wrapper + body strides.
Native tests: 5 focused `2FAA4_*` plus full suite **631/631**.
