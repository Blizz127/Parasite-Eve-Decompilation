# PE-CH1 — func_80017764 opcode 0x1C mailbox send

Native translation of the Carnegie Hall / first-play mailbox send
wrapper. Matching `src/` C was not added: this worktree has no `asm/`,
no era `cc1`, and no extracted SLUS (`scripts/verify_us.sh` cannot take
a new carve). Follows `func_800653B8_port.c`.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x80017764..0x800177A8  (18 words, 0x48)
file        0x7F64
yaml        inside [0x2A0C, asm]  (before matching 17E9C @ 0x869C)
table       D_800910A0[0x1C] @ 0x80091110 = 0x80017764
jal         func_800653B8 @ 0x80017794; extra sw $zero in delay slot
```

## Contract

`a0` is a triple of guest pointers (script operands):

```text
*(u32*)(a0+0) → lhu dest type
*(u32*)(a0+4) → lbu dest id
*(u32*)(a0+8) → lbu payload
sender = lhu(*(D_8009D2F0)+0x24)   # current-actor serial, 16-bit
extra  = 0                         # hardcoded; serial-arm of 65400 unused by 0x1C
return 1                           # no ACK
```

BTL0 m0004i mailbox 3/4 is `(type,id,payload)=(0,0,3|4)`.

## Verify

```text
python3 pc_port/tools/pe_ch1_17764_oracle.py
# from pc_port/build: PE_TEST_FILTER=17764 ./pe-native-tests
```

Oracle: 18/18 ROM words + jump table `0x80091110` + jal `653B8` + extra=0.
Native tests: 5 focused `17764_*` plus full suite **593/593**.
