# SAVE_BOUNDARY — persist serialization contract (not an implementation)

Evidence-only. No save file was written or parsed this rung.

## What is proven

`func_80040B80` (`0x80040B80`) builds a RAM image at `*D_800A0ED0`,
then `jal func_8003F800`.

`func_80042264` (`0x80042264`) consumes that image and
`jal func_8003FBD8`.

### Persist range

| Direction | Function | Source | Dest | Size |
|---|---|---|---|---|
| save | `func_8003F800` | `D_800A77F0` | `*D_800A0ED0` | **0x800** |
| load | `func_8003FBD8` | `*D_800A0ED0` | `D_800A77F0` | **0x800** |

Both are 16-byte memcpy loops (`lw`/`sw` or `lwl`/`lwr` if
misaligned) with end pointer `base + 0x800`. That is the entire
512-word persist bank. There is no slot-by-slot filter.

### Words packed immediately after persist (same function)

Cursor after the 0x800 copy, still in `func_8003F800`:

| Order | Source | Size | Notes |
|---:|---|---|---|
| 1 | `D_8009D2E8` | 4 | pad/inhibit word |
| 2 | `D_8009D280` | 4 | dest token |
| 3 | `D_8009D1A0` | 4 | field flags |
| 4 | `D_800B0CDC` | 4 | adjacent to CE0 pack |
| 5 | `D_800B0CE0`..`CE6` | 7 | package header bytes |
| 6 | `D_800BCFEE` | 1 | camera/fade byte |
| 7 | `D_800B8A20` | `0x70` | UNKNOWN block |
| 8 | `D_800B0CB0` | `0x18` | UNKNOWN block |
| 9 | `D_8009D1B0` | 8 | UNKNOWN pair |

Persist-adjacent payload after the 0x800 copy: `0xA8` bytes.
`func_8003F800` total from persist start: `0x8A8`.

### Prefix before persist

`func_80040B80` advances `D_800A0ED0` by **`0x12E4`** immediately
before `jal func_8003F800`. That prefix is a separate RAM range
(not persist). Identity of every field in the prefix is
**not** closed this rung.

### Checksum

After `func_8003F800` returns, `func_80040B80` walks bytes with:

```text
init  $v1 = 0xFFFF
xor with (table_byte << 8)
if bit15:  (x << 1) ^ 0x1021
else:      x << 1
table base D_8009EED0
```

This is CRC-16-CCITT reflected-style with poly `0x1021` and a
256-byte table at `D_8009EED0`. The exact byte span covered by
the CRC was **not** closed past the first loop setup. Slot-file
header magic, card-block count, and icon/title metadata are
**not** proven here.

Save-manager bring-up (`func_800844E4` / `func_80084644`) owns
two `0xF0` slot objects at `D_800A5B70` / `D_800A5C60` and a
`0x1E0` zero region. That is **card-slot UI/state**, not the
persist bank.

## What is reconstructed rather than saved

Proven **not** in the `func_8003F800` persist copy:

- actor pool `D_800BEA90` (rebuilt by `func_80034FC4`)
- binder mode-1 actor locals (`+0xAC`)
- binder mode-3 cond bank `D_8009DF70`
- binder mode-4 scratch `D_800B6A80` (zeroed on new-game only)

Field scripts, walkmesh, and camera containers come from the
loaded PE.IMG package, not from the save.

## Slot metadata

UNKNOWN this rung. `func_80084644` clears per-slot bytes at
`+0x49/+0x46/+0xE6/+0x14/+0x18/...` of the `0xF0` slot object.
Those offsets are card-manager fields, not persist indices.

## Contract for SAV0

A later save rung must:

1. Treat `D_800A77F0[0..0x1FF]` as one opaque 0x800-byte blob.
2. Keep the 0x12E4 prefix + 0x8A8 persist-adjacent tail as a
   second research target; do not invent their layout.
3. Close CRC span, header, and card block map before claiming
   a loadable file.
4. Not implement files in PST0.
