# PE-CH1 — func_80012700 task spawn

Native translation of the task-block pop used by mailbox deliver.
Matching `src/` C was not added: this worktree has no `asm/`, no era
`cc1`, and no extracted SLUS. Replaces the `func_80065400` hook.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x80012700..0x80012770  (29 words, 0x74)
file        0x2F00
yaml        inside [0x2A0C, asm]
jal sites   7; mailbox 0x80065484 and 0x8006553C (a1=0)
```

## Contract

```text
task = *D_8009CDFC                 # 0x8C($gp) freelist
*D_8009CDFC = task+0x24            # pop
if a1 != 0: insert task at a1+0x24 (doubly via +0x24/+0x28)
else:       task+0x24 = task+0x28 = 0
task+0x00 = entry
task+0x04 = 0
task+0x08 = 0                      # sh; mailbox |=4 after return
task+0x0A = lhu D_8009D308         # 0x598($gp) serial
task+0x0C = 0
task+0x10 = 1
D_8009D308 = serial+1              # sh
return task
```

No empty-list guard. `+0x14` is written by `func_80065400`, not here.

## Verify

```text
python3 pc_port/tools/pe_ch1_12700_oracle.py
# from pc_port/build: PE_TEST_FILTER=12700 ./pe-native-tests
```

Oracle: 29/29 ROM words + 7 jal sites + gp slots.
Native tests: 5 focused `12700_*` plus full suite **606/606**.
