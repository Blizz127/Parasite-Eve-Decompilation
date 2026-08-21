# PE-CH1 — func_800177AC opcode 0x1F mailbox poll

Native translation of the Carnegie Hall / first-play mailbox poll.
Matching `src/` C was not added: this worktree has no `asm/`, no era
`cc1`, and no extracted SLUS (`scripts/verify_us.sh` cannot take a
new carve). Follows `func_80017764_port.c`.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x800177AC..0x800177C4  (7 words, 0x1C)
file        0x7FAC
yaml        inside [0x2A0C, asm]  (after 17764; before matching 17E9C)
table       D_800910A0[0x1F] @ 0x8009111C = 0x800177AC
jal         none (leaf; jump-table only)
```

## Contract

`a0` is a script-arg dest pointer (same triple shape as `0x1C`; only
slot 0 is used):

```text
task = *(D_8009D300)               # 0x590($gp) current task
*(u32*)(*a0) = *(u32*)(task+0x14)  # copy payload word
return 1                           # no ACK
```

No store back to the queue, count, or `task+0x14`. BTL0 type-0 poll
after mailbox 3/4 drain reads that word
(`docs/evidence/pe-mbx1-task-state/`).

## Verify

```text
python3 pc_port/tools/pe_ch1_177ac_oracle.py
# from pc_port/build: PE_TEST_FILTER=177AC ./pe-native-tests
```

Oracle: 7/7 ROM words + jump table `0x8009111C` + `D_8009D300` + no jal.
Native tests: 5 focused `177AC_*` plus full suite **611/611**.
