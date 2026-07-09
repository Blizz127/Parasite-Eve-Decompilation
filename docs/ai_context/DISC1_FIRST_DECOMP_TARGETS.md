# Disc 1 First Decomp Targets Triage (Phase 4C / Phase 5B–5L update)

Conservative shortlist of safest initial candidates for C conversion.
Originally docs-only (Phase 4C).
**Phase 5B–5L:** nine production C leaves (exact SHA-1 each).

## Repo state (Phase 5L)

- Branch: `phase5l-func-8008F868`
- Converted C: `src/func_8008F694.c`, `src/func_8008F868.c`,
  `src/func_80090A0C.c`, `src/func_80090C38.c`, `src/func_80090C4C.c`,
  `src/func_80090C60.c`, `src/func_80090C74.c`, `src/func_80090F54.c`,
  `src/func_800C2B40.c`
- Production split map:
  ```
  [0x800,     rodata]  prefix jump tables + strings
  [0x2A0C,    asm]     main text through func_8008F690 area
  [0x7FE94,   c, func_8008F694]  VRAM 0x8008F694, size 0x14
  [0x7FEA8,   asm]     resume through func_8008F860 area
  [0x80068,   c, func_8008F868]  VRAM 0x8008F868, size 0x18
  [0x80080,   asm]     resume through func_800909C0 / 80090xxx cluster
  [0x8120C,   c, func_80090A0C]  VRAM 0x80090A0C, size 0x14
  [0x81220,   asm]     resume through func_80090BCC
  [0x81438,   c, func_80090C38]  VRAM 0x80090C38, size 0x14
  [0x8144C,   c, func_80090C4C]  VRAM 0x80090C4C, size 0x14
  [0x81460,   c, func_80090C60]  VRAM 0x80090C60, size 0x14
  [0x81474,   c, func_80090C74]  VRAM 0x80090C74, size 0x14
  [0x81488,   asm]     through func_80090E20
  [0x81754,   c, func_80090F54]  VRAM 0x80090F54, size 0x14
  [0x81768,   asm]     resume through func_80091080
  [0x818A0,   rodata]  mid-image data island
  [0xB2AF8,   asm]     tail code from func_800C22F8
  [0xB3340,   c, func_800C2B40]  VRAM 0x800C2B40, size 0x10
  [0xB3350,   asm]
  ```
- Oracle: `scripts/build_us.sh` exit 0, SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- C flags (Phase 4J): `-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls -ffreestanding -fno-builtin -O1`
- C object pad: GCC emits `.text` size 0x20; trim to 0x14 (align-16 zeros only)
- Generated asm/linker/objects: git-ignored, never committed

## Sources of truth consulted
- `docs/ai_context/ACTIVE_HANDOFF.md`
- `docs/ai_context/DISC1_FUNCTION_INVENTORY.md`
- `docs/ai_context/DISC1_CALL_ANCHOR_MAP.md`

## Selection methodology
- Used `glabel` / `endlabel` + "nonmatching ... size" hints for boundaries and size.
- Scanned for `jal` absence (leaf), small size (mostly <0x60 bytes), straight-line or minimal branches.
- Cross-checked call frequencies from Phase 4B map (avoided high-count like func_8005E8A4 ~247×).
- Inspected bodies for: no jtbl, no complex loops with side effects, minimal D_/global refs, no obvious CD/GPU etc. patterns in sampled asm.
- VRAM/file offsets from asm comments + 0x800 + (VRAM-0x80010000).
- Explicitly avoided all anchors and boot-critical areas.

## Candidate table (safest first targets)

