# func_8009090C — matching C leaf (707)

- VRAM 0x8009090C, file 0x8110C, size 0x3C / 15 words.
- Leaf: `src/func_8009090C.c`; carve `- [0x8110C, c, func_8009090C]` in
  `configs/USA/disc1.yaml` (asm resume at 0x81148).
- Triage: `python3 tools/analysis/try_leaf.py src/func_8009090C.c 0x8110C 0x3C`
  → WORDS MATCH.
- Authority: `bash scripts/build_us.sh` → EXACT SHA-1
  452fb033f2eaa4b18aa20a5bca60b8125af3a37b, "Matching claim: YES (707
  registered C leaves)"; `scripts/verify_us.sh` VERIFY_US=PASS.
- Lever: the address add must have the scaled index as the LEFT operand
  (`(idx * 2) + (unsigned int)a0`) so cc1 emits retail's `addu v1,v1,a0`
  rather than `addu v1,a0,v1`.
