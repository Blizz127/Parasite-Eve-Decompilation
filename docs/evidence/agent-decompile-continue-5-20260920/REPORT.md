# decompile-continue-5 — 2 new matching C leaves (699 -> 701)

Branch `agent/decompile-continue-5`, worktree `/tmp/pe-agent-decomp6`, base
`a598ddd0`.

## Result

`build_us.sh` → **EXACT MATCH**, candidate/orig SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, "Matching claim: YES (701
registered C leaves)". `verify_us.sh` → **VERIFY_US=PASS**, all 701 packed C
spans equal retail. Coverage 701/4524 functions (15.50%).

## Matched leaves

### func_80076B20 (file 0x67320, size 0x24) — new maspsx gate
Publishes `a0` through the `D_80095854` pointer and tags
`D_800A3348[a0>>24]` with the low byte.

Retail keeps the indexed symbolic store in the `jr $ra` delay slot:

```
lui   $v0,%hi(D_80095854) ; lw $v0,%lo(D_80095854)($v0) ; nop
sw    $a0,0($v0) ; srl $v0,$a0,24
lui   $at,%hi(D_800A3348) ; addu $at,$at,$v0
jr    $ra
sb    $a0,%lo(D_800A3348)($at)      # delay slot
```

cc1 emits `sb $a0,SYM($v0)` before `j $31`; GNU as expands it pre-jr, so the
leaf was 4 bytes long. The existing `MASPSX_FILL_STORE_DELAY_SLOT` gate only
handles the ABSOLUTE `sw $r,SYM` macro, so it did not fire.

**New per-leaf gate** `MASPSX_FILL_INDEXED_STORE_DELAY_SLOT=1` (default OFF):
for an indexed symbolic store (`sb/sh/sw $r,SYM($base)`) immediately before a
bare `j $31`, emit

```
lui $at,%hi(SYM) / addu $at,$at,$base / j $31 / op $r,%lo(SYM)($at)
```

Build profile `era_o2_g0_fill_indexed_store_delay_slot`.
Unit tests: `TestFillIndexedStoreDelaySlotGuards` in
`tools/era/maspsx/tests/test_fill_store_delay_slot.py` (fills; never fills a
load; a label blocks the fill).

### func_8008F4E8 (file 0x7FCE8, size 0x2C) — `-O1 -G0`
Advance the cursor, latch `*old_cursor << 8` into `+0x6C`, set `+0xF4` bits
0/1. `-O2 -G0` hoists the `+0xF4` load above the cursor store; `-O1 -G0`
keeps retail's order. Profile `era_o1_g0`.

## Honest non-matches (drafts removed, not committed)

- `func_8001A374` (0xAB74): semantically exact but cc1 emits a load-delay
  `nop` where retail's `lui $at,%hi(D_800BCFFC)` fills the slot (1 word skew).
- `func_8008F1B0` (0x7F9B0): five-word bit-clear over `D_800BCD50/6C/70/74/54`;
  cc1's `and` operand order and extra address materialization differ
  (first mismatch 0x14: retail `AC620000` vs cand `00451024`).
- `func_8008F4E8` variants under `-O2` (reordered); resolved by `-O1`.
- `func_8005DB8C` (0x4E38C): same size but cc1 hoists the load above the
  shift (first mismatch 0x08).

## Environment / hygiene

Split needs `splat`, which is a HOST python package and is **not** in the
`pe-mipsel` distrobox. Run `scripts/split_us.sh` on the host, then
`build_us.sh`/`verify_us.sh` inside the distrobox. Logging inside the
distrobox must target a path under the shared repo (its `/tmp` is shared but
the host's `/tmp` logging is fine too — the failure mode observed was a stale
container `/tmp/pe-agent-*`).

## Commands

```sh
git worktree add /tmp/pe-agent-decomp6 -b agent/decompile-continue-5 a598ddd0
# copy tools/era, build/extracted, include/*.inc, local/pe_disc1.path, rom/image
bash scripts/split_us.sh
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp6 && bash scripts/build_us.sh'
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp6 && bash scripts/verify_us.sh'
distrobox enter pe-mipsel -- bash -lc 'cd tools/era/maspsx && python3 -m unittest tests.test_fill_store_delay_slot'
```
