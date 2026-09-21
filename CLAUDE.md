# CLAUDE.md — AI agent guidance for this repository

This is a PS1 decompilation research project for **Parasite Eve** (USA,
NTSC-U: SLUS-00662 / SLUS-00668). Matching decomp is the first authority;
`pc_port/` is the in-tree native / battle-runtime research implementation.
Read `docs/project_plan.md` for the phase roadmap and
`docs/ai_context/ACTIVE_HANDOFF.md` for the current working state **before
doing anything**.

## Hard rules

1. **Never invent decompiled C.** Only add C that is verified against the
   original binary by the checksum/rebuild harness (Phase 4+). Until that
   harness exists, do not add code under `src/` at all.
2. **Never claim matching progress** unless verified by checksum/build
   tooling, with the exact command recorded.
3. **Never commit game data**: no ISO/BIN/CUE/CHD images, no extracted files,
   no assets, no proprietary Psy-Q/PsyQ SDK files. `rom/image/`, `assets/`,
   `asm/`, and `build/` are git-ignored on purpose — keep them that way.
4. **Every claim needs evidence**: a command, checksum, symbol map,
   disassembly excerpt, or documented observation. Record it in `docs/`.
5. **Every phase must be reproducible** via scripts in `scripts/` and
   configs in `configs/`. User-supplied disc images live under `rom/image/`
   locally and are inputs, never outputs.

## Working conventions

- Update `docs/ai_context/ACTIVE_HANDOFF.md` after every meaningful change:
  what was done, what was verified, what the next concrete step is.
- Prefer small commits with exact, descriptive messages.
- The Parasite Eve 2 decomp (GabeRealB/parasite-eve-2-decomp) is a
  **structural reference only** — study its layout and tooling flow, but do
  not copy source or configs blindly; PE1 is a different binary.
- Scripts must be idempotent and fail loudly (`set -euo pipefail`).
- Python tooling goes in `tools/`; keep it dependency-light and pinned.

## Current phase

**752 matching C leaves** — exact count via
**785 matching C leaves** — exact count via
`python3 tools/build/disc1_plan.py --check` (prints
`1133 spans (785 c, 346 asm, 2 rodata)`) or equivalently
`grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml`. This count is the
authoritative number; re-derive it, do not trust prose here.

**Newest (session cont. 23): 33 small leaves (752 → 785).** Carved from former
pure-`asm` residue across `0x80015xxx`–`0x8008Fxxx`: `func_80019CEC`,
`func_80017E68`, `func_80021054`, `func_800192DC`, `func_8004BC80`,
`func_80017DE4`, `func_800198C4`, `func_8008F178`, `func_8007DD74`,
`func_800828F4`, `func_80015AB8`/`func_80018B30`/`func_800193D8`,
`func_8004EC3C`/`func_8004EC78`/`func_8004FD68`/`func_800501C8`/
`func_800509A8`, `func_80080F64`, `func_80050308`, `func_80019260`,
`func_8001A43C`/`func_8001A474`, `func_8007DFE0`, `func_8007E0C0`,
`func_8007DE40`, `func_8004BCB4`, `func_8004F7D8`, `func_800504BC`,
`func_8008E7F4`, `func_80019450`, `func_80083790`, `func_8005022C`. **New
durable levers:** (1) **source statement order for independent stores is
load-bearing** — `func_80019CEC`'s `w28, w2C, w30` order reproduces retail's
`lh`/`sw` interleave (plain permutation, no aliasing; 4 mismatches otherwise);
(2) **branch polarity for an `if/else` call dispatch** — `func_80050308`
matches only with the *nonzero* case written first; (3) **`-G8` is required for
any leaf reading a `$gp` slot** (gp base `0x8009CD70`: `0x184($gp)` =
`D_8009CEF4`, `0x1A4($gp)` = `D_8009CF14`; a wrong symbol name emits the right
shape with the wrong displacement, so always convert `gp_off + 0x8009CD70`);
(4) **maspsx patch 3** closed `func_8007DD74` and `func_800828F4`; (5)
**unsigned comparison selects `sltu`** (`func_8008E7F4`); (6) **the element
type of a doubly-dereferenced parameter controls `lw` vs `lbu`**
(`func_80019450` needs `unsigned int **`, not `unsigned char *`); (7) **two
independent `$gp` stores are order-sensitive** (`func_8005022C`). New
`era_o2_g8` assignments: `func_80021054`, `func_80050308`, `func_800501C8`,
`func_800509A8`, `func_8004F7D8`; new `era_o2_g0_fill_epilogue_delay_slot`:
`func_8007DD74`, `func_800828F4`. Function-pointer wrappers are declared
**argument-less** where retail clears no argument registers.

**Prior (session cont. 22): `func_80067CBC` matched (751 → 752) + three parks.**
`func_80067CBC` (`0x584BC`, `0x5C`, fan-in 11) is a `D_800BCF88` status-word
flag setter, default `-O2 -G0`, `LINK_EXACT`. **Durable lever — dual pinned
pointer bases:** retail keeps the head base in `$a1` but *rematerializes a fresh
base* in `$v1` for the tail block; two distinct pointer locals pinned
`asm("$5")`/`asm("$3")` reproduce both (mismatch ladder: bare 23 → single
pointer 14 → single pinned 7 → two pinned 0). Three honest parks with full
evidenced lever lists: `func_8003708C` (7w, 64-bit multiply `mflo`/`mfhi` order
+ `$v0`/`$v1` alloc), `func_80073A44` (94w VSync; yielded the **pointer-to-
volatile defeats read-loop CSE** lever), `func_800374E8` (24w, symbol
rematerialization vs. loop-invariant base hoist).

