# Unified model

One queue serves every first-play `0x1C` / `0x1F` use, including
mailbox 3/4.

```text
script 0x1C (type, id, payload)
  func_80017764
    extra = 0
    func_800653B8  append at D_800A3180[count++]
next func_8003F3C4
  func_80065400
    match actor+0x0C/+0x0D
    func_80012700(actor+0x19C)
    task+0x08 |= 4
    task+0x14  = payload
    count = 0
receiver task at actor+0x19C
  0x1F  func_800177AC
    local = task+0x14
  script compares the local (0x09 / 0x05)
```

## First-play instances (same transport)

| Site | 0x1C args | Receiver | After 0x1F |
|---|---|---|---|
| RD4-D m0372i `+0x0BD4` | `(1,0,0x1B)` | type-1 `+0x0214` | message reel; no ACK |
| RD5-X m0004i `+0x0388` / `+0x04C8` | `(2,0,1)` / `(2,0,2)` | type-2 `+0x0DBC` | 1=no-op; 2=clip `0x0A` |
| RD6-A / RD7-R `0xFF` | `(0,0,0xFF)` | type-0 poll | restore + `0x3F` (PCs **per room**) |
| BTL0 m0004i `+0x0F78` / `+0x10E4` | `(0,0,3)` / `(0,0,4)` | type-0 `+0x05D0` | hop m0005i + `persist[1]=4` |

Mailbox 3/4 are **payloads 3 and 4** on dest type 0 id 0. They
are not a second opcode and not a persist-bank channel.

## What the three live treatments are

| Lane | Transport | Honest? |
|---|---|---|
| Python RD5/RD6 payload-byte machine | skips the queue; maps the byte to clip/restore/hop | outcome shim for **those scripts only**; not the mechanism |
| this checkout `func_8006536C` | zeros the table; `653B8`/`65400`/`12700` untranslated | table is empty after boot; cannot deliver |
| UE0 `pe_vm.cpp` | `0x1C`/`0x1F` fall into unknown-opcode fail-close | correct for an unimplemented opcode |

Do not promote the Python map to "the mailbox." Do not treat the
UE0 fail-close as a defect in the opcode itself.

A native MBX2 that implements the EXE path above replaces all
three for field **and** BTL1 3/4. Script-specific compare arms
stay in the receiver bytecode (`0x1F` + `0x09` + `0x05`), not in
a host switch on `{1,2,3,4,0xFF}`.

## Battle / save

BTL0: no persist-base site in the battle cluster. This rung: no
`D_800A3180` / count site there either. Battle references the
field actor (`D_8009D254`) but does not pump or append mail.

PST0: save copies `D_800A77F0` `0x800` plus a listed adjacent
`0xA8`. `func_80040B80`/`3F800`/`3FBD8` do not mention `3180` or
`CDB4`. Actor pool `D_800BEA90` is rebuilt, not saved.

SYS0 §4 flips DEBT-FID1-004 to BLOCKER when battle or save
**share** the task/mailbox tables. They do not. The flip stays
`NONBLOCKING`. BTL1 can still implement 3/4 on this model
without a rescope.

## MBX2 contract (not implemented here)

1. Keep the 28×12 table and `u8` count.
2. `0x1C` = `func_80017764` + `653B8` (extra 0).
3. Each field tick: `65400` then script/actor work.
4. `0x1F` = read `task+0x14`.
5. Do not ACK. Do not last-write-wins. Do not scene-clear the
   queue unless a later EXE site is proven.
6. Mailbox 3/4 need no extra opcode.
