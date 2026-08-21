# PE-CH1 — func_800653B8 mailbox append

Native translation of the Carnegie Hall mailbox append leaf. Matching
`src/` C was not added: this worktree has no `asm/`, no era `cc1`, and
no extracted SLUS, so `scripts/verify_us.sh` cannot take a new carve
(CLAUDE.md: never invent decompiled C). The established path for this
family is `func_8006536C_port.c`.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x800653B8..0x800653FC  (18 words, 0x48)
file        0x55BB8
yaml        inside [0x55430, asm]  (not a named C subsegment)
sole jal    func_80017764 @ 0x80017794  (opcode 0x1C)
```

## Contract

`D_800A3180 + count*12` record, count = `lbu 0x44($gp)` = `D_8009CDB4`:

```text
+0  u16 dest type     sh a2
+2  u8  dest id       sb a1
+3  u8  payload       sb a0
+4  u32 extra         sw 16($sp)
+8  u32 sender        sw a3
count++               sb; no wrap-at-28
```

Stride is `*3` then `*4` (sll 1, addu, sll 2). `$v1` is clobbered by
`lui 0x800A` / `addiu 0x3180`, so count is reloaded before increment.
No full check. A 29th append with `count==28` writes at `0x800A32D0`.

BTL0 m0004i mailbox 3/4 is this leaf with `(type,id,payload)=(0,0,3|4)`
and extra 0 (`docs/evidence/pe-mbx1-task-state/UNIFIED_MODEL.md`).

## Verify

```text
python3 pc_port/tools/pe_ch1_653b8_oracle.py
# from pc_port/build: ./pe-native-tests  (PE_TEST_FILTER=653B8)
```

Oracle: 18/18 ROM words + jal site + extra=0 delay slot.
Native tests: 6 focused `653B8_*` plus full suite **588/588**.

## Not this rung

`func_80017764` and `func_80065400` are now native-ported.
`12700` / `177AC` remain unmatched.
`scripts/verify_us.sh` was not claimed (harness inputs missing).