**Newest (session cont. 21): eleven small no-call leaves carved from former
pure `asm` residue, all `LINK_EXACT` — 739 → 751 — plus the reopened
`func_8005DB44` park.**
The marquee result is `func_8005DB44` (`0x4E344`, `0x48`, fan-in 9), previously
PARKED as a "reassociation residual" and closed with a genuinely new structural
lever: **make a symbol pointer a local that is decremented in place**
(`int *p = &D_800A8038; … p -= 4; return start + (sh + (int)p);`) rather than an
address constant (`&D_800A8038 - 0x10`, which cc1 reassociates constants-first),
plus a **block-local `int sh = a0 << 5;` assigned before the decrement** so
retail's `sll $v0,$a0,5` lands in the `beq` delay slot; era `-O1 -G0`
(load-bearing — `-O2` flips the order back). Its adjacent twin `func_8005DAFC`
is a genuine `$v0`/`$v1` allocation residual and is honestly left in `asm`.
Other new leaves:
`func_80078C94` (3-word copy; new profile `era_o1_g0_no_delayed_branch` —
`-O1 -G0 -fno-delayed-branch`, because retail puts the return `addu` before an
**unfilled** `jr $31`), `func_80038CE4` (two-level indexed byte getter through
`D_80091A28`), `func_800515C0` (guarded `+0xC` short store through
`*D_8009D254`; a fresh inner-pointer local defeats cc1's double-load CSE),
`func_800528C4` (`D_800A76A4[a0*3] = a1*60` on patch 5), `func_80042EDC` /
`func_80019410` (new profiles `era_o2_g8_force_d800bd024_absolute` /
`..._d800bcfee_absolute`), `func_800524D0` (guarded bit-9 extractor),
`func_80057ED8` (signed-index clamped `short` getter; `-O2 -G8` + patch 5,
both load-bearing), and `func_8008594C`/`func_800858B0` (bounded
16-byte-stride table pair). A latent `deep-interior-return` defect on
`func_8005257C` (declared `0x214` span contained **7** interior `jr $ra`) was
trimmed to its true `0x18` (restored `- [0x42D94, asm]`).
**Durable
levers**: (1) **`-fno-delayed-branch` for an unfilled return slot** — the
return move sits *before* `jr $31` in retail; plain `-O1` fills the slot;
(2) **an out-of-range `.data` byte alongside in-range gp words** needs
`-G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=<byte>` (word destinations stay
gp-relative; `-G0` loses every gp store and `-G8` alone fails to link with
`relocation truncated to fit: R_MIPS_GPREL16`); (3) **a separate
`int u = a0 & 0xFFFF;` local for a bounded index** gives retail's unsigned
`slti` (folding the mask into the condition yields signed `slti`); (4) **pin
the index/base pair when retail inverts cc1's choice** (`func_800858B0` needs
`register int u asm("$3")` / `register unsigned char *base asm("$2")`; the
setter twin does not); (5) **an explicit inner-pointer local defeats a
double-load CSE** (`func_800515C0`). The **`deep-interior-return`** guard also
caught a *pre-existing* latent defect: `func_8005257C`'s declared `0x214` span
contained 7 interior `jr $ra` (swallowing six adjacent functions); it is
trimmed back to its true `0x18` with `- [0x42D94, asm]` restored. Per-leaf
reports `docs/evidence/func-80078C94/` … `docs/evidence/func-800858B0/`.

**Prior (session cont. 20): the `D_800BCD80` state-setter family (40 leaves)
plus 4 more — 695 → 739, all `LINK_EXACT`.**
`docs/evidence/D_800BCD80_SETTERS/REPORT.md` has the full per-leaf table.
Setter shape: store a command byte into `D_800BCD80`, 0–4 arguments into
`D_800BCD84`/`88`/`8C`/`90`, then `jal func_8008CBA8`. **Durable levers**:
(1) **a real `switch` beats an if/else chain** for a command-byte selector —
retail emits `li $v0,1` / `beq $a0,$v0` / `li $v0,2` / `beq $a0,$v0` with the
**default constant in the `j` delay slot**; (2) **`int *p = &D_800BCD80;` for a
two-call setter** keeps the base in callee-saved `$s1` across the first `jal`
instead of re-materializing `lui $at` per store; (3) **argument-store order may
invert from source order** (`func_80086CA4` masks `a2`/`a3` right after the
command byte and stores the `a0` argument last); (4) **call-poll `do`-while**
materializes the `1` once into `$s2` before the loop; (5) `do { } while (i < N)`
with post-increments in the body for the guarded byte-copy twins
(`func_80080950`/`80080998`); (6) early-return polarity + patch 5 for the
bound-checked getter `func_80056C14`. Only `func_80086608` (guarded variant)
stays `asm`. The deep-size guard caught a real gap mid-batch: the first carve of
`func_80086464` declared `0x68` but compiled `0x40`, and the `0x28` tail was the
unmatched `func_80086498`, then matched — same defect class as the withdrawn
`func_800906B4`. A mandated structural re-audit confirmed **every `c` span ends
exactly on a `jr $ra` boundary and none contains more than one `jr $ra`**, so no
masked matches remain.

