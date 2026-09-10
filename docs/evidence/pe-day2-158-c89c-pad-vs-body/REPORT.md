# DAY2-158 — C89C pad-vs-body at tip `a03d599`

Dig branch: `dig/c89c-pad-vs-body` @ `a03d599` (throwaway; report only).
Live tip line under review:

```
c89c_tel calls=1 ret=0 pad=1 bound=0 a0=80142900 a1=80132700 a2=80162100
a0ram=1 a1ram=1 a2ram=1 out=7300 hdr=0002/00200280 admit=ready
[STUB] func_801909B4_post_movie_title_cut
[C89C] calls=1 pad_exits=1 bound_exits=0
```

## Root-cause hypothesis (high confidence)

**There is no pad-vs-body / cursor-offset bug.** Frame-1 of `FMV001.STR`
decoded successfully through live C89C and took the **normal** end-of-frame
pad exit. The apparent contradiction (“hdr non-pad to admit helper, yet
C89C pad-exits”) is two different predicates:

| Check | What it tests | This dump |
| --- | --- | --- |
| `c89c_stream_is_immediate_pad` | Header already is a **first-symbol** pad plant | **false** → admit used `StreamFrameReady` (`admit=ready`) |
| C89C `pad_exit` (CB84) | **Any** t5_dispatch sees pad symbol (incl. after main_loop) | **true** at end of a full frame → `ret=0`, `pad=1` |

`out=7300` is not “empty instant pad.” It matches the DAY2-153 complete-frame
oracle’s **frame-1 RLE extent including command** exactly
(`docs/ai_context/DAY2_MOVIE_COMPLETE_FRAMES.md`: frame 1 → 7300 bytes).

## 1. What `hdr=0002/00200280` means

Printed in `func_801924F8` got_frame (`pc_port/game/boot/func_801924F8_port.c`):

```text
hdr=%04x/%08x  ←  hdr_count / hdr_bits
```

Filled only on C89C **fresh** entry (`func_8010C89C_port.c`):

| Field | Source | This dump |
| --- | --- | --- |
| `hdr_count` | `lhu` at `a0+6` **before** `t2 -= 3` | `0x0002` |
| `hdr_bits` | `(lhu(a0+8) << 16) \| lhu(a0+10)` | `0x00200280` |

Derived (same math as admit helper + C89C header):

```text
word0     = lw(a0+0)          # not in hdr=; inferred below from out
count_hw  = 0x0002            # a0+6
t2        = count_hw - 3 = -1
bits v0   = 0x00200280
sym       = v0 >> 22 = 0x000   # top 10 bits of 0x00200280
```

Because `t2 < 0`, fresh path leaves `t5 = 0` (does **not** set `t5 = 1`).

`out=7300` + pad fill to `[EBB4]`:

```text
bound = a1_entry + ((word0 & 0xFFFF) << 2) + 4
out_bytes ≈ bound - a1_entry = 7300
⇒ (word0 & 0xFFFF) = (7300 - 4) / 4 = 0x0720
```

So stream word0 low half is `0x0720` (size field), **not** STR magic low
half `0x0160`.

## 2. Immediate-pad vs C89C pad-exit control flow

### Admit helper (`c89c_stream_is_immediate_pad`, duplicated in 924F8 / 92934)

```c
t2 = count_hw - 3;
sym = bits >> 22;
if (t2 < 0) return (sym ^ 0x1FF) == 0;  /* MV1D_PAD shape */
return (sym ^ 0x3FF) == 0;              /* MV1D_PAD3FF shape */
```

This dump: `t2 < 0` and `sym == 0` → **not** immediate pad.
Got_frame therefore required `TakeStreamFrameReady()` → `admit=ready`. Correct.

### C89C fresh header → first `t5_dispatch` (C960)

```text
t5 stays 0 (t2 < 0)
t0 = v0 >> 22 = 0
t5 == 0 → ca38
ca38: (t0 ^ 0x1FF) == 0 ? pad_exit : extract DC limb → main_loop
```

First dispatch: `0 ^ 0x1FF ≠ 0` → **does not** pad-exit. Decoder enters the
`t5 == 0` limb + `main_loop`. Later `FE00` restarts return to `t5_dispatch`
still with `t5 == 0`; when a later window has `sym == 0x1FF`, CB84 pad-exits
and FE00-fills to the word0-derived bound.

That is the normal end-of-frame path when `[EB8C] = 0x00FFFFFF` (no early
bound). One call, `ret=0`, `pad_exits=1` is **success**, not a skipped body.

