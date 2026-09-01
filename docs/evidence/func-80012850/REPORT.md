# func_80012850 — script VM step (24-opcode interpreter)

## Target

VRAM `0x80012850..0x80012C20` (exclusive), file `0x3050`, size `0x3D0`
(244 words). Frame `0x18` saving `$s0`/`$ra`; calls `func_8003708C` /
`func_800370A8`. Fetches `*ctx->pc`, range-checks `< 0x18`, dispatches
through `jtbl_80010000` (ascending order confirmed from rodata dump),
stores one result through `*ctx->dst`, returns 1.

Opcode map recovered: 0 ADD, 1 SUB, 2 OR, 3 AND, 4 XOR, 5 LOR,
6 LAND, 7 LNOT, 8 BNOT, 9 GT, 10 LT, 11 EQ, 12 GE, 13 LE, 14 NE,
15 MUL, 16 DIV, 17 SHL, 18 SHR(sra), 19 ASSIGN, 20 CALL1, 21 CALL2,
22 MOD, 23 NEG. Values are signed int (slt / srav / mult).

## First attempt — size gate failure (missing div guards)

One full switch-statement candidate on era `-O2 -G8` produced `0x390`
bytes against the required `0x3D0`:

```
ERROR: target size 0x3D0 > current 0x390
```

Exact cause established by disassembly + direct cc1 probe: era cc1 emits
a bare 3-operand `div $d,$s,$t` pseudo for both `/` (op 16) and `%`
(op 22) with NO guard sequence, and GNU as under the repo's fixed
ASFLAGS expands it without checks. Retail carries the full Psy-Q-style
guard sequences twice (`bnez divisor → ok; break 7` zero-check +
`li $at,-1; bne; lui $at,0x8000; bne; break 6` INT_MIN/-1 overflow
check) — 2 × ~8 words = exactly the missing 16 words. Everything else
(dispatch, case set/order, calls, tails) matched shape.

## Parked — new mismatch class

This is a NEW mismatch class: the pipeline has no mechanism yet to
produce the ASPSX division-guard expansion (would require a maspsx
div-macro patch or an assembler-flag change; tooling changes are out of
scope for this run). Any future leaf containing integer `/` or `%` will
hit the same wall until that exists.

Candidate source kept uncommitted for review (`src/func_80012850.c`,
semantics believed complete); registration reverted so the exact
278-leaf baseline remains. No commit was made.

## Tooling resolution (2026-08-22, tooling/maspsx-expand-div branch)

The div-guard blocker is RESOLVED without a fourth maspsx patch: upstream
maspsx already carries the full ASPSX signed div/rem guard expansion behind
`--expand-div` (`expand_div`), and at our `--aspsx-version=2.21`
configuration (`div_uses_tge=False`) its emitted sequence matches retail
instruction-for-instruction, including GNU-as one-arg `break 0x7` /
`break 0x6` encoding to retail's exact words (`0x0007000d` / `0x0006000d`,
verified empirically in-container). `scripts/build_us.sh` now exposes it as
a per-leaf env gate: `MASPSX_EXPAND_DIV=1 era_compile ...`. Flag-off
behavior is unchanged (full-tree rebuild EXACT SHA-1 with the gate unset).

End-to-end validation through this leaf partially succeeded: with the gate
ON the size gate passed at exactly `0x3D0` (the previous −16-word shortfall
closed precisely), both guard sequences appear byte-exact in the object,
and all 24 case bodies plus shared tails match retail.

## Second blocker discovered — switch-table ownership (1F814 class)

The rebuild then failed NON-MATCH on 690 bytes with exactly two divergences:

1. **Dispatch addressing (+1 word):** cc1 materializes its own local switch
   table as `lui/addiu/addu/lw` (4 words); retail uses the 3-word
   `lui $at,%hi / addu / lw %lo($at)` indexed-symbol form against the
   shared-pool table. Source-phrasing cannot change how gcc addresses a
   table it emits itself.
2. **Missing tail `nop`:** candidate epilogue omits the delay-slot `nop`
   after `jr ra` (−1 word, coincidentally balancing the dispatch +1 so the
   size gate passed).

Both stem from the jump table living inside the C object instead of the
shared rodata pool — the same class that keeps `func_8008001F814` parked
("jump table in the 0x800 rodata pool"). Closing it needs a decision on
table ownership/placement (e.g., maspsx rewrite of local-table dispatch
addressing + ld/trim surgery for the duplicate table), which is exactly
the kind of matching-semantics-affecting tooling change that requires its
own reviewed design.

Related tooling shipped on this branch regardless: the split-generated
rodata's `.word .L<addr>` references (1388 sites) are absolutized at build
time — byte-identical by construction (label value = embedded VRAM address,
guarded to plausible range; `.L00000000_main` forms untouched) and proven
so by the flag-off EXACT rebuild. This removes the link-dependency of pool
tables on carved-out asm labels, a prerequisite for ANY future carve of a
jtbl-using leaf.

## RESOLVED EXACT (2026-08-22, tooling/maspsx-expand-div)

The switch-table design was approved and implemented; this leaf became
the E2E acceptance gate for the whole stack:

- **Div guards:** `MASPSX_EXPAND_DIV=1` (upstream maspsx expansion;
  byte-exact vs retail incl. break encodings).
- **Dispatch shape (Axis A):** `MASPSX_THREE_WORD_SYMBOL_STORE=1`
  collapses cc1's compound table load to the 3-word ASPSX form, and new
  gate `MASPSX_DISPATCH_FOLD=jtbl_80010000` retargets the local `$L30`
  label to the shared pool symbol (`maspsx` commit "DISPATCH_FOLD",
  9 tests incl. negatives).
- **Table dedup (Axis B):** build-side step proves the object's
  `.rodata` is a pure self-text address array matching the pool
  literal sequence (size, word coverage, R_MIPS_32 type, defined .text
  targets), then strips the duplicate section AND its relocation
  section. Loud failure on any mismatch. (First attempt compared raw
  bytes — impossible pre-link since entries are relocations; structural
  proof replaces byte comparison. Two ELF bugs found and fixed during
  bring-up: missing e_shstrndx unpack; sh_link at Shdr+24 not +6.
  A first strip variant leaving `.rel.rodata` dangling segfaulted GNU ld;
  the relocation section is now zeroed together with the table.)

Result: size gate exactly `0x3D0`; fresh Docker build **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; `verify_us.sh` EXACT MATCH;
**279 leaves**. The gas reorder-fill delay-slot nop lands back inside
the trim window automatically once the dispatch shift is removed
(design doc audit addendum). Candidate committed on
`tooling/maspsx-expand-div`.
