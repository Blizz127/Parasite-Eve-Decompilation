# PE-B54B — loop-exit cut and VRAM upload table

```text
PE-B54B SUCCESS — LOOP EXIT CUT TAKEN AND VRAM UPLOAD TABLE ENUMERATED
```

Production C was **not** edited. The counted loop
`0x8006AE50..0x8006AE68` is already live (B54B `c1efff5`). This
rung enumerates all 13 `func_8006E1C0` entries from the hashed
PE.IMG packet and cross-checks named consumers.

```text
base_b54a_commit = d58ff9b8
b54b_impl        = c1efff5
tool             = python3 tools/research/pe_b54b_upload_table.py --peimg PE.IMG
peimg_sha1       = 146c0ce7308bf9fdc2ba5a84230e198db0663f3b
packet           = PE.IMG +0x4E800 (sector 157)
header           = [s3+0x28] = 0x0340B5B8  count=13 offset=0xB5B8
entry0           = 0x801229A0 + 0xB5B8 = 0x8012DF58
```

## Cut

Retail at the B54A mid-loop point:

```text
8006AE50  lw     v0,0x28(s3)
8006AE54  addiu  s1,s1,1
8006AE58  srl    v0,v0,22
8006AE5C  sltu   v0,s1,v0
8006AE60  bnez   v0,8006AE44
8006AE64   addiu s0,s0,0x14
8006AE68  .L8006AE68
```

`func_8006AD40_port.c` already issues entry 0 once, then entries
1–12, and does not enter `D_80091648` packing from *this* evidence
rung. Later B54D remains on the branch at `0x8006AF54` and is not
extended.

## Table

Every record is 0x14 bytes. `+4` image offset, `+7`/`+8` image
RECT, `+C` CLUT offset, `+F`/`+10` CLUT RECT. `func_8006E1C0`
LoadImages both. Word0 is image+CLUT byte length.

All 13 have a CLUT. Entries 1–12 use one 16-color row (`w=16`) →
**4bpp**. Entry 0 uses a 64-color row (four 16-color palettes) →
still **4bpp**. Effective pixel width = `w * 4`.

Entry 0 matches the accepted B54A pair `{704,64,32,64}` +
`{256,456,64,1}` from `0x8012B8B8`.

Full decode: `UPLOAD_ENTRIES.csv`.

## Consumers

| Named dest | In this table? |
|---|---|
| `D_80091644` tpage/CLUT | **no** — loop does not write it; `0x80037870` only reads |
| VIS1-E player page `(960,256,64,255)` / `(960,511,64,1)` | **no** — no dest x=960 |
| VIS1-E player CLUT `(0,448,256,2)` | **no** — this table’s 448/449 CLUTs start at x≥256 |

Player materials stay PE.IMG `[220,237)` / `func_8006BECC`.

## Font

No entry has ≥252 effective px of width. The 1–8 tile at
`(704,0)` is 256 px wide but has eight CLUTs and no text
consumer. Status **NOT_IN_TABLE**. TXT1 stays blocked.

---

```text
base_b54a_commit=d58ff9b8
cut_implemented=0x8006AE68
entries_enumerated=13

d80091644_source_entry=none
player_texture_source_entry=none
player_clut_source_entry=none

font_atlas_status=NOT_IN_TABLE
font_atlas_rect=
font_clut_rect=
txt1_unblocked=NO

gates=production C unchanged; B54B loop already at 0x8006AE68 (c1efff5); enumerator reproduces header 0x0340B5B8 and entry0 0x8012B8B8
frontier=func_8006AD40_prefix_cut @ 0x8006AF54 (live B54D; this rung does not advance it)

hard_blockers=none
unknowns=writer of D_80091644 tpage/CLUT (still TXT0-open; not this table)
warnings=entries 1-8 tile a 256px-wide 4bpp block at (704,0) that is size-plausible for a 21-col row but is not the atlas (eight CLUTs; no consumer)

SUCCESS

PE-B54B SUCCESS — LOOP EXIT CUT TAKEN AND VRAM UPLOAD TABLE ENUMERATED
```