Fixture contrast (`test_MV1D_c89c_pad`): count=0, bits=`0x7FC00000` →
`sym=0x1FF` → both immediate_pad **and** first ca38 pad-exit; `out_bytes=8`.

## 3. Is `s1=0x80142900` STR header or VLC body?

**VLC / demux body.** Not STR sector header. Not wrong by +0x20.

Evidence:

1. **7C564 payload copy** (`cd_stream_port.c`): RAM path copies
   `memory + (BCD7C<<11) + 32` → `C0DC4` for `0x1F8` words (= 2016 bytes).
   The `+32` skips the 32-byte STR header. Raw disc offset 56 =
   Mode2 sync/subheader (24) + STR header (32) — same strip
   (`DAY2_MOVIE_COMPLETE_FRAMES.md`, `DAY2_STREAM_RECORD_ASSEMBLY.md`).

2. **7C484 publish** (`func_8007A214_port.c`):  
   `dst_a = C0DC8 + (C20C4<<5) + active*2016` — start of that body slot.  
   With `C0DC8=80142100` (CDQ2d measured), `80142900 - 80142100 = 0x800 = 2016`
   → first active body slot (`count=1, active=1` → `1+63=64` records×32).

3. **Magic lives on the 32-byte record**, not at `s1`: 7C564 checks
   `lhu(rec) == 0x160` (low half of `0x80010160`) on the record before the
   body DMA. Comments in 924F8 already state this.

4. **word0.lo = 0x0720 ≠ 0x0160** — if `s1` were at magic, bound/out would be
   ~1412, not 7300.

## 4. `out=7300` / `a1=80132700` / `a2=80162100` reasonableness

| Arg | Value | Verdict |
| --- | --- | --- |
| `a1` / out | `0x80132700` | Known movie buffer: first got_frame loads `[0x801D1468]` (= `[0x801D0DEC]` copy). MV1c-map / B54K oracles. |
| `a2` / table | `0x80162100` | Known `[0x801D0DF8]`; C89C then does `a2 += 0x800`. |
| `out_bytes` | `7300` | **Exact** DAY2-153 frame-1 “RLE bytes including command.” Compressed size for that frame is 2712; pad exit writes the declared RLE extent. |

Together: live Disc1 got_frame admitted a last-chunk body, ran C89C once,
produced the authenticated frame-1 RLE footprint, then hit the post-movie
title stub — not a demux cursor miss.

## 5. How a stream fails immediate_pad yet still pad-exits

Concrete path for this hdr:

1. Header: count=2 → `t2=-1` → `t5=0`; bits → `sym=0`.
2. Admit: `sym ≠ 0x1FF` → not pad plant; `admit=ready` from 7C214 last-chunk latch.
3. C89C: first ca38 does not pad; decodes via t5==0 / main_loop.
4. End: later `sym == 0x1FF` → CB84 FE00-pad to bound → `out=7300`, `ret=0`.

No predicate bug; no need to treat pad-exit as “body was pad.”

## Recommended next change (smallest honest)

**Do not** adjust the 7C484/91B64 cursor by +32 / −32. Demux already strips
the STR header; this dump’s sizes match a known-good frame-1 decode.

Prefer (in order):

1. **Port/Auto observability (optional, small):** extend `c89c_tel` with
   `word0` and a `first_pad` / `late_pad` bit (set if CB84 hit on the first
   `t5_dispatch` vs after main_loop). Prevents re-reading successful
   `pad=1 out=7300` as “instant empty pad.”
2. **Frontier:** `func_801909B4_post_movie_title_cut` is the wall after this
   one good frame — dig title/menu handoff / `92934` multi-frame loop, not
   Stage-1b admit or demux body offset.
3. **Optional confirm (needs Disc1.bin):** dump 12 bytes at live `s1` vs
   assembled FMV001 frame-1 body (9×2016 from raw+56); expect
   `+6=0002`, `+8/+10 → 00200280`, word0.lo=`0720`.

## Patch

None. Confidence is high that a cursor patch would be wrong. No native
filter re-run required for a report-only dig.

## Related

- `docs/evidence/pe-day2-158-gotframe-no-c89c/REPORT.md` (quiet-log false negative)
- `docs/evidence/pe-mv1d-c89c/REPORT.md` / `pe-mv1c-c89c-map/NOTE.md`
- `docs/ai_context/DAY2_MOVIE_COMPLETE_FRAMES.md` (frame-1 out=7300 authority)
- `docs/ai_context/DAY2_STREAM_RECORD_ASSEMBLY.md` (+32 / 2016 demux)
