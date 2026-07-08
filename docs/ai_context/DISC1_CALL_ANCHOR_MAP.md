# Disc 1 Call/Anchor Map (Phase 4B)

Conservative, read-only map of high-confidence direct function calls and relationships in the Disc 1 text regions. Built strictly from generated disassembly evidence before any decompilation or symbol work.

This augments the Phase 4 function inventory. All claims are evidence-based only.

## Repo state

- Branch: `phase4-disc1-function-inventory`
- HEAD: 0a31fb1 (includes committed Disc 1 function inventory)
- Working tree: clean
- Confirmed split map (from config and prior audits):
  ```
  [0x800,     rodata]  prefix jump tables + strings
  [0x2A0C,    asm]     main text from func_8001220C
  [0x818A0,   rodata]  mid-image data island
  [0xB2AF8,   asm]     tail code from func_800C22F8
  ```
- `scripts/split_us.sh --check`: passes (dry-run, prerequisites and gitignore OK; no split run)
- Generated output status: `asm/disc1/2A0C.s` and `asm/disc1/B2AF8.s` (plus data/*.rodata.s) present locally, covered by `.gitignore`, **never committed**. Reproducible from `build/extracted/disc1/SLUS_006.62` (verified SHA-1).

## Sources of truth used
- `docs/ai_context/ACTIVE_HANDOFF.md`
- `docs/ai_context/DISC1_FUNCTION_INVENTORY.md`
- Raw bytes from `build/extracted/disc1/SLUS_006.62`
- Generated `asm/disc1/2A0C.s` (~2007 funcs) and `asm/disc1/B2AF8.s` (~352 funcs) — evidence only

## Known anchors (from inventory)
- **func_8001220C** (VRAM 0x8001220C, file ~0x2A0C): Main text start. Standard prologue. Many direct outgoing calls.
- **func_80072534** (VRAM 0x80072534, file 0x62D34): pc0 / startup (handwritten). Calls back into main text entry. Classic crt0 pattern.
- **func_800C22F8** (VRAM 0x800C22F8, file 0xB2AF8): Tail code resume. Multiple direct callers from within tail region.

## High-confidence direct-call table

Limited to explicit `jal func_xxxxxxxx` where both caller and callee have clear `glabel` in the text asm files. VRAM from label names and instruction comments. Call sites from disassembly comments. Confidence high only for direct, unambiguous cases inside the split text segments.

| Caller | Caller VRAM | Callee | Callee VRAM | Call site (VRAM) | Evidence | Confidence | Notes |
|--------|-------------|--------|-------------|------------------|----------|------------|-------|
| func_80072534 | 0x80072534 | func_8001220C | 0x8001220C | 0x800725D0 | `jal` inside pc0 body (after zero-fill + setup); explicit in 2A0C.s | high | Startup calls main entry point. |
| func_8001220C | 0x8001220C | func_800725DC | 0x800725DC | 0x80012224 | First `jal` in main text entry; site immediately after prologue | high | Early call to nearby startup-adjacent func. |
| func_8001220C | 0x8001220C | func_8003E610 | 0x8003E610 | 0x8001222C | Direct `jal` in 1220C body | high | Common early call (PsyQ-style naming in asm) |
| func_8001220C | 0x8001220C | func_8006A5BC | 0x8006A5BC | 0x8001224C | Direct `jal` | high | Representative of many outgoing from entry |
| func_800C7BA0 | 0x800C7BA0 | func_800C22F8 | 0x800C22F8 | 0x800C7BA8 | `jal` at start of caller (post-prologue) | high | Tail region caller of resume anchor |
| func_800C8D34 | 0x800C8D34 | func_800C22F8 | 0x800C22F8 | 0x800C8D3C | Explicit `jal` | high | Repeated pattern in tail |
| func_800C9A70 | 0x800C9A70 | func_800C22F8 | 0x800C22F8 | 0x800C9A78 | Explicit `jal` | high | Tail caller of anchor |
| func_800CA574 | 0x800CA574 | func_800C22F8 | 0x800C22F8 | 0x800CA57C | Explicit `jal` | high | Multiple similar in tail asm |
| func_800CBCA4 | 0x800CBCA4 | func_800C22F8 | 0x800C22F8 | 0x800CBCB0 | Explicit `jal` | high | Further tail caller |

(Additional direct calls from func_8001220C exist to ~15+ other funcs in the first ~200 instructions, e.g. func_8006A64C, func_8006AD40, func_8006ECEC, etc. Full enumeration omitted for conservatism; pattern is consistent direct jal to labeled funcs.)

## Anchor relationship summary

### func_8001220C (main text start)
- Outgoing: Direct jal to many functions (func_800725DC first, then numerous others including 0x8003E6xx / 0x8006Axxx range).
- Incoming: Explicitly called from pc0 (func_80072534).
- Role: Primary entry after crt0. High volume of early calls suggests initialization / dispatch.

### func_80072534 (pc0)
- Outgoing: One clear direct jal to func_8001220C (plus setup code and break).
- Incoming: Limited direct evidence in these segments (startup entry point).
- Role: Handwritten crt0-style. Zero-fills runtime area, sets up, transfers to main text. Note: BSS symbols are runtime, not ROM.

### func_800C22F8 (tail resume)
- Outgoing: Limited in its small body (size ~0x11C); calls into other tail funcs.
- Incoming: Multiple high-confidence direct jals from later tail functions (e.g. func_800C7BA0, func_800C8D34, func_800C9A70, func_800CA574, func_800CBCA4, and more visible in B2AF8.s).
- Role: Entry point for tail code region. Appears to be a common entry or service routine.

## Suspected clusters (no renaming)

Frequent direct callees (high call counts) suggest common routines or libraries. Listed by auto label only; no new names or behavior inference:

**In 2A0C.s (main text):**
- func_8005E8A4 (called ~247 times)
- func_80062A34 (~210)
- func_80062F3C (~149)
- func_8005EB64 (~113)
- func_80062CB8 (~94)
- func_800527C0 (~93)
- Others in 0x80062xxx and 0x8005D/5E range appear frequently.

**In B2AF8.s (tail):**
- func_80071A54 (called ~179 times)
- func_80077CF4 (~61)
- func_80077DC4 (~56)
- etc.

These are noted as "frequently called" for future reference. May correspond to system, graphics, memory, or game core services. Cross-references via rodata tables at 0x818A0 likely.

## Ambiguous / indirect / risky notes

- **No jalr with provable targets** observed in quick scans of text regions (jalr would be register-based indirect).
- Branch targets (beq/bne/j etc.) used for local control flow inside functions; not recorded as cross-function calls unless they cross glabel boundaries (rare in this data).
- Some jal targets have VRAM > 0x800Exxxx or in 0x8019xxxx range (visible in disassembly). These may be:
  - Overlay data loaded later.
  - Pointers from rodata tables resolved at runtime.
  - Noise from data-as-code in early splits.
  - Recorded only when the jal appears literally inside a labeled text function.
- Calls may go through the mid-image rodata island (func pointers at 0x800910A0+). These are **not** direct jal and are out of scope for this conservative direct-call map.
- No evidence of self-modifying code or unusual jalr patterns in the sampled regions.
- Unknown large regions: dense jal areas between labels may contain inlining or unlabelled subroutines (medium confidence on boundaries).
- Tail region has clear "service" pattern with repeated calls back to the 0x800C22F8 anchor.

All entries above are direct `jal <immediate label>` only.

## Recommended next step

- Use this call/anchor map + the function inventory as the survey before any code work.
- Only add more relationships or manual labels on true need (e.g., during matching or when a specific function must be understood).
- Future work can cross-reference these frequent callees against Psy-Q SDK or game-specific strings.
- Push/PR phase4 work only after useful docs are added.
- Do not proceed to C, symbol naming, or PC port until inventory + call map are stable and Phase 4 verify harness exists.

Evidence strictly limited to:
- Explicit jal instructions in the generated asm text files.
- glabel / endlabel boundaries.
- Instruction comment addresses (VRAM + file hints).
- Prior verified boundaries and anchors.

No decompilation, no assumptions about purpose beyond the disassembly text. 

(End of Phase 4B initial map.)