| Label | VRAM | Region | Size | Callers (direct approx) | Callees | Leaf? | Globals/rodata | CF complexity | Risk notes | Confidence | Suitability as first C target |
|-------|------|--------|------|-------------------------|---------|-------|----------------|---------------|------------|------------|-------------------------------|
| func_8008F694 | 0x8008F694 | **C: src/func_8008F694.c** | 0x14 | low (0 direct) | 0 | yes | none | none | `*(unsigned int *)arg0 += 2` | high | **DONE Phase 5K — exact match** |
| func_8008F868 | 0x8008F868 | **C: src/func_8008F868.c** | 0x18 | low | 0 | yes | none | none | `*(u16 *)(arg0+0x7C) = (*f + 1) & 0xF` | high | **DONE Phase 5L — exact match** |
| func_80090A0C | 0x80090A0C | **C: src/func_80090A0C.c** | 0x14 | low (0 direct) | 0 | yes | none (arg + immediate) | none | Bit clear 0x8 on field @0x38(a0) | high | **DONE Phase 5J — exact match** |
| func_80090C38 | 0x80090C38 | **C: src/func_80090C38.c** | 0x14 (~5 instr) | low (0 direct seen) | 0 | yes | none (arg + immediate const) | none (linear) | Bit set on struct field @0x38(a0) | high | **DONE Phase 5B — exact match** |
| func_80090C4C | 0x80090C4C | **C: src/func_80090C4C.c** | 0x14 | low (0 direct) | 0 | yes | none (arg + immediate) | none | Bit clear mask on same field | high | **DONE Phase 5C — exact match** |
| func_80090C60 | 0x80090C60 | **C: src/func_80090C60.c** | 0x14 | low | 0 | yes | none | none | Bit set (0x20 variant) | high | **DONE Phase 5E — exact match** |
| func_80090C74 | 0x80090C74 | **C: src/func_80090C74.c** | 0x14 | low | 0 | yes | none | none | Bit clear variant | high | **DONE Phase 5F — exact match** |
| func_80090F54 | 0x80090F54 | **C: src/func_80090F54.c** | 0x14 | low (0 direct) | 0 | yes | none (arg + 0x100000 const) | none | Bit set 0x100000 on field | high | **DONE Phase 5D — exact match** |
| func_800C2B40 | 0x800C2B40 | **C: src/func_800C2B40.c** | 0x10 | low-medium (~ few) | 0 | yes | 1 (D_800E2248 table) | none | Load global table, sw field, return | medium | **DONE Phase 5G — exact match** |
| func_800C2B10 | 0x800C2B10 | B2AF8.s | 0x18 | ~15 (tail) | 0 | yes | 1 (D_800E2248) | none | Array index calc + return ptr | medium | **BLOCKED Phase 5H** — GCC 14.2 schedule mismatch (see ACTIVE_HANDOFF) |
| func_800C7DC4 | 0x800C7DC4 | B3350.s | 0x10 | low | 0 | yes | none | none | Store 4 to *arg0; return 0 | medium | **BLOCKED Phase 5I** — delay-slot `addu` vs `move` |

(These 8 are the strongest matches from scans. The 90Cxx/90F54 cluster stands out as near-ideal due to zero side effects visible, pure leaf nature, and location away from anchors/boot.)

**Direct Evidence Verification (pre-commit, manual)**

Note: A prior Python scan for candidates had incomplete/truncated output (due to command escaping/syntax warning in the tool invocation; output was cut off). The final shortlist was **manually verified** by direct extraction from the generated asm files using awk/grep on `asm/disc1/2A0C.s` and `asm/disc1/B2AF8.s`. All data below is from raw disassembly comments, glabel/endlabel, and instruction counts. No automated scan trusted for final list.

Per-candidate direct evidence:

**func_80090C38**
- label: func_80090C38
- VRAM: 0x80090C38 (from asm comment /* 81438 80090C38 */ + glabel)
- asm source: asm/disc1/2A0C.s (late main text, near end before rodata island)
- size: 0x14 bytes (5 instructions: lw $v0,0x38($a0); nop; ori $v0,$v0,0x10; jr $ra; sw $v0,0x38($a0))
- caller count: 0 (grep -c "jal.*func_80090C38" across 2A0C.s + B2AF8.s = 0)
- callee count: 0 (no "jal " inside body)
- leaf: yes
- globals/rodata references: none (only $a0 arg + immediate 0x10; no D_ labels)
- jump tables or indirect calls: no (0 jtbl/jalr in body; no cross-glabel jumps)
- low risk: tiniest possible; pure leaf with straight-line bit-set on struct field; no calls, no branches, no init, no hardware visible; far from anchors (1220C/72534/C22F8) and boot; 0 direct callers.
- might still be risky: field write may have larger context side-effects unknown without decomp; could be called indirectly via pointer (not captured by direct jal); location in "unknown" late-main code; struct type unknown.

**func_80090C4C**
- label: func_80090C4C
- VRAM: 0x80090C4C
- asm: asm/disc1/2A0C.s
- size: 0x14 bytes (5 instr: lw; addiu -0x11; and; jr; sw)
- caller count: 0
- callee count: 0
- leaf: yes
- globals/rodata: none (arg + immediate mask)
- jtbl/indirect: no
- low risk: identical pattern to above (bit clear); same benefits.
- risky: same as above (context, indirect calls possible).

**func_80090C60**
- label: func_80090C60
- VRAM: 0x80090C60
- asm: asm/disc1/2A0C.s
- size: 0x14 bytes (5 instr: lw; ori 0x20; jr; sw)
- caller: 0
- callee: 0
- leaf: yes
- globals: none
- jtbl/indirect: no
- low risk / risky: same cluster analysis.

**func_80090C74**
- label: func_80090C74
- VRAM: 0x80090C74
- asm: asm/disc1/2A0C.s
- size: 0x14 bytes (5 instr: lw; addiu -0x21; and; jr; sw)
- caller: 0
- callee: 0
- leaf: yes
- globals: none
- jtbl/indirect: no
- low risk / risky: same.

