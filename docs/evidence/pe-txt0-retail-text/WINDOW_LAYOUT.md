# WINDOW_LAYOUT

## Default field message window (`$a1 == 0`)

From `func_800371B0` (subsystem init) and the `$a1==0` arm of
`func_80037870`:

| quantity | value | source |
|---|---:|---|
| window width | 320 (`0x140`) | `sh 0x140, 0xC(prim)` at `0x800373EC` |
| window height | 54 (`0x36`) | `sh 0x36, 0xE(prim)` at `0x800373F4` |
| window y | 170 (`0xAA`) | `0x800373FC` |
| text origin x | 20 (`0x14`) | `sh 0x14, 0x10($sp)` at `0x80037A10` |
| text origin y | 174 (`0xAE`) | `sh 0xAE, 0x12($sp)` at `0x80037A0C` |
| line height | 12 (`0x0C`) | F7 does `y += 12`, restore x from `0x58($sp)` |
| glyph cell | 12×12 | SPRT `w=h=0xC` |
| lines that fit | 3 | 174, 186, 198; window bottom 224 |

Top margin is 4 px (174−170). After three 12 px lines the cursor
is at y=210; 224−210 = 14 px bottom margin.

When `0x124($gp)==1` (set by `func_800371A4(1)` in the same boot
function that ORs the English-stream bit), a `0x0F` space first
subtracts 7 from x and then draws the 14-wide space glyph, net
advance **7**. When that byte is 0, space advances 14.

## Named / custom window (`$a1 != 0`)

`func_800375E0` copies `0x128..0x12E($gp)` into
`record+0x12..+0x18`. `$a1==3` ORs flag `0x00100000` and pulls
RGB from `0x154/158/15C($gp)`. `$a1==1` calls `func_8005E894` /
`func_80061C34` with those halfwords. Opcode 0x0D never takes
this arm.

## Types on the current slice

m0372i `0x14..0x20` and m0004i `0x21..0x23` all open through
0x0D with `$a1=0`. One default layout. FA in the stream draws a
name from `D_80091694` inside that same window; it does not
select a second rectangle.

## Wrapping and clipping

No automatic wrap was observed. Newlines are explicit `0xF7`.
There is no clip test against the 320×54 box in the generic
glyph path. Overflow is unproven.

## Advance indicator

No separate cursor sprite was isolated. F8 / FF wait on
processed pad bit `0x100` (`D_8009D1F4`). Whether a triangle
glyph is drawn on state 2 is **UNKNOWN**.
