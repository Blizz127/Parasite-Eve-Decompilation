# PE-TXT0 — retail text, font, message window, and string resource audit

```text
PE-TXT0 SUCCESS — RETAIL TEXT, FONT, AND MESSAGE WINDOW CONTRACT ESTABLISHED
```

Evidence-only. No production text runtime. No push.

```text
workspace = /var/home/blizz/dev/parasite-eve-port-black
branch    = phase6e-b-provider-frontier
tool      = python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" --message 0x21
disc_sha256 = 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
exe_sha256  = 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
exe_sha1    = 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

Prior field rungs knew message IDs and the 0x0D / 0x22 control flow
but printed `TEXT UNRESOLVED`. This rung recovers the retail byte
pipeline far enough that TXT1 can be written without transcribing
dialogue.

## 1. Opcode 0x0D

Dispatch `0x800910A0 + 0x0D*4` = `func_80017410`.

```text
local = -1
a1    = 0                  /* hardcoded; not a script operand */
a0    = *(s16*)bound[0]    /* message ID */
func_800375E0(a0, a1, &local)
return 1
```

m0004i: `+0x0434 0x0D 0x21`, `+0x044C 0x0D 0x22`, `+0x0464 0x0D 0x23`
(each `argc=1`). m0372i module 1: `0x0D` for `0x14..0x20` at
`+0x025C..+0x037C` (also `argc=1`).

`func_800375E0` only allocates a `D_800BCEA8` record. The first
ID→bytes conversion is the `(FF|F9) FE <id>` scan in
`func_80037870`.

## 2. Record

Canonical table is **`D_800BCEA8`**, 4 × 56. `0x800CCEA8` was a
sign-extension misread. Fields: state `+0`, cursor `+4`, open-mode
`+8`, flags `+0xC`, FB07 counter `+0xD`, id `+0x10`. See
`MESSAGE_RECORD.md`.

## 3. String resources

Every first-play package (m0002i / m0003i / m0372i / m0004i) carries
the same two slot7 blobs:

| store | size | sha256 | role |
|---:|---:|---|---|
| 0 | `0x1956` | `467bf214…eba1` | non-USA encoding |
| 1 | `0x2A25` | `231da625…262a` | USA English |

USA boot `0x8005286C` does `D_800B0CD8 |= 0x40000000`. Field init
then binds `D_800B162C` (store 1) into `0x120($gp)`.

Hard chain for `0x21`:

```text
0x0D 0x21
 -> F9 FE 21 at slot7[1]+0x0604
 -> FA 4A 0F 2B 2B FF
```

`func_8005DC4C` is a different 120-entry boot table. It is not
this dialogue bank.

## 4. Encoding

Stream 1 is a **custom single-byte glyph-index stream**, not
ASCII and not a compressed dictionary.

- `0x10..0x29` → `A..Z` by `code + 0x31`
- `0x30..0x49` → `a..z` by `code + 0x31`
- `0x0F` space (dedicated handler)
- `0x2B !`  `0x2E '`  `0x2F .`  `0x4A ,` from unique decoded words
- `0x4B` remains UNKNOWN (width 4; after `Father` in 0x14)
- `0xF7+` are controls (see `CONTROL_CODES.csv`)

No fan transcript was used. Letters are arithmetic. Punctuation
is assigned only where the letter map leaves one English token.

## 5. Control codes

Required on this slice: newline `0xF7`, name `0xFA`, pause
`FB 07 nn`, chain/close `0xF9`, terminal `0xFF`. Others in
`CONTROL_CODES.csv` stay unused or UNKNOWN.

## 6. Font

21-column atlas, 12×12 cells, `u=(code%21)*12`, `v=(code/21)*12`,
advance `D_800916A0[code*2+1]`. VRAM destination and pixels are
**not** identified. `retail_font_ready=NO`.

## 7. Window

Default (`a1=0`): 320×54 at y=170, text origin (20,174), line
height 12, three lines. Same layout for 0x14..0x23.

## 8. Reveal

Generic glyphs do not yield. A body is dumped in one
`func_80037870` call until a yielding control. `0x21..0x23` are
instant then `0xFF`. `0x14..0x20` pause `FB 07 nn` then auto-close
on `0xF9`. No per-glyph timer exists on this path.

## 9. 0x22 close

RD5-X rule stands. `0xFF` is a **stream control token**. First
sighting moves state 1→2. A later newly-pressed `D_8009D1F4 &
0x100` clears state to 0. Held-from-before does not. `0x22` only
polls.

## 10. Speaker

0x0D does not take a speaker operand. `$a1=0` means default
window geometry, not “narrator” and not a character id. Names
that appear are either letters in the body (`Prince`, `King`,
`Eva`) or `0xFA` drawing `D_80091694` (ROM bytes `10 48 30`,
count 3 → `Aya` under the same map). Runtime may overwrite that
buffer; this rung does not prove who writes it on 0x21/0x23.

## 11. Current-slice decode

See `CURRENT_SLICE_MESSAGES.csv`. Examples (stream 1):

```text
0x14  Prince, Father<4B> please give me / permission to marry Eva.
0x15  King, I FORBID IT!
0x21  [NAME], !!
0x22  Man, Jesus!! I... don't wanna...! / Oh my god...!! I... I...
0x23  [NAME], Go! Get outta here!! NOW!!
```

`<4B>` is the single unmapped body glyph on the slice.

## 12. Tool

```text
python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" --message 0x21
python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" --all-slice --png out.png
```

`--png` is a 320×70 host-font diagnostic labeled
`NON-PRODUCTION DIAGNOSTIC`.

---

```text
message_lookup_status=PROVEN
string_resource_status=PROVEN
encoding_status=PROVEN_LETTERS_AND_SLICE_PUNCT
font_resource_status=PARTIAL_UV_AND_WIDTH_ONLY
glyph_mapping_status=PROVEN_FOR_SLICE_EXCEPT_0x4B
window_layout_status=PROVEN_DEFAULT
text_reveal_status=PROVEN_INSTANT_UNTIL_YIELD
control_code_status=PROVEN_FOR_SLICE
speaker_status=PROVEN_A1_DEFAULT_WINDOW_FA_NAMEBUF

opcode_0x0D_status=PROVEN
opcode_0x22_status=PROVEN

message_0x14_0x20_decode_status=PROVEN_EXCEPT_0x4B_IN_0x14
message_0x21_0x23_decode_status=PROVEN

retail_font_ready=NO
retail_string_decoder_ready=YES
retail_window_renderer_contract_ready=YES

txt1_implementation_ready=NO

hard_blockers=retail glyph atlas VRAM rectangle and pixels not identified
unknowns=0x4B glyph; writers of D_800B1628/162C; writers of D_80091644 tpage/CLUT; runtime writer of D_80091694; whether state-2 draws an advance icon; FB 00-03/08/09 semantics beyond the jump targets
warnings=0x800CCEA8 is wrong (use D_800BCEA8); func_8005DC4C is not field dialogue; stream 0 must not be decoded with the English map; host PNG is not retail; F9 auto-closes so m0372i 0x14-0x20 do not need 0x22

SUCCESS
```
