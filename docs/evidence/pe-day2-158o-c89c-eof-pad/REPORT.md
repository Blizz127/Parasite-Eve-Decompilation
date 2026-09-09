# DAY2-158o — live C89C EOF pad + one-frame title cut (2026-09-09)

## Live dump (Bazzite Disc1 tip `a03d599`)

```text
[TRACE] func_801924F8_e0_promote
[TRACE] func_801924F8_got_frame
[TRACE] c89c_tel calls=1 ret=0 pad=1 bound=0
        a0=80142900 a1=80132700 a2=80162100
        a0ram=1 a1ram=1 a2ram=1 out=7300
        hdr=0002/00200280 admit=ready
[STUB:BOOTSTRAP_RET] func_801909B4_post_movie_title_cut
[HOST] stop_reason=unresolved-boundary
[C89C] calls=1 ret=0 pad_exits=1 … out_bytes=7300
       hdr=0x0002/0x00200280
```

## C89C pad-exit dig (CLOSED — do not patch admit gate)

Decomp interim + this rung:

| Field | Value | Reading |
| --- | --- | --- |
| `hdr_count` | `0x0002` | raw halfword at `a0+6` **before** `−3` |
| after `−3` | `t2 = −1` | `t5` stays 0 (fresh path) |
| `hdr_bits` | `0x00200280` | `sym = bits>>22 = 0` |
| immediate pad? | **no** | needs `sym == 0x1FF` when `t2 < 0` |
| `admit=` | `ready` | last-chunk `StreamFrameReady` — correct |
| `out_bytes` | `7300` | FMV001 frame-1 RLE including command (`DAY2_MOVIE_COMPLETE_FRAMES`) |
| `ret` / `pad_exits` | `0` / `1` | **normal EOF pad** after ~7KB write — not a no-op |

Pointers match the MV1c strict-path shape (`a1≈[D1468]`, `a2=[D0DF8]`).
Word0 at `0x80142900` (STR `0x80010160` vs VLC body) remains a Decomp
follow-up when a Disc1 carve is available; admit gate stays untouched.

## Why title-cut after one successful frame

1. `91FB8` leaves `D_800B0DBA = 1`.
2. `924F8` got_frame ran live C89C once (EOF pad, `out=7300`) then returned.
3. Prior port EC stores set `DBD=0` / `DBC=1` but **did not** `DBA++`.
4. Post-E08 media loop called `92934`; `DBA < 2` early-returns 0 → stream
   clear → `92CE8` returns 0 → `func_801909B4_post_movie_title_cut`.

Twin authority: movie-player `80121C04` first-frame exit does
`23F5=0 / B0DBA++ / B0DBC=1` (`DAY2_MOVIE_UPDATER`, oracle `B0DBA` 3→4).

## Fix on tip (158o)

1. `924F8` got_frame EC: `DBA++` with `DBD=0` / `DBC=1` (player-twin shape;
   PE.IMG byte confirm of the title-overlay EC block still preferred).
2. `92934` poll: last-chunk `7C214` promote / `HostFB_PumpCdProgress` so
   multi-frame can assemble after the bump.
3. `92CE8`: TRACE `func_80192CE8_media_clear` when status clears the stream.

`post_movie_title_cut` remains for a completed movie until the title/menu
tail at `0x80191120` is translated. No Day2-complete claim. No admit-gate
patch.
