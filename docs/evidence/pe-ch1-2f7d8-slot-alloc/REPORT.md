# PE-CH1 — func_8002F7D8 opcode 0x6F slot alloc

Native translation of the m0005i slot-table claim. Matching `src/` C
was not added: this worktree has no `asm/`, no era `cc1`, and no
extracted SLUS. Same `SlotRecord` as matching `func_8002F9CC` /
`func_8002F970`.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x8002F7D8..0x8002F96C  (102 words, 0x198)
file        0x1FFD8
yaml        [0x11718, asm] prefix before matching 2F970 @ 0x20170
table       D_800910A0[0x6F] @ 0x8009125C = wrapper 0x80018954
jal         sole site 0x80018964; wrapper a0 = *(D_8009D2F0)
callee      jal func_8001A680 @ 0x8002F924 (unresolved; recorded)
```

## Contract

```text
a0 = current actor
copy 216B D_800109B0 → claimed body   # 0xD0 loop + 8-byte tail
                                      # (BATTLE_RESOURCES.csv said 208;
                                      #  ROM copies the full body[216])
first free D_800A5D58[i].inUse == 0:
  inUse = 1
  *actor = body                       # actor+0
  D_8009D2EC++ (byte); body+7 = that
  body+8 = 1 << i
  if (actor+0x98 & 0x2000) == 0:
    body+0x18 = body+0x1C
    func_8001A680(actor, 2)
    D_8009D2A0++ (byte)
table full: no store
```

`func_8001A680` is not translated this rung (77 jal sites). The port
records the call through `Bootstrap_ReturnVoid4`.

## Verify

```text
python3 pc_port/tools/pe_ch1_2f7d8_oracle.py
# from pc_port/build: PE_TEST_FILTER=2F7D8 ./pe-native-tests
```

Oracle: 102/102 ROM words + 0x6F wrapper + 216B template + jal 1A680.
Native tests: 5 focused `2F7D8_*` plus full suite **621/621**.
