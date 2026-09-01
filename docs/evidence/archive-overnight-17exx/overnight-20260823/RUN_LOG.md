# Overnight Volume Run — 2026-08-23

**Baseline:** 279 matching C leaves (grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml)
**Baseline SHA-1:** 452fb033f2eaa4b18aa20a5bca60b8125af3a37b (verified exact)
**Branch:** `phase5fm-main-barrier-revisit` @ 481a911
**Target:** 8-12 leaves matched
**Hard stops:** H1-H5 per mission rules

---

## Run Log

| # | Target | Screen Summary | Outcome | Words | Iters | Commit/Stash |
|---|--------|----------------|---------|-------|-------|--------------|
| 1 | func_8006E7E8 | VSync poll helper; calls func_800811E4, checks -1/0 return, clears D_800B0CD8 bit. Callee asm (func_800811E4) — call-site-determined. Clean S2a-S2f. | MATCHED @ -O2 -G0 | 19 | 2 | 278006a |
| 2 | func_80017EC4 | Double-deref halfword clamp; whole 86C4.s segment (14 words). No callees. S2c pre-diagnosed (beqz to ret block). | MATCHED @ -O2 -G0 | 14 | 1 | 278... (pending) |

---

**Correction 1 (Record):** Initial park diagnosis was wrong — "link order disruption" was a botched carve registration (wrong SIZE_5B1E4, missing build-script refs). Fixed geometry (prefix 0x3E04, C 0x4C, resume 0x5F034) and trim passes. Mechanism: PARKED-INTEGRATION-ERROR → RESOLVED.

**Correction 2 (C4 Audit):** Candidate uses register variables for return (`$5`), temp (`$2`), mask (`$4`), ptr (`$3`) — C4 permits register bindings when proven necessary for matching (retail keeps mask in `$a0`/`$v1`, ptr in `$v1` across branch). No pins/inline asm.

**Correction 3 (Integration Diagnostic):** Template: 3E610 mid-2E7D8 / 2F970 mid-11718. Geometry arithmetic: prefix = 0x5EFE8 - 0x5B1E4 = 0x3E04, C = 0x4C, resume = 0x5F034. Diff vs template: yaml C segment between two asm segments (vs single C leaf); build script adds 3 ref blocks (trim, link .text/.data/.rodata/.bss). Verified: trim 0x3E10→0x3E04 (-12), exact link, EXACT SHA-1.

**Leaf 2 Integration:** Template: whole-asm-segment carves (3E610, 2F970). Geometry: 86C4.s segment IS func_80017EC4 (0x86C4–0x86FC, 0x38 bytes). Verified: trim 0x38→0x38 (exact), exact link, EXACT SHA-1.

**Status:** 281 matching C leaves (279 → 281). Run continues for remaining target.
