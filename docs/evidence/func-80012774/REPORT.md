# func_80012774 — work-table bucket drain walk

## Target

VRAM `0x80012774..0x80012850` (exclusive), file `0x2F74`, size `0xDC`
(55 words), from `asm/disc1/3050.s`. Frameless leaf, no calls.

Walks the head chain rooted at `D_8009D20C` (absolute `lui/lw` form;
kept absolute via unsized array-of-pointer typing so `-G8` small-data
does not claim it). Per head: iterates its 3 bucket lists (`buckets[3]`
at +0xA0; `unsigned char` bucket index with 0xFF mask and `sltiu < 3`),
and removes any node flagged by `head->f98 & 0x10` (reloaded every inner
iteration — removal stores may alias it) or by node u16 +0x08 `& 0x10`.
Removal unlinks via `prev->next` / bucket head, fixes `next->prev` with
a fresh read of `cur->next` after the stores (retail reloads), pushes
the row onto the `D_8009CDFC` cursor stack (`row->next = old cursor`),
then advances to the sibling successor.

## First attempt — EXACT

One minimal C candidate on era `-O2 -G8` (no maspsx knobs — no compound
symbolic lines) passed the size gate at exactly `0xDC` and produced:

```
Compare:  EXACT SHA-1 MATCH (452fb033f2eaa4b18aa20a5bca60b8125af3a37b)
verify_us.sh: Split verification OK; compare EXACT MATCH; 278 leaves
```

Statement order mirrored the ROM sequence (flags load first, then
sibling-next load; guard orientation `if ((flags & 0x10) || (cur->f08 &
0x10))` matched retail's bnez/beqz layout). Candidate left UNCOMMITTED
for human review.

Process note recorded for transparency: the initial wiring edit briefly
dropped the `[0x2F00, asm]` yaml entry (parked 12700's span), which made
the first post-wiring split omit `2F00.s`; caught via missing-artifact
check, repaired, and re-split before any build/claim was made.
