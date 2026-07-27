# CLAUDE.md — AI agent guidance for this repository

This is a PS1 decompilation research project for **Parasite Eve** (USA,
NTSC-U: SLUS-00662 / SLUS-00668). It is structured like a serious matching
decomp (compare: Xenogears decomp, PE2 decomp), not a PC port. Read
`docs/project_plan.md` for the phase roadmap and
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

**Phase 5FD — 223 matching C leaves. `func_8002F9CC` (7-slot table clear, 17
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
PC port is out of scope. `docs/ai_context/ACTIVE_HANDOFF.md` has the exact current
state and `docs/splitting.md` the split target and policy.
