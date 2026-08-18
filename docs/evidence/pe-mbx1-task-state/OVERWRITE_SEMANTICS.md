# Overwrite and hop persistence

## Before drain

Two `0x1C` with `count` still live write **two** records
(`count`, then `count+1`). There is no last-write-wins on the
queue. RD4-D module 3 fires seven first-play sends in one arm
(`0x16`..`0x1B`); that is the same append rule.

`func_800653B8` does not clamp at 28. A 29th append with
`count==28` writes past `0x800A32CF`. Count is `u8`, so 256
appends without a drain would wrap the **index** only after 255,
still without a 28-slot ring. First-play scripts do not do this:
`func_80065400` runs every `func_8003F3C4` frame.

## Drain

`func_80065400` walks `0 .. count-1` then `sb 0` to the count.
Record bytes are **not** cleared. The next `0x1C` overwrites
slot 0. Stale bytes are ignored while `count==0`.

## After drain / ACK

There is no ACK. `0x1F` only copies `task+0x14`. The consumer
does not write the queue or the count. RD4-D already recorded
"no ACK" for `0x1B`; the EXE has no write-back path to add one.

## Field hop

`func_8006536C` is boot-only (`0x8003E730`). Save
`func_80040B80` / `func_8003F800` / `func_8003FBD8` have no
`0x3180` / `0xCDB4` immediates. Scene load does not zero the
queue.

| State at hop | Survives? |
|---|---|
| `count>0` undrained records | **yes** — next `65400` offers them to whatever actors now match type/id |
| drained `count==0` | nothing in the table |
| delivered `task+0x14` | lives on the receiver task / actor; a hop that rebuilds actors drops it |

First-play mailbox 3/4 hop **after** the type-0 `0x1F` poll
(BTL0). By then `65400` has already zeroed `count`. The hop
token is `persist[1]=4` + `0x31 0xA80002C8`, not a live queue
record.

Same-script `0x1C` then `0x31` with no yield would hop with
`count>0` because drain ran **before** this frame's scripts
(`65400` then `35558`). No first-play 3/4 / `0xFF` / `0x1B`
path does that.

## Battle

The battle cluster `0x80029800–0x80031000` has no `3180`/`CDB4`
immediate and no `jal` of `653B8`/`65400`. If `func_8003F3C4` is
not the battle tick, undrained records sit until field returns
and the pump runs again. Battle does not read the queue.
