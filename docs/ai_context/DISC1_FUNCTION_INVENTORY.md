# Disc 1 Function Inventory (Phase 4)

Single source of truth for mapping the split text regions in the generated disassembly. This is a **read-only survey** of what the current splat split produced. No code changes, no C, no symbol renames, no boundary edits.

## Repo state at inventory time

- Branch: `phase4-disc1-function-inventory` (created after local simulation of Phase 3 merge per instructions; in real flow this follows push/PR/merge of `phase3-disc1-boundary-audit`)
- HEAD: (post-merge of Phase 3 parked state)
- Working tree: clean (generated asm/ and linkers/ are git-ignored)
- Config boundaries (verified):
  ```
  [0x800,     rodata]  prefix jump tables + strings
  [0x2A0C,    asm]     main text from func_8001220C
  [0x818A0,   rodata]  mid-image data island
  [0xB2AF8,   asm]     tail code from func_800C22F8
  ```
- `scripts/split_us.sh --check`: passes (prerequisites and gitignore coverage OK; no split invoked in this session)
- Generated output status: present locally under `asm/disc1/2A0C.s` and `asm/disc1/B2AF8.s` (and data rodata files), all git-ignored. Never committed. Reproducible via `scripts/setup_env.sh && scripts/extract_us.sh 1 && scripts/split_us.sh` against the known SLUS-006.62 (SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`).

## Generated asm files used for evidence

- `asm/disc1/2A0C.s` (~8 MB, main text region)
- `asm/disc1/B2AF8.s` (~17 MB, tail text region)

Raw bytes from `build/extracted/disc1/SLUS_006.62` used to corroborate VRAM ↔ file offset mapping and prologue bytes.

## Inventory summary

- **2A0C.s (main text)**: ~2007 auto-labeled functions (`glabel func_...` / `endlabel`). VRAM range approx. 0x8001220C – 0x80091080. File offsets 0x2A0C onward.
  - Starts immediately after prefix rodata at known boundary.
  - Contains the pc0 entry point.
  - Ends with small leaf functions before the mid-image rodata island.
  - Extremely low incidence of embedded data (only ~10 `.word`/`.asciz`/`.byte` directives total — confirms clean split).
- **B2AF8.s (tail text)**: ~352 auto-labeled functions. VRAM range approx. 0x800C22F8 – ~0x800E05xx. File offsets from 0xB2AF8.
  - Resumes cleanly after 8-byte zero padding in the prior rodata island.
- Total auto functions across text: ~2359.
- Function sizes (from splat "nonmatching" size hints): range from very small (0x14–0x20 bytes) to hundreds of bytes. Many have standard MIPS prologues (`addiu $sp,$sp,-N`) + `jr $ra` epilogues.
- "Handwritten function" markers appear on special/crt0-style code (including pc0).
- Control flow heavily uses `jal` to other `func_*` labels (consistent with C calling convention).

## Special anchors (preserved, high confidence)

### func_8001220C (VRAM 0x8001220C, file ~0x2A0C)
- **Evidence**: First `glabel` after the rodata prefix. `addiu $sp,$sp,-0x28` prologue. Target of `jal` from the pc0 crt0 code. Size hint 0x2EC. `endlabel` present. Starts the main text segment.
- **Confidence**: high
- **Notes**: Entry point after startup. Standard prologue. Calls into other early functions.

### func_80072534 (VRAM 0x80072534, file 0x62D34 inside 2A0C.s)
- **Evidence**: Marked `/* Handwritten function */`. pc0 from PS-X EXE header. Code: `lui`/`addiu` for zero-fill loop `D_8009CDF8` → `D_800C20C8`, stack setup, `$gp`, `jal func_800726B4`, `jal func_8001220C`, `break 0,1`. Matches classic Psy-Q crt0. Size hint 0xA8. Preceded/followed by nops and other labels.
- **Confidence**: high (already verified in Phase 3)
- **Notes**: Do not treat the BSS labels (D_8009CDF8 etc.) as ROM boundaries. This is runtime zero-fill range.

### func_800C22F8 (VRAM 0x800C22F8, file 0xB2AF8)
- **Evidence**: First `glabel` in B2AF8.s after the mid-image rodata island. `addiu $sp,$sp,-0x18` prologue (size hint 0x11C). `endlabel`. Called from many places in tail (cross-refs visible). Sustained valid MIPS follows.
- **Confidence**: high
- **Notes**: Clean resume of asm after rodata padding. No folding of prologue into prior rodata.

## Sample functions (illustrative, conservative confidence)

| File | Function label | VRAM | Approx. file offset | Evidence | Confidence | Notes |
|------|----------------|------|---------------------|----------|------------|-------|
| 2A0C.s | func_8001220C | 0x8001220C | 0x2A0C | glabel + 0x2EC size + prologue + jal target from pc0 | high | Main text start anchor |
| 2A0C.s | func_800124F8 | 0x800124F8 | ~0x2CF8 | glabel, small size 0x7C, early after entry | high | Typical small early func |
| 2A0C.s | func_80072534 | 0x80072534 | 0x62D34 | Handwritten + crt0 zero-fill + break + back-jal to 1220C | high | pc0 / startup anchor |
| 2A0C.s | func_80091080 | 0x80091080 | ~0x81880 | glabel + epilogue (jr $ra, addiu sp) + size 0x20; last before rodata island | high | Last function in main text segment |
| B2AF8.s | func_800C22F8 | 0x800C22F8 | 0xB2AF8 | glabel + 0x11C size + prologue + multiple tail callers | high | Tail asm resume anchor |
| B2AF8.s | func_800C2414 | 0x800C2414 | ~0xB3414 | glabel immediately following resume func | high | Early tail function |
| B2AF8.s | func_800C251C | 0x800C251C | ~0xB351C | glabel, multiple internal jal, size 0x23C | medium-high | Representative mid-tail |

(Full auto labels number in the thousands; only high-confidence structured examples shown. VRAM directly from label names. File offsets = 0x800 + (VRAM - 0x80010000).)

## Unknown / ambiguous regions

- Large contiguous code blocks between labeled functions (control flow is dense; no obvious large data islands inside text segments).
- Areas with heavy `jal` / branch density but few obvious leaf functions may contain inlined code or complex logic (medium confidence until cross-referenced).
- "Handwritten" marked functions (~dozens scattered) — treated as special (crt0, SDK glue, or hand-tuned) rather than standard compiler output. High confidence they are real code due to structure.
- No large zero or ASCII blocks misclassified as text (consistent with prior boundary audits).
- Functions near the end of 2A0C.s (near 0x800910xx) and start of B2AF8.s may have cross-island dependencies via the rodata pointers at 0x818A0.

## Recommended next step

- Treat this inventory + `ACTIVE_HANDOFF.md` as the map.
- Only tighten boundaries or add manual labels if a **true blocker** appears (e.g., code emitted as data in rodata, or instructions in strings).
- Future Phase 5+ work (symbol naming, fingerprinting compiler) can use this as base without re-auditing the terrain.
- Push/PR the phase4 branch only after completing useful inventory additions.
- Do not start C conversion or matching until Phase 4 survey is solid and Phase 4 harness (verify) exists.

All evidence derived strictly from:
- Raw SLUS_006.62 bytes at known offsets.
- Generated (but uncommitted) `2A0C.s` and `B2AF8.s`.
- Prior Phase 3 boundary facts.

No decompilation performed.
