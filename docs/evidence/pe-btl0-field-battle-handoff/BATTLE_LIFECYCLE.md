# BATTLE_LIFECYCLE — top-level only

Battle code is **not** a separate overlay in this audit. The consumer
sits in main EXE text around `0x80029818`–`0x8002F7xx`, next to the
already-matched 7-slot table leaves.

## Init

`0x80029818` (void-ish, `$s0` from `a0`):

- `D_8009D28C = 0`
- `D_8009D290 = 0`
- several `$gp` bytes/words zeroed (`+0x104/+0x108/+0x10C/+0x478/+0x508/+0x518/+0x594`)
- jal `func_80020EFC` (matched 5-byte clear)
- 10-wide halfword table at `0x800A7FF0` filled `{0, -1}`
- 7 words at `0x800C8A90` zeroed
- jal `func_80071A64`, `func_800293F4(0)`, `func_800209F0`
- clamps a record `+0x08` tick
- jal `func_80030640`, `func_800339A0`, `func_8001A680(D_8009D254, byte)`

This is **bring-up**, not per-command.

## Request / consume

`0x800299CC` tick:

1. If `record+0x4C & 0x00080000` and `D_8009D28C == 6`:
   set `gp+0x10C`, **`D_8009D28C = 0`** (consume the `0x89` edge).
2. Reload Aya `D_8009D254` / `gp+0x508`.
3. If mode != 0 → `0x8002A7F8` (non-zero mode path; compares 1 and 2).
4. If mode == 0 → continue active body.

So **6 is an edge**, not a sticky “we are in battle” value. The script
later waits for **7**.

## Active

The `0x800299CC` body is the active dispatcher (456-byte frame).
It is not decoded command-by-command. Visible sub-stores:

| VA | Store |
|---|---|
| `0x8001F41C` | `D_8009D28C = 3` |
| `0x80021F04` | `D_8009D28C = 4` |

`0x8002A7F8` compares the mode to 1 and 2 — other game modes, not
claimed as battle.

## Victory / defeat / exit

Not decoded as named states. Script-side after first `0x89`:

```text
poll D_8009D28C until == 7
then 0x40 / 0xAA / mailbox
```

`0x95` (`D_8009D28C = 0`) and `0x96` (`= 8`) appear later on the
same module. Defeat path, EXP apply, and return-to-title are
**RESEARCH_REQUIRED**.

## First stable post-handoff state

After `0x89` returns to the field VM (it returns 1 immediately):

```text
D_8009D28C               6          (until the next 0x800299CC consume)
slot table               claimed    (0x6F already ran)
Aya actor                same object
field scene              m0005i
script PC                +0x3514 wait loop
```

The first **consumed** stable state is mode 0 + `gp+0x10C` set,
still on m0005i, slot table live. That is the BTL1 target snapshot.
