# TXT1 IMPLEMENTATION CONTRACT

Research contract only. Do not implement production text in this rung.

## Allowed inputs

- Retail Disc 1 / PE.IMG / EXE.
- Message IDs from field scripts (0x0D operand).
- Shared slot7 stream 1 when `D_800B0CD8 & 0x40000000` (USA boot sets it).

Forbidden as authority: fan transcripts, host TTF as retail glyphs,
aesthetic guesses for unmapped codes.

## Required pipeline

```text
script 0x0D id
  -> allocate D_800BCEA8 record (state 1, id at +0x10, a1=0)
  -> bind D_8009CD90 = selected slot7 payload
  -> search (FF|F9) FE id
  -> decode body with ENCODING.csv / CONTROL_CODES.csv
  -> layout WINDOW_LAYOUT.md
  -> 0x22 polls byte0 until 0
```

## Must implement

1. ID lookup via the three-byte marker, not via a Python string table.
2. Stream 1 for USA first-play. Stream 0 stays available but undecoded
   here (different encoding).
3. Close rule from TEXT_LIFECYCLE.md / RD5-X:
   - `0xFF`: state 1→2, then newly-pressed `0x100` → 0
   - `0xF9`: state → 0 immediately (after any FB 07 pause)
4. Default window 320×54 at y=170, text origin (20,174), line height 12.
5. FA draws `D_80091694[0..count)`. Do not invent character names.

## Must not implement yet

- Retail atlas pixels (VRAM dest unproven).
- Automatic wrap.
- `$a1 != 0` named-window geometry (not used on 0x14..0x23).
- FB 00–03 / 08 / 09 / FC / FD beyond “UNKNOWN or skip” unless a
  later slice requires them.
- Mapping of code `0x4B`.

## Diagnostic exception

A host font is allowed only if labeled non-production and never used
as a fidelity gate.

## Ready flags

```text
retail_string_decoder_ready = YES
retail_window_renderer_contract_ready = YES   # geometry + lifecycle
retail_font_ready = NO
txt1_implementation_ready = NO               # blocked on atlas pixels
```
