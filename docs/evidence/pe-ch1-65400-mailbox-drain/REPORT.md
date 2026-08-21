# PE-CH1 — func_80065400 mailbox drain

Native translation of the Carnegie Hall / first-play mailbox drain.
Matching `src/` C was not added: this worktree has no `asm/`, no era
`cc1`, and no extracted SLUS (`scripts/verify_us.sh` cannot take a new
carve). Follows `func_800653B8_port.c` / `func_80017764_port.c`.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x80065400..0x800655D0  (117 words, 0x1D4)
file        0x55C00
yaml        inside [0x55430, asm]  (after 653B8)
sole jal    func_8003F3C4 @ 0x8003F4E8  (nop delay)
spawn       jal func_80012700 @ 0x80065484 and 0x8006553C (a1=0)
```

`func_80012700` is now native-ported (`func_80012700_port.c`); the
drain leaf calls the real translation.

## Contract

```text
if count == 0: sb 0; return
for i in 0 .. ; (i&0xFF) < lbu count; i++:
    rec = D_800A3180 + (i&0xFF)*12
    extra_b = lbu rec+4
    if extra_b != 0:          # serial; 0x1C never sets this
        walk D_8009D20C via actor+4
        match lhu(actor+0x24)==extra_b
        deliver at most once, then stop the walk
    else:                     # type+id (BTL0 mailbox 3/4)
        walk D_8009D20C via actor+4
        match lbu(actor+0x0C)==lhu(rec+0)
           and actor+0x19C != 0
           and lbu(actor+0x0D)==lbu(rec+2)
        deliver; continue walk
sb $zero, 0x44($gp)           # count=0; record bytes kept
```

Deliver: `func_80012700(actor+0x19C, 0)` then `task+0x08 |= 4`,
`task+0x0C = rec+8`, `task+0x14 = lbu rec+3`, insert at
`actor+0xA8` (`task+0x24` = old head; old `+0x28` = task).

## Verify

```text
python3 pc_port/tools/pe_ch1_65400_oracle.py
# from pc_port/build: PE_TEST_FILTER=65400 ./pe-native-tests
```

Oracle: 117/117 ROM words + both jal `12700` + sole caller `0x8003F4E8`.
Native tests: 8 focused `65400_*` plus full suite **601/601**.
