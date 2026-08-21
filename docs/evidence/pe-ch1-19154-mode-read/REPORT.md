# PE-CH1 — func_80019154 opcode 0x94 mode-word read

Native translation of the m0005i / BTL1 mode poll. Matching `src/` C
was not added: this worktree has no `asm/`, no era `cc1`, and no
extracted SLUS (`scripts/verify_us.sh` cannot take a new carve).
Twin of matching `src/func_80017FF0.c` (opcode `0x89` stores 6).

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x80019154..0x8001916C  (7 words, 0x1C)
file        0x9954
yaml        inside [0x98BC, asm]  (before matching 192B8 @ 0x9AB8)
table       D_800910A0[0x94] @ 0x800912F0 = 0x80019154
twin        D_800910A0[0x89] @ 0x800912C4 = 0x80017FF0
jal         none (leaf; jump-table only)
```

## Contract

`a0` is a script-arg dest pointer (same shape as `0x1F`):

```text
*(u32*)(*a0) = *(u32*)D_8009D28C   # word-copy; no arith/bits
return 1                           # does not store back
```

`D_8009D28C` is the READY-FROM-READER `int` mode word (matching
setters 0/5/6/8). BTL1 m0005i polls this after `0x89` until the
script compares 7 (`docs/evidence/pe-btl0-field-battle-handoff/`).

## Verify

```text
python3 pc_port/tools/pe_ch1_19154_oracle.py
# from pc_port/build: PE_TEST_FILTER=19154 ./pe-native-tests
```

Oracle: 7/7 ROM words + jump table `0x800912F0` + `D_8009D28C` + `0x89` twin.
Native tests: 5 focused `19154_*` plus full suite **616/616**.