**func_80090F54**
- label: func_80090F54
- VRAM: 0x80090F54 (/* 81754 80090F54 */)
- asm: asm/disc1/2A0C.s (late, just before rodata at ~818A0)
- size: 0x14 bytes (5 instr: lw $v0,0x38($a0); lui $v1,0x1000; or; jr; sw)
- caller: 0
- callee: 0
- leaf: yes
- globals/rodata: none (immediate lui 0x100000 const, no D_)
- jtbl/indirect: no
- low risk: same as 90C cluster + slightly later position; explicit const or for bit 4.
- risky: same caveats (unknown larger context for the field write).

**func_800C2B40**
- label: func_800C2B40
- VRAM: 0x800C2B40 (/* B3340 800C2B40 */)
- asm: asm/disc1/B2AF8.s (early tail)
- size: ~0x10 bytes (4 instr: lui D_800E2248; lw; jr; sw $a0 to offset)
- caller count: 0 (direct jal)
- callee count: 0
- leaf: yes
- globals/rodata: 1 (D_800E2248)
- jtbl/indirect: no
- low risk: extremely small leaf setter; simple load-modify-store on global table entry.
- risky: touches global table (D_800E2248, unknown purpose); may be part of data structure init in tail code; indirect calls possible; repeated similar accessors in tail may indicate system code.

**func_800C2B10**
- label: func_800C2B10
- VRAM: 0x800C2B10
- asm: asm/disc1/B2AF8.s
- size: 0x18 bytes (6 instr: sll; lui D_; lw; addiu; jr; addu)
- caller: 0 (direct; earlier scans noted ~15 for similar but direct grep 0 for this exact in split files)
- callee: 0
- leaf: yes
- globals: 1 (D_800E2248)
- jtbl/indirect: no
- low risk: small leaf index/offset calc + return; no control flow branches.
- risky: array-like access into global table; potential for larger data structure role in tail; same indirect-call caveat.

All bodies manually extracted and inspected. No jtbl, no jalr, no internal jal in any. No direct jal from anchors (1220C/72534/C22F8) to these.

## Rejected near-miss table (examples of what was filtered)

| Label | Why considered | Why rejected | Notes |
|-------|----------------|--------------|-------|
| func_800124F8 | Smallish (0x7C), early | Loops (bnez/sltiu), heavy global zeroing (D_8009D310, gp vars, D_8009DF70), init behavior | Violates "no complex global initialization" |
| func_80090E20 | Near the good cluster, some size | Has stack frame, branches (bnez), multiple loads, more complex flow | Not "simple control flow" |
| func_80091080 | Very small (0x20), late in 2A0C | Calls another func (90F68), boundary-adjacent | Not leaf, near data island transition |
| func_8005E8A4 (and similar high-freq) | Present in main | Called 247× (from call map); likely common util | "high-frequency common routines" rule |
| func_800725DC (near pc0) | Small, called from anchor | Proximity to pc0/startup, potential boot logic | Avoid boot/control-flow-critical |
| func_800C2DA0 | Small in tail | Not leaf (more instr, lbu, addiu), refs multiple D_ | More complex than pure leaves |
| Any with jtbl or jal inside | - | Direct violation of leaf/no-jtbl/no-indirect | Many such |

## First targets — DONE (Phase 5B–5F)

