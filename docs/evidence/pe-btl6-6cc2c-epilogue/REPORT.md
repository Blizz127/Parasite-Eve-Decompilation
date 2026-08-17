# PE-BTL6 — 6C5BC andi 0xFC epilogue at 0x8006CC2C

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. This is the retail clearer after
`jal 3D834`, not a stub and not a blanket `+0xE &= 0xFC`
from opcode `0x55`.

## Words

```text
0x8006CC18  jal 0x8003D834
0x8006CC1C  sw  $v0, 0x10($sp)     # delay
0x8006CC20  lbu $v1, 0xE($s4)
0x8006CC24  addiu $v0, $zero, 1    # return 1
0x8006CC28  sb  $zero, 0xEE($s4)
0x8006CC2C  andi $v1, $v1, 0xFC
0x8006CC30  j   0x8006CC3C
0x8006CC34  sb  $v1, 0xE($s4)      # delay
```

`0x8006CC38` `v0=0` is the EE 8/9/10 join and is skipped.
No jal sits between `3D834` and `andi 0xFC`.

Named cut `func_8006C5BC_ee13_epilogue_cut` is wired only on
EE=13 after the prefix (ROM order after 3D050/6698C/3D834).
`0x3B` can then pass (`+0xF4=0`). Next 144FC state is 0 at
`0x80014544` (lbu +0xE; andi 3; else sb 0x37). Do not stub
`6914C`, jump to mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_6cc2c_oracle.py
python3 pc_port/tools/pe_btl6_ee13_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```

STOP: `0x80014544` — 144FC state 0 after 0x3B stores +0xF4=0.
