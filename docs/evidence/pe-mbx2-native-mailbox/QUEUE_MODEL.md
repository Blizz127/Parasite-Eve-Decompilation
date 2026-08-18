# Queue model

`D_800A3180` is a 28-deep append queue of 12-byte records. Count is
the `u8` at `0x44($gp)` = `D_8009CDB4`. This is the PE-MBX1 EXE
path (`4eaccd1`), now live in `native/`.

```text
base     D_800A3180 = 0x800A3180
stride   12
rows     28                    # func_8006536C sltiu 0x1C
count    D_8009CDB4
extent   0x800A3180..0x800A32CF
index    u8, 0..255            # no wrap-at-28
```

## Record

```text
+0  u16 dest type      sh from 0x1C arg0
+2  u8  dest id        sb from 0x1C arg1
+3  u8  payload        sb from 0x1C arg2
+4  u32 extra          0x1C always writes 0
+8  u32 sender         current actor+0x24
```

`func_800653B8` writes payload, id, sender, type, extra, then
`count++`. There is no full check and no clamp.

## Boot clear

`func_8006536C` zeros the 28 rows and the count byte. Sole retail
caller is boot `func_8003E680`. Scene load does not call it.
Bytes past row 27 are not part of that loop.

Native `mailbox_boot_clear` runs from `runtime_boot_m0002i` only.
`enter_m0003i` does not call it.

## Send — `0x1C`

`func_80017764`: resolve type/id/payload, extra = 0, sender =
current actor serial, append, return 1. No ACK.

## Drain — `func_80065400`

Walk `0 .. count-1`. `extra` byte 0 matches
`actor+0x0C` / `actor+0x0D` when `actor+0x19C != 0`. Nonzero extra
matches `actor+0x24` (present in the body; `0x1C` never sets it).
Deliver via `func_80012700(actor+0x19C)`:

```text
task+0x00 = entry PC
task+0x08 |= 4
task+0x0C = sender
task+0x14 = payload byte
```

Then `count = 0`. Record bytes are not wiped.

## Read — `0x1F`

`func_800177AC`: copy current `task+0x14` into the dest operand.
No queue store. No ACK. The payload word stays on the task.

## Not this transport

The Python RD5-X / RD6-A payload-byte switch is a script-outcome
map. It is not rehosted. RD7-R `0xFF` PCs stay per-room; they are
not cloned from m0378i.
