# Font atlas finding

TXT0 proved a 21-column 12×12 `SPRT` atlas:

```text
u = (code % 21) * 12
v = (code / 21) * 12
```

A single row therefore needs **≥ 252 effective pixels**. This table
cannot supply that row as one `func_8006E1C0` entry.

## Every entry, measured

| Index | 4bpp width | ≥252? |
|---:|---:|:---:|
| 0 | 128 | no |
| 1–8 | 64 | no |
| 9 | 64 | no |
| 10 | 16 | no |
| 11 | 48 | no |
| 12 | 32 | no |

Entry 0 `{704,64,32,64}` is 128 px at 4bpp. It cannot hold a 21-cell
row alone (TXT0 already implied this). Stopping there would have
been wrong; the rest of the table was decoded.

## Split candidate (rejected)

Entries 1–8 tile a 64×64-word block at `(704,0)`:

```text
x = 704,720,736,752
y = 0 and 32
w,h = 16,32
=> VRAM (704,0)-(767,63)
=> 256 x 64 pixels at 4bpp
```

256 ≥ 252 and 64 ≥ 12, so the **geometry** could hold five glyph
rows. That is not a find.

Reasons it is not the atlas:

1. Each of the eight tiles has its **own** 16-color CLUT
   (`w=16,h=1` at distinct x on rows 448/449). A 21-column SPRT
   sheet uses one tpage/CLUT pair (`D_80091644+0x0C/+0x0E`).
2. This loop does not write `D_80091644`. `func_80037870` only
   *reads* it (`lui/addiu $fp, 0x80091644`). No `sw`/`sh` to
   `0x80091644` exists in the `0x8006AE50..0x8006AE68` range,
   nor in the later B54D packing of `D_80091648` records 2/3.
3. No LoadImage dest here is sampled by the glyph SPRT path.
   Appearance of the pixels was not used.

Entry 11 is 12 px tall and 48 px wide — one glyph high, four
cells, not 21.

## Verdict

```text
font_atlas_status=NOT_IN_TABLE
font_atlas_rect=
font_clut_rect=
txt1_unblocked=NO
```

The atlas is not this 13-entry boot/channel-1 table. TXT1 stays
blocked on a proven VRAM rectangle plus CLUT.
