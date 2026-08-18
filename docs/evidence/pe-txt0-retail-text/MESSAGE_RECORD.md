# MESSAGE_RECORD — D_800BCEA8

## Canonical address

The live table is **`D_800BCEA8`**, four records of 56 (`0x38`) bytes,
extent `+0xE0`.

`0x800CCEA8` is a sign-extension misread of `lui 0x800C` +
`lbu/lh -0x3158/-0x3148`. Matching leaf `func_80037548` and every
indexed access in `func_800375E0` / `func_80037870` resolve to
`0x800BCEA8`.

Stride proof: `i*8 - i` then `<< 3` = `i*56`. Bound `sltiu 4`.

## Field map (only proven uses)

| off | size | access | proven meaning |
|---:|---|---|---|
| `+0x00` | u8 | lbu/sb | **state**: 0 free/closed, 1 open/parsing, 2 terminal-ready |
| `+0x04` | u32 | lw/sw | **stream cursor**. Loaded from `D_8009CD90`, then advanced |
| `+0x08` | u8 | sb `$a1` | **open-mode byte**. 0x0D stores 0. Nonzero copies window geometry |
| `+0x09` | u8 | sb 1 | set when `$a1==0` and `0x160($gp)` is nonzero |
| `+0x0C` | u32 | lw/sw | **flags**. Open clears `0x00100000` and `0x00200000`. Close of `0xFF` refuses to zero state if `0x02000000` is set |
| `+0x0D` | u8 | lbu/sb | **FB 07 counter**. Incremented until it is not `<` the operand |
| `+0x0E` | u16 | lhu | nibble compared to the per-update index at `0x28($sp)` on FB 07 |
| `+0x10` | s16 | sh/lh | **message ID** |
| `+0x12` | u16 | sh | window x, copied from `0x128($gp)` only if `$a1 != 0` |
| `+0x14` | u16 | sh | window y, from `0x12A($gp)` |
| `+0x16` | u16 | sh | from `0x12C($gp)` |
| `+0x18` | u16 | sh | from `0x12E($gp)` |
| `+0x1A` | bytes | sb loop | decimal-digit buffer from the `$a2` list (unused on 0x0D) |

Bytes `+0x01..+0x03` and most of `+0x1A..+0x37` have no proven
semantic name on this path.

## State machine used by 0x14..0x23

```text
free (0)
  -- 0x0D / func_800375E0 --> open (1)
open (1)
  -- parser_stop && state==1 at 0x800388AC --> ready (2)
  -- in-stream F9 at 0x80037C50 --> free (0) immediately
ready (2)
  -- 0xFF path and newly-pressed 0x100 and !flags.0x02000000 --> free (0)
```

Opcode `0x22` only polls `func_80037548(id)` (returns signed
`byte0`). It never writes the record.
