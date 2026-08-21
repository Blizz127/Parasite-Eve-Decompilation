# HUMAN_VERIFY playbook — BTL83

The harness is already identity-checked. A human only
needs to drive the pad into a live fight. Do not poke RAM.

## Launch

```bash
docs/evidence/pe-btl83-retail-battle-transition/launch.sh
```

Env:

| Var | Default | Meaning |
|---|---|---|
| `PE_BTL83_OUT` | `…/captures/pe-btl83` | CSV sink |
| `PE_BTL83_NO_PAD=1` | off | watches only; you drive |
| `PE_BTL83_AUTO_LOAD=0` | on | disable lua Continue/walk |
| `PE_DISC1` | canonical Disc 1 BIN | |
| `PE_BIOS` | `…/pcsx-redux/bios/scph1001.bin` | |

Portable dir `portable-pe-btl83` already has the Day-1
Theater memcard as `memcard1.mcd`.

Recommended for a human:

```bash
PE_BTL83_NO_PAD=1 PE_BTL83_AUTO_LOAD=0 \
  docs/evidence/pe-btl83-retail-battle-transition/launch.sh
```

Confirm `IDENTITY.txt` says `identity=PASS` and
`ram64k.bin` SHA-1 is `668f4a90…706edf`.

## Route

1. Skip attract with Start.
2. Title: **Continue** (not New Game, not Tutorial).
3. Slot 1 → file `DAY 1 Theater` / Aya Lv1 / 40/45 / 00:11:17.
4. OK. Wait for the dressing room (costume racks, desk,
   burnt corpse, evening gown). Dest must become
   `0xA8002048` (`HEARTBEAT.csv` / `DEST_WRITES.csv`).
5. Walk. Interact. Enter the next room. Trigger a
   **real** encounter (battle HUD / ATB). Do not use
   the type-3 Carnegie door as a combat seed.
6. Attack once if the first HP subtract has not fired.

New Game is the alternate if this save is already past
the first `M0005I` `0x55(2)` fight. Skip FMVs with Start.
The same watches stay armed.

## What to copy back

From `$PE_BTL83_OUT` into this evidence directory:

- `FIRST_EVENTS.md` — must name the five firsts
- `4D4_WRITES.csv` `MODE_WRITES.csv` `SCRATCH_BIT4_WRITES.csv`
  `HP_WRITES.csv` `CALL_TRACE.csv` `DEST_WRITES.csv`
- `HEARTBEAT.csv` `capture.log` `IDENTITY.txt`

Then fill the SUCCESS OUTPUT lines in `REPORT.md`.

## Addresses (if you attach GDB on 3334)

Write-watch `0x8009D244`, `0x8009D28C`, `0x800B6A80`,
`0x8009D278`, and `*(D278)+0x0C` once the pointer is
nonzero. Exec-break `0x8001D340`.
