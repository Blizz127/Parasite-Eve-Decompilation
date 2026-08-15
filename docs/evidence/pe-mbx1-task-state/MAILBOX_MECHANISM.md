# Mailbox mechanism (EXE)

Evidence-only. Words from `build/disc1.candidate.exe` (taddr
`0x80010000`, SHA-256 `5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b`).

## Kind

`D_800A3180` is a **28-deep append queue** of 12-byte records, not a
single slot and not a field on the task.

```text
base     D_800A3180
stride   12
rows     28                    # func_8006536C sltiu 0x1C
count    D_8009CDB4 = 0x44($gp)
extent   0x800A3180..0x800A32CF
```

`func_8006536C` (`0x8006536C`, 19 words) zeros the 28 rows and
`sb $zero, 0x44($gp)`. Sole `jal`: `0x8003E730` (boot
`func_8003E680`). No scene-load caller.

## Record

Written only by `func_800653B8` (`0x800653B8..0x800653FC`):

```text
lbu  count, 0x44($gp)
rec  = D_800A3180 + count*12     # *3 then *4
sb   a0, rec+3                   # payload
sb   a1, rec+2                   # dest id
sw   a3, rec+8                   # sender
sh   a2, rec+0                   # dest type
sw   16($sp), rec+4              # extra
count++
sb   count, 0x44($gp)
```

No wrap, no full check. Count is an 8-bit index.

## Send — opcode `0x1C`

Jump table `0x800910A0 + 0x1C*4 = 0x80091110` holds
`func_80017764`. That leaf is the **only** `jal func_800653B8`
(`0x80017794`).

```text
a0 = lbu *arg2          # payload
a1 = lbu *arg1          # dest id
a2 = lhu *arg0          # dest type
a3 = lhu (D_8009D2F0)+0x24   # current-actor serial
16($sp) = 0             # extra always zero
return 1                # no ACK
```

Script form `0x1C (type, id, payload)` matches RD4-D
`(1,0,0x1B)`, RD7-R `(0,0,0xFF)`, BTL0 `(0,0,3)` / `(0,0,4)`.

## Drain — `func_80065400`

Sole `jal`: `0x8003F4E8` inside `func_8003F3C4` (field tick;
caller `0x800123D8`). Cadence is **once per field frame**, after
`D_8009D250++` and **before** `func_80035558` (`0x8003F4F0`, also
a unique `jal`).

```text
if count == 0: sb 0; return
for i in 0 .. count-1:
    rec = D_800A3180 + i*12
    extra_b = lbu rec+4
    if extra_b != 0:
        walk actor list D_8009D20C
        if lhu(actor+0x24) == extra_b and actor+0x19C != 0:
            deliver
    else:
        walk actor list D_8009D20C
        if lbu(actor+0x0C)==lhu(rec+0)
           and actor+0x19C != 0
           and lbu(actor+0x0D)==lbu(rec+2):
            deliver
sb $zero, 0x44($gp)     # count = 0; records not zeroed
```

`0x1C` never sets extra, so the serial arm is **present in the
body and unreachable from the only writer**. First-play mail uses
the type+id arm.

## Deliver

```text
task = func_80012700(actor+0x19C, 0)
task+0x08 |= 4          # lhu / ori / sh
task+0x0C  = rec+8      # sender serial
task+0x14  = lbu rec+3  # payload, stored as word
link task at actor+0xA8 (doubly via +0x24/+0x28)
```

`func_80012700` (`0x80012700`) pops a block from `0x8C($gp)`,
writes `+0x00=entry PC`, `+0x04=0`, `+0x08=0`, `+0x0A` serial,
`+0x0C=0`, `+0x10=1`. Mailbox bit 2 is added **after** return.

Seven `jal func_80012700` exist. Only `0x80065484` and
`0x8006553C` are the mailbox path.

## Read — opcode `0x1F`

Jump table `0x800910A0 + 0x1F*4 = 0x8009111C` holds
`func_800177AC`:

```text
lw  $v0, 0x590($gp)     # current task
lw  $v0, 0x14($v0)
sw  $v0, *arg0          # dest local
return 1
```

No store back to the queue. No ACK. `task+0x14` is left as
written.

## Same-frame order

```text
func_8003F3C4:
    ...
    D_8009D250++
    jal func_80065400      # drain previous 0x1C
    jal func_80035558      # actor / script tick (this frame's 0x1C)
```

A send in this frame is visible to `0x1F` on the **next** field
frame, after the next drain creates the poll task.

## Authority census

| Address | Role | Sites |
|---|---|---|
| `D_800A3180` lui/addiu | table base | `0x80065370`, `0x800653CC`, `0x80065434` only |
| `0x44($gp)` | count | `6536C` sb0; `653B8` lbu/sb+1; `65400` lbu/sb0 only |
| `D_8009CDB4` lui/addiu | none | count is gp-relative only |
| save `40B80`/`3F800`/`3FBD8` | 3180/CDB4 imm | none |
| battle `0x80029800–0x80031000` | 3180/CDB4 imm | none |
