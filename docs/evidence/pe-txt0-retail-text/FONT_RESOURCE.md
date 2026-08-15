# FONT_RESOURCE

## What is proven

Retail glyphs are **not** a host TTF. `func_80037870` builds 12×12
`SPRT` primitives.

UV from character code (`0x80038724`):

```text
col = code % 21          /* reciprocal 0x86186187 / 21 */
row = code / 21
u   = col * 12
v   = row * 12
w   = h = 12
```

Advance is `D_800916A0[code*2 + 1]` (EXE width table, odd byte of
each pair). The even byte is subtracted from x before the sprite
is placed and is 0 for every first-play code.

RGB for the sprite comes from `0x138/0x13C/0x140($gp)`, default
`0x80,0x80,0x80`. CLUT halfword is `lhu 0xE($fp)` with
`$fp = D_80091644`. Those halfwords are **zero in the EXE** and
are filled at runtime.

## What is not proven

The VRAM rectangle that holds the 21-column atlas is **not**
identified.

- Field-package uploads are 64×255 texture pages plus CLUT-sized
  strips at `(0,450)` / `(0,452)` and a `256×32` strip at
  `(0,480)`. None is tied to the 12×12 / 21-column formula by a
  LoadImage caller inside `func_80037870`.
- `D_80091644` / `+0x0C` / `+0x0E` (tpage / CLUT) are runtime
  state. Their writers are not pinned.
- EXE TIM signatures at file `+0x82E34` etc. are not shown to be
  this font.

Therefore `retail_font_ready=NO`. A TXT1 renderer may use the UV
and width tables, but it must not invent atlas pixels.

## Diagnostic host font

`pe_txt0_decode.py --png` draws a 5×7 host font into the 320×54
window rectangle and labels the image
`NON-PRODUCTION DIAGNOSTIC`. That path is research-only.
