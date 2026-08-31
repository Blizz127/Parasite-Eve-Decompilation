# ACTIVE HANDOFF

Single source of truth for current working state. Read this first; update after
every meaningful change. Prefer shortening over accruing.

## MACHINE / WORKTREE TOPOLOGY (read before running or committing anything)

One GitHub remote, several checkouts, one build environment. Every
failure of 2026-08-2x week (false-claim commit, wrong-machine `mv`,
a Codex session that couldn't find its branch) was an agent not knowing
one of these lines:

- **Remote (single source of truth):**
  `github.com/Blizz127/Parasite-Eve-Decompilation.git`. All lanes are
  branches of this one repo: `main` (published), `grind/continuous-decomp`
  (grind lane: PE-BTL leaf work + `pc_port/` native), `leaves/*` (desktop
  leaf lane), `sync/laptop-*` / `wip/laptop-*` (laptop machine). **Push
  after every commit** — local-only progress on any machine is unbacked.
- **`~/dev/parasite-eve`** (desktop): primary decomp worktree. This is
  the ONLY checkout where leaf work is built and committed.
- **`~/dev/pe-continuous-decomp`** (desktop): separate clone of the same
  remote, checked out on `grind/continuous-decomp`. Read-only source for
  ported leaves and evidence. Do not build or commit there unless the
  task explicitly says so. After `grind/continuous-decomp` is updated
  from elsewhere it needs a `git pull` before use.
- **Build environment:** the host has NO mipsel toolchain and no
  distrobox. Builds run in docker image `pe-mipsel-img:latest`, built
  from `dev/mipsel/Dockerfile` (tracked in-repo):
  `docker run --rm -v "$PWD:/workspace" -w /workspace --user
  "$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh`.
  If `python3: command not found` appears, the image is stale — rebuild
  it from the Dockerfile. Splat runs on the host (`.venv/bin/splat`).
- **Git-ignored build inputs that exist only locally** (never in the
  repo, must be provisioned per machine): `rom/image/`,
  `build/extracted/` (retail EXE), `tools/era/` (fetched by
  `scripts/setup_era.sh` around the tracked maspsx patch), `.venv/`.
- **Agent prompts:** hand work to agents using
  `docs/ai_context/PROMPT_TEMPLATE.md`. Named branches in the prompt,
  worktree path included, always.

## Grind-lane port complete — 287 matching C leaves (2026-08-21)

Current cross-lane status: see `~/dev/pe-continuous-decomp/RUNTIME_LANES.md`.

Current matching-lane status: 379 exact matching-C
leaves plus 24 `ACCEPTED-RESIDUAL` leaves; the 287-leaf line above is the
historical grind-lane port milestone. Residual policy:
`docs/acceptance/MATCHING_RESIDUAL_POLICY.md`.

Current native PC-port status (B54K-B6, 2026-08-30): `func_80030894` now
translates retail through the complete L9 group at
`0x80031110..0x800311EC` (55 new words; 598/788 total). The strict production
frontier is `func_80030894_L9_cut`, immediately before the next packet group,
then normal mode continues to `func_8006AD40_post30894_cut`. The independent
oracle verifies the full window, two already-native calls, sole four-entry
back-edge, and 97-byte bank-0 write map. Full normal and ASan/UBSan suites pass
938/938. Evidence: `docs/evidence/pe-b54kb6-30894-l9/REPORT.md`.

The remaining 190-word tail is partitioned read-only into four closed units:
L10 (77 words, `0x800311EC..0x80031320`), L11 (70 words,
`0x80031320..0x80031438`), outer close/final sprite (30 words,
`0x80031438..0x800314B0`), and the 13-word epilogue through `0x800314E4`.
All ten remaining static call sites target already-native helpers. Start with
L10; exact hashes and call/branch ownership are in
`docs/evidence/pe-b54kb-tail-partition/REPORT.md`. L10 is additionally
predecoded there as two fixed compound sprites plus a two-item array, with an
independent 74-byte bank-zero write-map oracle; this does not move the strict
frontier.

VIS1 remains available: a real Disc 1 run emits a deterministic, visibly
non-black read-only snapshot of the single PSX VRAM authority via
`--vram-raw` and `--vram-screenshot` (2,063 nonzero RGB555 words within
`256,64..735,456`). The B54K-B6 artifact is byte-identical to VIS1 because
L4-L9 only build guest packet state. This is diagnostic VRAM, not a rendered
320x240 frame; the legacy host framebuffer remains black. BTL151 remains a
human-operated local PCSX-Redux capture—do not retry it headlessly or claim
capture results without the GUI artifact.

**Function-hood screen rule (current):** a callable tiny span must end in a
canonical `jr ra`/delay slot **or** a provable tail jump into a shared function
body, and must have an exact-start caller/reference plus real boundaries. A
tail entry is recorded as `FUNCTION_HOOD=PROVEN_BY_TAIL_JUMP`; absence of
`jr ra` alone is not a padding classification.

**Address-retention screen rule (current):** exact symbolic-address retention
through a load/store or return delay slot is a no-attempt family screen once
the body shape is proven. Preserve historical parks and do not generalize the
rule to unrelated multi-access heuristic rows without body-level proof.

**Address-sharing pointer lever:** when retail materializes one symbolic
address, loads through it, and then reuses that same adjusted address in the
result, spell an explicit pointer local and dereference it. `func_8005DADC`
proved this can recover shared `$v1` retention. This is distinct from the
parked scalar-exchange family, where the assembler-selected store form remains
the blocker.

**By-value aggregate ABI-home lever:** when retail compares incoming words
but also stores `$a0..$a3` to `0/4/8/12(sp)` without a frame, test two
two-word structs passed by value. `func_80073244` proves this recovers the
argument-home prefix exactly; its remaining low-word result fold is a
separate canonicalization blocker.

All remaining pe-continuous-decomp grind leaves are ported. After 29388
(below): `func_800293F4` (0x19BF4, 124w, era `-O2 -G8` +
`MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2E8` — new era_compile knob that
strips a symbol's sdata `.extern` so its RMW stays absolute),
`func_8002F76C` (0x1FF6C, 27w, `-O2 -G0`, tail-of-19DE4 carve, no
resume), and the writer trio `func_8002FA10`/`FAA4`/`FAD8`
(0x20210/0x202A4/0x202D8, `-O2 -G0`, head-of-20210 carve, resume
`202F8.s` 0xA3C). Each landed as one commit with a fresh in-container
EXACT SHA-1 build; evidence under `docs/evidence/func-800293F4/`,
`func-8002F76C/`, `func-8002FA10-FAA4-FAD8/`. Branch
`leaves/from-grind-20260821`, pushed to origin.

**Lanes merged (2026-08-21):** `leaves/from-grind-20260821` merged into
`grind/continuous-decomp` (309-commit divergence). Leaf-file conflicts
resolved to the leaves lane (its `build_us.sh`/`disc1.yaml` are
authoritative); grind's `pc_port/` and PE-BTL evidence carried over
untouched. Gates re-run on the merged tree: split `c: 275`, docker build
EXACT SHA-1, `verify_us.sh` EXACT MATCH. The grind lane's pre-merge
handoff narrative (native/PE-BTL state) is preserved at commit
`29fe11b:docs/ai_context/ACTIVE_HANDOFF.md`. PR toward `main` carries
the unified 275 story; deeper doc reconciliation can follow on `main`.

## func_800125E0 — descriptor spawn loop matching C (35 words)

**276 matching C leaves.** `src/func_800125E0.c` matches era `-O2 -G8`,
VRAM `0x800125E0` / file `0x2DE0` / size `0x8C`. DrawSync(0), then walk
`**D_8009CE04` (`lbu` count, two-byte descriptors from offset 1) calling
`func_80035038(desc, 0, 1)`. A local list pointer keeps the header in `$a0`
(retail gp-load carry). Pins hold `$s0=count` / `$s1=offset`; an unused
`int` with an empty m-constraint supplies the retail `vars=8` / frame
`0x28` and emits no instructions. Mid-`2D74` carve: prefix `0x6C`, C
`0x8C`, resume `2E6C.s` `0x5830`. Evidence:
`docs/evidence/func-800125E0/REPORT.md`.

## func_80012574 — parked non-exact relocation leaf

Three bounded attempts on `func_80012574` (file `0x2D74`, 27 words) produced
a size-correct candidate with the retail unsigned tag test, empty 8-byte
frame, and loop cursor form, but retained a four-byte count/sum register
allocation mismatch (`$v0`/`$a2` instead of retail `$v1`/`$v0`). The leaf
stays parked and must not be retried without explicit authorization.
Evidence: `docs/evidence/func-80012574/REPORT.md`. The next unmatched
head after 125E0 is `func_8001266C` (file `0x2E6C`, 37 words).

## func_80029388 — slot-table clear + record-init wrapper matching C (27 words)

**270 matching C leaves (now 275, see above).** `src/func_80029388.c` matches era `-O2 -G8` +
`MASPSX_THREE_WORD_SYMBOL_STORE=1`, VRAM `0x80029388` / file `0x19B88` /
size `0x6C`. jal 2F658, 7×220B `SlotRecord` in-use clear (2F9CC shape,
andi-FILLED back-branch slot), gp byte zeros D_8009D2A0/D_8009D2EC, jal
20EFC. Mid-`11718` carve: prefix 0x8470, C 0x6C, resume `19BF4.s` 0x63E4.
Evidence: `docs/evidence/func-80029388/REPORT.md`.

**Build environment note:** the `pe-mipsel-img` docker image was rebuilt
from `dev/mipsel/Dockerfile` (2026-08-21) — the stale image lacked
`python3` and aborted `build_us.sh` at the maspsx step; a prior agent then
hashed a stale candidate and committed a false match (dropped via
`git reset --hard`). `build_us.sh` now deletes `build/disc1.candidate.exe`
at start so a stale artifact can never pass for a fresh build. Run builds
as: `docker run --rm -v "$PWD:/workspace" -w /workspace --user
"$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh`.

## func_8005288C — return-zero stub matching C (2 words)

`src/func_8005288C.c` matches era `-O2 -G0`, VRAM `0x8005288C` / file
`0x4308C` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-8005288c/REPORT.md`.

## func_800CA7B0 — return-zero stub twin matching C (2 words)

`src/func_800CA7B0.c` matches era `-O2 -G0`, VRAM `0x800CA7B0` / file
`0xBAFB0` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800ca7b0/REPORT.md`.

## func_800CA7A8 — return-zero stub matching C (2 words)

`src/func_800CA7A8.c` matches era `-O2 -G0`, VRAM `0x800CA7A8` / file
`0xBAFA8` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800ca7a8/REPORT.md`.

## func_800C9C18 — return-zero stub twin matching C (2 words)

`src/func_800C9C18.c` matches era `-O2 -G0`, VRAM `0x800C9C18` / file
`0xBA418` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800c9c18/REPORT.md`.

## func_800C9C10 — return-zero stub matching C (2 words)

`src/func_800C9C10.c` matches era `-O2 -G0`, VRAM `0x800C9C10` / file
`0xBA410` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800c9c10/REPORT.md`.

## func_800CD978 — return-zero stub twin matching C (2 words)

`src/func_800CD978.c` matches era `-O2 -G0`, VRAM `0x800CD978` / file
`0xBE178` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800cd978/REPORT.md`.

## func_800CD970 — return-zero stub matching C (2 words)

`src/func_800CD970.c` matches era `-O2 -G0`, VRAM `0x800CD970` / file
`0xBE170` / size `0x8`. It returns 0. Evidence:
`docs/evidence/func-800cd970/REPORT.md`.

## func_80018F0C — five-reader call wrapper matching C (18 words)

`src/func_80018F0C.c` matches era `-O2 -G0`, VRAM `0x80018F0C` / file
`0x970C` / size `0x48`. It forwards five unsigned-halfword reader values to
`func_80066BD8` and returns 1. Evidence:
`docs/evidence/func-80018f0c/REPORT.md`.

## func_80018F54 — D_800BCFEE bit-0x40 clearer matching C (8 words)

`src/func_80018F54.c` matches era `-O2 -G0`, VRAM `0x80018F54` / file
`0x9754` / size `0x20`. It clears bit `0x40` in `D_800BCFEE` and returns 1.
Evidence: `docs/evidence/func-80018f54/REPORT.md`.

## func_80018EE0 — unsigned-halfword reader wrapper twin matching C (11 words)

`src/func_80018EE0.c` matches era `-O2 -G0`, VRAM `0x80018EE0` / file
`0x96E0` / size `0x2C`. It forwards an unsigned halfword from its reader to
`func_80066C7C` and returns 1. Evidence:
`docs/evidence/func-80018ee0/REPORT.md`.

## func_80018EB4 — unsigned-halfword reader wrapper matching C (11 words)

`src/func_80018EB4.c` matches era `-O2 -G0`, VRAM `0x80018EB4` / file
`0x96B4` / size `0x2C`. It forwards an unsigned halfword from its reader to
`func_80066B60` and returns 1. Evidence:
`docs/evidence/func-80018eb4/REPORT.md`.

## func_80018E58 — one-reader call wrapper matching C (11 words)

`src/func_80018E58.c` matches era `-O2 -G0`, VRAM `0x80018E58` / file
`0x9658` / size `0x2C`. It forwards a dereferenced reader value to
`func_80066800` and returns 1. Evidence:
`docs/evidence/func-80018e58/REPORT.md`.

## func_80018D20 — complemented two-reader call wrapper matching C (12 words)

`src/func_80018D20.c` matches era `-O2 -G0`, VRAM `0x80018D20` / file
`0x9520` / size `0x30`. It forwards a dereferenced reader and the complement
of the second to `func_80065A9C`, then returns 1. Evidence:
`docs/evidence/func-80018d20/REPORT.md`.

## func_80018CF0 — two-reader call wrapper matching C (12 words)

`src/func_80018CF0.c` matches era `-O2 -G0`, VRAM `0x80018CF0` / file
`0x94F0` / size `0x30`. It forwards the two dereferenced reader values to
`func_80065A9C` and returns 1. Evidence:
`docs/evidence/func-80018cf0/REPORT.md`.

## func_80018CB8 — three-reader call wrapper matching C (14 words)

`src/func_80018CB8.c` matches era `-O2 -G0`, VRAM `0x80018CB8` / file
`0x94B8` / size `0x38`. It forwards three dereferenced reader values to
`func_80065A60` and returns 1. Evidence:
`docs/evidence/func-80018cb8/REPORT.md`.

## func_80018C88 — two-reader call wrapper matching C (12 words)

`src/func_80018C88.c` matches era `-O2 -G0`, VRAM `0x80018C88` / file
`0x9488` / size `0x30`. It forwards the two dereferenced reader values to
`func_800659F8` and returns 1. Evidence:
`docs/evidence/func-80018c88/REPORT.md`.

## func_80018C58 — two-reader call wrapper matching C (12 words)

`src/func_80018C58.c` matches era `-O2 -G0`, VRAM `0x80018C58` / file
`0x9458` / size `0x30`. It forwards the two dereferenced reader values to
`func_800659C8` and returns 1. Evidence:
`docs/evidence/func-80018c58/REPORT.md`.

## func_80018BEC — D_8009D2F0 flag-0x20 setter matching C (9 words)

`src/func_80018BEC.c` matches era `-O2 -G0`, VRAM
`0x80018BEC` / file `0x93EC` / size `0x24`. Evidence:
`docs/evidence/func-80018bec/REPORT.md`.

## func_80018BC8 — D_8009D2F0 flag-0x20 clearer matching C (9 words)

`src/func_80018BC8.c` matches era `-O2 -G0`, VRAM
`0x80018BC8` / file `0x93C8` / size `0x24`. It clears bit `0x20` in offset
`0x98` of `D_8009D2F0` and returns 1. Evidence:
`docs/evidence/func-80018bc8/REPORT.md`.

## func_80018B68 — two-reader call wrapper twin matching C (12 words)

`src/func_80018B68.c` matches era `-O2 -G0`, VRAM
`0x80018B68` / file `0x9368` / size `0x30`. It forwards two nested reader
values to `func_8006590C` then returns 1. Evidence:
`docs/evidence/func-80018b68/REPORT.md`.

## func_80018B00 — two-reader call wrapper matching C (12 words)

`src/func_80018B00.c` matches era `-O2 -G0`, VRAM `0x80018B00` / file
`0x9300` / size `0x30`. It forwards two nested reader values to
`func_80067678` then returns 1. Evidence: `docs/evidence/func-80018b00/REPORT.md`.

## func_800182E0 — D_8009D2F0 offset-0x20 reader commit matching C (8 words)

`src/func_800182E0.c` matches era `-O2 -G0`, VRAM `0x800182E0` / file
`0x8AE0` / size `0x20`. It stores the nested reader value at offset `0x20` of
`D_8009D2F0` and returns 1. Evidence: `docs/evidence/func-800182e0/REPORT.md`.

## func_800182A0 — D_800BCF88 bits-0xC0 clearer matching C (8 words)

`src/func_800182A0.c` matches era `-O2 -G0`, VRAM `0x800182A0` / file
`0x8AA0` / size `0x20`. It clears `0xC0` from `D_800BCF88` and returns 1.
Evidence: `docs/evidence/func-800182a0/REPORT.md`.

## func_800182C0 — D_800BCF88 bits-0xC0 setter matching C (8 words)

`src/func_800182C0.c` matches era `-O2 -G0`, VRAM
`0x800182C0` / file `0x8AC0` / size `0x20`. It ORs `0xC0` into
`D_800BCF88` and returns 1. Evidence: `docs/evidence/func-800182c0/REPORT.md`.

## func_80017FB0 — D_8009D1A0 dynamic bit clearer matching C (11 words)

`src/func_80017FB0.c` matches era `-O2 -G0`, VRAM `0x80017FB0` / file
`0x87B0` / size `0x2C`. It clears a nested reader mask from `D_8009D1A0` and
returns 1. Evidence: `docs/evidence/func-80017fb0/REPORT.md`.

## func_80017F88 — D_8009D1A0 dynamic bit setter matching C (10 words)

`src/func_80017F88.c` matches era `-O2 -G0`, VRAM `0x80017F88` / file
`0x8788` / size `0x28`. It ORs a nested reader mask into `D_8009D1A0` and
returns 1. Evidence: `docs/evidence/func-80017f88/REPORT.md`.

## func_80017F20 — D_8009D2F0 flag clearer matching C (9 words)

`src/func_80017F20.c` matches era `-O2 -G0`, VRAM `0x80017F20` / file
`0x8720` / size `0x24`. It clears bit `0x100` in the offset-`0x98` field.
Evidence: `docs/evidence/func-80017f20/REPORT.md`.

## func_80017EFC — D_8009D2F0 flag setter matching C (9 words)

`src/func_80017EFC.c` matches era `-O2 -G0`, VRAM `0x80017EFC` / file
`0x86FC` / size `0x24`. It sets bit `0x100` in the offset-`0x98` field and
returns 1. Evidence: `docs/evidence/func-80017efc/REPORT.md`.

## func_80017EA4 — reader result commit matching C (8 words)

`src/func_80017EA4.c` matches era `-O2 -G0`, VRAM `0x80017EA4` / file
`0x86A4` / size `0x20`. It commits the nested reader value to offset `0x1C`
of `D_8009D2F0` and returns 1. Evidence: `docs/evidence/func-80017ea4/REPORT.md`.

## func_8005184C — dynamic bit setter matching C (8 words)

`src/func_8005184C.c` matches era `-O2 -G0`, VRAM `0x8005184C` / file
`0x4204C` / size `0x20`. The `$v0` pointer and `$v1` mask pins reproduce
retail's address/mask allocation and store delay slot. Evidence:
`docs/evidence/func-8005184c/REPORT.md`.

## func_80033A2C — D_8009D244 byte-flag setter matching C (5 words)

`src/func_80033A2C.c` matches era `-O2 -G0`, VRAM `0x80033A2C` / file
`0x2422C` / size `0x14`; `2422C.s` now resumes at `24240.s`. Evidence:
`docs/evidence/func-80033a2c/REPORT.md`.

## func_80037140 — packet setup/submit twin matching C (25 words)

`src/func_80037140.c` matches era `-O2 -G0`, VRAM `0x80037140` / file
`0x27940` / size `0x64`. It is the 370DC wrapper twin with the `77C44` packet
configuration call; its full span is now C. Evidence:
`docs/evidence/func-80037140/REPORT.md`.

## func_800370DC — packet setup/submit wrapper matching C (25 words)

`src/func_800370DC.c` matches era `-O2 -G0`, VRAM `0x800370DC` / file
`0x278DC` / size `0x64`. It initializes a packet, configures the `+8` member,
submits it, and reports `-1` on error. `278BC.s` resumes at `27940.s`.
Evidence: `docs/evidence/func-800370dc/REPORT.md`.

## func_800370A8 — fixed-point quotient helper matching C (5 words)

`src/func_800370A8.c` matches era `-O2 -G0`: `sra; div; mflo; jr; sll`.
VRAM `0x800370A8` / file `0x278A8` / size `0x14`. `26C48.s` now resumes at
`278BC.s`; the full Docker rebuild is the exact target SHA-1 with 236 leaves.
Evidence: `docs/evidence/func-800370a8/REPORT.md`.

## func_800124F8 — boot-table clear leaf matching C (31 words)

`src/func_800124F8.c` matches byte-exact on era `-O2 -G8`. VRAM
`0x800124F8` / file `0x2CF8` / size `0x7C`. The leaf clears the
`D_8009D310` 72×11 work table, the `D_8009DF70` 16-word table, and the
gp-relative state fields using the retail pointer and delay-slot loop shape.
The former `2A0C.s` chunk is split at the leaf and resumes at `2D74.s`.
`scripts/build_us.sh` and `scripts/verify_us.sh` report **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with 235 leaves. Evidence:
`docs/evidence/func-800124f8/REPORT.md`.

## func_800305C8 — angle-wrap helper matching C (30 words)

`src/func_800305C8.c` matches byte-exact on era `-O2 -G0`. VRAM
`0x800305C8` / file `0x20DC8` / size `0x78`. The non-leaf uses a 0x18-byte
frame, preserves the second record pointer in `$s0`, calls `func_80079FB4`,
then performs the signed-i16 truncation and `+0xFFF` negative wrap shown by
the retail branch. `$v0`/`$v1` register pins preserve the exact allocation.
`scripts/build_us.sh` and `scripts/verify_us.sh` report **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with 234 leaves. The split resumes
at `20E40.s` for `func_80030640`. Evidence:
`docs/evidence/func-800305c8/REPORT.md`.

## func_80030584 — angle helper matching C (17 words)

`src/func_80030584.c` matches byte-exact on era `-O2 -G0`. VRAM
`0x80030584` / file `0x20D84` / size `0x44`. ratan2 of two `lh<<16`
vs `a1[0]`/`a1[2]`, then `+2048` as i16. Head of former `20D84.s`;
resume `20DC8.s` `0x78`. `scripts/build_us.sh` **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Evidence: `docs/evidence/func-80030584/REPORT.md`.

## func_8002F7D8 — 0x6F body create matching C (102 words)

`src/func_8002F7D8.c` matches byte-exact on era `-O2 -G0` +
`MASPSX_THREE_WORD_SYMBOL_STORE=1`. VRAM `0x8002F7D8` / file `0x1FFD8` /
size `0x198`. Both 216-byte copies are gcc aligned `Body216` block
moves: four `lw` `$v0/$v1/$a0/$a1`, four `sw`, `addiu` 0x10 in the
`bne` delay, 2-word tail. First-cut `dst[i]=src[i]` unrolls miss that
shape. Tail of 11718: prefix `0xE8C0`, C `0x198`, then existing 2F970.
`scripts/build_us.sh` **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Evidence: `docs/evidence/func-8002F7D8/REPORT.md`.

## func_80030534 — 2D distance helper matching C (20 words)

`src/func_80030534.c` matches byte-exact on era `-O2 -G0` + maspsx
`--aspsx-version=2.30`. VRAM `0x80030534` / file `0x20D34` / size `0x50`.
The leftover nop sits between the second `subu` and `mult` because
ASPSX ≥ 2.30 requires two instructions between `mflo` and the next
`mult` (`nop_mflo_mfhi`). 2.21 omits it. Mid-20210 carve: prefix `0xB24`,
C `0x50`, resume `20D84.s` `0xBC`. `scripts/build_us.sh` **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Evidence: `docs/evidence/func-80030534/REPORT.md`.

## func_80030640 — RNG gate matching C (40 words)

`src/func_80030640.c` matches byte-exact on era `-O2 -G0`. VRAM
`0x80030640` / file `0x20E40` / size `0xA0`. Mid-20210 carve: prefix
`0xC30`, C `0xA0`, resume `20EE0.s`. `lui $v1,1` is bit 16 (`0x10000`),
not `andi 1`. Second `D_8009D278` load is `$v1` because signed `%100`
clobbers `$a0`. `scripts/build_us.sh` **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Evidence: `docs/evidence/func-80030640/REPORT.md`.

## Current state

| Fact | Value | Derive |
| --- | --- | --- |
| Branch / tip | `phase5fm-main-barrier-revisit` @ 5FU-17EA4 | `git branch --show-current` / `git status --short` |
| Phase | **5FU-17EA4 / 241 exact leaves** (six nearby exact leaves added after 5FM; parked candidates remain untouched) | `scripts/verify_us.sh` summary + exact rebuild |
| Matching C leaves | **241** (non-integrated candidates: parked src/func_800698D4.c / func_8001220C.c / func_800725DC.c + IN-PROGRESS src/func_8006A9E4.c) | `grep -c ',\s*c,' configs/USA/disc1.yaml` |
| Yaml asm segments | **153** | `grep -c ',\s*asm\]' configs/USA/disc1.yaml` |
| Era leaf compiles | **83** | `grep -c '^era_compile \|^\w*=1 era_compile ' scripts/build_us.sh` |
| Target SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` | `scripts/build_us.sh` compare |
| Progress | https://blizz127.github.io/parasite-eve-progress/ | `scripts/publish_progress.sh` |

**Yaml `asm` segments are not remaining functions.** One segment can hold
dozens of glabels; do not subtract it from anything as a function count.

Oracle: bare `scripts/build_us.sh` exits 0 on exact SHA-1; `scripts/verify_us.sh`
reports Phase 5FU-17EA4 / 241. Disc images / `asm/` / `build/` / `tools/era/`
are git-ignored inputs — never commit them.

**Toolchain**

- Default leaves: GCC 14.2 in Distrobox `pe-mipsel` (Phase 4J flags; selective
  `-G 8` / `-fno-delayed-branch` / `-fno-tree-ter`).
- Era leaves (opt-in): `scripts/setup_era.sh` → `era_compile` =
  cpp → cc1 → maspsx → GNU as, typically `-O2 -G0` (some leaves `-O1 -G0`).
- Era maspsx: `ERA_ASPSX_VER=2.21` + `--dont-expand-li`. **Why:**
  `expand_load_immediate` turns positive small `li` into `ori`; ROM wants
  `addiu`. Defer `li` expansion to GNU as. Same config also preserves
  large-literal `lui;ori` (cc1 emits PSY-Q `li` high + `ori` low natively).
  Do **not** bump aspsx-version casually — that also flips `nop_at_expansion`
  / `addiu_at`.
- **Vendored maspsx LOCAL PATCH:** `tools/era/maspsx/maspsx/__init__.py` is
  repo-tracked (`.gitignore` negations; `setup_era.sh` re-clones upstream
  AROUND it, restores the tracked file from git if absent). Patch 1 =
  **sw-store delay-slot fill**, opt-in per `era_compile` line via env
  `MASPSX_FILL_STORE_DELAY_SLOT=1`: an absolute `sw $r,SYM` macro immediately
  before a bare `j $31` is emitted as `lui $at,%hi` / `j $31` /
  `sw $r,%lo($at)`. sw only — sb/sh macro stores and multi-store epilogues
  are ROM-proven to stay pre-jr with a nop slot (e.g. func_8003FFAC vs
  func_8007FBC0: identical C shape, different ROM scheduling — the original
  units were assembled with different ASPSX scheduling).
- Patch 2 (`f0b9155`) / Patch 3 (`439c244`): **three-word indexed symbolic
  store AND load expansion**, opt-in per leaf via `MASPSX_THREE_WORD_SYMBOL_STORE=1`.
  Standalone `op $r,SYMBOL($index)` uses the retail/ASPSX-2.30-shaped
  `lui $at,%hi` / `addu $at,$at,$index` / `op $r,%lo($at)` sequence,
  for stores (2) and standalone indexed symbolic loads (3: lb/lbu/lh/lhu/lw/lwl/lwr;
  `lwc2` stays outside, durable negative test). Compound semicolon lines retain the
  2.21 four-word expansion. Flag-off rebuild is the exact leaf-count retail SHA.
  Full 224-leaf regression (flag OFF and flag ON over the three existing 3W store
  leaves) both exact; 153 vendored tests; re-clone restore byte-identical.
- Maspsx stdin: closed with `</dev/null` in `era_compile` (non-TTY hang under
  agent sockets). Bare `scripts/build_us.sh` is fine.

## How to count (do not hand-maintain)

```bash
grep -c ',\s*c,' configs/USA/disc1.yaml            # C leaves
grep -c ',\s*asm\]' configs/USA/disc1.yaml          # yaml asm segments (NOT fn count)
grep -c '^era_compile \|^\w*=1 era_compile ' scripts/build_us.sh  # era leaf compiles
git log --oneline -1
```

**Do not** count `asm/disc1/*.s` from disk. That tree is git-ignored and
contains orphans, stale duplicates, and nop-pads. **Yaml is the source of truth.**
Known stale orphans (counter ignore-list): `2E7D0.s` (superseded by live
`2E7D8.s`) and `807C.s` (stale duplicate of live `2A0C.s`; unreferenced in
yaml). Keep both out of function scans.

**asm/ sync invariant:** `$at` family totals from
`tools/analysis/at_absolute_store_counter.py` hard-fail (no SUMMARY) when asm/
is missing units or still holds glabels for yaml C leaves. Re-split with
`scripts/split_us.sh` before planning off a family count. Leaf count stays
yaml-only and still works when asm/ is stale.

## Proven era fingerprints (evidence, not claims)

| Fingerprint | Status |
| --- | --- |
| `move` → `addu` in delay slot | Proven 5EA / 5EB / 5EC / 5ED |
| `$v0` / `$v1` allocation | Proven 5EC / 5ED (sb+ret0 reuse) |
| `li` const materialization (`addiu` not `ori`) | Proven 5EC via `--dont-expand-li` |
| `$at` absolute `sw` macro expansion | Proven by scratch probe; integrated exact in 5EE |
| Branch delay-slot constant hoist (`beqz` slot) | **PROVEN** (5EG-first-branch): era cc1 `-O1 -G 8` reproduces the retail schedule on `func_8004F448` word-for-word |
| Test-and-clear-return if/else (`bnez` + j-over) | **PROVEN, VOLUME** (5ER): era `-O2 -G0` matches the adjacent byte/word twins `func_80038D1C` / `func_80038D48` — shared address in `$v1`, `addu $v0,$zero,$zero` in the `bnez` slot, `addiu $v0,$zero,0xFF` in the unconditional-jump slot, then `sb`/`sw` clear. Direct-global C rebuilt the address and used a 12-word `beq` form; one natural explicit-pointer phrasing retry matched all 11 words without pinning |
| `$a0`-in/`$v0`-out + redundant double store | **PROVEN** (5EH): era `-O2 -G8` preserves both stores + `addu` return-0 on `func_800438C0`; GCC 14.2 `-O1` merges stores and emits `move` — **era required for value-returning leaves**; era+gp `-G8` first proven here |
| Non-leaf stack frame + `jal` | **PROVEN** (5EI; repeated as volume in 5EK): era matches the `func_800197D0` / `func_800197F0` void-callee twins — `addiu $sp,-0x18` / `sw $ra,0x10($sp)` / `jal`+nop / `lw $ra` / `addiu $v0,1` / `jr $ra` with the `addiu $sp,+0x18` teardown **in the `jr` delay slot**, word-exact; 197F0 uses `-O2 -G0` and adds no primitive |
| Outgoing `$a0` + `jal` after double dereference | **PROVEN** (5EJ-outgoing-arg): era `-O2 -G0` on `func_80019484(int **)` emits `lw $v0,0($a0)` / load-delay nop / `lw $a0,0($v0)` / `jal func_800438C0` + nop, then the proven return-1 frame teardown shape; all 11 words exact |
| Return-forwarded `$v0` + teardown-before-`jr` epilogue | **PROVEN** (5EL-return-forwarding): era `-O2 -G0` on `func_8007F7A8` emits the frame + `jal func_8007FCAC` + nop, forwards `$v0` untouched, then `lw $ra`; `addiu $sp,+0x18`; `jr $ra`; nop. Era reproduces this per-function schedule as well as 197D0/F0's opposite teardown-in-slot schedule |
| Straight-line boot pointer-layout scheduling | **PROVEN, COMPILER-CONSTRAINED C** (5EM-boot-6a8d4): era `-O2 -G0` matches all 68 words / 19 absolute pointer stores in retail order. Both the initial plain-local source and one retail-order retry allocate cursors to `$a0/$a1`, constants to `$v0/$v1`, and sink `D_800B0E28` past `D_800B0E2C/E30`. The exact fallback therefore uses the established explicit-register convention (`$v0/$v1` cursors, `$a0/$a1` constants); it is target-specific matching C, not portable natural C |
| Empty-asm scheduling barrier for materialization placement | **PROVEN (5FJ `func_8006E9A0`)**: era `-O2`'s pre-RA scheduler sank a callee-saved `$s2 = &SYM` lui/addiu pair past a `jal` (source position = retail words 29-30; cc1 put it after the first post-arena call); `asm volatile("" : : "r"(reg) : "memory")` right after the assignment pins the pair to retail position, emits no code, and the leaf matches 141/141. `-fno-schedule-insns` is NOT the lever (double materialization, breaks arena scheduling) |
| Paired register pins + `"=r":"0"` zero-code barrier for call-result home | **PROVEN (5FK `func_8006E834`)**: retail keeps a `$v1` backup of a call result across a range test and restores it to `$v0` for the equality tests; natural C makes cc1 coalesce the tests onto `$v1` and drop both restore copies (90/91). Pinning `register int t asm("$3")` (backup) and `register int rt asm("$2")` (test home) plus empty `asm volatile("" : "=r"(x) : "0"(x))` barriers (emit nothing; block copy-prop folding) restores the retail shape, and reorg threads the idempotent merge copy into the `beqz` delay slot — 91/91. Single-register pins alone (V1/V3) are inert; pinning only `$v0` without the decoupling barrier leaves the tests on `$v1` |
| Counting-loop back-edge scheduling | **PROVEN; VOLUME-ELIGIBLE** (5EN/5EP `func_8006A674` probe): era `-O2 -G0` puts pointer advances in all five retail back-branch delay slots — `bnez` up-counters (`$a0+4`, `$v1+2`, `$a1+8`) and `bgez` down-counters (`$a3-4`, `$v0-4`) — and preserves the final store in the `jr` delay slot. The leaf remains parked for unrelated constant-hoist scheduling; the loop primitive passed. |
| Natural counting loop in volume | **PROVEN, VOLUME** (5ES `func_8004BF08`): era `-O2 -G0` matches a natural pointer-walk loop over parallel signed `int[8]` arrays in all 14 words, with no pins or maspsx opt-in. Explicit initialization in retail order (`i`, first pointer, second pointer) plus `do/while` phrasing gives `$a1/$a0/$v1` allocation; the first pointer advances before the bound test and the second pointer advances in the backward `bnez` delay slot. The declaration-initialized `for` form was semantically correct but allocated the three live values differently. |
| Pure-register bit-serial loop in volume | **PROVEN, VOLUME** (5ET `func_8005186C`): era `-O2 -G0` matches all 15 words on the first natural-C try — no loads/stores, calls, or `$gp`; explicit-init `do/while`; the unconditional `result <<= 1` fills the forward `bnez` skip-branch delay slot, the `bgez` back-edge keeps a nop slot, and the return lands as `addu $v0,$a1,$zero` in the `jr` delay slot |
| Indexed global-array store/load expansion | **PROVEN, TOOL-SOLVED** (`f0b9155` stores; `439c244` loads): per-leaf `MASPSX_THREE_WORD_SYMBOL_STORE=1` reproduces `lui` / indexed `addu` / op `%lo` and removed the extra L3 word in `func_8006A674` (153→152 words). `439c244` extends the gate to standalone indexed symbolic LOADS (all seven widths; `lwc2` stays outside — durable negative test; compound lines retain the 4-word expansion). Default off is byte-identical. |
| `lui;ori` large-literal synthesis | **PROVEN** (capability probe): both bit15-clear and bit15-set; cc1 emits PSY-Q `li` high + `ori` low; ROM-exact under 2.21 + `--dont-expand-li` |
| Rotated/peeled loop idiom | **PROVEN SHAPE** (5EV `func_80052BCC`, leaf parked on unrelated allocation): write the first iteration explicitly, then `while (cond) { body }` → era `-O2 -G0` emits the rotated shape: `beq`-exit head, bottom-tested `bne` back-edge, pointer advance in both delay slots |
| Signed `char` vs 0xFF-range constant | **PROVEN SHAPE** (5EV `func_80052BCC`, same parked leaf): signed `char c` compared against `0xFF` emits the conversion `andi` on the compare path even after `lbu`; `unsigned char` does not. Typing controls the mask |
| Return-accumulator vs direct-return phrasing | **PROVEN (5FH `func_80037548`)**: a search loop with a default return value must hold the result in an ACCUMULATOR (`signed char result = 0; ... result = v; break; return result;`). Direct `return v;` on the match path makes cc1 emit a SEPARATE `addu $v0,$zero,$zero` default path before `jr` (28 words vs ROM's 27) — the accumulator keeps one `$a2` merge with the `sll/sra` sign-extension pair hoisted to the merged exit |
| Guard-branch delay-slot result via accumulator | **PROVEN, VOLUME (`func_800631C0`)**: when retail shows `beqz` with a value-setting instruction in the delay slot (`addu v0,zero,zero`), an explicit result accumulator plus guarded assignment reproduces it; an early-return-null phrasing leaves `nop`. Reach for the accumulator before spending a second phrasing. |
| One-base volatile MMIO pointer | **PROVEN, VOLUME (`func_80087728`)**: direct fixed-address C stores materialize separate pointers and overflow the body; one volatile base pointer with halfword indices reproduces retail's single `$at` base and two offsets. Use this first for the remaining `0x877xx` hardware setter cluster. |
| `-fschedule-insns2` load-delay `li` hoist | **PROVEN, FIRST LEAF** (5EW `func_80052BCC`, era `-O1 -G0 -fschedule-insns2`): the post-allocation scheduler hoists an independent `li` above `sb`/`andi` into the `lbu` delay — the exact spot retail's ccpsx scheduled it. At plain `-O1` the same `li` emits after the `andi` (14/15). Paired phrasing: two `0xFF` consts of different modes (u8 head const dies at the guard → loop re-materializes into the freed `$v1`; `int` loop byte → mask-free raw `bne`); comparing the loop byte against a *variable* or both consts sharing a mode cross-jumps/CSE-shares head and loop |
| sched2 scope (negative result) | **NARROWED (5EY `func_8003E610`)**: `-fschedule-insns2` is NOT a universal retail fingerprint — it governs **store-adjacent `li`/`addiu` placement and load-delay hoists** only (52BCC head-`li`, 6A674's 21 order swaps). Straight-line `jal`-arg scheduling (`$a0` hoisted + `$a1` in slot for two-arg calls; `$a0` slot-filled single-arg; nop slot no-arg) is already correct at plain `-O2`. Do NOT flip sched2 into the era default |
| dbr_sched `$v0`-steal screening rule | **CHARACTERIZED (5FB `func_800698D4`, PARKED)**: a `beqz`/`beq` whose delay-slot steal candidate is a `$v0`-setter gets the fill when the branch target hits a `jal` immediately (kills `$v0`), but retail DECLINES the steal when the target is the return-computation block (`$v0` live to `jr $ra`) — our cc1 steals anyway. Screening rule: nop slot + `$v0`-constant load on fall-through + branch to a RETURN block → expect divergence; same pattern to a `jal`-adjacent block → matches. reorg.c liveness skew (ccpsx vs 2.7.2-psx), not source-expressible |
| Nested-if defeats range-test collapse | **PROVEN IDIOM (5FB `func_800698D4`)**: `v != 0 && v != -1` folds to `addiu $v0,$v0,1; sltiu $v0,$v0,2; bnez` under era `-O2` (range test, not retail's shape). Two nested `if`s keep the separate `beqz`/`beq` compares. -O1 keeps compares but flattens other structure |
| Frame-size arithmetic for struct locals | **PROVEN (5FB `func_800698D4`)**: size opaque locals from the frame, not the type's rounded size — DsSearchFile's CdlFILE local is `0x18` (pos 4 + size 4 + name 16): `0x10` args + `0x18` local + `$s0` + `$ra` = frame `0x30`. A `0x20` local emits frame `0x38` and fails at word 0 |
| Five-arg call (o32 stack arg) | **PROVEN, FIRST LEAF** (5FC `func_8006E834`; leaf integrated exact in 5FK): the 5th argument emits `sw $v0,0x10($sp)` in the `jal`'s delay slot — plain C `f(a,b,c,d,e)` with an immediate 5th arg, era `-O2 -G0`, worked first try. `sb $v0,0x29($sp)` (struct byte field) also lands in a `jal` slot |
| Frame decomposition before writing | **PROVEN METHOD (5FB/5FC)**: decompose the frame BEFORE choosing local sizes — `args + locals + saves + pad = frame` must be exact (5FB: CdlFILE `0x18` not `0x20`; 5FC: args `0x18` + env `0x18` + local30 `0x8` + regs `0xC` + pad `0x4` = `0x48`, byte field lands at `env[0x11]` = `0x29($sp)`). Wrong local size fails at word 0 |
| Aggregate element type as addressing-mode lever | **PROVEN (5FD `func_8002F9CC`)**: for an indexed store into a symbol array, declaring the real aggregate element (`SlotRecord D_800A5D58[]`, `arr[i].field = 0`) makes cc1 emit the standalone indexed symbolic store (`sw $0,SYM($3)`) at plain `-O2` — flat `arr[i*55] = 0` instead hoists `la $5,SYM` out of the loop (invariant under `-O2`/`-O1`/`-O1 -fschedule-insns2`; an addressing choice, not scheduling). With the symbol store present, `MASPSX_THREE_WORD_SYMBOL_STORE=1` passes it to GNU as for retail's 3-word `lui $at / addu / sw %lo($at)` form. Also: a lone symbol materialization is NOT an `-O1` signal — the `-O1` lever is for *repeated* constant/address materialization |
| `-O1` per-use constant materialization — SELECTION RULE | **PREDICTIVE (three leaves)**: if ROM materializes the same constant/address more than once, try `-O1` FIRST. `-O2`'s shared hoist runs through the hardwired `optimize>1` path (not flag-reachable); `-O1` re-materializes per use. 6A674 (discovered: per-use `-1`), 6A5BC (applied: `$s0=1` twice), 3E680 (predicted from five per-store `lui`s with a shared `0x8009` high half retail didn't CSE) |
| Return-use readiness of asm callees | **VALIDATED (5EZ `func_8006A5BC`)**: a caller may USE a still-asm callee's return and stay matchable when the use is a **raw full-width compare** (`beq $v0,$s0`, no mask/sign-extend) or a **bare store** (`sh $v0`). Both are codegen-determined regardless of the callee's true return type, so `int f(void)` externs suffice. Extends the 5EY rule (immediates-only args, returns ignored) |
| Fn-ptr arg to still-asm callee | **PROVEN, FIRST LEAF** (5FA `func_8003E680`): `f(func_8003E91C)` emits `lui $a0,%hi(sym)` / `addiu $a0,$a0,%lo(sym)` with R_MIPS_HI16/LO16 relocs against a same-segment TEXT symbol; the linker resolves it exactly like a data symbol. Declare `extern void g(void);` and pass the bare name |
| Unsigned loop-bound compare | **PROVEN (5FA `func_8003E680`)**: ROM `sltiu` (unsigned) vs cc1's `slt` for `int i < const` — declare the counter `unsigned int`. One-word type-driven fix, no flag involvement |
| Two-word `lui/addiu` zero from C | **PROVEN (5FN `func_800725DC` probe)**: no C zero spelling emits `lui/addiu` (ten forms probed, all `move`; a constant-0 loop bound deletes the whole loop at `-O2` AND `-O1`). The address expression `(int)SYM - BASE` compiles to `la $r,SYM+(0-BASE)` → `R_MIPS_HI16/LO16` with addend; when `SYM==BASE` the final words are `lui 0x0000 / addiu 0x0000` and the loop body STAYS ALIVE. The only known source-expressible origin for a baked two-word zero |
| No-args-area frame via asm call | **PROVEN (5FN `func_800725DC` probe)**: era cc1 reserves the 16-byte o32 outgoing-args area for EVERY C call form (direct/indexed-pointer/pinned-pointer all `args=16`). An inline-asm `jalr` (counter decrement as tied `"=r"/"0"` operand in the delay slot) is not a CALL insn → `.frame args=0`, frame = saves only. Diagnostic for retail frames smaller than saves+16 |
| Era prologue save order is fixed descending | **CHARACTERIZED (5FN)**: multi-`$s` prologue saves emit `$ra`-first / offsets top-down under `-O2`, `-O1`, `-fno-schedule-insns`, `-fschedule-insns2`, `-G8` alike — flag-invariant. PE1's matched/asm units are all descending or slot-interleaved; only the 0x800725xx SDK-runtime unit is contiguous-ascending (per-TU toolchain skew; see `per-tu-725dc`) |
| Carve sizes are boundary arithmetic, never object sizes | **PROVEN RULE (6E7E8 redo)**: a carve/prefix size = `resume − start` (0x5EFE8 − 0x5B1E4 = 0x3E04), NEVER the assembled object's padded size (16-align made the same prefix 0x3E10). Using the padded value shifts everything downstream +0xC and mimics a link-order symptom — the "PARKED-INTEGRATION-ERROR" behind the first 6E7E8 park |
| Inline range-test restores paired result-home moves | **PROVEN (6E7E8)**: with `int t = ret + 1; if ((unsigned)t < 2)`, cc1 coalesces ret+t into one `$v0` web → drops retail's paired `move a1,v0` / `move v0,a1` home moves (17/19 words). Writing the test INLINE (`if ((unsigned)(ret + 1) < 2)`) keeps ret as an accumulator and restores both moves pinlessly; a single hard-register pin on the temp also works but is unnecessary. Register pins need this class of shape matrix before they stand (6A8D4 bar) |

All four fingerprints from the original 5EA era claim are now proven in bytes.
The “~290 era-blocked functions” figure remains an **ESTIMATE**, not a countdown.

## Known-open families

- **sb+ret0:** **done** in 5ED (family closed).
- **`$at` absolute-store population:** counter committed
  (`tools/analysis/at_absolute_store_counter.py`). The historical integration
  inventory was **18 pre-jr** / 14 delay-slot / 5 sb-sh; the current yaml-live
  population is **0 pre-jr** / **0 delay-slot** / 5 sb-sh. Weak-int policy **NO**.
  - **Pinned by 5EG-readers:**
    - `D_8009D240` = `unsigned short *`, `D_8009D260` = `unsigned char *`
      via `func_8008AB1C` (era `-O1 -G0`).
    - `D_800A1870` = `void (*)(void)` via `func_80042B6C` (era `-O2 -G0`).
  - **Integrated:** `func_80085728`; 5EI readers-typed trio; 5EJ `D_8009D28C`
    int-state (4); 5EK `D_8009D270` unsigned flags (2); **5EF all 14
    delay-slot `sw` members**. The pilot `func_8007FBC0` plus the remaining 13
    typed leaves are integrated exact. Current leaf count **217**.
  - **Delay-slot shape: FAMILY CLOSED (5EF).** Vendored maspsx LOCAL PATCH
    (`MASPSX_FILL_STORE_DELAY_SLOT=1`) fills the `j $31` slot with the trailing
    absolute `sw`. Pilot gate exact + objdump-probed (`3C01800A 03E00008
    AC2436A0`). The remaining 13 members now have per-global typing evidence,
    and all 14 members pass the full exact-match gate; see
    `docs/ai_context/PHASE5EF_TYPING.md`.
  - **sb-sh-five: RECLASSIFIED — never tool-blocked.** ROM words show sb/sh
    macro stores stay **pre-jr with a nop slot** (func_80033A2C sb,
    func_800C6ED8/C6EE8 sh, func_800C6EC0 dual-sh; func_8001A374 has a
    cc1-filled `li` slot). Current maspsx already emits that shape; the patch
    deliberately does not touch sb/sh. Remaining work is typing + integration,
    toolchain-independent.
  - **Still open (typing):** remaining opaque-word (`D_800A1868` other writers).
- **`lui;ori`:** **CAPABILITY-VERIFIED** — not a blocker. Constant-heavy
  computational functions (mult/div/mask, e.g. ÷100 via `0x51EB851F`) are
  approachable as a **separate future phase**; synthesis itself is solved.
- **gp arena loop `func_80055724`:** **PARKED-SCHEDULING** (branch
  `phase5eu-gp-loop-55724`; closest candidate stashed as `park phase5eu
  func_80055724 while-form 13-15`). Empty 8-byte frame **solved** (cc1 2.7.2
  `vars=8` home slots, natural). Blocker: three-way scheduling tension —
  while-form keeps frame+regs but hoists the cursor load above the `blez`
  guard (13/15); if+for keeps frame+regs but duplicates the guard and steals
  the prologue into its slot; if+do/while gets word order but `vars=0` and
  flipped regs. era `-O1 -G8` output is **byte-identical** to `-O2` for both
  leading phrasings — no per-function `-O` support from this leaf. Residual is
  scheduling, not proven allocation. Detail: `docs/ai_context/parked_blockers.json`.
- **disc mount `func_800698D4`:** **PARKED-SCHEDULING** (branch
  `phase5fb-boot-698d4`; closest candidate stashed as `park phase5fb
  func_800698D4 nested-ifs 140-141 (search3 beqz-slot residual)`). Disc
  identification/mount — clears the mount flag, verifies drive ready, searches
  for `\FMV1\PEDISC01.IDF;1` / `\PE.IMG;1` / `\FMV2\PEDISC02.IDF;1` via
  `DsSearchFile`, records via `func_80080C48` → `D_800B0DD8` + `D_800B0DCD`
  flag bits. 140/141 words; everything exact except ONE delay-slot steal:
  search #3's `beqz` (`0x5A24C`) — retail nop, ours steals `addiu $v0,$zero,-1`.
  Mechanism is the dbr_sched `$v0`-liveness screening rule (fingerprint table);
  not source-expressible. Banked idioms: nested-if defeats range-test collapse;
  CdlFILE local is `0x18` not `0x20` (frame arithmetic). Detail:
  `docs/ai_context/parked_blockers.json` (`boot-698d4-dbr-sched`).
- **post-mount loader `func_8006E834`:** **RESOLVED — INTEGRATED (5FK,
  91/91 exact).** The 5FC call-result register-home residual (`$v0`+two
  restores vs `$v1`) WAS source-expressible after all: paired hard-register
  pins ($v1 backup / $v0 test home) + two zero-code `"=r":"0"` barriers
  (fingerprint table). Historical park evidence preserved in
  `docs/ai_context/parked_blockers.json` (`boot-6e834-register-home`,
  status INTEGRATED).
- **flag-clear loop `func_800374E8`:** **PARKED-ALLOCATION, register COLORING**
  (branch `phase5ff-374e8`; candidate stashed as `park phase5ff func_800374E8
  (register-coloring skew; structure correct)`). Flag-clear loop over 4 x 56-byte
  records at `D_800BCEA8` — **RECORD TYPE ESTABLISHED** (durable deliverable;
  propagates to `func_80037548`): +0x00 `unsigned char` (lbu/sb), +0x0C
  `unsigned int` flags (lw/sw; bit 0x02000000 cleared here), +0x10 `signed short`
  (lh/sh); extent closes EXACTLY at +0xE0 = 4 x 56. **STRUCTURE CORRECT**: 5FD
  aggregate-subscript rule (no `rec` pointer) + the landed load gate (`439c244`)
  produce retail's 3-word indexed-symbolic shape (no `la` hoist, correct DAG and
  scheduling). **RESIDUAL — register coloring only**: era cc1 assigns
  mask->`$v0`/chain->`$v1`/value->`$v0`; ROM is mask->`$v1`/chain->`$v0`/value->`$v1`.
  Five phrasings x two loop forms x ladder rungs are ALL byte-identical —
  invariant under phrasing. Same class as 6E834's call-result home:
  hard-register-assignment skew. This leaf MOTIVATED the maspsx load-gate patch.
  Detail: `docs/ai_context/parked_blockers.json` (`register-coloring-374e8`).
  **TWIN FALSIFIED (5FH)**: `func_80037548` was probed and MATCHES 27/27
  (accumulator shape) — no coloring skew. Refined rule: coloring skew is
  LIVE-VALUE-PRESSURE DEPENDENT (374E8: mask+chain+value all live;
  37548: needle/accumulator/index in $a0/$a2/$a1 leave $v0/$v1 free), not
  per-table. Predict skew only when 3+ scratch values compete.
- **sentinel walk `func_80062CE4`:** **PARKED-SCHEDULING, loop-LAYOUT**
  (do/while form stashed as `park func_80062CE4 (loop-layout scheduling;
  do/while lever proven source-invariant)`). Sentinel validate-and-consume
  over the D_8009D154 list: if D_8009D160 (pending) is still linked, promote
  it to D_8009D15C (confirmed); clear D_8009D160 either way. 12/18; PROVEN
  source-invariant — both while-form and do/while produce BYTE-IDENTICAL
  output; ROM has sentinel-at-top->advance->null-back-edge. cc1
  canonicalizes loop body order before block layout. SIXTH skew instance.
  CARVE CORRECTION: spimdisasm 0x5C label OVERSHOOTS — active span 0x48;
  trailing 5 words are func_80062Fxx prologue. Postmortem: dual gp-four
  filters could not catch loop-layout skew (no pre-compile tell known).
  Detail: `docs/ai_context/parked_blockers.json` (`loop-layout-62ce4`).

- **SDK-runtime runner `func_800725DC` + twin `func_8007264C`:** **PARKED-PER-TU-TOOLCHAIN**
  (candidate preserved at `src/func_800725DC.c`, PARKED, semantically complete;
  detail: `docs/ai_context/parked_blockers.json` `per-tu-725dc`). One-shot
  callback runner over the empty fn-ptr table at `jtbl_80010000` (main's
  first callee). **THREE BANKED LEVERS** (durable, probe-verified): (1) the
  retail two-word zero count (`lui/addiu` of 0) is unreachable from any C
  zero spelling — the address expression `(int)jtbl_80010000 - 0x80010000`
  emits `la SYM+0x7FFF0000` whose hi/lo relocs resolve to 0000/0000 and KEEP
  the loop body alive at -O2; (2) an inline-asm `jalr` call (decrement as
  tied operand in the slot) yields `.frame args=0` — the no-args-area frame;
  (3) the loop body + flag load/store shape is word-exact on era `-O2 -G0`
  with pins. **RESIDUAL — three coupled per-TU mechanisms, flag-invariant**
  (-O2/-O1/-fno-schedule-insns/-fschedule-insns2/-G8 ladder): ascending
  contiguous prologue saves (era's base order is fixed descending; PE1's
  other units are descending/slot-interleaved), `li→ori` expansion (retail
  slot constant is `ori`, era pipeline yields `addiu`), and the
  no-args-area frame model. **Third per-TU datapoint** — the 0x800725xx
  SDK-runtime unit was built with a different ccpsx/aspsx configuration.

- **boot-read `func_8006A9E4` (215w):** **IN PROGRESS (5FO checkpoint d10)** —
  candidate preserved at `src/func_8006A9E4.c` (d10: frame EXACT 0x30,
  all zone/copy structure word-count-exact, **216 words (+1)**; sole
  structural extra = zone-1 gate steal, 698D4-class; NOT integrated).
  Key levers: `__builtin_memcpy(dst,src,16)` = retail's lwl/lwr+swl/swr
  block shape; `called→$17` pin = retail's zone-2 register split;
  do/while = no trip guard; sentinel VARIABLE fixes the zone-1/3 steal
  but ripples zone 2 (next: pin combo). Main's post-init
  boot-read: ClearImage rect setup, FOUR table-driven retry-read zones over
  the u16 boundary pairs at `D_800930DC[0]/[1]/[4]/[5]` (issue `func_8006E6A8`,
  poll `func_8006E7E8` through a goto-gate: `flag=1; poll: if (flag==-1)
  goto restart; flag=poll(); if (flag) goto poll;` — reproduces retail's
  dead-edge outer loop), zone-2 one-shot `func_800527C8`, 0x10A50-byte
  alignment-split copy to `D_800E2858` (unaligned = lwl/lwr blocks), two
  `func_8006E498` decode calls (data[0x144] = SECOND call's return),
  `func_80087090(D_800B0E6C,1)`, 0x1400-byte copy to data[0x130].
  Resume from `src/func_8006A9E4.c` + the REMAINING-DELTAS list in its header.

- **CC1 PROVENANCE INVESTIGATION — COMPLETE (NULL RESULT):** **no closer community build exists.**
  Four-phase read-only investigation (Phases 1–4) into the era toolchain's cc1, the retail PE1 compiler
  (ccpsx), and whether a closer community build is obtainable. Blinded two GCC MIPS-backend mechanisms
  across the 2.7→2.8 version boundary (loop-body layout via `62CE4`, dbr_sched `$v0`-liveness via `698D4`);
  both survived REORGED (the `reorg.c` rewrite in 2.8 produced identical steal-vs-decline decisions).
  The six parks reflect GCC 2.x MIPS-backend ARCHITECTURE DECISIONS, not version-local divergences.
  DISPOSITION: the source-patch path is declined. The residual policy accepts
  the documented structurally-correct C candidates without counting them as
  matching C; current disposition is 374 exact leaves plus 24
  `ACCEPTED-RESIDUAL` leaves. See
  `docs/acceptance/MATCHING_RESIDUAL_POLICY.md`.
  Full report: `docs/ai_context/cc1_investigation.md`. Pipeline reconstructible from the report's
  candidate hashes and `git show stash@{N}^3:path` recovery procedure.

- **`main` (`func_8001220C`, 187 words):** **PARKED-SCHEDULING, WITH COMPLETE CANDIDATE**
  (candidate preserved at stash; five drafting iterations on scratch /tmp/mainvN.c).
  The boot keystone: init sequence, 20-call-site mount/read/dispatch loop, volume gate,
  A8-code three-way state switch. ~180 words match at opcode/position. DURABLE DELIVERABLES:
  the 9-word scratchpad stack handoff (sp → 0x1F8003FC, jal 8019234C, restore) is
  BYTE-EXACT as fenced inline asm with full caller-saved clobbers — the fenced-exception
  mechanism (register-pinning precedent) is validated for when main integrates. Role map
  pinned: $s0 data ptr (D_800B0CD8), $s1 dispatch, $s2 flagbyte (+0xF5), $s3 state_val
  (0xA9400048), $s4 bitmask (0x100000). All 20 externs typed (69B08 int, 1909B4→6E9A0
  raw-flow chain). RESIDUAL — ONE mechanism, proven scheduler-driven by an
  init-placement lever test (draft 4 declared bitmask at top, draft 5 moved init after two
  calls; cc1 kept the li at the same position and the $s-save interleaving identical —
  source cannot express the difference): prologue save-batching + invariant-constant
  placement, cc1 ordering pass vs ccpsx. Fifth scheduling-family instance.
  NOTE: with 6E834 resolved in 5FK, main and 698D4 are the TWO remaining
  boot-chain parks — the cc1 archaeology still directly gates boot-to-black.
  Detail: `docs/ai_context/parked_blockers.json` (`main-prologue-scheduling`).

- **ccpsx-vs-2.7.2 SKEW SET — four distinct mechanisms:** (1) the
  allocation/scheduling family (`6A674`/`55724`/`52BCC`; two recovered via
  `-O1`), (2) dbr_sched `$v0`-liveness slot-steal (`698D4`), (3) call-result
  register home (`6E834`) — **RESOLVED 5FK via paired register pins +
  zero-code barriers; NOT unreachable from C**, (4) register coloring /
  pseudo-numbering (`374E8`). (4) remains register-ASSIGNMENT skew; (2)
  remains a liveness-screening skew.
  SIX instances documented (four scheduling, two register-assignment); 6E834
  left the set in 5FK. TWO boot-chain functions remain parked (`main`,
  `698D4`) — the cc1 question still directly gates boot-to-black under plan A.
  Do not chase mid-leaf.
- **PARKED-ALLOCATION/SCHEDULING family:** cc1 2.7 register
  allocation/scheduling decisions that natural C cannot steer and `-O` level
  does not change. **FAMILY INVESTIGATED (read-only, accepted): NO SINGLE
  KNOB.** All residuals are present in cc1's **raw** output, pre-maspsx
  (maspsx does only `move`→`addu`, delay-slot nops, the 2.21 indexed-store
  expansion — no reordering/renaming), so a maspsx patch cannot fix any of
  them; the `addiu_at` template does not apply. Pass attribution
  (flag-probed) and current status:
  - `52BCC`: **MATCHED (5EW, leaf 218)** — the `-O1`→`-O2` flip required
    exactly `-fexpensive-optimizations` + `-fschedule-insns2` (regclass +
    post-alloc scheduler; bisection-proven minimal pair). Retried at era
    `-O1 -G0 -fschedule-insns2`: two-const-mode phrasing (u8 head const dies
    at the guard → loop const re-materializes into `$v1`; `int` loop byte →
    mask-free raw `bne`) + sched2 hoisting the head `li` into the `lbu`
    delay = all 15 words exact. First `-fschedule-insns2` leaf.
  - `55724`: pre-reorg RTL emission order (C statement order); NOT
    `dbr_sched` (`-fno-delayed-branch` doesn't move it), `-O`-invariant.
    Retail *sank* the p-load below the guard; 2.7.2-psx has no pass that
    sinks loads past conditional branches. **No lever** — constrained-C or
    acceptance. (Still parked; see entry above.)
  - `6A674`: **MATCHED (5EX, leaf 219)** — the `-O`-sensitive constant
    materialization runs through a hardwired `optimize>1` path (not
    flag-reachable), so `-O1` is the only lever; at `-O1 -G0` the residual
    shrank 45→21 (all pure `li`/`addiu`-before-store order swaps), and
    `-fschedule-insns2` closed them to **0/152** with the 5EP pins intact.
  - **`-fschedule-insns2` is a GENERAL RETAIL FINGERPRINT** (two independent
    leaves, 22 positions): retail's ccpsx ran post-allocation scheduling;
    our default doesn't. Try sched2 early on future scheduling-position
    residuals. Hypothesis to test later (carefully; current leaves match
    without it): sched2 may belong in the era default flag set.
  Evidence: scratch compiles `/tmp/fam_inv` + `/tmp/o1` (session-recorded).
- Complex `$gp` / GTE / BIOS / mult-div / large non-leaves: still open; not
  inventoried here. Path forward is matching real logic, not harvesting
  trivial setters.

## Boot Rung 1 — COMPLETE, climbing `main`'s call chain

```text
main -> func_8006A5BC ✓ exact C (5EZ, leaf 221)   # boot init, VSync waits
     -> func_8006A64C ✓ exact C -> { func_8006A8D4 ✓ exact C,
                                     func_8006A674 ✓ exact C (5EX, leaf 219) }
     -> func_8003E610 ✓ exact C (5EY, leaf 220)   # display/graphics bring-up
     -> func_8003E680 ✓ exact C (5FA, leaf 222)   # subsystem-init dispatcher
```

- `func_8003E680` is **MATCHED (5FA)**: era `-O1 -G0`, all 53 words exact.
  Zero five state globals (Stage-0 reader types: D1C4/D280 unsigned compares,
  D1A0 flags, D250 opaque, CDDC `int` index), 2000-pass poll loop with `i++`
  in the `jal` delay slot (ROM `sltiu` → `unsigned int` counter — the only
  phrasing fix needed), callback registration, ~11 subsystem inits. **First
  fn-ptr-to-asm-callee arg**: `func_80073D24(func_8003E91C)` →
  `lui $a0,%hi` / `addiu $a0,$a0,%lo` with R_MIPS_HI16/LO16 against the
  same-segment text symbol; links exactly. `-O1` predicted by the selection
  rule (five per-store `lui`s, no CSE). Segment-head carve of 2EE80:
  C `0xD4`, resume `2EF54.s` `0x1858`. Next candidates: `func_800698D4`
  (159L, disc mount w/ SDK `DsSearchFile`), `func_8003F3C4` (245L).

- `func_8006A5BC` is **MATCHED (5EZ)**: era `-O1 -G0`, all 36 words exact on
  the first attempt. Four setup calls, two structurally identical
  `while (f() != 1) VSync(0);` loops (VSync = `func_80073A44`, SDK), then
  `func_8007F7A8()`'s return stored to `D_800B0DD4` (`unsigned short`, typed
  by its `lhu` reader). `-O1` reproduces retail's per-use `$s0=1`
  materialization — in `func_80086FF8`'s delay slot AND re-materialized
  between the loops (the 6A674 `-O1` lever, third leaf). Return-use safety:
  both loop conditions compare `$v0` raw (full 32-bit `beq`, no
  mask/sign-extend), so asm callees declared `int(void)` are codegen-safe.
  Identical loop bodies did NOT cross-jump. Mid-55430 carve: prefix
  `0x598C`, C `0x90`, then the three existing boot C leaves — **four
  contiguous C carves, no asm between**. Next candidates up the chain:
  `func_8003E680` (56L, state zeroing + 2000-pass poll + callback
  registration), `func_800698D4` (159L, disc mount w/ SDK `DsSearchFile`).

- `func_8003E610` is **MATCHED (5EY)**: era `-O2 -G0`, all 28 words byte-exact
  on the **first** attempt — no sched2, no pins. Straight-line dispatcher of
  ten calls with immediate args (`0x140`/`0xE0` = 320x224 display res), no
  branches/loops/`$gp`/globals; plain `-O2` reproduces ccpsx's mixed
  arg-load/delay-slot placement exactly. All ten callees are extern-declared
  with call-site-determined signatures (immediate args, no returns used —
  callee bodies don't affect codegen; one already C: `func_80080CC8`).
  Mid-2E7D8 carve: prefix `0x638`, C `0x70`, resume `2EE80.s` `0x192C`.
  Readiness ranking of `main`'s remaining callees (size + callee C/SDK
  coverage) put it first; next candidates in order: `func_8006A5BC` (42L,
  two wait loops + one `sh` global), `func_8003E680` (56L, state zeroing +
  2000-pass poll loop + callback registration).

- `func_8006A674` is **MATCHED (5EX)**: era `-O1 -G0 -fschedule-insns2` +
  `MASPSX_THREE_WORD_SYMBOL_STORE=1`, all 152 words byte-exact. `-O1` gives
  retail's per-use `-1` materialization (the `-O2` shared hoist is hardwired
  `optimize>1`, not flag-reachable); sched2 places every `li`/`addiu` before
  its adjacent store (21 order swaps). The six semantic pins from the 5EP
  bounded candidate are load-bearing (dropping all six → 46 mismatches).
  Mid-55430 carve fills the 6A64C/6A8D4 gap exactly (0x260); the three boot
  C carves are contiguous.
- `func_8006A64C` matches all 10 words on era `-O2 -G0`: two sequential
  `void(void)` calls, teardown before `jr`, and a nop delay slot. Both
  `R_MIPS_26` relocations resolve at link time; matching a caller requires a
  known callee signature, not that every callee already be C.

## Standing policy

1. **PROBE BEFORE GRIND.** The two biggest unblocks (maspsx stdin hang;
   `expand_load_immediate` forcing `ori`) were short diagnostics, not
   integrations. When a family is blocked, diagnose before more members.
2. **Homogeneous families may be batched.** Risk lives in the first member.
3. **`asm/` is not a source of truth for counts.** Use `configs/USA/disc1.yaml`.
4. **Commit messages are not evidence.** A claim is proven when a gate is green
   and the leaf is objdump-probed (not SHA alone on carves).
5. **No weak-int cheat:** do **not** invent width a narrower store contradicts
   (e.g. `sh`/`sb` → `int`). Distinct from **opaque-word** typing (consistent
   32-bit `sw`/`lw` everywhere) — that is a separate lead ruling, currently
   open under `TYPING-POLICY` in `parked_blockers.json`.
6. **Width-only setters are triaged in `parked_blockers.json`.**
   `READY-FROM-READER` (src reader already *types* it), `BLOCKED-ON-READER`
   (undecompiled reader not yet proven to be a mere use-site),
   `TYPING-POLICY` (opaque 32-bit word; use-site found, no narrowing possible),
   or `DECISION-BLOCKED` (write-only; no reader). A use-site is not a type-site
   (`func_800405A4` lesson). Re-check after every reader phase.
   `5EF-delay-slot` **CLOSED** (14/14 integrated); `sb-sh-five` reclassified
   typing-only.
7. **Register pinning is an evidence-backed fallback, not a shortcut.** Use it
   only after natural C and a retail-order phrasing retry prove that the
   residual is register **allocation**, not statement order. Pins must have
   semantic names and a source comment recording the allocation proof
   (`func_8006A8D4` exact; `func_8006A674` bounded parked example).

## Resolved blockers

- **Phase 5I** delay-slot (`move`/`or` vs `addu`): **SOLVED in 5EC** by era.
- **Maspsx non-TTY hang:** **SOLVED** (`</dev/null` in `era_compile`).
- **`lui;ori` large-literal synthesis:** **CAPABILITY-VERIFIED** (scratch probe;
  both sign cases; no flag change).
- **5EF delay-slot (sw in `j $31` slot):** **CLOSED in 5EF** by the
  vendored maspsx LOCAL PATCH (`MASPSX_FILL_STORE_DELAY_SLOT=1`). Key evidence:
  `func_8003FFAC` vs `func_8007FBC0` — identical C, different ROM scheduling
  (pre-jr+nop vs in-slot) ⇒ original units assembled under different ASPSX
  scheduling; behavior is opt-in per leaf. All 14 members are integrated exact;
  sb/sh never fill (ROM-proven).

## History (append-only, truncated)

| Phase | **224 exact leaves** (tools: maspsx load gate `439c244` on main; parked: 698D4/6E834/374E8) | `scripts/verify_us.sh` summary + exact rebuild |
| --- | --- | --- |
| 4I–4J | 0→1 path | Exact asm rebuild; GCC 14.2 first leaf |
| 5B–5CW | →98 | Empty stubs, getters, store/setter batch |
| 5CX–5DB | →103 | Countdown memset/memcpy (`$2`/`$3` pins) |
| 5DC–5DJ | →156 | `$gp` small-data (`_gp`+`-G 8`); `-fno-tree-ter` |
| 5EA | 157 | Era dual-toolchain; return-0 `addu` |
| 5EB | 161 | Return-0 twins via mid-segment holes |
| 5EC | 163 | sb+ret0; `--dont-expand-li`; 5I dead |
| 5ED | 170 | sb+ret0 batch harvest (family closed) |
| 5EE | 171 | `$at` absolute-`sw` integrated pilot; delay-slot shapes blocked |
| 5EG-readers | 173 | Type-pinning readers `func_8008AB1C` / `func_80042B6C`; `D_800A1870` decl fix |
| 5EG-setter | 174 | `func_80085728` dual-store; first reader-recoverable pre-jr setter |
| 5EH-opaque-word | 182 | u32 opaque-word ruling; 8 A182x setters (`42BD8`…`42C64`) |
| 5EI-ready-from-reader | 185 | READY-FROM-READER setters `42910`/`42B38`/`42B50` |
| 5EJ-d8009d28c-state | 189 | `D_8009D28C` int-state setters `17FDC`/`17FF0`/`192B8`/`192C8` |
| 5EK-d8009d270-bitwise | 191 | `D_8009D270` unsigned flags setters `87198`/`87414` |
| lui-ori probe | 191 | Large-literal `lui;ori` CAPABILITY-VERIFIED (docs only) |
| 5EF-pilot | 192 | Vendored maspsx LOCAL PATCH (sw delay-slot fill); `func_8007FBC0` integrated |
| 5EF | 205 | Remaining 13 delay-slot `sw` members typed and integrated; family closed 14/14 |
| 5EG-first-branch | 206 | First branchy leaf `func_8004F448`; era cc1 `-O1 -G 8` hoists const into `beqz` delay slot word-exact (branch scheduling capability proven) |
| 5EH-arg-return | 207 | First value-returning leaf `func_800438C0` on era path: `-O2 -G8` preserves double store, `addu` return-0, era+gp proven; GCC 14.2 store-merge + `move` documented as $CC-path limits |
| 5EI-first-nonleaf | 208 | First non-leaf `func_800197D0` on era `-O2 -G8`: frame (`addiu $sp,∓0x18`, `sw/lw $ra,0x10($sp)`) + `jal func_800375B4`; teardown `addiu $sp,+0x18` lands **in the `jr` delay slot** word-exact |
| 5EJ-outgoing-arg | 209 | `func_80019484(int **)` on era `-O2 -G0`: double-dereference load schedule sets outgoing `$a0` before `jal func_800438C0`; load-delay nop, jal nop, frame, and teardown-in-`jr`-slot all word-exact |
| 5EK-volume-197f0 | 210 | First post-probe volume leaf: `func_800197F0` on era `-O2 -G0` transfers the proven 197D0 frame + void `jal` + return-1 + teardown-in-`jr`-slot shape word-exact; no new primitive |
| 5EL-return-forwarding | 211 | `func_8007F7A8` on era `-O2 -G0` forwards `func_8007FCAC`'s `$v0` untouched and reproduces retail's opposite epilogue schedule: teardown before `jr`, nop in the delay slot; all eight words exact |
| 5EM-boot-6a8d4 | 212 | First Rung-1 boot leaf: `func_8006A8D4` on era `-O2 -G0` lays out boot memory regions with 19 ordered absolute pointer stores; register-pinned byte cursors reproduce all 68 retail words exactly after two plain-local phrasings fail the retail register allocation/store schedule. Compiler-constrained, target-specific C is documented in source |
| maspsx indexed-store | 212 | Toolchain patch `f0b9155`: default-off `MASPSX_THREE_WORD_SYMBOL_STORE=1` opt-in adds the three-word symbol+register store form; exact 212-leaf regression, 148 tests, and live re-clone durability passed |
| 5EN/5EP-loop-probe | 212 | `func_8006A674` proves five `bnez`/`bgez` loop back-edge delay slots plus store-in-`jr`-slot; L2 and late allocation deltas cleared, but the leaf is parked with a 45-word `$v1` constant-hoist residual and no 213 claim |
| 5EQ-boot-6a64c | 213 | Boot wrapper `func_8006A64C` on era `-O2 -G0`: calls matched-C `func_8006A8D4` then live-asm `func_8006A674`, both proven `void(void)`; both `R_MIPS_26` relocations resolve and teardown-before-`jr` + nop-slot matches all 10 words |
| 5ER-d1c-d48 | 215 | Adjacent byte/word test-and-clear-return twins `func_80038D1C` / `func_80038D48` on era `-O2 -G0`; explicit pointer reuse gives retail `bnez` + j-over delay-slot returns and `sb`/`sw` clears, all 11 words each exact after one natural phrasing retry |
| 5ES-loop-4bf08 | 216 | First loop-as-volume leaf: natural explicit-init pointer walk in `func_8004BF08` clears two parallel `int[8]` arrays; era `-O2 -G0` reproduces all 14 words, including the split pointer advances and backward-`bnez` delay slot, with no pinning or tool flag |
| 5ET-loop-5186c | 217 | Loop-as-volume repeats: pure-register 16-pass bit-serial loop `func_8005186C` on era `-O2 -G0`, all 15 words on the first natural-C try; unconditional `result <<= 1` fills the forward `bnez` skip slot, nop `bgez` back-edge; mid-4204C carve (prefix 0x20, C 0x3C, resume 420A8.s 0x5A0) |
| 5EU/5EV parks | 217 | `func_80055724` (p-load hoist; `-O1`≡`-O2`) and `func_80052BCC` (rotated-loop `$v0`/`$v1` role swap, 13/15) parked as the **PARKED-ALLOCATION/SCHEDULING family** (with `6A674`): cc1 global allocation/scheduling choices natural C can't steer. Banked idioms: rotated loop = explicit first iteration + `while`; signed `char` vs `0xFF` emits the `andi`. Docs only, no carve |
| family diagnosis | 217 | Read-only investigation: **NO SINGLE KNOB**. All three residuals are in cc1 raw output (maspsx can't fix any). `55724` = pre-reorg emission order, no lever; `52BCC` = regclass+sched2 pair (`-fexpensive-optimizations`+`-fschedule-insns2`), `-O1` shows retail loop roles — retry at `-O1`; `6A674` = hardwired `optimize>1`, only lever `-O1` (untested). Toolchain-patch hypothesis closed; per-leaf `-O1` is the route |
| 5EW-52bcc-o1 | 218 | `func_80052BCC` MATCHED: era `-O1 -G0 -fschedule-insns2` (first sched2 leaf) + two-const-mode phrasing (u8 head const dies at guard → loop reload into `$v1`; `int` loop byte → raw `bne`); sched2 hoists head `li` into the `lbu` delay like ccpsx. All 15 words exact; mid-42FC8 carve (prefix 0x404, C 0x3C, resume 43408.s 0x2A8). Also fixed a latent pipefail/SIGPIPE flake in toolchain detection (`grep -q` → `grep … >/dev/null`) |
| 5EX-6a674-o1 | 219 | `func_8006A674` MATCHED after three parked attempts: era `-O1 -G0 -fschedule-insns2` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`, all 152 words + relocs exact with the 5EP semantic pins (load-bearing; dropping → 46 mismatches). `-O1` = per-use `-1` materialization; sched2 = `li`/`addiu`-before-store placement (21 fixes) — **sched2 confirmed as a general retail fingerprint**. Boot Rung 1 complete (`main → 6A64C ✓ → {6A8D4 ✓, 6A674 ✓}`); mid-55430 gap filled exactly (0x260), three contiguous C carves |
| 5EY-boot-3e610 | 220 | Boot display/graphics bring-up `func_8003E610` on era `-O2 -G0` — all 28 words exact on the **first** attempt, no sched2/pins: straight-line dispatcher, ten calls with immediate args (`0x140`/`0xE0` = 320x224), callees extern-declared with call-site-determined signatures. Readiness ranking of `main`'s callees (callee C/SDK coverage, not raw size) picked it; next up the chain: `func_8006A5BC`, `func_8003E680`. Mid-2E7D8 carve (prefix 0x638, C 0x70, resume 2EE80.s 0x192C) |
| 5EZ-boot-6a5bc | 221 | Boot init `func_8006A5BC` on era `-O1 -G0`, all 36 words exact first attempt: four setup calls, two identical `while (f() != 1) VSync(0);` loops (no cross-jump), `7F7A8()` return → `D_800B0DD4` (`unsigned short` via `lhu` reader). `-O1` per-use `$s0=1` materialization (delay-slot + between-loops re-materialization) — third `-O1`-lever leaf; return-use confirmed codegen-safe (raw `$v0` `beq`, no mask). Mid-55430 carve extends the boot block backward: **four contiguous C carves** (prefix 0x598C, C 0x90, then 6A64C/6A674/6A8D4) |
| 5FA-boot-3e680 | 222 | Boot subsystem-init dispatcher `func_8003E680` on era `-O1 -G0`, all 53 words exact: zero 5 globals (Stage-0 reader types), 2000-pass poll loop (`i++` in `jal` slot; `unsigned int` counter for ROM `sltiu` — the only phrasing fix), callback registration + ~11 inits. **First fn-ptr-to-asm-callee arg** (`&func_8003E91C` via R_MIPS_HI16/LO16 against a text symbol). `-O1` predicted by the per-use selection rule (five per-store `lui`s, no CSE). Fingerprint table banks: `-O1` selection rule, return-use readiness, sched2 scope narrowing, fn-ptr arg, unsigned loop compare. Segment-head carve of 2EE80 (C 0xD4, resume 2EF54.s 0x1858) |
| 5FB park | 222 | `func_800698D4` (disc mount, 141 words) PARKED-SCHEDULING at 140/141: nested-if phrasing defeats gcc's range-test collapse (`v!=0 && v!=-1` → `addiu`+`sltiu`+`bnez`), everything exact except one dbr_sched delay-slot steal at search #3's `beqz` — retail declines a `$v0`-setter steal when the branch target is the return block (`$v0` live to `jr`); ours steals. Screening rule + CdlFILE `0x18` frame note banked; candidate stashed; no carve, no leaf claim |
| 5FC park | 222 | `func_8006E834` (post-mount loader + display env, 91 words) PARKED-ALLOCATION at 89/91: five-arg call PROVEN (5th arg `sw $v0,0x10($sp)` in `jal` slot, first try); frame decomposition method banked; retail folds the range test in this unit (per-TU datapoint vs 698D4). Residual: call-result register home (`$v0`+restores vs `$v1`), not source-expressible. Third ccpsx-vs-2.7.2 skew mechanism recorded; candidate stashed; no carve, no claim |
| 5FD-table-2f9cc | 223 | Table clear `func_8002F9CC` (17 words) on era `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`: zero the in-use flag of all 7×220-byte records at `D_800A5D58` (record typed from the `func_8002F7D8` reader; extent `0x604` = 7×220). Key finding: aggregate element type is an addressing-mode lever — `arr[i].field = 0` keeps the symbolic indexed store; flat `arr[i*55] = 0` hoists `la` (flag-invariant). `unsigned char` counter (`andi 0xFF` masks), `sltiu` bound, stride 220B/55W (not 196B/49W). Mid-11718 carve (prefix 0xEAB4, C 0x44, resume 20210.s 0x4010) |
| 5FE-table-2f970 | 224 | Table twin `func_8002F970` (23 words) on era `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`: pointer-match search-and-clear over the 2F9CC table (`SlotRecord` typing inherited unchanged); `*p == D_800A5D58[i].body` → clear `inUse`, then `*p = 0` with the `sw` in the `jr` delay slot (5EN pattern). `$a3` body-base hoist = the aggregate lever producing (not preventing) a hoist; back-branch slot FILLED vs 2F9CC's nop — slot fill is per-shape, not per-table. One phrasing fix: operand order in the compare (`body == *p`) for `bne $v0,$v1`. Object-level `%lo` difference on the hoisted base (`D_800A5D58+4` vs `D_800A5D5C`) resolves to identical bytes at link. Contiguous carve with 2F9CC (prefix 0xEA58, C 0x5C, C 0x44, resume 20210.s) |
| 5FF-maspsx-loads | 224 | Toolchain patch `439c244`: `MASPSX_THREE_WORD_SYMBOL_STORE` extended from stores to standalone indexed symbolic LOADS (lb/lbu/lh/lhu/lw/lwl/lwr) under addiu_at — pass-through emits the ASPSX 2.30 three-word lui/addu/op-%lo form; compound lines retain legacy; `lwc2` stays outside (durable negative). Store path untouched; one gate, existing name. Full gate: flag-OFF 224 exact SHA; flag-ON 224 exact SHA (6A674/2F9CC/2F970 unchanged under the extended meaning); 153 vendored tests (was 148, +5 load); re-clone restores all three tracked files byte-identical. `func_800374E8` (which motivated the patch) PARKED — register-coloring residual (structure correct; see Known-open families + parked_blockers.json). 224 unchanged, no carve.
| 5FG-363f4 | 225 | Search-and-clear `func_800363F4` (21 words / 0x54 @ 0x26BF4): 16-entry D_800A7624 scan, clear key on match, break. era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 — FIRST leaf exercising the load gate; the probe EXPOSED the 439c244 bug (GNU as uses the DESTINATION reg as temp for lw; ROM uses $at), fixed at 5dac87e. 21/21 words; mid-2422C carve (prefix 0x29C8, C 0x54, resume 26C48.s 0xD5C); full 225 build EXACT SHA.
| 5FI-62a34 | 227 | 2-key node-list search `func_80062A34` on era `-O2 -G8` (gp head); `&&` short-circuit matches two-target block layout |
| 5FJ-6e9a0 | 228 | Boot display init + pointer arena + ClearOTagR poll loop + dispatch exit `func_8006E9A0` (141 words / 0x234 @ 0x5F1A0) on era `-O2 -G0`, **141/141 exact**: 6A8D4 arena pins (`$v0/$v1` cursors, `$a0/$a1` consts) reused verbatim; NEW pins `$s0`=saved_arg / `$s2`=&D_800B0E38 (natural allocation swaps them and sinks the materialization past a call); empty-asm barrier holds the `$s2` lui/addiu at retail words 29-30. Tail carve of 5B1E4: asm prefix 0x3FBC, C 0x234, no resume (6EBD4 C sibling follows). P3 types: D_800BCE80/D_800BCFEE/D_800B0DC6 `unsigned char` (opaque DISP_ENV addr, lbu poll, sb); arena globals + D_80011614 `unsigned char *`; D_8009D280 `unsigned int`, D_8009CDDC `int` (3E680). Full 228 build EXACT SHA. **Uncommitted on branch `phase5fj-6e9a0`** |
| 5FK-6e834 | 229 | Post-mount image loader + display env setup `func_8006E834` (91 words / 0x16C @ 0x5F034) on era `-O2 -G0`, **91/91 exact** — the 5FC-parked call-result register-home residual resolved by the 5FJ control technique: paired pins `register int t asm("$3")` ($v1 backup across the range test) + `register int rt asm("$2")` ($v0 equality-test home) and two zero-code `asm volatile("" : "=r"(x) : "0"(x))` barriers (block copy-prop folding, emit nothing); reorg threads the idempotent restore copy into the `beqz` delay slot, reproducing retail's two restores. Bounded matrix: natural V0 = historical 90-word residual; single-pin V1/V3 inert; paired pins without the rt barrier leave tests on $v1. Mid-5B1E4 carve: asm prefix 0x3E50, C 0x16C, then 6E9A0 C — 6E834→6E9A0→6EBD4 now contiguous C. Full 229 build EXACT SHA. **Uncommitted on branch `phase5fk-6e834-pin-revisit`** |
| 5FL-698d4-revisit | 229 | **PARKED** — Bounded pin/barrier revisit of `func_800698D4` (disc mount, 141w) following the 5FK proof. V0-V8 barrier matrix tested: empty `asm volatile("")` and 5FK-style value barriers prevent the delay-slot steal at searches #3/#4 but add scheduling-boundary overhead bloating to 144 words (+3 vs retail). Residual is pure instruction-scheduling (delay-slot fill), not register allocation; the 5FK control family cannot resolve without unacceptable overhead. sltiu fix confirmed. Candidate preserved at `src/func_800698D4.c` (PARKED, 140/141). Production unchanged at 229. **Uncommitted on branch `phase5fl-698d4-barrier-revisit`** |
| 5FM-main-revisit | 229 | **PARKED** — Bounded revisit of `func_8001220C` (main, 187w). V0 baseline (era -O2 -G0) produces 188 words with 150/187 word-level mismatches across all 7 zones: prologue save-batching order fundamentally differs, $s2/$s3 register assignment is swapped (state_val→$s2 vs retail $s3; flagbyte→$s4 vs retail $s2), invariant bitmask 0x100000 materialization point diverges, and the skew cascades through the entire dispatch loop. Scratchpad stack-handoff atom remains byte-exact. The bounded hard-register/barrier family from 5FJ/5FK cannot address pervasive global register-allocation skew of this scope; the V0-V6 matrix was not executed because the baseline already exceeds what localized barriers control. Candidate preserved at `src/func_8001220C.c` (PARKED, semantically complete, 20 callee declarations verified). Production unchanged at 229. Fresh Docker build confirms EXACT SHA-1. **Uncommitted on branch `phase5fm-main-barrier-revisit`** |
| 5FN-725dc | 229 | **PARKED-PER-TU-TOOLCHAIN** — bounded campaign on `func_800725DC` (main's first callee, 28w) + twin `func_8007264C` (26w): one-shot callback runner over the EMPTY fn-ptr table at `jtbl_80010000`. THREE LEVERS BANKED (probe-verified): the retail `lui/addiu`-zero count is unreachable from any C zero spelling (10 forms probed) but `(int)jtbl_80010000 - 0x80010000` → `la SYM+0x7FFF0000` resolves to 0000/0000 at link and keeps the loop alive at -O2; an inline-asm `jalr` call (tied decrement in the slot) gives the retail no-args-area frame (`.frame args=0`); the loop body + flag load/store shape is word-exact on era `-O2 -G0` with pins. RESIDUAL is three coupled per-TU mechanisms, flag-invariant across the ladder: ascending contiguous prologue saves (era fixed descending; all other PE1 units descending/interleaved), `li→ori` expansion, no-args-area frame model — the 0x800725xx SDK-runtime unit used a different ccpsx/aspsx config (third per-TU datapoint). Candidate preserved at `src/func_800725DC.c`. Production unchanged at 229 |
| 5FO-6a9e4 | 229 | **IN PROGRESS, CHECKPOINT d10** — `func_8006A9E4` (main's boot-read, 215w): twelve-draft campaign, **216 words (+1)**, frame EXACT 0x30, all zone/copy structure word-count-exact. NEW PROVEN LEVERS: `__builtin_memcpy(dst,src,16)` on char* emits EXACTLY retail's lwl/lwr×4 + swl/swr×4 block shape; `called→asm("$17")` pin flips zone 2 to retail's called=$s1/sentinel2=$s4; do/while copies kill the trip guard. NEGATIVE RESULTS banked: packed struct → byte-wise synthesis (+119w), aligned(1) ignored, `-fno-strength-reduce` regresses (208/221w). Sole structural extra = zone-1 gate steal (698D4-class); remaining deltas are copy-loop register/offset encodings + zone-3/4 gate li extras. ROUND-3 matrix (d13-d15) brackets the solution: block-scoped sentinel pins make ALL gates word-exact but bloat copies (+10) — gates-vs-copies register-pressure trade-off; next: narrow sentinel lifetimes or pin copy cursors. Candidate at `src/func_8006A9E4.c` (NOT integrated) |
| 5FH-twin-37548 | 226 | Record-field lookup twin `func_80037548` (27 words / 0x6C @ 0x27D48): scan 4 x 56-byte D_800BCEA8 records for short needle at +0x10, return signed byte0 (+0x00) on match else 0. era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 (lh/lbu indexed pair through the 5dac87e $at gate). Twin-hypothesis FALSIFIED: matches 27/27, no coloring skew — live-value-pressure rule refined (see register-coloring-374e8 parked entry). Accumulator phrasing banked (fingerprint table). Mid-27C6C carve: prefix 0xDC, C 0x6C, resume 27DB4 (existing sibling); full 226 build EXACT SHA; packed-span byte-exact.

| PARKED-ASSEMBLER-TEMP-703F0 | 281 | `func_8007FBF0` @ `0x703F0`, six-word table getter for `D_8009B574[index]`. Function hood proven: `jr ra` + `nop`, eleven direct callers, and real instruction boundaries on both sides. The 3W indexed-symbolic-load gate fixes the form, but retail uses `$v0` (the load destination) as the address temporary while the candidate uses `$at`; a pointer-form retry is unchanged, so this is in the assembler macro rather than C/cc1. The `$at` form remains required by the 0x363F4-class site. Do not change the global gate without a fourth-patch investigation and full regression covering both branches. Candidate/retry objdumps remain in stash `hold rejected 0x703F0 R6 retries`; no leaf integration or count increase. Detail: `docs/evidence/func-8007fbf0/PARK.md`.
| VOLUME-1-CBFB4 | 282 | `func_800CBFB4` @ `0xBC7B4`, two-word return-zero callback. Function hood: `jr ra` + `move v0,zero`, exact-start callback-table reference at file `0xD1568`, real boundaries both sides. Natural `return 0;`, era `-O2 -G0`, 2/2 object words exact. Boundary carve: `0x8 + 0xFC4 = 0xFCC`; packed span and full executable exact SHA-1; verify green at 282. |
| VOLUME-2-CBFBC | 283 | `func_800CBFBC` @ `0xBC7BC`, adjacent two-word return-zero callback. Function hood: `jr ra` + `move v0,zero`, exact-start callback-table reference at file `0xD1550`, real boundaries both sides. Natural `return 0;`, era `-O2 -G0`, 2/2 object words exact. Boundary carve: `0x8 + 0xFBC = 0xFC4`; packed span and full executable exact SHA-1; verify green at 283. |
| VOLUME-3-CCF90 | 284 | `func_800CCF90` @ `0xBD790`, two-word return-zero callback. Function hood: `jr ra` + `move v0,zero`, exact-start callback-table reference at file `0xD16B4`, real boundaries both sides. Natural `return 0;`, era `-O2 -G0`, 2/2 object words exact. Boundary carve: `0x8 + 0x344 = 0x34C`; packed span and full executable exact SHA-1; verify green at 284. |
| VOLUME-4-CCF98 | 285 | `func_800CCF98` @ `0xBD798`, adjacent two-word return-zero callback. Function hood proven by exact-start callback-table entry at `0xD169C`, canonical return, and real boundaries. Natural `return 0;`, era `-O2 -G0`, 2/2 exact. Carve `0x8 + 0x33C = 0x344`; packed/full-image exact and verify green at 285. |
| VOLUME-5-CE1EC | 286 | `func_800CE1EC` @ `0xBE9EC`, two-word return-zero callback. Function hood proven by exact-start callback-table entry at `0xD17F8`, canonical return, and real boundaries. Natural `return 0;`, era `-O2 -G0`, 2/2 exact. Carve `0x8 + 0x1B8 = 0x1C0`; packed/full-image exact and verify green at 286. |
| VOLUME-6-CE1F4 | 287 | `func_800CE1F4` @ `0xBE9F4`, adjacent two-word return-zero callback. Function hood proven by exact-start callback-table entry at `0xD17E0`, canonical return, and real boundaries. Natural `return 0;`, era `-O2 -G0`, 2/2 exact. Carve `0x8 + 0x1B0 = 0x1B8`; packed/full-image exact and verify green at 287. |
| VOLUME-7-79024-SDK | 287 | `func_80079024` @ `0x69824` is a proven callable 3-word handwritten libGTE helper (`ctc2 a0,$26; jr ra; nop`) with five direct callers. Historical ordinary-C attempt missed the COP2 side effect; the family screen reclassifies it `SKIP-SDK-LIBRARY-COP2` under the PsyCross redirect policy. No integration/count change. |
| VOLUME-8-78FAC-SDK | 287 | `func_80078FAC` @ `0x697AC` is a proven callable 3-word handwritten libGTE helper (`ctc2 a0,$27; jr ra; nop`) with a direct caller. Historical disposition was `PARKED-HANDWRITTEN-COP2`; current pool disposition is `SKIP-SDK-LIBRARY-COP2`. |
| VOLUME-9-78FB8-SDK/STOP | 287 | `func_80078FB8` @ `0x697B8` is a proven callable 3-word handwritten libGTE helper (`ctc2 a0,$28; jr ra; nop`) with a direct caller. Current disposition is `SKIP-SDK-LIBRARY-COP2`. The third consecutive historical park still correctly triggered the volume hard stop; final campaign result remains 9 attempts, 6 matches, 3 parks, 287 leaves. |
| VOLUME-10-72714-SYSCALL | 287 | Refreshed pool screen: `func_80072714` @ `0x62F14` is a proven four-word handwritten syscall wrapper (`addiu a0,1; syscall 0; jr ra; nop`) with 26 unique direct callers and real boundaries. Adjacent `func_80072724` has the same shape with immediate 2. Both are `SKIP-SDK-LIBRARY-SYSCALL`; no ordinary-C spelling is sanctioned, no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-80072714/PARK.md`; candidate source is in the labeled stash. |
| VOLUME-11-62A20 | 288 | `func_80062A20` @ `0x53220` is a five-word argument-indexed getter with 39 unique direct callers and real boundaries. Era `-O2 -G0`; the pointer-plus-scaled-index phrasing reproduces retail’s `$a1` address temporary, all 5 words exact. Mid-531BC carve closes `0x64 + 0x14 = 0x78`; full SHA exact and verify green at 288. Evidence: `docs/evidence/volume-campaign-20260825/func-80062a20/REPORT.md`. |
| VOLUME-12-824C8 | 288 | `func_800824C8` @ `0x72CC8` is function-hood proven (12 callers), but parks after two `-O2 -G0` phrasings: retail retains the scalar-global address in `$v1` through the load and store-in-`jr` delay slot; natural C uses `$v0`/`$at` and emits a seven-word form. Adjacent `func_800824DC` has the identical exchange shape and is screened with it. `PARKED-ADDRESS-RETENTION`; no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-800824c8/PARK.md`; source is in the labeled stash. |
| VOLUME-13-85084 | 289 | `func_80085084` @ `0x75884` is a five-word constant-add getter with six unique direct callers and real boundaries. Era `-O2 -G0`, first phrasing exact (`*a0 + 0xB0BEB4BFu`), mid-74FB0 carve closes `0x8D4 + 0x14 + 0x690 = 0xF78`; full SHA exact and verify green at 289. Evidence: `docs/evidence/volume-campaign-20260825/func-80085084/REPORT.md`. |
| VOLUME-14-78120-SDK | 289 | `func_80078120` @ `0x68920` is a five-word callable tail-entry wrapper with three callers: three loads into `$t0–$t2`, then a branch into `func_80078134`’s shared path and handwritten `func_80078194` COP2 continuation. `SKIP-SDK-LIBRARY-GTE-TAIL`; no C attempt or count change. Evidence: `docs/evidence/volume-campaign-20260825/func-80078120/SKIP.md`. |
| VOLUME-15-81E5C | 289 | `func_80081E5C` @ `0x7265C` is a five-word scalar-global exchange twin (2 callers), identical to the 824C8/824DC address-retention family. Screened without a duplicate attempt as `PARKED-ADDRESS-RETENTION-FAMILY`; no count change. Evidence: `docs/evidence/volume-campaign-20260825/func-80081e5c/SKIP.md`. |
| VOLUME-16-77AA4 | 290 | `func_80077AA4` @ `0x682A4` is a six-word packed-coordinate helper with 50 unique direct callers and canonical `jr ra`/delay-slot return. Era `-O2 -G0` natural C reproduces all six words; carve `0x58 + 0x18 + 0xA8 = 0x118`, packed span and full SHA exact, verify green at 290. Evidence: `docs/evidence/volume-campaign-20260825/func-80077aa4/REPORT.md`. |
| VOLUME-17-5DBF8 | 290 | `func_8005DBF8` @ `0x4E3F8` is a six-word callable scalar-base helper with eight direct callers. Two `-O2 -G0` natural-C phrasings were bounded: the first differs only in address/load register coloring; the pointer-local retry exceeds the 0x18-byte span and trim rejects nonzero overflow. `PARKED-ADDRESS-REGISTER-COLORING`; no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-8005dbf8/PARK.md`; candidate is in the labeled stash. |
| VOLUME-18-877BC | 291 | `func_800877BC` @ `0x77FBC` is a six-word callable volatile hardware halfword setter with three direct callers and real boundaries. Era `-O2 -G0` reproduces all six words on the first phrasing; carve `0x394 + 0x18 + 0x3348 = 0x36F4`, packed span and full SHA exact, verify green at 291. Evidence: `docs/evidence/volume-campaign-20260825/func-800877bc/REPORT.md`. |
| VOLUME-19-5DC10 | 291 | `func_8005DC10` @ `0x4E410` is the exact six-word address-register-coloring twin of parked `func_8005DBF8`, over `D_800A8048` with offset `-0x20` and two direct callers. Screened without a duplicate attempt; no count change. Evidence: `docs/evidence/volume-campaign-20260825/func-8005dc10/SKIP.md`. |
| VOLUME-20-534CC | 291 | `func_800534CC` @ `0x43CCC` is a six-word gp-indexed signed-halfword getter with one direct caller and real boundaries. Two `-O2 -G8` phrasings both emit an absolute address instead of retail `$gp+0x2D8`; `PARKED-GP-ABSOLUTE-FORM`, no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-800534cc/PARK.md`; candidate is in the labeled stash. |
| VOLUME-21-5DE70 | 291 | `func_8005DE70` @ `0x4E670` is the same six-word address-register-coloring family over `D_800A8044` with offset `-0x1C` and one caller. Screened without a duplicate attempt; no count change. Evidence: `docs/evidence/volume-campaign-20260825/func-8005de70/SKIP.md`. |
| VOLUME-22-57D18 | 291 | `func_80057D18` @ `0x48518` is a six-word gp-indexed signed-halfword getter/clear twin of `func_800534CC`, with an exact-start function-pointer construction and real boundaries. Screened under the same gp-absolute-form blocker; no attempt/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-80057d18/SKIP.md`. |
| VOLUME-23-3708C | 291 | `func_8003708C` @ `0x2788C` is a seven-word fixed-point multiply with 65 direct callers and canonical return. Two `-O2 -G0` natural product forms retain wrong HI/LO temporary coloring and exceed the exact body; `PARKED-FIXED-POINT-REGISTER-COLORING`, no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-8003708c/PARK.md`; candidate is in the labeled stash. |
| VOLUME-24-762A0 | 291 | `func_800762A0` @ `0x66AA0` is a seven-word packed GPU command helper with three direct callers and canonical return. Two `-O2 -G0` phrasings retain the first masked/shifted value in `$v0` instead of retail `$a1`; `PARKED-REGISTER-COLORING`, no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-800762a0/PARK.md`; candidate is in the labeled stash. |
| VOLUME-25-631C0 | 292 | `func_800631C0` @ `0x539C0` is a seven-word nullable flag query with two direct callers and canonical return. The explicit result accumulator in `-O2 -G0` fills the null-branch delay slot exactly; carve `0x1C + 0x186C = 0x1888`, packed span/full SHA exact, verify green at 292. Evidence: `docs/evidence/volume-campaign-20260825/func-800631c0/REPORT.md`. |
| VOLUME-26-87728 | 293 | `func_80087728` @ `0x77F28` is a seven-word volatile MMIO halfword-pair setter with two direct callers and canonical return. Direct-address C overflows; a single-base indexed phrasing reproduces the `$at` store form on `-O2 -G0`; carve `0x300 + 0x1C + 0x78 = 0x394`, packed span/full SHA exact, verify green at 293. Evidence: `docs/evidence/volume-campaign-20260825/func-80087728/REPORT.md`. |
| VOLUME-27-8770C | 294 | `func_8008770C` @ `0x77F0C` is the adjacent seven-word volatile MMIO halfword-pair setter. The proven one-base pointer idiom matches on the first `-O2 -G0` phrasing; carve `0x2E4 + 0x1C = 0x300`, packed span/full SHA exact, verify green at 294. Evidence: `docs/evidence/volume-campaign-20260825/func-8008770c/REPORT.md`. |
| VOLUME-28-87744 | 295 | `func_80087744` @ `0x77F44` is the next seven-word volatile MMIO halfword-pair setter. The proven one-base pointer idiom matches on the first `-O2 -G0` phrasing; carve `0x1C + 0x1C + 0x5C = 0x90`, packed span/full SHA exact, verify green at 295. Evidence: `docs/evidence/volume-campaign-20260825/func-80087744/REPORT.md`. |
| VOLUME-29-87760 | 296 | `func_80087760` @ `0x77F60` is the next seven-word volatile MMIO halfword-pair setter. The proven one-base pointer idiom matches on the first `-O2 -G0` phrasing; carve `0x1C + 0x1C + 0x40 = 0x78`, packed span/full SHA exact, verify green at 296. Evidence: `docs/evidence/volume-campaign-20260825/func-80087760/REPORT.md`. |
| VOLUME-30-8777C | 297 | `func_8008777C` @ `0x77F7C` is the next seven-word volatile MMIO halfword-pair setter. The proven one-base pointer idiom matches on the first `-O2 -G0` phrasing; carve `0x1C + 0x1C + 0x24 = 0x5C`, packed span/full SHA exact, verify green at 297. Evidence: `docs/evidence/volume-campaign-20260825/func-8008777c/REPORT.md`. |
| VOLUME-31-877D4 | 298 | `func_800877D4` @ `0x77FD4` is a seven-word per-voice SPU halfword setter, distinct from the paired-control helpers. Natural indexed volatile C matches on the first `-O2 -G0` phrasing; carve `0x1C + 0x332C = 0x3348`, packed span/full SHA exact, verify green at 298. Evidence: `docs/evidence/volume-campaign-20260825/func-800877d4/REPORT.md`. |
| VOLUME-32-877F0 | 299 | `func_800877F0` @ `0x77FF0` is the per-voice SPU halfword-setter twin of `877D4`. Natural indexed volatile C matches on the first `-O2 -G0` phrasing; carve `0x1C + 0x3310 = 0x332C`, packed span/full SHA exact, verify green at 299. Evidence: `docs/evidence/volume-campaign-20260825/func-800877f0/REPORT.md`. |
| VOLUME-33-83C20 | 300 | `func_80083C20` @ `0x74420` is a seven-word callback initializer. Function hood is proven by the exact-start HI/LO callback-address construction in `func_80083BB8`, canonical return, and real boundaries. Natural `-O2 -G0` C matched first phrasing; carve `0x660 + 0x1C + 0x234 = 0x8B0`, packed span/full SHA exact, verify green at 300. Evidence: `docs/evidence/volume-campaign-20260825/func-80083c20/REPORT.md`. |
| VOLUME-34-5E8A4 | 301 | `func_8005E8A4` @ `0x4F0A4` is an eight-word gp-relative two-component accumulator with 247 direct calls. Natural `D_8009D124 += a0; D_8009D128 += a1;` matches first phrasing under era `-O2 -G8`; all GPREL16 relocations normalize to retail. Carve `0x20 + 0x294 = 0x2B4`, packed span/full SHA exact, verify green at 301. Evidence: `docs/evidence/volume-campaign-20260825/func-8005e8a4/REPORT.md`. |
| VOLUME-35-5E968 | 302 | `func_8005E968` @ `0x4F168` is an eight-word gp-relative packed-value setter with 16 direct calls. Natural signed shift/mask C matches first phrasing under era `-O2 -G8`; all GPREL16 relocations normalize to retail. Carve `0xA4 + 0x20 + 0x1D0 = 0x294`, packed span/full SHA exact, verify green at 302. Evidence: `docs/evidence/volume-campaign-20260825/func-8005e968/REPORT.md`. |
| VOLUME-36-661CC-COP2 | 302 | `func_800661CC` @ `0x569CC` is a proven callable handwritten projection reset helper with seven direct calls. Its semantic body writes GTE OFX/OFY via two `ctc2` instructions, unavailable in sanctioned ordinary C. Screened without an attempt as `SKIP-HANDWRITTEN-COP2`; it is not labeled SDK without provenance. Evidence: `docs/evidence/volume-campaign-20260825/func-800661cc/SKIP.md`. |
| VOLUME-37-5BEE8 | 303 | `func_8005BEE8` @ `0x4C6E8` is an eight-word gp-selected pointer getter with six direct calls. An explicit result pointer retains `D_800C0DE0` in `$v1`, conditionally adds `0x10`, and fills the return delay slot exactly under era `-O2 -G8`. Carve `0x20 + 0x580 = 0x5A0`, packed span/full SHA exact, verify green at 303. Evidence: `docs/evidence/volume-campaign-20260825/func-8005bee8/REPORT.md`. |
| VOLUME-38-5DADC | 304 | `func_8005DADC` @ `0x4E2DC` is an eight-word retained-address index helper with five direct calls. Attempt 1 duplicated the symbolic materialization; an explicit pointer local shared the load/base address in `$v1` and matched attempt 2 under era `-O2 -G0`. Carve `0x1644 + 0x20 + 0x618 = 0x1C7C`, packed span/full SHA exact, verify green at 304. Evidence: `docs/evidence/volume-campaign-20260825/func-8005dadc/REPORT.md`. |
| VOLUME-39-5DB8C | 304 | `func_8005DB8C` @ `0x4E38C` is an eight-word retained-address helper with four direct calls. Two `-O2 -G0` phrasings bounded the DAG: one shared the address with the wrong allocation/order; one duplicated materialization. Retail requires address-in-`$v0`, adjusted copy in `$v1`, then load overwriting `$v0`. `PARKED-ADDRESS-DAG-COLORING`; no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-8005db8c/PARK.md`; source is in the labeled stash. |
| VOLUME-40-GTE-POOL-SCREEN | 304 | Pool drift correction: callable `func_80078E04` and `func_80078E94` still appeared in Tier 1 despite the authoritative 23-function handwritten libGTE/COP2 screen. Each has three direct callers, canonical return, real boundaries, and unavoidable `ctc2` semantic bodies. Both moved to `SKIP-SDK-LIBRARY-COP2` without attempts; Tier 1 66→64, SKIP 743→745. Evidence: `docs/evidence/volume-campaign-20260824/COP2_SDK_SCREEN.md`. |
| VOLUME-41-83E50 | 305 | `func_80083E50` @ `0x74650` is an eight-word object initializer with two direct calls. Natural argument-relative pointer/byte stores match first phrasing under era `-O2 -G0`, including the final state store in the return delay slot. Carve `0x214 + 0x20 = 0x234`, packed span/full SHA exact, verify green at 305. Evidence: `docs/evidence/volume-campaign-20260825/func-80083e50/REPORT.md`. |
| VOLUME-42-83E84 | 306 | `func_80083E84` @ `0x74684` is the eight-word tag-`0x4C` initializer twin with two direct calls. Proven source shape matched first phrasing under era `-O2 -G0`; carve `0x20 + 0x40 = 0x60`, packed span/full SHA exact. The verifier first caught its stale `[0x74684, asm]` manifest marker; corrected C/asm registrations pass at 306. Evidence: `docs/evidence/volume-campaign-20260825/func-80083e84/REPORT.md`. |
| VOLUME-43-83EC4 | 307 | `func_80083EC4` @ `0x746C4` is the eight-word tag-`0x47` initializer twin with two direct calls. Proven source shape matched first phrasing under era `-O2 -G0`; carve `0x20 + 0x20 = 0x40`, packed span/full SHA exact, verify green at 307. Evidence: `docs/evidence/volume-campaign-20260825/func-80083ec4/REPORT.md`. |
| VOLUME-44-83EA4 | 308 | `func_80083EA4` @ `0x746A4` is the eight-word tag-`0x46` initializer twin with one direct call. Proven source shape matched first phrasing under era `-O2 -G0`; the old asm span closes exactly as `0x20 = 0x20`, packed span/full SHA exact, verify green at 308. Evidence: `docs/evidence/volume-campaign-20260825/func-80083ea4/REPORT.md`. |
| VOLUME-45-87798-PARK | 308 | `func_80087798` @ `0x77F98` is a proven callable nine-word per-voice SPU register-pair writer with three direct calls. Two `-O2 -G0` phrasings compile identically: cc1 keeps the scaled index in `$a0`, first address in `$v0`, and rematerializes the second through `$at`, while retail retains the shared address in `$a0`. `PARKED-MMIO-ADDRESS-RETENTION`; no integration/count change; consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-80087798/PARK.md`; source is in the labeled stash. |
| VOLUME-46-5D970 | 309 | `func_8005D970` @ `0x4E170` is a nine-word signed-threshold state selector with one direct call. An explicit default accumulator matched first phrasing under era `-O2 -G8`, including load/branch delay slots and normalized gp relocations. Carve `0x14D8 + 0x24 + 0x148 = 0x1644`, packed span/full SHA exact, verify green at 309; consecutive-park count reset. Evidence: `docs/evidence/volume-campaign-20260825/func-8005d970/REPORT.md`. |
| VOLUME-47-78C94-PARK | 309 | `func_80078C94` @ `0x69494` is a proven callable nine-word three-word-copy helper. Aggregate C keeps nine words but homes destination/result in `$v0` at entry; scalar C also fills the return delay slot and shrinks to eight words. Retail retains `$a0` through all stores and copies it to `$v0` afterward. `PARKED-AGGREGATE-RETURN-COLORING`; no integration/count change; consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-80078c94/PARK.md`; source is in the labeled stash. |
| VOLUME-48-792D4-SDK | 309 | `func_800792D4` @ `0x69AD4` is a proven callable ten-word handwritten libGTE operation helper with six direct calls. Its `lwc2`/`mvmva`/`swc2`/`cfc2` side effects have no sanctioned ordinary-C spelling. Screened as `SKIP-SDK-LIBRARY-COP2` without an attempt/count change; consecutive-park count remains 1. Evidence: `docs/evidence/volume-campaign-20260825/func-800792d4/SKIP.md`. |
| VOLUME-49-77B04 | 310 | `func_80077B04` @ `0x68304` is a ten-word argument-relative byte flag-2 set/clear helper with five direct calls and canonical return. Natural `-O2 -G0` C matched first phrasing, including the branch-local loads, local jump relocation, and return-delay-slot store. The alignment nops on either side remain asm; carve `0x48 + 0x28 + 0x38 = 0xA8`, packed span/full SHA exact, verify green at 310; consecutive-park count reset. Evidence: `docs/evidence/volume-campaign-20260825/func-80077b04/REPORT.md`. |
| VOLUME-50-77B34 | 311 | `func_80077B34` @ `0x68334` is the ten-word bit-0 byte flag helper, with four exact-start calls and canonical return; prior B54I evidence identifies `SetShadeTex`. Natural `-O2 -G0` C matched first phrasing. Both adjacent two-nop alignment fragments remain asm; carve `0x08 + 0x28 + 0x08 = 0x38`, packed span/full SHA exact, verify green at 311. Evidence: `docs/evidence/volume-campaign-20260825/func-80077b34/REPORT.md`. |
| VOLUME-51-8783C-PARK | 311 | `func_8008783C` @ `0x7803C` is a proven callable ten-word per-voice SPU halfword RMW. Two `-O2 -G0` phrasings match the first eight words but both emit `sh; jr; nop` instead of retail `jr; sh`; returning the computed `$v0` value does not alter volatile-store scheduling. `PARKED-VOLATILE-STORE-SCHEDULING`; no integration/count change, consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-8008783c/PARK.md`; source is in the labeled stash. |
| VOLUME-52-87864-SCREEN | 311 | `func_80087864` @ `0x78064` is the proven callable nibble-update twin over the same per-voice SPU register, with exact caller `0x80087A80`. The direct volatile `sh` return-slot blocker is identical to bounded `8783C`; screened without an attempt as `PARKED-VOLATILE-STORE-SCHEDULING-FAMILY`. Count stays 311 and consecutive park stays 1. Evidence: `docs/evidence/volume-campaign-20260825/func-80087864/SKIP.md`. |
| VOLUME-53-9059C | 312 | `func_8009059C` @ `0x80D9C` is a ten-word stream-byte flag helper. Function hood is proven by direct call `0x80090AC0`, exact-start callback-table entry `0x8009CBA8`, canonical return, and real boundaries. The adjacent `906B4` family phrasing matched first attempt under modern GCC 14 `-O1 -G0`; carve `0xD8 + 0x28 + 0xF0 = 0x1F0`, packed span/full SHA exact, verify green at 312. Consecutive-park count resets to zero. Evidence: `docs/evidence/volume-campaign-20260825/func-8009059c/REPORT.md`. |
| VOLUME-54-905C4 | 313 | `func_800905C4` @ `0x80DC4` is the adjacent ten-word stream-byte flag helper (`0x8000`, output `+0x112`). Function hood is independently proven by direct call `0x80090ACC`, callback-table entry `0x8009CBAC`, canonical return, and real boundaries. Proven modern GCC 14 `-O1 -G0` C matched first attempt; carve `0x00 + 0x28 + 0xC8 = 0xF0`, packed span/full SHA exact, verify green at 313. Evidence: `docs/evidence/volume-campaign-20260825/func-800905c4/REPORT.md`. |
| VOLUME-55-905EC | 314 | `func_800905EC` @ `0x80DEC` is the adjacent ten-word stream-byte flag helper (`0x2200`, output `+0x114`). Function hood is proven by exact-start callback-table entry `0x8009CBB4`, canonical return, and real boundaries, correcting the pool's stale `0/0` reference screen. Proven modern GCC 14 `-O1 -G0` C matched first attempt; carve `0x00 + 0x28 + 0xA0 = 0xC8`, packed span/full SHA exact, verify green at 314. Evidence: `docs/evidence/volume-campaign-20260825/func-800905ec/REPORT.md`. |
| VOLUME-56-90614 | 315 | `func_80090614` @ `0x80E14` is the adjacent ten-word stream-byte flag helper (`0x4400`, output `+0x116`). Function hood is proven by exact-start callback-table entry `0x8009CBB8`, canonical return, and real boundaries, correcting another stale `0/0` reference screen. Proven modern GCC 14 `-O1 -G0` C matched first attempt; carve `0x00 + 0x28 + 0x78 = 0xA0`, packed span/full SHA exact, verify green at 315. Evidence: `docs/evidence/volume-campaign-20260825/func-80090614/REPORT.md`. |
| VOLUME-57-9063C | 316 | `func_8009063C` @ `0x80E3C` is the adjacent ten-word stream-byte flag helper (`0x0100`, word output `+0x100`). Function hood is proven by exact-start callback-table entry `0x8009CBCC`, canonical return, and real boundaries, correcting its stale `0/0` reference screen. Explicit word-output GCC 14 `-O1 -G0` C matched first attempt; carve `0x00 + 0x28 + 0x50 = 0x78`, packed span/full SHA exact, verify green at 316. Evidence: `docs/evidence/volume-campaign-20260825/func-8009063c/REPORT.md`. |
| VOLUME-58-90664 | 317 | `func_80090664` @ `0x80E64` is the adjacent ten-word stream-byte flag helper (`0x0200`, word output `+0x104`). Function hood is proven by exact-start callback-table entry `0x8009CBDC`, canonical return, and real boundaries, correcting its stale `0/0` reference screen. Proven modern GCC 14 `-O1 -G0` C matched first attempt; carve `0x00 + 0x28 + 0x28 = 0x50`, packed span/full SHA exact, verify green at 317. Evidence: `docs/evidence/volume-campaign-20260825/func-80090664/REPORT.md`. |
| VOLUME-59-9068C | 318 | `func_8009068C` @ `0x80E8C` is the adjacent ten-word stream-byte flag helper (`0x0400`, word output `+0x108`). Function hood is proven by exact-start callback-table entry `0x8009CBEC`, canonical return, and real boundaries, correcting its stale `0/0` reference screen. Proven modern GCC 14 `-O1 -G0` C matched first attempt; final carve `0x00 + 0x28 + 0x00 = 0x28`, packed span/full SHA exact, verify green at 318. Evidence: `docs/evidence/volume-campaign-20260825/func-8009068c/REPORT.md`. |
| VOLUME-60-55FE0-PARK | 318 | `func_80055FE0` @ `0x467E0` is a proven callable eleven-word gp-backed bitset query with twelve direct callers. Two era `-O2 -G8` phrasings canonicalize retail's explicit mask/intersection/boolean DAG into nine-word shift-and-low-bit extraction; signedness changes only `srlv` to `srav`. `PARKED-BIT-TEST-CANONICALIZATION`; no integration/count change, consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-80055fe0/PARK.md`; source is in the labeled stash. |
| VOLUME-61-79244-SDK | 318 | `func_80079244` @ `0x69A44` is a proven callable eleven-word handwritten libGTE `RTPS` wrapper with six direct callers. Its `lwc2`/`rtps`/`swc2`/`cfc2`/`mfc2` side effects are not expressible in sanctioned ordinary C. Screened as `SKIP-SDK-LIBRARY-COP2` without an attempt or count change; consecutive parks remain 1. Evidence: `docs/evidence/volume-campaign-20260825/func-80079244/SKIP.md`. |
| VOLUME-62-3E0D0 | 319 | `func_8003E0D0` @ `0x2E8D0` is an eleven-word conditional state initializer, with exact caller `0x80019270`, canonical return, and real boundaries. Natural era `-O2 -G0` C matched first phrasing, including load/branch/jump delay slots and the normalized local jump. Carve `0x0F8 + 0x02C + 0x514 = 0x638`, packed span/full SHA exact, verify green at 319; consecutive parks reset. Evidence: `docs/evidence/volume-campaign-20260825/func-8003e0d0/REPORT.md`. |
| VOLUME-63-55FB4 | 320 | `func_80055FB4` @ `0x467B4` is an eleven-word gp-backed bitset setter, with exact caller `0x80044CC4`, canonical return, and real boundaries. Natural era `-O2 -G8` C matched first phrasing, including normalized `D_8009D058-_gp=0x2E8` and the return-slot store. Carve `0x1D14 + 0x002C + 0x1EEC = 0x3C2C`, packed span/full SHA exact, verify green at 320. Evidence: `docs/evidence/volume-campaign-20260825/func-80055fb4/REPORT.md`. |
| VOLUME-64-82ADC-PARK | 320 | `func_80082ADC` @ `0x732DC` is a proven callable eleven-word callback-record initializer with exact caller `0x80084618`. Retail retains one `D_800A5AB4` base in `$v0` across four stores; direct stores and an explicit-local retry both compile to fourteen words with repeated `$at` materialization. `PARKED-SYMBOLIC-BASE-RETENTION`; no integration/count change, consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-80082adc/PARK.md`; source is in the labeled stash. |
| VOLUME-65-74478 | 321 | `func_80074478` @ `0x64C78` is an eleven-word indexed callback-table setter, proven by an exact-start pointer returned from `func_800743B4` and stored into a callback slot. Reversing commutative comparison operands on phrasing 2 selected retail's `beq a1,v0`; all 11 words match under era `-O2 -G0`. Carve `0x124 + 0x02C + 0x000 = 0x150`, packed span/full SHA exact, verify green at 321; consecutive parks reset. Evidence: `docs/evidence/volume-campaign-20260825/func-80074478/REPORT.md`. |
| VOLUME-66-CE470 | 322 | `func_800CE470` @ `0xBEC70` is an eleven-word signed-byte callback, proven by exact-start table word `0x800E0FD0`, canonical return, and real boundaries. Natural era `-O2 -G0` C matched first phrasing, including the post-store sign extension and branch-delay constant. Carve `0x0000 + 0x002C + 0x63B4 = 0x63E0`, packed span/full SHA exact, verify green at 322; consecutive parks remain zero. Evidence: `docs/evidence/volume-campaign-20260825/func-800ce470/REPORT.md`. |
| VOLUME-67-8780C-PARK | 322 | `func_8008780C` @ `0x7800C` is a proven callable twelve-word per-voice SPU control update with three direct callers. Attempt 2 fixes size, register homes, expression DAG, and `jr; sh`, but GCC schedules the independent `$a2` shifts before retail's `$a0` address pair; four words remain reordered. `PARKED-INDEPENDENT-OP-SCHEDULING`; no integration/count change, consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-8008780c/PARK.md`; source is in the labeled stash. |
| VOLUME-68-878C0 | 323 | `func_800878C0` @ `0x780C0` is a twelve-word per-voice SPU halfword bitfield update, proven by direct caller `0x80087A48`, canonical return, and real boundaries. A grouped nonvolatile RMW matched first phrasing under era `-O2 -G0`, including retail's `$a2 |= $a1` dataflow and `jr; sh` ending. Carve `0x00B4 + 0x0030 + 0x322C = 0x3310`, packed span/full SHA exact, verify green at 323; consecutive parks reset. Evidence: `docs/evidence/volume-campaign-20260825/func-800878c0/REPORT.md`. |
| VOLUME-69-8788C | 324 | `func_8008788C` @ `0x7808C` is a thirteen-word per-voice SPU halfword bitfield update with three direct callers, canonical return, and real boundaries. The grouped nonvolatile RMW matched first phrasing under era `-O2 -G0`, including field-shift order, `$a2 |= $a1`, and `jr; sh`. Carve `0x080 + 0x034 + 0x000 = 0x0B4`, packed span/full SHA exact, verify green at 324; consecutive parks remain zero. Evidence: `docs/evidence/volume-campaign-20260825/func-8008788c/REPORT.md`. |
| VOLUME-70-C8C4C | 325 | `func_800C8C4C` @ `0xB944C` is a thirteen-word signed-halfword callback, proven by exact-start table word `0x800E0848`, canonical return, and real boundaries. Natural era `-O2 -G0` C matched first phrasing, including post-store sign extension and the branch-delay constant. Carve `0x08C + 0x034 + 0x288 = 0x348`, packed span/full SHA exact, verify green at 325; consecutive parks remain zero. Evidence: `docs/evidence/volume-campaign-20260825/func-800c8c4c/REPORT.md`. |
| VOLUME-71-C9A00 | 326 | `func_800C9A00` @ `0xBA200` is the thirteen-word signed-halfword callback twin, proven by exact-start table word `0x800E09BC`, canonical return, and real boundaries. Proven era `-O2 -G0` C matched first phrasing. Carve `0x08C + 0x034 + 0x1CC = 0x28C`, packed span/full SHA exact, verify green at 326; consecutive parks remain zero. Evidence: `docs/evidence/volume-campaign-20260825/func-800c9a00/REPORT.md`. |
| VOLUME-72-CA540 | 327 | `func_800CA540` @ `0xBAD40` is the third thirteen-word signed-halfword callback twin, proven by exact-start table word `0x800E0AAC`, canonical return, and real boundaries. Proven era `-O2 -G0` C matched first phrasing. Carve `0x08C + 0x034 + 0x224 = 0x2E4`, packed span/full SHA exact, verify green at 327; consecutive parks remain zero. Evidence: `docs/evidence/volume-campaign-20260825/func-800ca540/REPORT.md`. |
| VOLUME-73-CBBBC | 328 | `func_800CBBBC` @ `0xBC3BC` is the fourth thirteen-word signed-halfword callback twin, proven by exact-start table word `0x800E0BA8`, canonical return, and real boundaries. Proven era `-O2 -G0` C matched first phrasing. Carve `0x08C + 0x034 + 0x3B4 = 0x474`, packed span/full SHA exact, verify green at 328; consecutive parks remain zero. Evidence: `docs/evidence/volume-campaign-20260825/func-800cbbbc/REPORT.md`. |
| VOLUME-74-77CB4 | 329 | `func_80077CB4` @ `0x684B4` is a fourteen-word packet-chain merge helper, proven by eleven direct calls, canonical return, and real boundaries. A signed length plus explicit result accumulator matched second phrasing under era `-O2 -G0`; the local jump is exact after relocation normalization. The audit corrects old 13-word evidence: `0x80077CE8` is the return delay slot. Carve `0x003C + 0x0038 + 0x2638 = 0x26AC`, packed span/full SHA exact, verify green at 329; consecutive parks remain zero. Evidence: `docs/evidence/volume-campaign-20260825/func-80077cb4/REPORT.md`. |
| VOLUME-75-83790-PARK | 329 | `func_80083790` @ `0x73F90` is a proven callable fourteen-word packed-offset calculator with exact caller `0x80083738`. Attempt 2 reaches 9/14 exact and recovers load order, register homes, main arithmetic, and the base addition in the return delay slot; GCC's residual is reassociating independent `+4` onto the first term instead of retail's post-mask second term. `PARKED-ARITHMETIC-ASSOCIATION-SCHEDULING`; no integration/count change, consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-80083790/PARK.md`; source is in the labeled stash. |
| VOLUME-76-77AC4-PARK | 329 | `func_80077AC4` @ `0x682C4` is a proven callable fifteen-word low-24 link exchange with 55 direct callers. Direct C reproduces size, memory schedule, semantics, and return-delay store, but colors low/high masks into `$a3/$a2` versus retail `$a2/$a3`; the explicit-value retry does not move the masks. `PARKED-MASK-CONSTANT-COLORING`; no integration/count change, consecutive park 2. Evidence: `docs/evidence/volume-campaign-20260825/func-80077ac4/PARK.md`; source is in the labeled stash. |
| VOLUME-77-77A64 | 330 | `func_80077A64` @ `0x68264` is the fifteen-word Psy-Q `GetTPage` bit packer, proven by 36 direct calls, canonical return with a live delay slot, and real boundaries. B54I evidence proves the semantics; the natural expression matched first phrasing under era `-O2 -G0`. Carve `0x0018 + 0x003C + 0x0004 = 0x0058`, packed span/full SHA exact, verify green at 330; the match resets consecutive parks from two to zero. Evidence: `docs/evidence/volume-campaign-20260825/func-80077a64/REPORT.md`. |
| VOLUME-78-749D8 | 331 | `func_800749D8` @ `0x651D8` is a fifteen-word display-environment initializer, proven by four direct calls, canonical return with a live store delay slot, and real adjacent functions. Its 20-byte layout makes Psy-Q `SetDefDispEnv` probable but not string/symbol-proven. Natural struct C matched first phrasing under era `-O2 -G0`; carve `0x0268 + 0x003C + 0x0000 = 0x02A4`, packed span/full SHA exact, verify green at 331. Evidence: `docs/evidence/volume-campaign-20260825/func-800749d8/REPORT.md`. |
| VOLUME-79-63158 | 332 | `func_80063158` @ `0x53958` is a sixteen-word null-safe paired position updater, proven by 30 direct calls, canonical return, and real adjacent functions. Natural struct C updates argument words `+0x18/+0x1C` and proven gp pair `D_8009D124/D_8009D128`; it matched first phrasing under era `-O2 -G8` after gp relocation normalization. Carve `0x0474 + 0x0040 + 0x0000 = 0x04B4`, packed span/full SHA exact, verify green at 332. Evidence: `docs/evidence/volume-campaign-20260825/func-80063158/REPORT.md`. |
| VOLUME-80-5E988-PARK | 332 | `func_8005E988` @ `0x4F188` is a proven callable sixteen-word three-way packed-value selector with twelve direct callers, canonical return, and real adjacent functions. Two era `-O2 -G8` phrasings preserve the semantics but not retail's interleaving of three constant constructions with two branch slots and a local-jump slot: natural nested selection chooses an early jump, while an explicit default hoists the first constant and shrinks to fifteen content words. `PARKED-CONTROL-FLOW-CONSTANT-SCHEDULING`; no integration/count change, consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-8005e988/PARK.md`; source is in the labeled stash. |
| VOLUME-81-3F758-COP2 | 332 | `func_8003F758` @ `0x2FF58` is a proven callable sixteen-word handwritten GTE helper with exact caller `0x8003F0D0`, canonical return, and real boundaries. It writes control registers 13–15 (`RBK/GBK/BBK`) via three `ctc2` instructions and zeroes nine buffer halfwords. Screened without an attempt as `SKIP-HANDWRITTEN-COP2`; exact SDK provenance/name is not claimed. Count stays 332 and consecutive parks remain 1. Evidence: `docs/evidence/volume-campaign-20260825/func-8003f758/SKIP.md`. |
| VOLUME-82-63428 | 333 | `func_80063428` @ `0x53C28` is a seventeen-word guarded value calculator, proven by 59 direct calls, canonical return, and real adjacent functions. Natural explicit-result C matches first phrasing under era `-O2 -G0`, including all load-delay nops and `mult/mflo` allocation. Carve `0x024C + 0x0044 + 0x15DC = 0x186C`, packed span/full SHA exact, verify green at 333; consecutive parks reset. Evidence: `docs/evidence/volume-campaign-20260825/func-80063428/REPORT.md`. |
| VOLUME-83-6E454-PARK | 333 | `func_8006E454` @ `0x5EC54` is a proven callable seventeen-word decimal parser for field-name bytes 2–4, with two direct callers, canonical return, and real boundaries. Explicit signed bytes recover retail's `lb` operations, but both bounded era `-O2 -G0` phrasings hoist byte 4 before the scaled hundreds/tens terms; retail defers it and reuses `$v1`. `PARKED-INDEPENDENT-LOAD-SCHEDULING`; no integration/count change, consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-8006e454/PARK.md`; source is in the labeled stash. |
| VOLUME-84-653B8 | 334 | `func_800653B8` @ `0x55BB8` is the eighteen-word mailbox queue append, proven by its sole direct opcode-`0x1C` caller, canonical return, real boundaries, and independent PE-MBX1/2 state evidence. A natural 12-byte aggregate subscript matches first phrasing under era `-O2 -G8` after ordinary relocations. Carve `0x0788 + 0x0048 + 0x51BC = 0x598C`, packed span/full SHA exact, verify green at 334; consecutive parks reset. Evidence: `docs/evidence/volume-campaign-20260825/func-800653b8/REPORT.md`. |
| VOLUME-85-5DBAC-PARK | 334 | `func_8005DBAC` @ `0x4E3AC` is a proven callable nineteen-word clamped symbolic-table address helper with eleven direct callers, canonical return, and real boundaries. Both bounded era `-O2 -G0` phrasings preserve proven B28 semantics but miss retail's lifetime split: retail copies index to `$v1` so `$a0` can later hold the symbolic address/value; natural C either keeps index in `$a0` or hoists the pointer into `$a1`, yielding eighteen content words. `PARKED-SYMBOLIC-ADDRESS-LIFETIME-COLORING`; no integration/count change, consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260825/func-8005dbac/PARK.md`; source is in the labeled stash. |
| VOLUME-86-43474-PARK | 334 | `func_80043474` @ `0x33C74` is a proven callable nineteen-word signed threshold classifier with two direct callers, canonical return, and real boundaries. Both bounded era `-O2 -G0` phrasings emit twenty words: cc1 keeps a separate category-4 branch/jump, while retail hoists the fifth compare into that branch's delay slot and places category 4 after the 5/6 jump. `PARKED-THRESHOLD-LADDER-BLOCK-LAYOUT`; no integration/count change, consecutive park 2. Evidence: `docs/evidence/volume-campaign-20260825/func-80043474/PARK.md`; source is in the labeled stash. |
| VOLUME-87-73244-PARK/STOP | 334 | `func_80073244` @ `0x63A44` is a proven callable twenty-word unsigned word-pair comparator with two direct callers, canonical return, and real boundaries. A by-value two-struct phrasing recovers retail's first six words, including all four ABI argument homes, but cc1 folds the low-word three-way comparison into `sltu` + `negu` and shrinks to seventeen content words. `PARKED-LEXICOGRAPHIC-COMPARE-CANONICALIZATION`; no integration/count change, consecutive park 3. Hard stop fires honestly after `5DBAC/43474/73244`, with no Tier-1 row skipped. Evidence: `docs/evidence/volume-campaign-20260825/func-80073244/PARK.md`; source is in the labeled stash. |
| VOLUME-88-6DB48 | 335 | `func_8006DB48` @ `0x5E348` is a proven callable twenty-one-word byte-field updater and flag setter over word-backed `D_800B0CD8`, with four exact-start callers, canonical return, and real boundaries. A union overlay preserves the shared symbolic base and matches all 21 words under era `-O2 -G0` after ordinary relocations; carve `0x3164 + 0x54 + 0xC4C = 0x3E04`, packed span/full SHA exact, verify green at 335. Evidence: `docs/evidence/volume-campaign-20260825/func-8006db48/REPORT.md`. |
| VOLUME-89-6E6B0-PARK | 335 | `func_8007E6B0` @ `0x6EEB0` is hood-proven by four direct calls and real boundaries. Two era `-O2 -G0` phrasings preserve its ring-buffer record-address semantics but color the returned symbolic base into `$a0`, rematerialize the `-8` load, and emit 0x60 versus retail 0x54. `PARKED-SYMBOLIC-ADDRESS-REGISTER-COLORING`; no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-8007e6b0/PARK.md`. |
| VOLUME-90-5BCBC-PARK | 335 | `func_8005BCBC` @ `0x4C4BC` is hood-proven by two direct calls and real boundaries. Two era `-O2 -G8` phrasings preserve the GP-backed table/status selection but keep status in `$a1` rather than retail `$v0`, schedule the table base later, and emit 20 versus retail 21 words. `PARKED-GP-STATUS-REGISTER-AND-DELAY-SCHEDULE`; no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-8005bcbc/PARK.md`. |
| VOLUME-91-83D9C-PARK | 335 | `func_80083D9C` @ `0x7459C` is hood-proven by exact-start callback pointer storage and real boundaries. Two era `-O2 -G0` switch phrasings preserve its two object-mode cases but cc1 reloads the mode and lays out the shared tail differently, emitting 0x60 versus retail 0x54. `PARKED-SWITCH-TAIL-BLOCK-LAYOUT`; no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-80083d9c/PARK.md`. |
| VOLUME-92-79178-SKIP | 335 | `func_80079178` @ `0x69978` is a hood-proven handwritten GTE/COP2 wrapper using `cfc2`, `ctc2`, `lwc2`, a COP2 operation, and `swc2`. It is `SKIP-SDK-LIBRARY-COP2` without an ordinary-C attempt; no integration/count change. Evidence: `docs/evidence/volume-campaign-20260825/func-80079178/SKIP.md`. |
| VOLUME-93-791D0-SKIP | 335 | `func_800791D0` @ `0x699D0` is the hood-proven structural twin of `func_80079178`, differing in its COP2 operation word. It is `SKIP-SDK-LIBRARY-COP2` without an ordinary-C attempt; no integration/count change. Evidence: `docs/evidence/volume-campaign-20260829/func-800791d0/SKIP.md`. |
| VOLUME-94-HARD-STOP | 335 | The continuation campaign hard-stops after three consecutive bounded parks: `func_8007E6B0`, `func_8005BCBC`, and `func_80083D9C`. The later `79178/791D0` COP2 family screens spent zero C attempts and do not reset the stop. No further ordinary-C attempt until review/refresh; 16 Tier-1 rows remain queued. |
| VOLUME-95-POOL-REFRESH | 335 | A fresh campaign authorization resets the prior stop and rebuilds the pool from current YAML-selected asm spans. Closure is 1,094 active spans: Tier 1 16, Tier 2 214, Tier 3 99, SKIP/suppressed 765. The 47-row reconciliation includes five one-word `func_` labels independently classified as alignment padding after zero exact-start references and real neighboring function boundaries. Base Docker build is exact SHA, verify is green, and count remains 335. Evidence: `docs/evidence/volume-campaign-20260830/POOL_REFRESH.md`. |
| VOLUME-96-78C34-SKIP | 335 | `func_80078C34` @ `0x69434` is function-hood proven by 25 raw direct callers, canonical `jr ra; nop`, and real boundaries. Its 23-word split is explicitly handwritten and performs matrix/vector work through `ctc2`, `lwc2`, `mvmva`, and `mfc2`; the existing libGTE/PsyCross policy therefore classifies it `SKIP-SDK-LIBRARY-COP2` with zero C phrasings and no integration. Tier 1 falls 16→15; consecutive parks remain zero. Evidence: `docs/evidence/volume-campaign-20260830/func-80078c34/SKIP.md`. |
| VOLUME-97-3C5D8 | 336 | `func_8003C5D8` @ `0x2CDD8` is a hood-proven 24-word reciprocal-byte initializer with 26 direct callers, canonical return, and real boundaries. Phrasing 1 used an `int` formal and lost retail's entry copy; the proven signed-`short` formal forces `a1 -> a2`, and phrasing 2 matches all 24 words under era `-O2 -G0` plus the sanctioned signed-div guard expansion. Carve `0x3864 + 0x0060 + 0x11F4 = 0x4AB8`; packed span/full SHA exact, verify green at 336. Evidence: `docs/evidence/volume-campaign-20260830/func-8003c5d8/REPORT.md`. |
| VOLUME-98-70D6C-PARK | 336 | `func_80070D6C` @ `0x6156C` is hood-proven by four direct callers (correcting the pool's stale 3), canonical return, and real boundaries. Independent RNG oracles close its self-adjacent state semantics and Stage-0 census. Two natural era `-O2 -G0` phrasings compile byte-identically to 24 versus retail 25 words: cc1 uses `$v0/$a*` homes, steals both cursor decrements into load gaps, computes the sum directly in `$v0`, and schedules different guard slots; retail preserves a full `$t0–$t8` home plan and explicit result copy. `PARKED-REGISTER-HOME-AND-CROSS-BLOCK-SCHEDULING`, `ACCEPTED-RESIDUAL`; no integration/count change. Candidate stash: `park func_80070D6C register-schedule residual`. Evidence: `docs/evidence/volume-campaign-20260830/func-80070d6c/PARK.md`. |
| VOLUME-99-6346C | 337 | `func_8006346C` @ `0x53C6C` is a hood-proven 26-word guarded record-bit query with four direct callers, canonical return, and real boundaries. Phrasing 1 initialized the default result at entry, extending its lifetime into `$a2`, forcing `mflo a3`, and emitting 27 words. Delaying only that default assignment frees `$a2` for the product and selects `$a0` as the late result home; phrasing 2 matches all 26 words under era `-O2 -G0`, including the late guard-slot constant. Carve `0x0000 + 0x0068 + 0x1574 = 0x15DC`; packed span/full SHA exact, verify green at 337, and consecutive parks reset. Banked lever: delay default-result initialization when retail needs an argument register for an earlier product and another as the late result accumulator. Evidence: `docs/evidence/volume-campaign-20260830/func-8006346c/REPORT.md`. |
| VOLUME-100-3F798-SKIP | 337 | `func_8003F798` @ `0x2FF98` is hood-proven by three direct callers, canonical `jr ra; nop`, and real boundaries. The split explicitly marks its 26-word body handwritten, and five required `ctc2` instructions write GTE control registers 16–20 after argument-relative buffer updates. The established COP2 expressibility rule classifies it `SKIP-HANDWRITTEN-COP2` with zero C phrasings, no integration, and no count change; exact Psy-Q routine provenance is not asserted. Pool remains 1,092 active spans, with Tier 1 11 and SKIP/suppressed 768. Evidence: `docs/evidence/volume-campaign-20260830/func-8003f798/SKIP.md`. |
| VOLUME-101-12700 | 338 | `func_80012700` @ `0x2F00` is a hood-proven 29-word task-freelist pop/link initializer with seven direct callers, canonical return with live result delay slot, and real boundaries. The split-local serial phrasing swapped `$v0/$v1` homes and hoisted the active store; preserving `task->serial = D_8009D308++` matched phrasing 2 under era `-O2 -G8`, including four normalized gp relocations and one local jump. Carve `0x0000 + 0x0074 + 0x0000 = 0x0074`; 29/29 object, packed span, full SHA exact, verify green at 338. Banked lever: preserve a scalar postincrement assignment when retail stores the old value into a record and writes old+1 back later; splitting the operation can extend liveness and reorder independent stores. Evidence: `docs/evidence/volume-campaign-20260830/func-80012700/REPORT.md`. |
| VOLUME-102-79304-SKIP | 338 | `func_80079304` @ `0x69B04` is function-hood proven by two direct callers, canonical `jr ra` with a live shift delay slot, and real handwritten neighbors separated by two-nop alignment gaps on each side. Its explicitly handwritten 30-word body requires `lwc2`, `rtpt`, `rtps`, `swc2`, `cfc2`, and `mfc2`; the established libGTE/PsyCross rule therefore classifies it `SKIP-SDK-LIBRARY-COP2` with zero C phrasings and no integration/count change. Tier 1 falls 10→9; exact Psy-Q routine naming remains unproven. Evidence: `docs/evidence/volume-campaign-20260830/func-80079304/SKIP.md`. |
| VOLUME-103-3DF50 | 339 | `func_8003DF50` @ `0x2E750` is a hood-proven 30-word signed-index 16-byte-record initializer with one direct caller, canonical return with live halfword-store delay slot, and immediate real boundaries. Natural typed-struct C matched first phrasing under era `-O2 -G0`: the signed-short formal recovers retail's `sll 16; sra 12` scale, and repeated source-field expressions preserve four alias-sensitive table-base reloads. Carve `0x071C + 0x0078 + 0x0000 = 0x0794`; 30/30 object, packed span, full SHA exact, verify green at 339. Banked lever: keep a signed-short formal plus typed 16-byte array and avoid hoisting an explicit record pointer when retail reloads a potentially aliased source base between destination stores. Evidence: `docs/evidence/volume-campaign-20260830/func-8003df50/REPORT.md`. |
| VOLUME-104-78554-SKIP | 339 | `func_80078554` @ `0x68D54` is function-hood proven by two direct callers and canonical `jr ra; nop`; its following one-word alignment gap precedes another real handwritten function. The split explicitly marks this 31-word fixed-point three-byte interpolation helper handwritten, and its semantics require `mtc2`, `gpf`, `gpl`, and `mfc2`. The established libGTE/PsyCross screen classifies it `SKIP-SDK-LIBRARY-COP2` with zero C phrasings and no integration/count change; exact routine naming is not asserted. Tier 1 falls 8→7, SKIP/suppressed rises 769→770, and consecutive parks remain zero. Evidence: `docs/evidence/volume-campaign-20260830/func-80078554/SKIP.md`. |
| VOLUME-105-CE870 | 340 | `func_800CE870` @ `0xBF070` is a hood-proven 32-word two-source position selector with 27 direct callers, canonical return, and immediate real boundaries. Phrasing 1 recovered the full schedule but modeled three isolated shorts and emitted `lhu`; modeling the evidence-backed aligned signed 16.16 fields and `>> 16` conversions folds to retail's three high-half `lh` operations. Phrasing 2 matches 32/32 under era `-O2 -G0` after two local-jump relocations. Carve `0x03D4 + 0x0080 + 0x5F60 = 0x63B4`; packed span/full SHA exact, verify green at 340. Banked lever: a signed high-half `lh` at `+2` within aligned four-byte fields can arise from `signed int >> 16`, while isolated short-to-short copies may canonicalize to `lhu`. Evidence: `docs/evidence/volume-campaign-20260830/func-800ce870/REPORT.md`. |
| VOLUME-106-339A0-PARK | 340 | `func_800339A0` @ `0x241A0` is hood-proven by six direct callers, canonical return, and immediate real boundaries. It copies a 16-byte four-pair constant table, selects by the low byte of its argument, and is the sole writer of gp state `D_8009CE80/84/86`. Two era `-O2 -G8` source orders compile byte-identically: cc1 keeps the selected pair address in `$v0`, hoists the second `lhu`, and groups the three GP stores, while retail keeps the address in `$v1`, the result in `$v0`, and stores the first half before the second load. `PARKED-GP-LOAD-STORE-SCHEDULING-AND-REGISTER-HOME`, `ACCEPTED-RESIDUAL`; no integration/count change, consecutive park 1. Evidence: `docs/evidence/volume-campaign-20260830/func-800339a0/PARK.md`; source is in the labeled stash. |
| VOLUME-107-80C48-PARK | 340 | `func_80080C48` @ `0x71448` is hood-proven by six direct callers, canonical return with live arithmetic, and immediate real boundaries. Its libCD `CdPosToInt` semantics are independently tested: decode BCD minute/second/frame and compute `((m*60+s)*75+f)-150`. Both era `-O2 -G0` phrasings emit exactly 32 words, but cc1 hoists the independent byte-2 load to entry and changes all later BCD accumulator homes; the explicit-result phrasing recovers only the first two load homes. `PARKED-INDEPENDENT-LOAD-AND-BCD-ACCUMULATOR-SCHEDULING`, `ACCEPTED-RESIDUAL`; no integration/count change, consecutive park 2. This is a recognized independent-load family, so H2 does not fire. Evidence: `docs/evidence/volume-campaign-20260830/func-80080c48/PARK.md`; source is in the labeled stash. |
| VOLUME-108-762BC-PARK/STOP | 340 | `func_800762BC` @ `0x66ABC` is hood-proven by four direct callers, canonical return with live frame teardown, and immediate real boundaries. It is the plain-C libGPU E2 texture-window packer. The attested direct `_get_tw` expression compiles to 25 content words without retail's component homes; a four-element local-array retry recovers the 16-byte frame, all four stores, and exact 32-word size, but cc1 chooses the opposite null-arm layout, schedules Y before W, and changes X/W homes. `PARKED-TEXTURE-WINDOW-CONTROL-FLOW-LOAD-SCHEDULE-AND-COLORING`, `ACCEPTED-RESIDUAL`; no integration/count change. This is the third consecutive park after `339A0/80C48`, so H5 fires with no Tier-1 row skipped. Evidence: `docs/evidence/volume-campaign-20260830/func-800762bc/PARK.md`; source is in the labeled stash. |
| VOLUME-109-7AA34-FAMILY | 340 | New explicit close-out authorization resets the historical H5 stop. `func_8007AA34` @ `0x6B234` is hood-proven by exact caller `0x8007C2C0`, canonical return with live subtract, and immediate real boundaries. Its entire 32-word retail body has SHA-256 `91ea1fb7…bdfecf2`, exactly equal to parked `func_80080C48`; it is the same `CdPosToInt` semantics and independent-load/BCD-accumulator residual. Screened as a family member with zero duplicate phrasings, `ACCEPTED-RESIDUAL`; count remains 340. Evidence: `docs/evidence/volume-campaign-20260830/func-8007aa34/PARK.md`; source is in the labeled stash. |
| VOLUME-110-21850 | 341 | `func_80021850` @ `0x12050` is a hood-proven 34-word swap of two 12-byte records selected by signed-byte indices, with three direct callers, canonical return, and immediate real boundaries. Natural typed aggregate C matches 34/34 on the first era `-O2 -G0` phrasing, including the retail 16-byte stack temporary and both retained record addresses. Carve `0x0938 + 0x0088 + 0x7AB0 = 0x8470`; packed span/full SHA exact, verify green at 341. Evidence: `docs/evidence/volume-campaign-20260830/func-80021850/REPORT.md`. |
| VOLUME-111-783E4-SKIP | 341 | `func_800783E4` @ `0x68BE4` is function-hood proven by three raw direct callers, canonical `jr ra; nop`, and immediate real handwritten boundaries. The split explicitly marks its 34-word body handwritten, and its semantics require `mtc2`, `gpf`, `gpl`, and `mfc2`. The established libGTE/PsyCross policy classifies it `SKIP-SDK-LIBRARY-COP2` with zero C phrasings and no integration/count change. Tier 1 is now closed at zero rows. Evidence: `docs/evidence/volume-campaign-20260830/func-800783e4/SKIP.md`. |
| VOLUME-112-C6ED8 | 342 | The separately authorized Tier-2 probe opens with `func_800C6ED8` @ `0xB76D8`, a hood-proven four-word unsigned-halfword state setter with six direct callers, canonical return, and immediate real boundaries. Stage-0 finds this sole `sh` writer and one `lhu` reader of `D_800F33E4`. Natural era `-O2 -G0` C matches 4/4 on phrasing 1. Carve `0x4370 + 0x0010 + 0x0EDC = 0x525C`; packed span/full SHA exact, verify green at 342. Tier 2 falls 214→213. Evidence: `docs/evidence/volume-campaign-20260830/func-800c6ed8/REPORT.md`. |
| VOLUME-113-C6EC0 | 343 | `func_800C6EC0` @ `0xB76C0` is the hood-proven six-word dual-halfword setter immediately before C6ED8, with seven direct callers, canonical return, and real boundaries. It is the sole `sh` writer of `D_800F346C/D_800F3414`; one reader loads both with `lhu`. Ordered natural era `-O2 -G0` C matches 6/6 on phrasing 1. Carve `0x4358 + 0x0018 = 0x4370`; packed span/full SHA exact, verify green at 343. Tier 2 falls 213→212. Evidence: `docs/evidence/volume-campaign-20260830/func-800c6ec0/REPORT.md`. |
| VOLUME-114-50020 | 344 | `func_80050020` @ `0x40820` is hood-proven by its canonical return, immediate real boundaries, and an exact-start address construction stored into callback slot `+0x8C` at `0x80048420–0x8004842C`. Natural indexed word getter C matches 6/6 on phrasing 1 under era `-O2 -G0` with the established default-off three-word symbolic-load gate, selecting retail's `$at` address temporary without changing maspsx. Carve `0x07E8 + 0x0018 + 0x0CE0 = 0x14E0`; packed span/full SHA exact, verify green at 344. Tier 2 falls 212→211. Evidence: `docs/evidence/volume-campaign-20260830/func-80050020/REPORT.md`. |
| VOLUME-115-7C544 | 345 | `func_8007C544` @ `0x6CD44` is hood-proven by one direct caller, a canonical return with live store delay, and real neighboring functions separated by explicit alignment nops. Natural ordered stores to three Stage-0-censused state words match 7/7 on phrasing 1 under era `-O2 -G0` with the established default-off store-delay gate. Carve `0x0408 + 0x001C + 0x1944 = 0x1D68`; packed span/full SHA exact, verify green at 345. Tier 2 falls 211→210. Evidence: `docs/evidence/volume-campaign-20260830/func-8007c544/REPORT.md`. |
| VOLUME-116-812F4 | 346 | `func_800812F4` @ `0x71AF4` is hood-proven by one direct caller, canonical `jr ra; nop`, a preceding real return with live teardown, and a following one-word alignment gap before the next real prologue. Natural unsigned range-guarded state-setter C matches 7/7 on phrasing 1 under era `-O2 -G0` with no maspsx gate. Carve `0x008C + 0x001C + 0x0F9C = 0x1044`; packed span/full SHA exact, verify green at 346. Tier 2 falls 210→209. Evidence: `docs/evidence/volume-campaign-20260830/func-800812f4/REPORT.md`. |
| VOLUME-117-62F1C | 347 | `func_80062F1C` @ `0x5371C` is hood-proven by 57 unique executable direct callers, canonical return, and immediate real boundaries. Its one unresolved callee is the recursive node-removal routine `func_8006269C`; natural void forwarding-wrapper C matches all eight words on phrasing 1 under era `-O2 -G0`, with only the normalized call relocation. Carve `0x0238 + 0x0020 + 0x021C = 0x0474`; packed span/full SHA exact, verify green at 347. Tier 2 falls 209→208. Evidence: `docs/evidence/volume-campaign-20260830/func-80062f1c/REPORT.md`. |
| VOLUME-118-C2AF0 | 348 | `func_800C2AF0` @ `0xB32F0` is hood-proven by eight direct callers, canonical return with live indexed-store delay slot, and immediate real boundaries. Natural typed C advances the base by 12 bytes, publishes it in Stage-0-censused `D_800E2248`, writes the indexed `+0x48` slot, and returns zero; it matches all eight words on phrasing 1 under era `-O2 -G0` with no gate. Carve `0x07F8 + 0x0020 + 0x0030 = 0x0848`; packed span/full SHA exact, verify green at 348. Tier 2 falls 208→207. Evidence: `docs/evidence/volume-campaign-20260830/func-800c2af0/REPORT.md`. |
| VOLUME-119-84FC4 | 349 | `func_80084FC4` @ `0x757C4` is hood-proven by seven direct callers, canonical return with live store delay, and real boundaries separated from its predecessor by explicit alignment. It samples root counter 2 at `0x1F801120`, stores the caller limit and sampled baseline, and matches all eight words on natural phrasing 1 under era `-O2 -G0` with the established store-delay gate. Carve `0x0814 + 0x0020 + 0x00A0 = 0x08D4`; packed span/full SHA exact, verify green at 349. Tier 2 falls 207→206. Evidence: `docs/evidence/volume-campaign-20260830/func-80084fc4/REPORT.md`. |
| VOLUME-120-80AE4 | 350 | `func_80080AE4` @ `0x712E4` is hood-proven by three direct callers, canonical return with live teardown delay, and immediate real boundaries. Its unresolved callee `func_8007BF44` performs an ordered hardware transaction; natural `callee(...) == 0` C reproduces the eight-word frame and in-place `sltiu` result on phrasing 1 under era `-O2 -G0`, with only the normalized call relocation. Carve `0x0194 + 0x0020 + 0x01C4 = 0x0378`; packed span/full SHA exact, verify green at 350. Tier 2 falls 206→205. Evidence: `docs/evidence/volume-campaign-20260830/func-80080ae4/REPORT.md`. |
| VOLUME-121-4DC84 | 351 | `func_8004DC84` @ `0x3E484` is hood-proven by one direct caller, canonical return, and immediate real boundaries. Natural constant forwarding-wrapper C passes `0x2A` to unresolved `func_80062F3C` in the call delay slot and matches all eight words on phrasing 1 under era `-O2 -G0`, with only the normalized call relocation. Carve `0x01E0 + 0x0020 + 0x0CCC = 0x0ECC`; packed span/full SHA exact, verify green at 351. Tier 2 falls 205→204; the bounded ten-leaf probe is closed before the native-runtime pivot. Evidence: `docs/evidence/volume-campaign-20260830/func-8004dc84/REPORT.md`. |
| VOLUME-122-7A488 | 352 | A sustained-work continuation resumes Tier 2 with `func_8007A488` @ `0x6AC88`, hood-proven by its canonical return, immediate real boundaries, and the exact caller at `0x8007C5E8`. Natural result-returning forwarding-wrapper C calls the string-identified PsyQ `CD_ready` body and matches all eight words on phrasing 1 under era `-O2 -G0`, with one normalized call relocation. Carve `0x0088 + 0x0020 = 0x00A8`; packed span/full SHA exact, verify green at 352, and Tier 2 falls 204→203. Evidence: `docs/evidence/volume-campaign-20260830/func-8007a488/REPORT.md`. |
| VOLUME-123-7A88C | 353 | `func_8007A88C` @ `0x6B08C` is hood-proven by its canonical return, immediate real boundaries, and exact caller `0x80087180`. Natural call-then-return-one C forwards the caller's four-byte command buffer to `func_8007B964`, discards that callee's zero, and matches all eight words on phrasing 1 under era `-O2 -G0`, with one normalized call relocation. Carve `0x03BC + 0x0020 + 0x1884 = 0x1C60`; packed span/full SHA exact, verify green at 353, and Tier 2 falls 203→202. Evidence: `docs/evidence/volume-campaign-20260830/func-8007a88c/REPORT.md`. |
| VOLUME-124-7F788 | 354 | `func_8007F788` @ `0x6FF88` is hood-proven by its canonical return, immediate real boundaries, and exact caller `0x80069D88`. Natural unsigned-byte forwarding C calls already-matched global getter `func_8007FC54` and matches all eight words on phrasing 1 under era `-O2 -G0`, including the post-call `andi v0,0xFF` and one normalized call relocation. The former span is exactly the `0x20`-byte leaf; packed span/full SHA exact, verify green at 354, and Tier 2 falls 202→201. Evidence: `docs/evidence/volume-campaign-20260830/func-8007f788/REPORT.md`. |
| VOLUME-125-80AC4 | 355 | `func_80080AC4` @ `0x712C4` is hood-proven by exact caller `0x80016894`, canonical return, and a real predecessor epilogue separated by two explicit alignment nops. Its body is word-identical to independently proven `func_8007A88C`; natural call-then-return-one C matches all eight words on phrasing 1 under era `-O2 -G0` with one normalized call relocation. Carve `0x0174 + 0x0020 = 0x0194`; packed span/full SHA exact, verify green at 355, and Tier 2 falls 201→200. Evidence: `docs/evidence/volume-campaign-20260830/func-80080ac4/REPORT.md`. |
| VOLUME-126-80B04 | 356 | `func_80080B04` @ `0x71304` is hood-proven by exact caller `0x80080FF0`, canonical return with live teardown, and immediate real boundaries. Natural `func_8007C044(buffer, value) == 0` C independently reproduces the adjacent 80AE4 wrapper shape and matches all eight words on phrasing 1 under era `-O2 -G0`, with one normalized call relocation. Carve `0x0020 + 0x01A4 = 0x01C4`; packed span/full SHA exact, verify green at 356, and Tier 2 falls 200→199. Evidence: `docs/evidence/volume-campaign-20260830/func-80080b04/REPORT.md`. |
| VOLUME-127-816F4 | 357 | `func_800816F4` @ `0x71EF4` is hood-proven by exact caller `0x8008161C`, canonical return with live teardown, and immediate real boundaries. Natural fixed-length equality C forwards two pointers to BIOS-vector wrapper `func_80071A04`, supplies length 12 in the call delay slot, and matches all eight words on phrasing 1 under era `-O2 -G0`, with one normalized call relocation. Carve `0x03E4 + 0x0020 + 0x0B98 = 0x0F9C`; packed span/full SHA exact, verify green at 357, and Tier 2 falls 199→198. Evidence: `docs/evidence/volume-campaign-20260830/func-800816f4/REPORT.md`. |
| VOLUME-128-82534 | 358 | `func_80082534` @ `0x72D34` is hood-proven by exact caller `0x8003E95C`, canonical return, and immediate real boundaries. Natural no-argument void C forwards to subsystem-reset routine `func_80082CF0`, discards its result by contract, and matches all eight words on phrasing 1 under era `-O2 -G0`, with one normalized call relocation. Carve `0x0278 + 0x0020 + 0x0788 = 0x0A20`; packed span/full SHA exact, verify green at 358, and Tier 2 falls 198→197. Evidence: `docs/evidence/volume-campaign-20260830/func-80082534/REPORT.md`. |
| VOLUME-129-50088 | 359 | `func_80050088` @ `0x40888` is hood-proven by an exact-start construction at `0x80048E4C–0x80048E50` stored into callback slot `+0x30`, plus canonical return and immediate real boundaries. Natural null-callback forwarding C passes `$a0` and zero to `func_800638D8`, matches all eight words on phrasing 1 under era `-O2 -G0`, and needs only one normalized call relocation. Carve `0x0050 + 0x0020 + 0x0C70 = 0x0CE0`; packed span/full SHA exact, verify green at 359, and Tier 2 falls 197→196. Evidence: `docs/evidence/volume-campaign-20260830/func-80050088/REPORT.md`. |
| VOLUME-130-50260 | 360 | `func_80050260` @ `0x40A60` is hood-proven by six exact-start constructions stored into callback slot `+0x88`, plus canonical return and immediate real boundaries. Natural no-argument forwarding C calls `func_80055760`, matches all eight words on phrasing 1 under era `-O2 -G0`, and needs only one normalized call relocation. Carve `0x01B8 + 0x0020 + 0x0A98 = 0x0C70`; packed span/full SHA exact, verify green at 360, and Tier 2 falls 196→195. Evidence: `docs/evidence/volume-campaign-20260830/func-80050260/REPORT.md`. |
| VOLUME-131-506E8 | 361 | `func_800506E8` @ `0x40EE8` is hood-proven by an exact-start construction passed as a callback to `func_800638D8`, plus canonical return and immediate real boundaries. Natural no-argument forwarding C calls `func_80058AA8`, matches all eight words on phrasing 1 under era `-O2 -G0`, and needs only one normalized call relocation. Carve `0x0468 + 0x0020 + 0x0610 = 0x0A98`; packed span/full SHA exact, verify green at 361, and Tier 2 falls 195→194. Evidence: `docs/evidence/volume-campaign-20260830/func-800506e8/REPORT.md`. |
| VOLUME-132-50708 | 362 | `func_80050708` @ `0x40F08` is hood-proven by an exact-start construction passed as a callback to `func_800638D8`, plus canonical return and immediate real boundaries. Natural `value + 0x1F` forwarding C calls `func_80064C54`, matches all eight words on phrasing 1 under era `-O2 -G0`, and fills the call delay slot exactly with the argument adjustment. Carve `0x0020 + 0x05F0 = 0x0610`; packed span/full SHA exact, verify green at 362, and Tier 2 falls 194→193. Evidence: `docs/evidence/volume-campaign-20260830/func-80050708/REPORT.md`. |
| VOLUME-133-50728 | 363 | `func_80050728` @ `0x40F28` is independently hood-proven by an exact-start callback registration, canonical return, and immediate real boundaries. Natural `value + 0x5D` forwarding C calls `func_80064C54`, matches all eight words on phrasing 1 under era `-O2 -G0`, and fills the call delay slot exactly like its `50708` sibling. Carve `0x0020 + 0x05D0 = 0x05F0`; packed span/full SHA exact, verify green at 363, and Tier 2 falls 193→192. Evidence: `docs/evidence/volume-campaign-20260830/func-80050728/REPORT.md`. |
| VOLUME-134-50BE8 | 364 | `func_80050BE8` @ `0x413E8` is hood-proven by an exact-start construction passed to callback registrar `func_800638D8`, plus canonical return and immediate real boundaries. Natural no-argument forwarding C calls `func_80057F14`, matches all eight words on phrasing 1 under era `-O2 -G0`, and needs one normalized call relocation. Carve `0x04A0 + 0x0020 + 0x0110 = 0x05D0`; packed span/full SHA exact, verify green at 364, and Tier 2 falls 192→191. Evidence: `docs/evidence/volume-campaign-20260830/func-80050be8/REPORT.md`. |
| VOLUME-135-50C50 | 365 | `func_80050C50` @ `0x41450` is hood-proven by an exact-start callback registration, canonical return, and immediate real boundaries. Natural `value + 0x28` forwarding C calls `func_80064C54`, matches all eight words on phrasing 1 under era `-O2 -G0`, and fills the call delay slot with the argument adjustment. Carve `0x0048 + 0x0020 + 0x00A8 = 0x0110`; packed span/full SHA exact, verify green at 365, and Tier 2 falls 191→190. Evidence: `docs/evidence/volume-campaign-20260830/func-80050c50/REPORT.md`. |
| VOLUME-136-50CF8 | 366 | `func_80050CF8` @ `0x414F8` is hood-proven by an exact-start callback registration, canonical return, and immediate real boundaries. Natural fixed-8 forwarding C calls `func_80064C54`, matches all eight words on phrasing 1 under era `-O2 -G0`, and fills the call delay slot with the fixed argument. Carve `0x0088 + 0x0020 = 0x00A8`; packed span/full SHA exact, verify green at 366, and Tier 2 falls 190→189. Evidence: `docs/evidence/volume-campaign-20260830/func-80050cf8/REPORT.md`. |
| VOLUME-137-813E8 | 367 | `func_800813E8` @ `0x71BE8` is hood-proven by an exact-start callback registration, canonical return, a real predecessor, and explicit alignment before the following real function. Natural no-argument forwarding C calls `func_8007C564` and matches all eight words on phrasing 1 under era `-O2 -G0`, with one normalized call relocation. Carve `0x00D8 + 0x0020 + 0x02EC = 0x03E4`; packed span/full SHA exact, verify green at 367, and Tier 2 falls 189→188. Evidence: `docs/evidence/volume-campaign-20260830/func-800813e8/REPORT.md`. |
| VOLUME-138-824F0 | 368 | `func_800824F0` @ `0x72CF0` is hood-proven by seven direct callers, canonical return, and immediate real boundaries. Natural `(3, value)` forwarding C calls `func_80073CF4`, matches all nine words on phrasing 1 under era `-O2 -G0`, and fills the call delay slot with the fixed slot number. Carve `0x0234 + 0x0024 + 0x0020 = 0x0278`; packed span/full SHA exact, verify green at 368, and Tier 2 falls 188→187. Evidence: `docs/evidence/volume-campaign-20260830/func-800824f0/REPORT.md`. |
| VOLUME-139-7DD14 | 369 | `func_8007DD14` @ `0x6E514` is hood-proven by two direct callers, canonical return, and real neighboring functions separated by explicit alignment. Natural `(4, value)` forwarding C calls `func_80073CF4`, matches all nine words on phrasing 1 under era `-O2 -G0`, and fills the call delay slot with the fixed slot number. Carve `0x17B4 + 0x0024 + 0x016C = 0x1944`; packed span/full SHA exact, verify green at 369, and Tier 2 falls 187→186. Evidence: `docs/evidence/volume-campaign-20260830/func-8007dd14/REPORT.md`. |
| VOLUME-140-52534 | 370 | `func_80052534` @ `0x42D34` is hood-proven by two direct callers, canonical return with live stack teardown, and immediate real boundaries. Natural signed-byte-return C calls `func_80021080`, matches all nine words on phrasing 1 under era `-O2 -G0`, and recovers retail's `sll 24 / sra 24` canonicalization. Carve `0x0024 + 0x0024 = 0x0048`; packed span/full SHA exact, verify green at 370, and Tier 2 falls 186→185. Evidence: `docs/evidence/volume-campaign-20260830/func-80052534/REPORT.md`. |
| VOLUME-141-52558 | 371 | `func_80052558` @ `0x42D58` is hood-proven by three direct callers, canonical return with live stack teardown, and immediate real boundaries. Natural signed-byte-return C calls `func_800210D4`, matches all nine words on phrasing 1 under era `-O2 -G0`, and independently reproduces the adjacent sibling's `sll 24 / sra 24` canonicalization. The active asm span was exactly `0x0024`; packed/full SHA exact, verify green at 371, and Tier 2 falls 185→184. Evidence: `docs/evidence/volume-campaign-20260830/func-80052558/REPORT.md`. |
| VOLUME-142-52790 | 372 | `func_80052790` @ `0x42F90` is hood-proven by three direct callers, canonical return, and immediate real boundaries. Natural era `-O2 -G8` C stores the input to GP state `D_8009D020`, forwards `value == 0` to `func_80086728`, and matches all nine words on phrasing 1 after normalizing GP and call relocations. Carve `0x01FC + 0x0024 = 0x0220`; packed/full SHA exact, verify green at 372, and Tier 2 falls 184→183. Evidence: `docs/evidence/volume-campaign-20260830/func-80052790/REPORT.md`. |
| VOLUME-143-4D4A0 | 373 | `func_8004D4A0` @ `0x3DCA0` is hood-proven by one direct caller, canonical return with live stack teardown, and immediate real boundaries. Natural fixed-argument C calls `func_80062A34(1,0x24)`, booleanizes the result with `!= 0`, and matches all nine words on phrasing 1 under era `-O2 -G0`. Carve `0x0208 + 0x0024 + 0x05D8 = 0x0804`; packed/full SHA exact, verify green at 373, and Tier 2 falls 183→182. Evidence: `docs/evidence/volume-campaign-20260830/func-8004d4a0/REPORT.md`. |
| VOLUME-144-64C30 | 374 | `func_80064C30` @ `0x55430` is hood-proven by one direct caller, canonical return, and immediate real boundaries. Natural era `-O2 -G8` C forwards the input and GP state `D_8009D164` to `func_8005F354`, matching all nine words on phrasing 1 after GP/call relocation normalization. Carve `0x0024 + 0x0764 = 0x0788`; packed/full SHA exact, verify green at 374, and Tier 2 falls 182→181. Evidence: `docs/evidence/volume-campaign-20260830/func-80064c30/REPORT.md`. |
| VOLUME-145-5F594 | 375 | `func_8005F594` @ `0x4FD94` is hood-proven by seven direct callers, canonical return, and immediate real boundaries. Natural era `-O2 -G8` C forwards the input and GP state `D_8009D138` to `func_8005F354`, matching all nine words on phrasing 1 after GP/call relocation normalization. Carve `0x06C0 + 0x0024 + 0x1EE8 = 0x25CC`; packed/full SHA exact, verify green at 375, and Tier 2 falls 181→180. Evidence: `docs/evidence/volume-campaign-20260830/func-8005f594/REPORT.md`. |
| VOLUME-146-4E94C | 376 | `func_8004E94C` @ `0x3F14C` is hood-proven by one direct caller, canonical return, and immediate real boundaries. Natural era `-O2 -G8` C forwards `D_8009CF0C - 1` to `func_8004E704`, matching all nine words on phrasing 1 after GP/call relocation normalization and filling the call delay with the decrement. Carve `0x0CA8 + 0x0024 = 0x0CCC`; packed/full SHA exact, verify green at 376, and Tier 2 falls 180→179. Evidence: `docs/evidence/volume-campaign-20260830/func-8004e94c/REPORT.md`. |
| VOLUME-147-36E34 | 377 | `func_80036E34` @ `0x27634` is hood-proven by one direct caller, canonical return, and immediate real boundaries. Natural era `-O2 -G0` C initializes `D_800A76BC=0`, `D_800A76C0=0`, and `D_800A76B8=1`, matching all nine words and three independent `$at` symbolic-store pairs on phrasing 1. Carve `0x09EC + 0x0024 + 0x0250 = 0x0C60`; packed/full SHA exact, verify green at 377, and Tier 2 falls 179→178. Evidence: `docs/evidence/volume-campaign-20260830/func-80036e34/REPORT.md`. |
| VOLUME-148-36E58 | 378 | `func_80036E58` @ `0x27658` is independently hood-proven by its own direct caller, canonical return, and immediate real boundaries. Natural era `-O2 -G0` C initializes `D_800A76B0=0`, `D_800A76B4=0`, and `D_800A76AC=1`, independently matching all nine words and three `$at` store pairs on phrasing 1. Carve `0x0024 + 0x022C = 0x0250`; packed/full SHA exact, verify green at 378, and Tier 2 falls 178→177. Evidence: `docs/evidence/volume-campaign-20260830/func-80036e58/REPORT.md`. |
| VOLUME-149-64E90 | 379 | `func_80064E90` @ `0x55690` is hood-proven by one direct caller, canonical return, and immediate real boundaries. Natural era `-O2 -G0` typed C clears nested field `object->state[0x20]`, calls `func_8006269C`, and matches all nine words on phrasing 1, including the store in the call delay slot. Carve `0x023C + 0x0024 + 0x0504 = 0x0764`; packed/full SHA exact, verify green at 379, and Tier 2 falls 177→176. Evidence: `docs/evidence/volume-campaign-20260830/func-80064e90/REPORT.md`. |

Detail and leaf-by-leaf narrative: git history + wiki
([Current Status](https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki/Current-Status)).
PC port remains out of scope. Redump.org cross-check still open (non-blocking).