Prior (session cont. 19): **a span-size defect repair (693 → 695) plus three fixes**.
`disc1_preflight.py --deep` (the compile-and-compare mode — the fast path does
NOT catch sizes) found three `c` spans whose declared size exceeded their
compiled `.text`, silently swallowing the next real function:
`func_80076B58` (`0x88`→`0x40`; swallowed `func_80076B98`, now a carved
`LINK_EXACT` leaf), `func_800906B4` (`0x68`→`0x30`; swallowed
`func_800906E4`, now a carved `LINK_EXACT` leaf on
`era_o2_g0_symbol_at_temp` — patch 5 is required for its 3-word `$at` form),
and `func_80077D30` (`0x94`→`0x90`; 4-byte layout pad now its own asm span).
`func_800906B4` was a **masked false match** — its oversized span hid a
register-allocation mismatch; its C was rewritten to a temp-local form
(`BYTE_EXACT`/`LINK_EXACT` at `0x30`). Durable guard added:
`tools/analysis/check_leaf.sh <func> <vram> <size> [flags]` runs
`era_link_check.py` AND `disc1_preflight.py --deep --only <func>`; the full
`--deep` run must pass before any batch is declared done. **Env-leak hazard:**
exporting a maspsx knob (`MASPSX_SYMBOL_AT_TEMP=1`, …) in the shell leaks into
`exact_rebuild.sh` and poisons every leaf that does not want it — use a
subshell, never the persistent shell. Prior matches (session cont. 18): **four
leaves (689 → 693)**. The `0x561C8` row-accessor cluster
contributes three, plus `func_8005DB8C` (global word + record-base helper; a **data
symbol** `&D_800A8038`, not an address literal, so the base materialises as
`addiu v1,v0,-16` with two separate address locals). `func_800659C8` (halfword insert at row+8) and `func_80065A60` (byte
insert; the index is folded into the base argument —
`a1 = (a1 << 1) + (unsigned int)q` — so the store keeps the `+1` displacement
and fills the `jr` delay slot) both reuse `func_8006599C`'s
**pointer-to-volatile** `D_800B1624` double-load, and `func_80065B70` (the
`0x800BCF88` BGM state-block init) is a **tail-call target with no prologue**:
its only reference is a `li $t2,0x70 / jr $t2` tail jump, so cc1 emits the
frameless straight-line `li`/`lui $at`/`sw|sb|sh` sequence. All four `LINK_EXACT`.
`func_800659F8` (same cluster) is **not** carved — an unexplained 8-byte frame
(no C phrasing lands between the 56-byte body and a 16-byte frame). Prior
matches (session cont. 17): **fifteen leaves (674 → 689)** plus a tightening of
the `cop-inline` split. Highlights: eight countdown-timer twins across two overlays
(`func_800C8C80`/`C8CBC`/`C8CF8` and `func_800CBBF0`/`CBC2C`/`CBC68`, plus
`func_800CCA40`/`CCA78`/`CCB6C`/`C9A34`/`CD5B0`) where the `if` must **re-read**
`a2+4` (or `a2+3` as `signed char`) rather than fold the compare into the value
in hand, `func_80089F08` (fold the index base into `$a0` so the `+0xC`
displacement stays on the base register), `func_8008C6D0` (`unsigned int`
`do/while` counter → `sltiu`), `func_80016FE0`/`func_80018718` (`if/else` beats
early return for 1/0 gate writers) and `func_8001784C` (`era_o2_g8` keeps the
`0x590($gp)` pointer global gp-relative). All fifteen `LINK_EXACT`.
**`cop-inline` retighten**: an isolated COP **command** op (a lone `rtps`) is a
literal Psy-Q `gte_rtps` macro with no C spelling, so `nonmatchable_spans.py`
now counts any GTE command op as `handwritten-gte-wrapper`; only an isolated
COP2 register-transfer pair inside branch/call-bearing code stays informational
`gte-inline`. `handwritten-gte-wrapper` 103/19008 → **107 / 19958**;
`gte-inline` 9/1142 → **5 / 192** (`func_8003EAC8`, `func_800130B4`,
`func_80077F7C`, `func_80078004`, `func_80078094`). A hard-wrapped YAML comment
(a bare ` cursor by` line in the `func_8008F430` block) that broke strict YAML
parsing was repaired; the sibling's `disc1_preflight.py` is clean.
Prior newest matches (session cont. 16): **four leaves (670 → 674)** plus the
`cop-inline` split.
Highlights: `func_80038910` (seven gp-relative destinations, `era_o2_g8` —
under `-G0` each store splits into an absolute `lui $at`), `func_8007DBC8`
(narrow `unsigned short` local homes the table value in `$a0`), `func_80087050`
(`volatile` polled word + `a0==0`-arm-first layout) and `func_8007E594`
(parallel `q = a0 + 3` keeps the down-counting byte walk at ROM size). All four
`LINK_EXACT`. **`cop-inline` accounting fix**: the old 117-span / 20196-word
informational bucket is split by `tools/analysis/nonmatchable_spans.py` into
**`handwritten-gte-wrapper`** (103 spans / 19008 words, now **counted** as
non-C: a contiguous run >=3 of COP2/GTE ops, or a branch/call-free body with a
COP op — 1810 of 2419 transfer ops carry the disassembler's
`/* handwritten instruction */` flag, and **0 of the 674 matched C leaves
contains any COP op**) and **`gte-inline`** (9 spans / 1142 words, informational:
an isolated COP pair inside clean branch/call-bearing integer code —
`func_800D2B58`/`DB25C`/`D1DEC`/`D2104`, `func_800130B4`, `func_80078094`/
`78004`/`77F7C`, `func_8003EAC8`). `handwritten-cop` grew 17 → 22. The manifest
contract is unchanged for `route_coverage.py`: counted classes stay under
`spans`, `gte-inline` stays out of `spans` in the retained
`matchable_cop_inline` key. Honest whole-image totals: matched C 674; non-C
**180 funcs / 19302 words**; 31 alignment spans; `gte-inline` 9 / 1142. Route
view: matched C 326 / 5750, provably non-C 76 / 5414, **real remaining asm
569 / 66291**. Prior matches (session cont. 15): **twenty-three leaves
(647 → 670)** from a systematic sweep
of the small no-call asm in `asm/disc1/`. Highlights: the tail-dispatch family
`func_80073D24` (arguments `(4, a0)` through `D_8009566C->f+0x14`) plus
argument-less twins `func_80073D58`/`73D88`/`73DB8`; the sound-buffer pair
`func_80076B58` (`0x4000000` header then a `do … while (--c != -1)` word copy)
and `func_80076BE0`; `func_80087864` (SPU nibble RMW) and the cursor readers
`func_8008F784`/`8008F430`; `func_80051684` (double-null-guarded store);
`func_80067B40`; the bound-checked table getters `func_8007A400`/`7A434`
(patch 5) and `func_800858E8` (patch 5) / `func_80085918` (patch 4); the strided
byte setter `func_800428D4`; the signed range getters `func_800556E8`/`58E08`
(**new profile `era_o2_g8_symbol_at_temp`** = `-O2 -G8` + patch 5); the
`0x40`-stride pair incrementer `func_8008A02C`; the ring-buffer push
`func_8009071C`; the volatile five-word seed `func_80036DF8`; and the volatile
double-base byte OR `func_80065A9C`. All twenty-three `LINK_EXACT`.
**Durable levers**: (1) **patch 4 vs patch 5 is decided by whether the indexed
symbolic load's address temp is the destination register** —
`func_80085918`'s `nor` immediately consumes the load, so retail keeps the
dest-register temp with `%lo` (patch 4); the patch-5 `$at` form costs it 3 words
and the default 9. (2) **patch 5 composes with `-G8`** via the new
`era_o2_g8_symbol_at_temp`; dropping the env from those two leaves costs 9
words. (3) **early-return polarity for a table getter** — write
`if (i >= N) return fallback; return table[i];`; the `i < N` order emits a
`bnez`-into-fallthrough and misses by 9. (4) a **signed `int` index gives
`slti`**, `unsigned` gives `sltiu`. (5) **two separate signed early returns**
reproduce retail's double `addu $v0,$zero,$zero` zero-init. (6) **loop-sentinel
spellings**: `c = a1 - 1` with `while (--c != -1)`, and the `do … while (a2)`
pair incrementer with a pre-loop `d = a1 - *a0` + parallel `p = a0 + 1`.
(7) **named address temporaries** fix operand order (`q = p + 2`; named `t` for
a second computed address). (8) **`volatile` keeps deliberate duplicate stores**
(`func_80036DF8`, `func_80065A9C`). (9) **the `cop-inline` bucket's top fan-in
entries are PSY-Q GTE macro wrappers, not liftable C** — `func_800661A4`/`661CC`,
`func_8006698C`, `func_8003B97C` are literally `lw`/`sll`/`ctc2 $t4,$N`; no cc1
2.7.2 emits `ctc2` from C. (10) a **`register long long p asm("$2")` pin cannot
fix DImode half-read order or a dead `sra` tail** — `func_8003708C` (fan-in 16)
is PARKED (`docs/evidence/func-8003708C/PARK.md`). Two stale PARK files removed
(`func-8006E6D4`, `func-8007FBF0`). `profile_necessity.py` → `506/506 era leaves
clean`.
Newest matches
(session cont. 14): **fifteen leaves (647 → 662)**, a wide sweep of small
no-call leaves rather than one family. `func_80073D24` (tail-dispatch with
arguments `(4, a0)` through `D_8009566C->f+0x14`) and the argument-less twins
`func_80073D58`/`73D88`/`73DB8` (slots `+0x14`/`+0x10`/`+0x18`);
`func_80076B58` (`*D_80095854 = 0x4000000` then a `do … while (--c != -1)` word
copy) and `func_80076BE0` (`*D_80095854 = a0 | 0x10000000`, returns
`*D_80095850 & 0xFFFFFF`); the SPU per-voice twins `func_80087864`
(nibble RMW) and `func_8008F784`/`8008F430` (cursor readers);
`func_80051684` (double-null-guarded `+0x8` store); `func_80067B40`
(`D_800BCFFA/FB` bytes + a `D_800BCF88 & ~0xC00 | 0x400` RMW with a `$2` base
pin); the bound-checked table getters `func_8007A400`/`7A434` (patch 5) and
`func_800858E8` (patch 5) / `func_80085918` (patch 4). All fifteen `LINK_EXACT`.
**Durable levers**: (1) **patch 4 vs patch 5 is decided by whether the indexed
symbolic load's address temp is the destination register** — `func_80085918`'s
`nor` immediately consumes the load, and retail keeps the dest-register temp
with `%lo` (patch 4); the patch-5 `$at` form costs it 3 words and the default
9. (2) **early-return polarity for a table getter** — write
`if (i >= N) return fallback; return table[i];`; the `i < N` order emits a
`bnez`-into-fallthrough and misses by 9. (3) a **signed `int` index gives
`slti`**, `unsigned` gives `sltiu`. (4) **loop-sentinel spelling**: `c = a1 - 1`
with `while (--c != -1)` reproduces retail's preheader/sentinel pair. (5) named
cursor locals keep store order (`q = p + 2`; split nested `if (p)` guards).
(6) **the `cop-inline` bucket's top fan-in entries are PSY-Q GTE macro
wrappers, not liftable C** — `func_800661A4`/`661CC`, `func_8006698C`,
`func_8003B97C` are literally `lw`/`sll`/`ctc2 $t4,$N`; no cc1 2.7.2 emits
`ctc2` from C. (7) a **`register long long p asm("$2")` pin cannot fix DImode
half-read order or a dead `sra` tail** — `func_8003708C` (fan-in 16) is PARKED
(`docs/evidence/func-8003708C/PARK.md`): the pin recovers the register home but
cc1 still reads `mfhi` before `mflo` and keeps the dead `sra $3,$3,16`. Two
stale PARK files removed (`func-8006E6D4`, `func-8007FBF0` — both leaves have
long since matched). `profile_necessity.py` → `498/498 era leaves clean`.
Newest matches
(session cont. 13): eleven leaves, ten of them the **serial-cursor-reader
family** in the `0x7F000`–`0x81000` stream block — `func_8008FFC0`/`800900E4`
(byte `<<8`/`<<7` to `+0xA6`/`+0xB4`), `func_80090054`/`80090178`
(flag-teardown twins: clear bit 1 / bit 2 at `+0x38`, OR `0x3` into `+0xF4`,
zero `+0xEA`/`+0xEC`), `func_8008F4E8` (byte `<<8` to `+0x6C` **after** the
`+0xF4` RMW), `func_8008FBFC`/`8008FCE4` (signed-byte accumulators into
`+0xDE`/`+0xE0`), `func_8008F6B0` (`+0x74` zero, byte `<<8` to `+0xD8`,
conditional `+0xF4 |= 3`), `func_8008FC28` (two-field: `+0x7E` or `0x100` when
zero, then a sign-extended byte to `+0xE4`), plus `func_8007CE80` (word copy
with a pre-tested count and a named loop-local load) and **`func_80026FD0`,
whose PARK is resolved** (`-O2 -G8`, `era_o2_g8`). All eleven `LINK_EXACT`.
**Durable levers**: (1) **`lb` comes from `signed char`, not `char`**, and a
`signed char[16]` array declaration forces an **absolute** base while keeping
the sign-extending load (the `func_80026FD0` gate); its two stores have
**different signedness** (`unsigned char = 0x80` → `li $v0,0x80`;
`signed char = -8` → `li $v0,-8`); (2) a **named loop-local `v = *a1`** keeps
retail's `lw`/`addiu a1`/`addiu i`/`sw a0` order; (3) a **named value local
between the pointer-advance and the store** keeps the `lbu` late; (4) **two
`(unsigned char **)` casts, not a struct**, give `lw+addiu+sw` then
`lbu+sll+sh`; (5) **fresh pointer locals per cursor walk** (one expression
reading `*(unsigned char **)a0` twice gets CSE'd); (6) **flag-teardown masks are
bit-exact** (`&= ~2` is `0xFFFD`, not `0xFFFC`). `func_80067CBC` re-attacked and
re-PARKed (4-word mask-vs-load ordering residual); `func_8005DB44` re-PARKed
with the address base proven (`D_800A8028+16`, one `addiu -16`) and the residual
narrowed to **reassociation** (`(a0<<5)+base+start` vs retail's
`((a0<<5)+base)+start`).
**Honest non-C-matchable coverage** (`tools/analysis/nonmatchable_spans.py`,
report `docs/evidence/non-c-matchable/REPORT.md`): of the route's remaining asm
functions, **47 are provably non-C-matchable** (`handwritten-jr-t2` 53 total /
163w in `621E4.s`/`64B54.s`/`6E538.s`/`6E6C0.s`/`75F44.s`, `handwritten-cop`
17 / 77w in `68664.s`, `handwritten-syscall` 2 / 8w; 25 more off-path), 31
lone-`nop` spans are alignment filler, leaving **616 real remaining asm
functions / 71954 words**. The taxonomy now also reports a **`cop-inline`**
bucket (117 spans / 20196 words) for spans that merely *contain* a COP2/GTE op
but are otherwise ordinary integer code — **informational only, NOT counted as
non-C**, emitted as the separate top-level `matchable_cop_inline` key so the
consumer's non-C contract is unchanged; it is the next-pass frontier (a C front
end cannot emit the raw COP2 op, but the bulk of each body is liftable).
`func_8001F814` (already
`NONMATCHING_C`) and the 24 ACCEPTED-RESIDUAL leaves keep their own taxonomy and
are not reclassified. Newest matches
(session cont. 11): the mirrored 0x1000-step short-table family
`func_80077CF4`/`func_80077D30`/`func_80077DC4` (the last **on-path fan-in 17**)
on **maspsx patch 5** (`MASPSX_SYMBOL_AT_TEMP=1`), plus `func_80052E30`
(dispatch-config selector) on `-O2 -G8` +
`MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800A1F84,D_800C0E48,D_8009D05C` (profile
`era_o2_g8_force_descriptor_absolute`: the six config words stay gp-relative
while the three descriptor addresses must be absolute or the link aborts with
`small-data section too large`). **New durable gate
`tools/analysis/profile_necessity.py`**: for every era leaf it proves the
recorded profile is load-bearing (default must NOT reproduce an assigned leaf,
assigned must reproduce it, unassigned leaves must be default-exact); it found
and removed 7 provably-redundant assignments and caught a real carve regression
(`func_8008FBD4` had overshot into the asm function `func_8008FBFC` at
`0x803FC`; the span is now the true `0x28`). It also honours `ERA_ASPSX_VER` /
`MASPSX_EXPAND_DIV` now in `era_link_check.py`. `func_80079FB4` (top remaining
fan-in 19) is PARKED (`docs/evidence/func-80079FB4/PARK.md`: duplicate-`slt`
block layout + inline ASPSX div guards). Prior newest matches:
eleven small leaves around the CD-audio/stream-block readers and the
`D_8009566C` handler dispatch — `func_8008FED8` (stream rewind),
`func_80090948` (cursor advance + byte broadcast), `func_8008783C` (SPU voice
pitch field insert), the `func_8008FBD4`/`func_8008FCBC` sign-extended cursor
read pair, `func_80036DC8` (three-call init frame), `func_80073C94`/
`func_80073CC4` (`D_8009566C->f+0xC`/`+0x8` argument-less fn-ptr dispatch),
`func_8006599C` (row getter through a **pointer-to-volatile** double load),
`func_80064C54` (`-O2 -G8`, `func_8005F354(func_8005DC4C(), D_8009D164)`) and
`func_80085F44` (guarded `D_8009B434` setter).
Also closed out registration for `func_80042770`/`func_80042964`
(stride-1048 readers, **maspsx patch 5** `MASPSX_SYMBOL_AT_TEMP=1`, profile
`era_o2_g0_symbol_at_temp`: the `$at` temp *with* the `%lo` displacement kept
on the load — distinct from both the legacy path and patch 4), plus
`func_8007FBF0` (patch 4) and `func_80075B84` (patch 3, load-bearing:
`word mismatches=2` without it). Durable new levers: intermediate narrow
`unsigned char c` preserves retail's `lbu`+`sll 24`/`sra 24` instead of a
folded `lb`; an `unsigned int` broadcast value avoids a dead `andi 0xFF`;
pointer-to-volatile defeats a second-load CSE. Patch-5 durable test
`tools/era/maspsx/tests/test_symbol_at_temp.py`. Prior: `func_80052F70` (the
`0x32`-clamped `D_800C0E0C` byte accumulator in `43724.s`; fan-in 61),
`func_800762A0` (GP0 `0xE5` draw-mode packer in `66970.s`), and the
`D_800A3348` byte-table pair `func_80076B44`/`func_80076B20` in `66B54.s` —
all `-O2 -G0`, the latter two on **vendored maspsx patch 4**,
`MASPSX_SYMBOL_LOAD_DEST_TEMP=1` (profile
`era_o2_g0_symbol_load_dest_temp`): retail's indexed symbolic load uses the
**destination register** as the address temp and keeps the `%lo` displacement
(`lui $d,%hi(SYM)` / `addu $d,$d,$b` / `op $d,%lo(SYM)($d)`), and an indexed
symbolic store that precedes a bare `j $31` materializes `$at` before the jump
and schedules the store into the delay slot. Neither the legacy path nor
`MASPSX_THREE_WORD_SYMBOL_STORE` covers this (both use `$at` and drop the
`%lo`); patch 1's fill is *absolute* `sw` only. Durable test
`tools/era/maspsx/tests/test_symbol_load_dest_temp.py`; registered in
`scripts/setup_era.sh` `MASPSX_TRACKED` + `.gitignore` negation. Survey: 69
dest-temp sites in disc1 text. Prior: the `654C8` cluster continued —
`func_800751E4` (next-pointer link walk plus
`D_8009580C`/`D_800957F8` window seed), `func_800752AC` (`D_80095744->f(0x2C)`
handler + same seed), `func_80075358` (`f(0x3C)(0)` then `f(0x14)(a0+4,
a0[3])`), and the twins `func_80075424`/`func_800754E4` (force/splice the low
24 bits of the record word at `a0+0x1C`, push `f(0x8)`, 0x14-byte
`D_800957B8` template) — era `-O2 -G0` + patch 3
(`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`, profile
`era_o2_g0_fill_epilogue_delay_slot`). Two new levers:
**element-form OR keeps the displacement on the base register**
(`((int *)a0)[7] |= 0xFFFFFF;` → retail's `lw v0,0x1C(base)`/`sw
v0,0x1C(base)` and `lui 0xFF` in `$a0`; a local base splits them), and
**loop-local named mask constants** (hoisting `lo`/`hi` out of the
`func_800751E4` walk promotes `lo` to `$s2`). All five `LINK_EXACT`;
`func_800755F0` (`0x4F8`, the cluster's big gate state machine) is now PARKED
(`docs/evidence/func-800755F0/PARK.md`, prologue-scheduling + shared-tail
clamp residual). Prior: the boot-spine tail `func_80074D28`/`func_80074DC0`
(state gate + `D_80095748` log vector + `D_80095744` handler dispatch) and
`func_80077AC4` (fan-in 16; both directions' 24-bit payload swap), plus the
`0x80075xxx` display cluster `func_8007506C`/`750CC`/`753B4`
(reset-then-push), `func_80075B1C`/`75B4C`/`75C04`/`75C6C`/`75C94`/`75AE8`/
`755BC` — three levers there: **data-symbol arguments beat address
literals** (`&D_80011870`, not `(char *)0x80011870` — a literal folds to one
`lui`); **`D_80095744` is a pointer global** (`extern unsigned int *`, one
loaded base register reaches both the `+0x8` handler and the argument word —
a struct pointer reloads it); and pinned `asm("$16")`/`asm("$17")` locals for
fixed callee-saved pairs. `func_80075C44` is PARKED
(`docs/evidence/func-80075C44/PARK.md`, `sltu` boolean-fold residual). The
`func_800755F0` flag seed is `0x08000000`, not `0x80000000` (`lui $s0,0x800`).
Prior: `func_8006DB9C`/`func_8006DBE0` (tagged byte-pair searches over
`D_800B0CD8+0xDC`; aggregate-member indexing keeps the `0xDC` displacement on
the `lb`), plus five field-VM `D_800910A0` handlers (`func_80017294`,
`func_8001731C`, `func_8001735C`, `func_80017410`, `func_800176B8`) on era
`-O2 -G8` with `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0` (state object
absolute, VM cursor `D_8009CE00` stays gp-relative at `0x90($gp)`) — profile
`era_o2_g8_force_d8009d2f0_absolute`; `func_80017410` needs only `-O2 -G8`
plus a `short buf[8]` array (a scalar reserves too small a frame). All seven
`LINK_EXACT`; see the top session in ACTIVE_HANDOFF. `func_8006DE80` is now
PARKED (`docs/evidence/func-8006DE80/PARK.md`, argument-promotion/schedule
residual). Prior: `func_8001A680` (highest on-path fan-in, 27), `func_8006DED4`,
`func_8006DF50`.
Prior matches: `func_8006EC84`, `func_8006F224`, `func_8006F2C4`,
`func_8006F6D4`, `func_8006F820`, `func_8006F8EC`, then `func_8006E6D4`
(69w CD read issuer) and `func_800811E4` (28w CD read poll).
`func_8001F814` remains `NONMATCHING_C` (jump table in the `0x800` rodata
pool); 24 ACCEPTED-RESIDUAL leaves are documented but **not** counted.
`func_8006F9F0` is PARKED (reorg delay-slot fill residual,
`docs/evidence/func-8006F9F0/PARK.md`); `func_800701B4` is PARKED
(callee-saved-`idx`-init scheduling residual, 9/74 words, all in the
prologue — `docs/evidence/func-800701B4/PARK.md`).
Native ports under `pc_port/` are not matching leaves. Exact SHA-1
rebuild via `scripts/build_us.sh` / `scripts/verify_us.sh` (build runs in
the `pe-mipsel-img` docker container — see ACTIVE_HANDOFF). Native suite:
`cmake -S pc_port -B pc_port/build` then `./pc_port/build/pe-native-tests`.

**This lane (session 2026-09-11 cont. 13): the serial-cursor-reader family +
a PARK resolved.** Eleven leaves, ten of them the cursor readers in the
`0x7F000`–`0x81000` stream block investigated as a family from the retail
`lw $v1,0x0($a0)` / `addiu $v0,$v1,0x1` / `sw $v0,0x0($a0)` / `lbu` trace:
`func_8008FFC0`, `func_800900E4` (byte `<<8`/`<<7`), `func_80090054`,
`func_80090178` (flag-teardown twins), `func_8008F4E8`, `func_8008FBFC`,
`func_8008FCE4` (signed-byte accumulators), `func_8008F6B0`, `func_8008FC28`
(two-field), plus `func_8007CE80` and the **`func_80026FD0` PARK resolution**.
Durable levers: (1) **`lb` needs `signed char`** (plain `char` is unsigned →
`lbu`), and a **`signed char[16]` array** both keeps the sign-extending load and
pushes the base out of the `-G8` window to absolute — this is exactly the
`func_80026FD0` gate, whose earlier PARK was a mistyping artefact, not a
frontend divergence; (2) the same leaf's two stores must have **different
signedness** (`unsigned char = 0x80` → `li $v0,0x80`; `signed char = -8` →
`li $v0,-8`); (3) a **named loop-local `v = *a1`** fixes the `lw`/`addiu`/
`addiu`/`sw` order of a copy loop; (4) a **named value local between the
pointer-advance and the store** keeps `lbu` late (`func_8008F4E8`); (5) **two
`(unsigned char **)` casts, not a struct**, produce `lw+addiu+sw` then
`lbu+sll+sh` (`func_8008FFC0`); (6) **fresh pointer locals per cursor walk** —
re-reading `*(unsigned char **)a0` in one expression gets CSE'd
(`func_8008FC28`); (7) **flag masks are bit-exact** (`&= ~2` → `0xFFFD`; `~3`
would be `0xFFFC`, a one-word miss). `func_80067CBC` re-attacked: the residual
is exactly a 4-word **constant-materialisation-before-base-reload** ordering
(retail emits `~0xC000` before `lui $v0,0x800C`); 2 pointer-local forms,
`volatile`, a ternary, a carried local and 4 rungs all keep the mask after the
load. `func_8005DB44` re-PARKed with the address base now **proven** as
`D_800A8028+16` (one `addiu -16`) and the residual narrowed to cc1
**reassociation** of `(a0<<5) + base + start`. `func_800739C4` retry log added
(early-`return` and a compare-ladder restructure both give 18 mismatches vs the
shared-tail form's 12 → the tail-duplication is downstream of C control flow).
Taxonomy: new `cop-inline` bucket in `nonmatchable_spans.py` (117 spans /
20196 words, **informational, not counted non-C**) for spans containing a raw
COP2/GTE op but otherwise ordinary integer code.

**This lane (session 2026-09-11 cont. 10): patch-5 registration + eleven small
leaves.** `docs/evidence/func-8008FED8/REPORT.md`, `func-80090948`,
`func-8008783C`, `func-8008FBD4`, `func-8008FCBC`, `func-80036DC8`,
`func-80073C94`, `func-8006599C`, `func-80064C54`, `func-80085F44`, plus
registrations for
`func-80042770`/`func-80042964` (patch 5), `func-8007FBF0` (patch 4) and
`func-80075B84` (patch 3). Durable levers: (1) **an intermediate
narrow type controls `lbu`+`sll`/`sra` vs a folded `lb`** — keep the fetched
byte in `unsigned char c` and cast only at the store (`func_8008FBD4`/
`8008FCBC`); (2) **pointer-to-volatile defeats a second-load CSE** —
`extern unsigned char *volatile D_800B1624` preserves retail's two
independent `lui`/`lw` pairs (`func_8006599C`); (3) **an `unsigned int`
broadcast value avoids a dead `andi 0xFF`** (`func_80090948`); (4) **stale
profile assignments silently fall back to `era_o2_g0`** — the object can still
match while the profile is wrong, so every patched leaf needs an explicit
`assignments` entry; (5) patch 5's `$at`-with-`%lo` form is registered under
`era_o2_g0_symbol_at_temp` alongside patch 4's dest-temp form.

**This lane (session 2026-09-11 cont. 8): `654C8` cluster continued.**
`docs/evidence/func-800751E4/REPORT.md`, `func-800752AC`, `func-80075358`,
`func-80075424`, `func-800754E4` (`func_800755F0` PARK). Durable levers:
(1) **element-form OR** — `((int *)a0)[7] |= 0xFFFFFF;` and
`((int *)a1)[7] = (((int *)a1)[7] & 0xFF000000) | (a0 & 0xFFFFFF);` give
retail's `lw v0,0x1C(base)`/`sw v0,0x1C(base)` and keep the `lui 0xFF` mask
in `$a0`; routing through a local base pointer splits the load/store;
(2) **loop-local mask constants** — `func_800751E4`'s `lo`/`hi` must be
declared inside the walk loop, or hoisting promotes `lo` to the callee-saved
`$s2`; (3) `func_80075358` is `void` (its `0x60` object size is one word of
gas 16-byte pad past the `0x5C` body); (4) `func_800755F0`'s flag seed is
`0x08000000`, not `0x80000000`.

**This lane (session 2026-09-11 cont. 7): boot-spine + display cluster.**
`docs/evidence/func-80074D28/REPORT.md`, `func-80074DC0`, `func-80077AC4`, and
the `0x80075xxx` display leaves (`func-8007506C/750CC/753B4/75B1C/B4C/C04/C6C/
C94/AE8`, `755BC`; `func-80075C44` PARK). Three durable facts: (1) **data
symbols, not address literals** — `extern char D_80011870` + `&D_80011870`
gives retail's `lui`/`addiu` relocs where a cast constant folds to one `lui`;
(2) **fixed-register pins** via `register T x asm("$16")` reproduce retail's
callee-saved allocation when C's natural order differs (`func_80074D28` s0/s1,
`func_80077AC4` mask constants in `$6`/`$7`); (3) `D_80095744` is a **pointer
global** (`extern unsigned int *`) and the display cluster reads both the
`+0x8` handler and the argument word through one loaded base register.
Function-pointer table slots are declared **argument-less**
(`unsigned int (*f)()`) when retail clears no argument registers — a prototype
with a parameter makes cc1 materialize `$a0` in the `jalr` delay slot.

**This lane (session 2026-09-11 cont. 9): new maspsx patch 4 + 4 leaves.**
`docs/evidence/func-80052F70/REPORT.md`, `func-800762A0`, `func-80076B44`,
`func-80076B20`. **Patch 4 — `MASPSX_SYMBOL_LOAD_DEST_TEMP=1`** (profile
`era_o2_g0_symbol_load_dest_temp`): an indexed symbolic load `op $d,SYM($b)`
gets the naive GNU-as dest-register address temp (`lui $d,%hi(SYM)` /
`addu $d,$d,$b` / `op $d,%lo(SYM)($d)`); an indexed symbolic store that
immediately precedes a bare `j $31` materializes `$at` before the jump and
schedules the store into the delay slot. Neither the legacy path nor
`MASPSX_THREE_WORD_SYMBOL_STORE` covers this — both use `$at` *and drop the
`%lo`* (`op $d,0x0($at)`); patch 1's fill is *absolute* `sw` only. Durable
test `tools/era/maspsx/tests/test_symbol_load_dest_temp.py` (6 tests, in
`MASPSX_TRACKED` + `.gitignore` negation); 69 dest-temp sites in disc1 text.
Two more levers: **operand order beats a flat OR** (`func_800762A0` must
compute `y` (the `a1` shift) before `x` (the `a0` mask | base) or cc1 hoists
the `0xE5` constant), and **`unsigned int` for a `srl` index**
(`func_80076B20`'s `a0 >> 24`; signed gives `sra`). CC1-vendor divergence is
not a link failure: `func_80052F70` spills `$ra` where retail spills `$v0`,
and `func_800762A0`/`76B44`/`76B20` carry trailing gas alignment pads — all
`LINK_EXACT`.

**This lane (session 2026-09-11 cont. 4): CD-stream + command-record
teardown.** `docs/evidence/func-8006{ECEC,F044,F39C,FC18,FE14,70064,702DC}/
REPORT.md`. Durable typing facts reinforced: `D_800942E0` must be
`extern void **` (pointer-to-handler-pointer-table), never `void *[]` — an
array declaration makes cc1 take the address instead of loading the pointer
(3 mismatched words per lookup). Overlay handler descriptors are referenced
as **data symbols** (`(unsigned int)D_801F1BD8` with `extern unsigned char
D_801F1BD8[]`), not integer constants, so HI16/LO16 relocs are emitted and
the split asm defines them. `D_800B0CD8` bit-set goes through a pointer
local to reproduce retail's shared `&D_800B0CD8` (bare `|=` → absolute
`lui/lw` + `lui $at/sw`). `D_800B0DD8` is `int` for the `base+offset`
arg. Poll loops: `func_8006ECEC` reuses the `func_8006E834` `$v1`-backup/
`$v0`-restore pins; `func_8006F39C`'s prelude poll uses the plain
`-1`-then-`0` order without pins. The seven-slot teardown clear is
`D_800E0EF0[k]` for `k = 0x6C..0x72` (`D_800E10A0 == D_800E0EF0 + 0x1B0`)
in the sweep leaves but the absolute `D_800E10A0` symbol in
`func_8006F39C`/`func_8006FC18`. `func_8006FC18` needs a **distinct second
pointer local** (`q`) after its handler call; `func_800702DC`/`70064` must
use `i` as both counter and call argument (an `idx` biv perturbs the
callee-saved allocation order — that is the `func_800701B4` PARK).

**This lane (session 2026-09-11 cont. 3): the command-record family.**
`docs/evidence/func-8006{F224,F2C4,F6D4,F820,F8EC}/REPORT.md`. Two durable
typing facts: `D_800942E4`/`D_800942E8` are **pointer globals**
(`extern unsigned char *`, retail `lui`+`lw` base) with strides `0xA0C`
(ids `0..0xA`) and `0x10C` (ids `0xB..0x15`); `D_800942E0` is a
pointer-to-handler-pointer-table (`extern void **`). Arena-selection and
handler-guard branch polarity are load-bearing block-layout levers — write
`if ((unsigned)idx >= 0xB) <E8> else <E4>` and the `== 0`-guard form.

**Prior (this lane): `func_8006E6D4` and `func_800811E4` on era `-O2 -G0`.**
`func_8006E6D4` closes its old PARK residual (`docs/evidence/func-8006E6D4/`):
the stray `lui $v1,0x100` rematerialization vanishes once `func_800719E4`
(BIOS `B(38h)` = `exit`) is declared `__attribute__((noreturn))` — the
`0x01000000` mask stays live across the call and the `beq` delay slot reuses
it. `func_800811E4` (`docs/evidence/func-800811E4/`) needed two levers: one
retained `$a0` base for the adjacent pending/issue words (`&D_8009B6B4.issue`
reached through a struct plus a zero-code `asm volatile("": "=r"(base) :
"0"(base))` barrier that stops cc1 folding the pointer back into the loads),
and a **new vendored maspsx patch 3**, `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`,
which schedules `addiu $sp,sp,32` into the `jr $31` delay slot (ASPSX reorder
fill; same class as patch 1's store fill, inverse direction). Durable test
`tools/era/maspsx/tests/test_fill_epilogue_delay_slot.py` (maspsx suite 167
tests OK); `scripts/setup_era.sh` `MASPSX_TRACKED` restores it on re-clone.
Link-level proof at the retail VMA: `LINK_EXACT`, 0 word mismatches.

**Prior: leaves lane merged (269 leaves) + grind lane (`func_80029388`/
`func_800293F4` on era `-O2 -G8`, `func_8002F76C`,
`func_8002FA10`/`FAA4`/`FAD8` on era `-O2 -G0`).** The leaves-lane
`build_us.sh`/`disc1.yaml` are authoritative for the rebuild.

**Prior: Phase 5FE — 224 matching C leaves. `func_8002F970` (slot-table
pointer-match search-and-clear, 23 words) matches byte-exact on era
`-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1` — the table twin of 2F9CC,
reusing the exact `SlotRecord` typing (inheritance, no variant layouts).
Finds the record whose body address equals `*p` (`unsigned char **p`: the
comparison target is the record body), clears its `inUse` flag, and nulls
the caller's pointer — the `sw $zero,0($a0)` lands in the `jr` delay slot
(5EN pattern). The `$a3 = &D_800A5D5C` body base hoists before the loop:
the aggregate lever working in the OPPOSITE direction from 5FD (there it
prevented a hoist; here it produces one). Back-branch delay slot is FILLED
here (`andi`), inverse of 2F9CC's nop — slot fill is not per-table but
per-shape. Contiguous carve with 2F9CC in 11718: prefix `0xEA58`, C `0x5C`,
C `0x44`, resume `20210.s`. Prior: 5FD matched `func_8002F9CC` (7-slot
table clear) — the aggregate-element-type lever (third lever class:
addressing mode, alongside `-O1` materialization and sched2 placement).**
Exact SHA-1 rebuild via `scripts/build_us.sh` / `scripts/verify_us.sh`. The retail `func_8002F9CC` (7-slot table clear, 17
words) matches byte-exact on era `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`.
**Element type is an addressing-mode lever, not just typing.** Retail keeps
symbol+register at the store (`sw $zero,D_800A5D58($v1)` → `lui $at` / `addu` /
`sw %lo($at)`). Writing the clear as a flat `unsigned int[]` with a computed
index (`D_800A5D58[i*55] = 0`) makes cc1 hoist `la $5,SYM` out of the loop and
store through the register — a residual **invariant under every flag rung**
(-O2, -O1, -O1 -fschedule-insns2 were byte-identical), because it is an
addressing choice, not a scheduling one. Declaring the real 220-byte aggregate
element (`struct { unsigned int inUse; unsigned char body[216]; }`) makes cc1
emit the symbol store directly; the three-word knob then picks retail's 3-word
expansion over GNU as's 4-word one. Stride is **220B/55W** (`4*(8*(8-1)-1)`),
proven from the `func_8002F7xx` reader (`lw`+`bnez` on word 0 = in-use flag,
body pointer via `D_800A5D5C`, whose extent `0x604 = 7*220` fixes the count).
Ladder note: a lone symbol materialization is NOT an `-O1` signal — the `-O1`
lever is for repeated *constant/address* materialization, and `-O1` did not
move this hoist.

Prior: **Phase 5FA — 222 matching C leaves. `func_8003E680` (boot subsystem-init
dispatcher, 53 words) matches byte-exact on era `-O1 -G0` — zero five state
globals (types from Stage-0 reader evidence: D1C4/D280 `sltu` unsigned
compares, D1A0 `andi`-0x2 flags, D250 opaque, CDDC `int` ×36 index), a
2000-pass hardware-poll loop (`i++` in the `jal` delay slot; ROM `sltiu` →
`unsigned int` counter), callback registration, then ~11 subsystem inits.
**First fn-ptr-to-asm-callee arg**: `func_80073D24(func_8003E91C)` emits
`lui $a0,%hi(func_8003E91C)` / `addiu $a0,$a0,%lo` with R_MIPS_HI16/LO16
relocs against the same-segment text symbol — links exactly like a data
symbol. `-O1` chosen predictively by the selection rule (five separate
`lui $at,%hi` stores, retail didn't CSE the shared `0x8009` high half).
Prior: 5EZ matched `func_8006A5BC` (two VSync-polled wait loops) at era
`-O1 -G0` — the first leaf where the `-O1` lever was predicted from ROM
structure (per-use `$s0=1` materialization) rather than discovered.**
Exact SHA-1 rebuild via `scripts/build_us.sh` / `scripts/verify_us.sh`. The retail
EXE was built with **Psy-Q `ccpsx` (GCC 2.7.x)**. Proven era fingerprints include
`move`→`addu`, `$at` absolute-`sw` macros, operand order, and `$v0`/`$v1` alloc;
`lui;ori` large-literal synthesis is **CAPABILITY-VERIFIED** (both bit15 sign
cases; cc1 emits PSY-Q `li` high + `ori` low natively under 2.21 +
`--dont-expand-li`). **`scripts/setup_era.sh`**
fetches `gcc-2.7.2-psx` (decompals/
old-gcc) + `maspsx` into git-ignored `tools/era/`; `build_us.sh`'s `era_compile`
runs `cpp`→`cc1`→`maspsx --aspsx-version=2.21 --dont-expand-li`→`as` **per-file**
(maspsx `li`→`ori` for positive small consts; ROM wants `addiu` — defer to GNU as),
so GCC-14.2 leaves stay byte-identical. **Vendored maspsx LOCAL PATCH:**
`tools/era/maspsx/maspsx/__init__.py` is repo-tracked (`.gitignore` negations;
`setup_era.sh` re-clones upstream AROUND it). Patch 1 = sw-store delay-slot fill:
env `MASPSX_FILL_STORE_DELAY_SLOT=1` per `era_compile` line expands an absolute
`sw $r,SYM` before a bare `j $31` into `lui $at` / `j $31` / `sw $r,%lo($at)`
(5EF delay-slot family; sb/sh and multi-store epilogues stay pre-jr in ROM).
Patch 2 landed at `f0b9155`: per-leaf
`MASPSX_THREE_WORD_SYMBOL_STORE=1` selects the three-word ASPSX-2.30-shaped
`lui` / indexed `addu` / `%lo` store while leaving compound lines and indexed
loads unchanged; flag-off remains byte-identical and its durable tests survive
`setup_era.sh` re-clones.
**Patch 3** (`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`, used by `func_800811E4`):
moves a trailing `addu/addiu $sp,$sp,N` that immediately precedes a bare
`j $31` into the delay slot (ASPSX reorder fill; 272 such filled epilogues in
retail). Guards: only `$sp,$sp,imm`, only when the next non-blank/non-comment
input line is exactly `j $31` (labels and `.set` block it). Durable test
`tools/era/maspsx/tests/test_fill_epilogue_delay_slot.py`; all three patch
tests are in `setup_era.sh` `MASPSX_TRACKED` and `.gitignore` negations.
**Patch 4** (`MASPSX_SYMBOL_LOAD_DEST_TEMP=1`, used by `func_80076B44`/
`func_80076B20`): an indexed symbolic load `op $d,SYM($b)` uses the naive
GNU-as dest-register address temp and keeps the `%lo` displacement; an
indexed symbolic store preceding a bare `j $31` materializes `$at` before the
jump and schedules the store into the delay slot. Durable test
`tools/era/maspsx/tests/test_symbol_load_dest_temp.py` is in the same
`MASPSX_TRACKED` / negation lists. Flag-off byte-identical (173 tests OK).
`scripts/split_us.sh` also needed `LC_ALL=C comm`/`sort` (comm compares
byte-wise, `sort` honours `LC_COLLATE`; a UTF-8 locale made the ignore-check
abort the split).
**Opaque-word ruling:** globals with only
bare 32-bit `sw`/`lw` use (no arith/pointer/bitwise) type as `unsigned int` —
not the rejected sh/sb→int cheat. Integrated: 8 A182x setters
(`func_80042BD8`…`func_80042C64`). **`D_8009D28C` = `int` state** (READY-FROM-READER;
equality-tested + word-copied; not opaque-word) — 4 setters
(`func_80017FDC`/`17FF0`/`192B8`/`192C8`). **`D_8009D270` = `unsigned int` flags**
(READY-FROM-BITWISE; `andi` 1/2 + clear-bit) — 2 setters (`func_80087198`/`87414`).
**5EF typing closure:** seven `int` state/value globals, four callback-pointer
globals proven by `jalr`, and one write-only `unsigned int` opaque word support
the final 13 delay-slot leaves; see `docs/ai_context/PHASE5EF_TYPING.md`.
`func_800405A4` is a use-site only.
Population counter: `tools/analysis/at_absolute_store_counter.py`.
Native PC-port / battle-runtime research lives in `pc_port/` and is incomplete
where evidence says it is. UE5 is a separate consumer repo
(`Blizz127/parasite-eve-ue5`); this repo owns gameplay semantics.
`docs/ai_context/ACTIVE_HANDOFF.md` has the exact current state and
`docs/splitting.md` the split target and policy.
