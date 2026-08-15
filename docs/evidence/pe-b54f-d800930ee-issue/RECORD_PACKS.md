# Record 0 and record 1 packs

Same GetTPage/GetClut formula as B54D records 2/3. Sources are
initialized EXE halfwords at `D_80091648`. Dest halfwords are 0
in the EXE and written here.

## Record 0 — font atlas

```text
source  D_80091648 {A=0x0140, B=0, C=0x0140, D=0x00FC}
        = {320, 0, 320, 252}
dest    D_80091650 / D_80091652
pack    0x0025 / 0x3F14
        GetTPage(0, 1, 320, 0) / GetClut(320, 252)
```

These equal the TIM dests walked at `0x8006AFA8`:
`{320,0,64,256}` and `{320,252,16,1}`. `func_80037870` reads
them as `lhu 0xC($fp)` / `lhu 0xE($fp)` with `$fp = D_80091644`.

## Record 1 — packed, not a second font

```text
source  D_80091658 {A=0x0180, B=0, C=0x0150, D=0x00FC}
        = {384, 0, 336, 252}     # EXE constants, not CD payload
dest    D_80091660 / D_80091662
pack    0x0026 / 0x3F15
        GetTPage(0, 1, 384, 0) / GetClut(336, 252)
```

That is **not** the font atlas (`320,0` / `320,252`) and **not**
the newly issued TIM at PE.IMG `[197,200)` (that file is a TIM
with flag 8; its CLUT header is `{304,504,16,8}`). `37870` does
not read record 1. No consumer is named this rung.

Adjacency (tpage immediately right of record 0; CLUT at y=252,
x=336 vs 320) is consistent with a second glyph page or an
extended-character region. It is not proof. TXT0 decoded 57
glyphs against a 21-column atlas; if TXT1 cannot place a reel
glyph — including unmapped `0x4B` — record 1 is the first place
to look. The consumer settles it.

```text
record1_packs=0x0026/0x3F15
record1_source=EXE D_80091658 {384,0,336,252}
record1_is_font=NOT_ASSUMED
```