**func_80090C38** — `src/func_80090C38.c` (Phase 5B, PR #9):

```c
void func_80090C38(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) |= 0x10u;
}
```

**func_80090C4C** — `src/func_80090C4C.c` (Phase 5C, PR #10):

```c
void func_80090C4C(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) &= ~0x10u;
}
```

**func_80090F54** — `src/func_80090F54.c` (Phase 5D, PR #12):

```c
void func_80090F54(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) |= 0x100000u;
}
```

**func_80090C60** — `src/func_80090C60.c` (Phase 5E, PR #13):

```c
void func_80090C60(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) |= 0x20u;
}
```

**func_80090C74** — `src/func_80090C74.c` (Phase 5F):

```c
void func_80090C74(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) &= ~0x20u;
}
```

All exact SHA-1 via `scripts/build_us.sh`. No semantic struct/field names yet.

## Recommended next target

**Parked clusters:**
- `D_800E2248` accessors (`func_800C2B10`, `func_800C2B28`) — Phase 5H schedule
- `func_800C7DC4` byte-store/return-0 pattern (+ duplicates `func_800C8F08`,
  `func_800C9C00`) — Phase 5I delay-slot `addu` vs `move`

**Next:** pick a different small GCC-friendly leaf outside both parked clusters.
Probe must match in **production rebuild**, not scratch-only. One function only;
exact SHA-1 or stop. See `ACTIVE_HANDOFF.md` Phase 5H / 5I blockers.

## Exact next-step instructions for future Phase 5 first C conversion

1. Ensure Phase 4 harness (rebuild + checksum verify against original SLUS) is in place or implemented first.
2. Pick one (start with func_80090C38).
3. In a new branch (e.g. `phase5-decomp-80090C38`), manually recreate matching asm or use the existing split.
4. Write minimal C equivalent (likely `void func_80090C38(SomeStruct *s) { s->field |= 0x10; }` or similar; determine struct from context/cross-refs).
5. Use correct compiler/flags (Psy-Q era gcc, specific opts) to match bytes.
6. Verify via `scripts/verify_us.sh` (or equivalent) that only this function changes and overall binary matches.
7. Do not touch other functions or boundaries in the same pass.
8. Document the match evidence (size, prologue/epilogue, exact bytes) in the target doc or handoff.
9. Only after one successful match, consider the next sibling (they form a natural set).

All evidence for this triage came from:
- Generated asm comments (addresses, instructions).
- `glabel`/`endlabel` + size hints.
- Direct `jal` counts from Phase 4B map + asm scans.
- Body inspection for control flow and memory ops.
- Strict avoidance of listed categories.

No decompilation or C was performed. This is pure triage/survey.

(End of Phase 4C triage.)

---

## Phase 5 First C Conversion Attempt (2026-07-08)

**Branch:** phase5-disc1-first-c-leaf (created after merge simulation of Phase 4C)

**Recommended target selected:** func_80090C38 (per this document)

**Pre-edit verification performed:**
- Branch: phase5-disc1-first-c-leaf, clean working tree
- Config boundaries confirmed unchanged (0x800 rodata, 0x2A0C asm, 0x818A0 rodata, 0xB2AF8 asm)
- `scripts/split_us.sh --check` passed
- Re-inspected generated asm/disc1/2A0C.s for func_80090C38:
  - Function label: present (`glabel func_80090C38`)
  - Size: 0x14 bytes (nonmatching hint), 5 instructions
  - Leaf: yes (no `jal` inside body; ends with `jr $ra`)
  - Jump table: no
  - Indirect call: no (no `jalr`)
  - Dangerous hardware/system side effects: no (simple `lw` from $a0 + `ori` const + `sw` back; no CD/GPU/SPU/MDEC/BIOS/interrupt patterns)
  - Anchor role: no (not 1220C, 72534, or C22F8)

**Decision:** Do **not** perform C conversion.

**Missing infrastructure (exact blocker):**
- `scripts/verify_us.sh`: is a placeholder that prints "ERROR: verify_us.sh is a Phase 0 placeholder and not implemented yet." and exits 1. No rebuild + checksum harness.
- `src/main/`: only contains an empty `.gitkeep`. No C source files, no types, no scaffolding.
- `configs/USA/disc1.yaml`: only disassembly segments; `compiler: GCC` is noted as boilerplate. No C subsegments or src integration.
- No top-level build system (no Makefile for PSX EXE from mixed asm + C objects, no object comparison target).
- No documented way to compile a single C function into the EXE while preserving the rest of the split.

Per explicit rules and CLAUDE.md ("Until that harness exists, do not add code under `src/` at all"), no C file was created or modified.

**Result:** Phase 5 blocked. One function only (this attempt). No other targets touched.

**Recommended action:** Implement Phase 4 harness (verify_us.sh + build integration) before any C work. Then resume on this target.

(End of Phase 5 blocker report.)

---

## Phase 4J — MIPS GCC provisioned; leaf codegen OK (2026-07-08)

**Branch:** `phase4j-mipsel-gcc-provisioning`

### Provisioning (container only)

- Distrobox `pe-mipsel` (Debian trixie): `apt install -y gcc-mipsel-linux-gnu`
- `mipsel-linux-gnu-gcc (Debian 14.2.0-13) 14.2.0` — package `4:14.2.0-1`
- Host PATH: still no mipsel gcc

### Codegen probe (scratch `/tmp` only — not in `src/`)

```c
void func_80090C38(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) |= 0x10;
}
```

Flags that work (`-mips1` **requires** `-mfp32`):

```text
-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls \
-ffreestanding -fno-builtin -O1   # also -O2 -O3 -Os — same 5 words
```

At **-O1 and above**, GCC emits the **exact** original 0x14 bytes:

```text
3800828c 00000000 10004234 0800e003 380082ac
```

| Check | Result |
| --- | --- |
| Load-delay nop | yes |
| `sw` in `jr` delay slot | yes |
| Prologue at -O1+ | none |
| gp/pic/abicalls | none |
| ELF32 mipsel R3000 | yes |
| Object `.text` size | 0x20 (align-16 pad; first 0x14 exact) |

**Not production-integrated.** Next: Phase **5B** — one function in `build_us.sh` + splat.

(End of Phase 4J.)
