# PE-MBX1 — retail mailbox task-state mechanism

Evidence-only. No implementation. DEBT-FID1-004 is not promoted
and not repaired.

```text
PE-MBX1 SUCCESS — RETAIL MAILBOX TASK-STATE MECHANISM PROVEN
```

## Isolation

```text
checkout=/var/home/blizz/dev/parasite-eve-port-black
base_debt1_commit=9601c6e
exe=build/disc1.candidate.exe
exe_sha256=5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
```

Command: Python one-shot over that EXE (jal census, lui/addiu
`D_800A3180`, `0x44($gp)` uses, opcode words at `0x800910A0`,
immediates in save/battle windows). Disassembly of
`0x8006536C`, `0x800653B8`, `0x80065400`, `0x80017764`,
`0x800177AC`, `0x80012700`, `0x8003F4E8`.

## Answer

`D_800A3180` is a **queue**. `0x1C` appends. `func_80065400`
polls it once per `func_8003F3C4` field frame and creates a
task whose `+0x14` holds the payload. `0x1F` reads that word.
No ACK.

Mailbox 3/4 are `0x1C (0,0,3)` and `0x1C (0,0,4)` into the same
queue (BTL0 volumes `+0x0F78` / `+0x10E4`). One native model
serves RD5/RD6 field mail and BTL1.

Python's payload-byte switch is a script-outcome shim. This
checkout only zeros the table. UE0 fail-closes `0x1C`/`0x1F`.
None of those is the EXE path; the fail-close is the honest
unimplemented opcode.

Battle and save do not touch `D_800A3180` or the count byte.
SYS0's flip-to-BLOCKER condition is not met.

Known-good uses sit on this path without contradiction:
RD4-D `0x1B`, RD6-A / RD7-R `0xFF` (room-local PCs), BTL0 3/4.

```text
base_debt1_commit=9601c6e
d800a3180_kind=QUEUE
delivery=POLLED
ack_present=no
overwrite_semantics=append_then_reuse_after_drain
persists_across_hop=undelivered_yes

writers_located=yes
readers_located=yes

serves_0x1C_0x1F=yes
serves_mailbox_3_4=yes
one_model_sufficient=yes

battle_shares_task_state=no
save_shares_task_state=no
debt_fid1_004_flip=NONBLOCKING

mbx2_implementation_ready=YES
btl1_rescope_required=no

hard_blockers=
unknowns=func_80035558 body (unique jal after the pump; treated as the actor/script tick only from call order); unused extra!=0 serial-match arm has no 0x1C writer
warnings=653B8 does not clamp at 28; Python outcome map must not be rehosted as the transport; RD7-R 0xFF PCs must not be cloned from m0378i

SUCCESS

PE-MBX1 SUCCESS — RETAIL MAILBOX TASK-STATE MECHANISM PROVEN
```
