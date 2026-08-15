# PE-MBX2 — native mailbox transport

```text
PE-MBX2 SUCCESS — RETAIL MAILBOX TRANSPORT ACTIVE IN NATIVE FIELD
```

Replaces the UE0 fail-close on `0x1C` / `0x1F` with the PE-MBX1
queue. Tree is `native/` only. Python RD5-X / RD6-A payload-byte
switch is not rehosted. RD7-R `0xFF` PCs are not cloned from
m0378i.

## Isolation

```text
worktree=/var/home/blizz/dev/parasite-eve-port-black-worktrees/pe-ue0-native-field-bootstrap
branch=feature/pe-ue0-native-field-bootstrap
base_mbx1_commit=4eaccd1
native_head=a697ab9
```

One local commit. Not pushed.

## Transport

`D_800A3180` is a 28-deep append queue of 12-byte records. Count
is `0x44($gp)`. `0x1C` appends type/id/payload/sender with extra
0. `func_80065400` drains once per `func_8003F3C4`, before
`func_80035558`. Drain matches `actor+0x0C` / `+0x0D`, allocates
a task at `actor+0x19C`, sets `task+0x08 |= 4` and
`task+0x14 = payload`, then zeros the count. `0x1F` copies
`task+0x14` into a local. No ACK.

See `QUEUE_MODEL.md` and `PUMP_ORDERING.md`.

## Hazards (reproduced, not fixed)

See `HAZARDS_REPRODUCED.md`. DEBT1 class
`RETAIL_FAITHFUL_UNSAFE`:

| id | hazard |
|---|---|
| DEBT-FID1-034 | no clamp at 28; 29th append writes past the table |
| DEBT-FID1-035 | drain zeros count only; stale bytes stay readable |
| DEBT-FID1-036 | undelivered records persist across a field hop |

## Parity

m0002i→m0003i has no `0x1C`. The pump is a no-op on that path.
The fixture was not rewritten.

```text
base_mbx1_commit=4eaccd1
queue_depth=28
record_size=12
append_opcode=0x1C
read_opcode=0x1F
ack_implemented=no

pump_site=func_8003F3C4
pump_order=before_func_80035558
drain_per_tick=1

clamp_added=no
records_wiped_on_drain=no
undelivered_persists_across_hop=yes

fail_close_removed=yes
python_shim_rehosted=no

ue_trace_sha256=9cefa0bc9ad95afcf47a7bd0b790e7f741b3bfce42427e4bdaed94b7fcb87b27
oracle_trace_sha256=9cefa0bc9ad95afcf47a7bd0b790e7f741b3bfce42427e4bdaed94b7fcb87b27
row_for_row_parity=yes
trace_rebaselined=no

determinism=3/3 + 30Hz/60Hz present
tests=1647/1647 native pe-ue0-tests PE_DISC1_BIN set

hard_blockers=
warnings=extra!=0 serial-match arm is present and unused by 0x1C; task free-list is a 32-slot native pool not the retail 0x8C($gp) chain; DEBT-FID1-004 Python payload machine still exists on the research lane

SUCCESS

PE-MBX2 SUCCESS — RETAIL MAILBOX TRANSPORT ACTIVE IN NATIVE FIELD
```
