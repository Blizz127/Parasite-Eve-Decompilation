# FIDELITY_RISKS — clean-runtime vs retail persist

No production patch this rung. Risks only.

## How to read this list

A row is a place where an existing field/playtest convenience
stores an **outcome** (next map, “story started”, a short persist
vector) while retail later reads an **intermediate or extra**
persist cell. Those cells are save-backed (`0x800` memcpy) and
are compared with exact thresholds.

## Risks

| ID | Existing approximation | Retail later reader | Why it becomes unsafe | Severity |
|---|---|---|---|---|
| R1 | 8-word persist vector (`list[int] * 8` in `pe_rd3b_transition.py`) | binder mode 2 allows indices `0..0x1FF`; Day-1 uses `0x4A` | `persist[0x4A]` is index 74; an 8-slot host array cannot hold it; save will emit 512 words | **HIGH** |
| R2 | Treat `persist[0x4A]==0` as equivalent to `9` (RD4 contract “0-equivalent”) | m0002i `persist[0x4A] < 9` writer; lobby `< 0x18` / `>= 0x11`; concert `< 0x30` / `> 0x78` | 0 and 9 take different m0002i arms; later maps distinguish 9 / 0x11 / 0x12 / 0x18 / 0x30 / 0x78 | **HIGH** |
| R3 | Skip writing `persist[0x4A]` and only load the next package | m0372i / m0004i / m0378i / m0005i all branch on `0x4A` | A hop that only sets the dest token leaves the previous gate; first-play dest of m0003i north is m0372i only because `9 < 0x11` | **HIGH** |
| R4 | Skip `persist[1]` and pick a spawn by “nearest triangle” or a hard-coded pose | dest modules equality-test `persist[1]` against 1 / 2 / 3 / 4 / 0x179 / 0x17A / 0x3E7 | Wrong pose, wrong return door, or the unused persist[1]==3 lobby leftover becoming live | **HIGH** |
| R5 | Collapse `persist[0]` to unused / always 0 | m0002i / m0004i / m0372i `andi 2` and `andi 4`; m0004i cutscene arm is `& 2 == 0` | Setting bit 2 skips the m0004i reel; a later save restores the bit | **MEDIUM** |
| R6 | Zero persist on field load (reuse `func_80034F10`) | `func_80034FC4` rebuilds actors only | Wipes entrance selector and gate mid-route | **HIGH** |
| R7 | Save only “story flags” invented for UE | `func_8003F800` copies all 0x800 bytes + sibling words | Missing cells come back as 0 on load and take the new-game arm | **HIGH** |
| R8 | Host convenience `storyProgress` int | no such retail cell; `0x4A` is a compared word, `0` is flags, `1` is entrance | Two authorities; battle/menu will read the bank, not the convenience int | **HIGH** |
| R9 | Ignore `persist[0x18]` / `[0x19]` / m0005i slots 8/0x0A/0x12/0x50/0x54/0x64 | m0005i (Day-1 Carnegie interior, 75 accesses) | First-play does not yet enter m0005i; the moment it does, those cells are live and save-backed | **MEDIUM** |
| R10 | Assume battle never reads persist | `func_80053128` reads **other** index ranges (`28..33`, `49..53`, …) as item-like `0x100..0x17F` values | Day-1 slots 0/1/0x4A are **not** in that table; a later item-id persist cell would be | **LOW now / HIGH later** |

## Highest-risk existing shim

**R1+R2+R3 together:** a short persist vector plus “0 means 9” plus
token-only hops. That is the current field-research convenience
shape. It is enough to play one hop and is already wrong for
save, m0004i reel gating, and m0378i south vs m0001i.

Do not promote those shims. Replace them with the 512-word bank
and the exact writers in `CURRENT_ROUTE_TIMELINE.csv`.

## What is *not* a collision (this rung)

- GPU/boot `persistent guest writes` in the Phase 6E-B oracles
  are DMA/GPU words, not `D_800A77F0`.
- Card-slot object `0xF0` at `D_800A5B70` is not persist[].
- Scratch `D_800B6A80[0x14]` mailbox mutex is not save-backed.
