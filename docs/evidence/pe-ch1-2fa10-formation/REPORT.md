# PE-CH1 — func_8002FA10 opcode 0x70 formation write

Native translation of the m0005i formation store. Matching `src/` C
was not added: this worktree has no `asm/`, no era `cc1`, and no
extracted SLUS.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x8002FA10..0x8002FAA0  (37 words, 0x94)
file        0x20210
yaml        head of [0x20210, asm] after matching 2F9CC
table       D_800910A0[0x70] @ 0x80091260 = wrapper 0x8001897C
jal         sole site 0x80018A30; wrapper a0 = *(D_8009D2F0)
```

## Contract

`a0` is the current actor; `*a0` is the slot body from `0x6F`.

```text
i = a1 & 0xFF
p16 = *actor + i*16 + 0x1C
  +0=0  +1=a2  +2=a3  +3=st10(lbu)
  +0xC=st14(lhu)  +0xE=st28  +0xF=st2C
p4 = *actor + i*4
  +0x7C..+0x7F = st18..st24 (lb then sb)
```

m0005i `+0x21A0` operands: `0,0,8,9,1,5,-1,-1,-1,3,15`
(`docs/evidence/pe-btl0-boss-identity/FORMATION_SELECTION.md`).

## Verify

```text
python3 pc_port/tools/pe_ch1_2fa10_oracle.py
# from pc_port/build: PE_TEST_FILTER=2FA10 ./pe-native-tests
```

Oracle: 37/37 ROM words + 0x70 wrapper + body strides.
Native tests: 5 focused `2FA10_*` plus full suite **626/626**.
