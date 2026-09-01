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

## Native field-runtime library boundary (2026-08-31)

The reusable CMake target `pe_field_runtime` now produces
`pc_port/build/libpe_field_runtime.a` from the 201 current translated/runtime
translation units. `parasite-eve-port`, the 994-case native suite, and a
standalone external-consumer smoke test all link the archive; CLI-only
`port_main.c` and `host_window.c` remain outside it. Normal and fresh
ASan/UBSan CTest runs pass both consumers, and real-disc strict execution
now stops at `func_80081314_func_8007F0C8_cut`, after the authenticated
libpress, record-pool, stream-control, CD idle-wait, blocking CdlSetloc, and
CdlReadS callback-registration sequence.

This is a verified product/build boundary, not a semantic-completeness claim:
there is still no complete Day 1 field runtime and scheduler provenance is
still `NEEDS_ARTIFACT`. Evidence:
`docs/evidence/pe-field-runtime-library/REPORT.md`; consumer notes:
`pc_port/docs/field_runtime_library.md`.

## Main-lane YAML build authority (merged 2026-09-01)

`configs/USA/disc1.yaml` owns Disc-1 span edges, source/object mapping, trim
size, link order, verifier entry, and the published exact count; compiler-only
exceptions live in `configs/USA/disc1_build_profiles.json`. The build and
verifier scripts are generic drivers with no leaf lists, and tracked extra
function C files require explicit nonmatching dispositions. Generated status
documents provide the current matching and native metrics. The preceding
main-lane B54K-B1..B6 narrative remains historical evidence; later B54K and
field-runtime work in this handoff supersedes its old strict frontier.

## Matching leaf — func_8004CDAC (2026-09-01)

`func_8004CDAC` is now a registered 10-word exact C leaf: it forwards the
constants `0x28` and `0x3D` to `func_80062F3C` in order. The YAML split now
ends the preceding assembly span at `0x3D5AC` and resumes it at `0x3D5D4`.
The full rebuild and packed-span verifier pass at retail SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; matching count is 412.

## Matching leaf — func_8004CDD4 (2026-09-01)

`func_8004CDD4` is now a registered 21-word exact C leaf. It calls the
two-component accumulator with `(0, 10)`, selects `D_800A1A20` or its
`+0x40` record by `state+0x24 == 0x3D`, then calls `func_8005F594`. The
record pointer is deliberately constrained to `$a0` only after the first
call; that matches retail's address materialization and avoids extending the
frame. The full build and all packed-span checks are exact at 413 leaves.

## Matching leaf — func_800703F4 (2026-09-01)

`func_800703F4` is now a registered 10-word exact C leaf. It invokes the
retail teardown phases `func_800702DC` then `func_800701B4`; the YAML spans
the wrapper at `[0x60BF4, 0x60C1C)` and resumes generated assembly afterward.
The full rebuild, public verifier, and all packed-span checks are exact at
414 leaves.

## Matching leaf — func_800504F4 (2026-09-01)

`func_800504F4` is now a registered 10-word exact C leaf. It forwards the
two GP-relative words at `D_8009CF44` and `D_8009CF48` to `func_80042020`.
It uses the existing era `-O2 -G8` profile, which reproduces retail's two
loads before the call frame. The full rebuild, public verifier, and all
packed-span checks are exact at 415 leaves.

## Matching leaf — func_8005051C (2026-09-01)

`func_8005051C` is now a registered 10-word exact C leaf. It is the adjacent
GP-relative twin of `func_800504F4`, forwarding `D_8009CF44` and
`D_8009CF48` to `func_80042170` under the same era `-O2 -G8` profile. The
full rebuild, public verifier, and all packed-span checks are exact at 416
leaves.

## Matching leaf — func_80050544 (2026-09-01)

`func_80050544` is now a registered 15-word exact C leaf. When its second
argument is nonzero, it performs the two retail constant calls (`0x1F`, then
`0x45`) and publishes `func_800504F4` through `func_80042B50`; otherwise it
returns after the shared epilogue. The full rebuild, public verifier, and all
packed-span checks are exact at 417 leaves.

## Matching leaf — func_8005DA8C (2026-09-01)

`func_8005DA8C` is now a registered 10-word exact C leaf. It returns null for
an unsigned index at least `0x41`, otherwise it returns the corresponding
16-byte record address in `D_80092478`. The source uses the retail inverse
predicate (`>= 0x41`) so era emits the required `beqz` branch polarity. The
full rebuild, public verifier, and all packed-span checks are exact at 418
leaves.

## Matching leaf — func_8005DAB4 (2026-09-01)

`func_8005DAB4` is now a registered 10-word exact C leaf, the 32-byte-stride
twin of `func_8005DA8C`. It returns null for an unsigned index at least
`0x41`, otherwise the matching record address in `D_80092888`. The full
rebuild, public verifier, and all packed-span checks are exact at 419 leaves.

## Matching leaf — func_8005F844 (2026-09-01)

`func_8005F844` is now a registered 12-word exact C leaf. It writes two
GP-relative state words from independent boolean-selected literal expressions
(`0x3A1C`/`0x395D` and `0xCC`/`0x84`), then always writes `0xA4`. The
independent assignment form is required to reproduce the two retail `bnez`
delay slots. The full rebuild, public verifier, and all packed-span checks are
exact at 420 leaves.

## Matching leaf — func_8005E54C (2026-09-01)

`func_8005E54C` is now a registered 12-word exact C leaf. It reads the
GP-relative activation word `D_8009D0E8`, returns zero when inactive, and
otherwise returns `func_8005E038()`. The full rebuild, public verifier, and
all packed-span checks are exact at 421 leaves.

## Matching leaf — func_8005DC28 (2026-09-01)

`func_8005DC28` is now a registered 9-word exact C leaf. It adds its input to
the global offset `D_800A8050` and returns the resulting byte from
`D_800A8028`. It uses the existing era `-O2 -G0` three-word-symbol profile so
maspsx retains retail's indexed symbolic byte-load macro shape. The full
rebuild, public verifier, and all packed-span checks are exact at 422 leaves.

## Matching leaves — func_8005DBF8 / func_8005DC10 (2026-09-01)

`func_8005DBF8` and `func_8005DC10` are now registered six-word exact C
leaves. Each loads a global word then adds its record-base address minus its
field offset (`0x18` and `0x20`, respectively). Explicit `$v0` address and
`$v1` loaded-value lifetimes reproduce retail's materialization order. The
full rebuild, public verifier, and all packed-span checks are exact at 424
leaves.

## Matching leaf — func_8005DE70 (2026-09-01)

`func_8005DE70` is now a registered six-word exact C leaf. It loads
`D_800A8044` and adds that record-base address minus `0x1C`; the explicit
`$v0` address and `$v1` value lifetimes preserve retail materialization. The
full rebuild, public verifier, and all packed-span checks are exact at 425
leaves.

## Matching leaf — func_8005421C (2026-09-01)

`func_8005421C` is now a registered nine-word exact C leaf. It passes its
input minus one to `func_8005DB44` and returns byte 6 of the resulting record.
The full rebuild, public verifier, and all packed-span checks are exact at 426
leaves.

## Matching leaves — func_80057D18 / func_8005C144 (2026-09-01)

`func_80057D18` reads and clears a signed halfword in the GP-relative
`D_8009D048` table, returning the original `int` value; the return width keeps
retail's `lh` plus return-delay `sh` form. `func_8005C144` stores the low byte
of `func_80033A20()` to `D_8009D02C` and then calls `func_800339A0(0)`. Both
use era `-O2 -G8`. The full rebuild, public verifier, and all packed-span
checks are exact at 429 leaves.

## Matching leaf — func_8006EC6C (2026-09-01)

`func_8006EC6C` is now a registered six-word exact C leaf. It sign-extends a
halfword index, scales it by four, reads an offset at that base location, and
returns base plus the offset. The full rebuild, public verifier, and all
packed-span checks are exact at 430 leaves.

## Matching leaf — func_80073E10 (2026-09-01)

`func_80073E10` is now a registered six-word exact C leaf. It returns the old
unsigned halfword from `*D_80095674` and stores its input to that same address
in the return delay slot. The full rebuild, public verifier, and all
packed-span checks are exact at 431 leaves.

## Matching leaf — func_8007A3CC (2026-09-01)

`func_8007A3CC` is now a registered eight-word exact C leaf. It is a direct
frame-and-return wrapper around `func_8007B9EC`. The full rebuild, public
verifier, and all packed-span checks are exact at 432 leaves.

## Matching leaf — func_8007A468 (2026-09-01)

`func_8007A468` is now a registered eight-word exact C leaf. It is a direct
frame-and-return wrapper around `func_8007B010`. The full rebuild, public
verifier, and all packed-span checks are exact at 433 leaves.

## Matching leaf — func_8007D054 (2026-09-01)

`func_8007D054` is now a registered eight-word exact C leaf. It forwards zero
to `func_8007D074`, with the zero materialization in the call delay slot. The
full rebuild, public verifier, and all packed-span checks are exact at 434
leaves.

## Matching leaf — func_8007EE64 (2026-09-01)

`func_8007EE64` is now a registered eight-word exact C leaf. It is a direct
frame-and-return wrapper around `func_8007FB04`. The full rebuild, public
verifier, and all packed-span checks are exact at 435 leaves.

## Matching leaf — func_800C6EE8 (2026-09-01)

`func_800C6EE8` is now a registered four-word exact C leaf. It truncates its
input and stores the resulting unsigned halfword to `D_800F3420`. The full
rebuild, public verifier, and all packed-span checks are exact at 436 leaves.

## Matching leaves — integer exchanges (2026-09-01)

`func_80081E5C`, `func_800824B4`, `func_800824C8`, and `func_800824DC` are
now registered five-word exact C leaves. Each loads an integer global, stores
its input through a pointer pinned to `$v1` in the return delay slot, then
returns the old value. The full rebuild, public verifier, and all packed-span
checks are exact at 440 leaves.

## Matching leaves — forwarding wrappers (2026-09-01)

`func_80080B24`, `func_80082514`, and `func_80082554` are now registered
eight-word exact C leaves. They are direct frame-and-return wrappers around
`func_8007BDDC`, `func_80082CDC`, and `func_80082DBC`, respectively. The full
rebuild, public verifier, and all packed-span checks are exact at 443 leaves.

## Matching leaves — boolean and forwarding wrappers (2026-09-01)

`func_8007A8AC` and `func_8007A8CC` return the boolean negation of their
callees; `func_8007A910` directly forwards to `func_8007BDDC`. All three
eight-word leaves are byte-exact C matches. The full rebuild, public verifier,
and all packed-span checks are exact at 446 leaves.

## Matching leaf — func_80091080 (2026-09-01)

`func_80091080` is now a registered eight-word exact C leaf. It is a direct
frame-and-return wrapper around `func_80090F68`. The full rebuild, public
verifier, and all packed-span checks are exact at 447 leaves.

## Matching leaves — call-and-return-one wrappers (2026-09-01)

`func_800193B8` and `func_80019D24` now call their respective `void` callees
and return one as registered eight-word exact C leaves. The full rebuild,
public verifier, and all packed-span checks are exact at 449 leaves.

## Matching leaf — func_8007F7C8 (2026-09-01)

`func_8007F7C8` is now a registered eight-word exact C leaf. It forwards the
unsigned-byte result from `func_8007FC08`, preserving the required byte mask.
The full rebuild, public verifier, and all packed-span checks are exact at 450
leaves.

## Matching leaf — func_8007DE78 (2026-09-01)

`func_8007DE78` is now a registered ten-word exact C leaf. It calls
`func_8007E334` and then `func_8007E514` through a shared minimal frame. The
full rebuild, public verifier, and all packed-span checks are exact at 451
leaves.

## Matching leaf — func_8003E91C (2026-09-01)

`func_8003E91C` is now a registered ten-word exact C leaf. It calls
`func_80070D6C` followed by `func_80036F7C` through a minimal shared frame.
The full rebuild, public verifier, and all packed-span checks are exact at 452
leaves.

## Matching leaf — func_80085098 (2026-09-01)

`func_80085098` is now a registered ten-word exact C leaf. It passes zero to
`func_80085F44`, then clears `D_8009D24C`. The full rebuild, public verifier,
and all packed-span checks are exact at 453 leaves.

## Matching leaf — func_800850C0 (2026-09-01)

`func_800850C0` is now a registered thirteen-word exact C leaf. It sets
`D_8009D24C` to one, then registers `func_80085098` through `func_80085F44`.
The full rebuild, public verifier, and all packed-span checks are exact at 454
leaves.

## Matching leaves — func_800850F4 / func_80085134 (2026-09-01)

`func_800850F4` and `func_80085134` are now registered sixteen-word exact C
leaves. Both preserve their two arguments across `func_800850C0`, then forward
them to their respective dispatchers (`func_80085E54` and `func_80085DF4`).
The full rebuild, public verifier, and all packed-span checks are exact at 456
leaves.

## Matching leaves — func_8004C5DC / func_8004D9D8 (2026-09-01)

`func_8004C5DC` and `func_8004D9D8` are now registered eleven-word exact C
leaves. Each obtains a handle with `func_80062A34(1, constant)` and immediately
dispatches it through `func_80062F1C`; their constants are 19 and 39. The full
rebuild, public verifier, and all packed-span checks are exact at 458 leaves.

## Matching leaf — func_80076C10 (2026-09-01)

`func_80076C10` is now a registered nine-word exact C leaf. It forwards its
three arguments to `func_80076C34` as `(arg0, arg1, 0, arg2)`, including the
retail `$a3` move and zeroed delay-slot argument. The full rebuild, public
verifier, and all packed-span checks are exact at 459 leaves.

## Matching leaf — func_80077A00 (2026-09-01)

`func_80077A00` is now a registered ten-word exact C leaf. It registers
`func_80076EE4` with mode two through `func_80073CF4`; the function-pointer
argument and immediate delay slot are exact. The full rebuild, public verifier,
and all packed-span checks are exact at 460 leaves.

## Matching leaf — func_80080D34 (2026-09-01)

`func_80080D34` is now a registered ten-word exact C leaf. It forwards its
byte-typed first parameter and second word parameter to `func_8007EE84`, with
two zero trailing fields. The full rebuild, public verifier, and all packed-span
checks are exact at 461 leaves.

## Matching leaf — func_80017820 (2026-09-01)

`func_80017820` is now a registered eleven-word exact C leaf. It double-derefs
a pointer to obtain a signed halfword, calls `func_8003746C`, and returns one.
The full rebuild, public verifier, and all packed-span checks are exact at 462
leaves.

## Matching leaf — func_80018954 (2026-09-01)

`func_80018954` is now a registered ten-word exact C leaf. It passes the global
record pointer `D_8009D2F0` to `func_8002F7D8` and returns one. The full rebuild,
public verifier, and all packed-span checks are exact at 463 leaves.

## Matching leaf — func_800C7D2C (2026-09-01)

`func_800C7D2C` is now a registered ten-word exact C leaf. It forwards its
incoming first argument with the static buffer `D_800E0824` to `func_800C2414`
and returns zero. The full rebuild, public verifier, and all packed-span checks
are exact at 464 leaves.

## Matching leaf — func_800C8E70 (2026-09-01)

`func_800C8E70` is the matching fixed-buffer sibling using `D_800E09A0`.
All rebuild and verifier checks are exact at 465 leaves.

## Matching leaf — func_800C9B68 (2026-09-01)

`func_800C9B68` is the matching fixed-buffer sibling using `D_800E0A94`.
All rebuild and verifier checks are exact at 466 leaves.

## Matching leaf — func_800CA700 (2026-09-01)

`func_800CA700` is the matching fixed-buffer sibling using `D_800E0B84`.
All rebuild and verifier checks are exact at 467 leaves.

## Matching leaf — func_800CBF0C (2026-09-01)

`func_800CBF0C` is the matching fixed-buffer sibling using `D_800E0D08`.
All rebuild and verifier checks are exact at 468 leaves.

## Matching leaf — func_800CCEE8 (2026-09-01)

`func_800CCEE8` is the matching fixed-buffer sibling using `D_800E0E60`.
All rebuild and verifier checks are exact at 469 leaves.

## Matching leaf — func_800CD8C8 (2026-09-01)

`func_800CD8C8` is the matching fixed-buffer sibling using `D_800E0F28`.
All rebuild and verifier checks are exact at 470 leaves.

## Matching leaf — func_800CE144 (2026-09-01)

`func_800CE144` is the matching fixed-buffer sibling using `D_800E0FC0`.
All rebuild and verifier checks are exact at 471 leaves.

## Matching leaf — func_80071944 (2026-09-01)

`func_80071944` is an exact conditional pointer helper: when bit 3 of its
flags word is set it returns the offset-0xC field address, otherwise null.
All rebuild and verifier checks are exact at 472 leaves.

## Matching leaf — func_800719C4 (2026-09-01)

`func_800719C4` is its matching sibling, returning the offset-0x14 field
address when bit 3 of the flags word is set. All rebuild and verifier checks
are exact at 473 leaves.

## Matching leaf — func_80076150 (2026-09-01)

`func_80076150` is an exact no-global command-word builder. It combines the
`0xE1000000` base, two boolean-derived flag bits, and an `0x9FF` masked field.
All rebuild and verifier checks are exact at 474 leaves.

## Matching leaf — func_80018754 (2026-09-01)

`func_80018754` is now a registered eight-word exact C leaf. It sets bit 2
in `D_800A76C4` through a `$v1`-pinned global pointer and returns one. The
full rebuild, public verifier, and all packed-span checks are exact at 475
leaves.

## Matching leaf — func_80019618 (2026-09-01)

`func_80019618` is now a registered eight-word exact C leaf. It sets bit 13
in `D_800B0CD8` through a `$v1`-pinned global pointer and returns one. The
full rebuild, public verifier, and all packed-span checks are exact at 476
leaves.

## Matching leaf — func_80019638 (2026-09-01)

`func_80019638` is now a registered eight-word exact C leaf. It clears bit 13
in `D_800B0CD8`, with the global pointer pinned to `$v0` and the mask to
`$a0`, reproducing the retail register allocation. The full rebuild, public
verifier, and all packed-span checks are exact at 477 leaves.

## Matching leaf — func_800196A0 (2026-09-01)

`func_800196A0` is now a registered nine-word exact C leaf. It sets bit 14 in
the `+0x98` flag word of `D_8009D2F0`, keeping the state pointer in `$v1` to
reproduce retail loads. The full rebuild, public verifier, and all packed-span
checks are exact at 478 leaves.

## Matching leaf — func_800196C4 (2026-09-01)

`func_800196C4` is now a registered nine-word exact C leaf. It clears bit 14
in the `+0x98` flag word of `D_8009D2F0`, with the state pointer in `$v0` and
mask in `$a0` to match retail allocation. All checks are exact at 479 leaves.

## Matching leaf — func_8001967C (2026-09-01)

`func_8001967C` is now a registered nine-word exact C leaf. It clears bit 7
in the `+0x98` flag word of `D_8009D2F0`, keeping the state pointer in `$v0`
and mask in `$a0`. The full rebuild, public verifier, and packed-span checks
are exact at 480 leaves.

## Matching leaf — func_80019658 (2026-09-01)

`func_80019658` is now a registered nine-word exact C leaf. It sets bit 7 in
the `+0x98` flag word of `D_8009D2F0`, retaining the state pointer in `$v1`
for the retail load order. The full rebuild, public verifier, and packed-span
checks are exact at 481 leaves.

## Matching leaf — func_80019728 (2026-09-01)

`func_80019728` is now a registered eight-word exact C leaf. It sets bit 2
in `D_8009D2E8`; the compiler naturally reproduces retail's `$v0` load and
`$at`-addressed store. The full rebuild, public verifier, and packed-span
checks are exact at 482 leaves.

## Matching leaf — func_80019748 (2026-09-01)

`func_80019748` is now a registered eight-word exact C leaf. It clears bit 2
in `D_8009D2E8`, retaining retail's `$v0` global value and `$at` store
address. The full rebuild, public verifier, and packed-span checks are exact
at 483 leaves.

## Matching leaf — func_80019768 (2026-09-01)

`func_80019768` is now a registered 12-word exact C leaf. It forwards the
halfword reached through its pointer argument with `D_8009D2F0` to
`func_8001ACE0`, then returns one. The full rebuild, public verifier, and
packed-span checks are exact at 484 leaves.

## Matching leaf — func_80019798 (2026-09-01)

`func_80019798` is now a registered 14-word exact C leaf. It retains its
output pointer across `func_800392EC`, masks the result to one byte, writes it
through that pointer, and returns one. The full rebuild, public verifier, and
packed-span checks are exact at 485 leaves.

## Matching leaf — func_80019904 (2026-09-01)

`func_80019904` is now a registered nine-word exact C leaf. It clears bit 0
in the `+0x98` flag word of `D_8009D2F0`, with the state pointer pinned to
`$v0` and the mask to `$a0`. The full rebuild, public verifier, and packed-span
checks are exact at 486 leaves.

## Matching leaf — func_80019928 (2026-09-01)

`func_80019928` is now a registered nine-word exact C leaf. It sets bit 0 in
the `+0x98` flag word of `D_8009D2F0`, retaining the state pointer in `$v1`
to reproduce retail loads. The full rebuild, public verifier, and packed-span
checks are exact at 487 leaves.

## Matching leaf — func_8001994C (2026-09-01)

`func_8001994C` is now a registered 16-word exact C leaf. It dereferences four
pointer slots, forwards their words to `func_800676CC`, and returns one. The
full rebuild, public verifier, and packed-span checks are exact at 488 leaves.

## Matching leaf — func_8001998C (2026-09-01)

`func_8001998C` is now a registered 16-word exact C leaf. It dereferences four
pointer slots, forwards their words to `func_80067730`, and returns one. The
full rebuild, public verifier, and packed-span checks are exact at 489 leaves.

## Matching leaf — func_800199F8 (2026-09-01)

`func_800199F8` is now a registered nine-word exact C leaf. It clears bit 4
in the `+0x250` halfword state field of `D_8009D2F0`, retaining `$v1` for the
retail load order. The full rebuild, public verifier, and packed-span checks
are exact at 490 leaves.

## Matching leaf — func_80019A9C (2026-09-01)

`func_80019A9C` is now a registered nine-word exact C leaf. It clears bit 3
in the `+0x250` halfword state field of `D_8009D2F0`, retaining `$v1` for the
retail load order. The full rebuild, public verifier, and packed-span checks
are exact at 491 leaves.

## Matching leaf — func_80019AC0 (2026-09-01)

`func_80019AC0` is now a registered nine-word exact C leaf. It sets bit 10
in the `+0x98` state flag word of `D_8009D2F0`, retaining `$v1` for the retail
load order. The full rebuild, public verifier, and packed-span checks are
exact at 492 leaves.

## Matching leaf — func_80019AE4 (2026-09-01)

`func_80019AE4` is now a registered nine-word exact C leaf. It clears bit 10
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$v0` and
mask in `$a0` to match retail allocation. The full rebuild, public verifier,
and packed-span checks are exact at 493 leaves.

## Matching leaf — func_80019C28 (2026-09-01)

`func_80019C28` is now a registered nine-word exact C leaf. It sets bit 17
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$v0` and
mask in `$a0` to match retail allocation. The full rebuild, public verifier,
and packed-span checks are exact at 494 leaves.

## Matching leaf — func_80019C04 (2026-09-01)

`func_80019C04` is now a registered nine-word exact C leaf. It clears bit 17
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$a0` and
wide mask in `$v1` to match retail allocation. The full rebuild, public
verifier, and packed-span checks are exact at 495 leaves.

## Matching leaf — func_8001A32C (2026-09-01)

`func_8001A32C` is now a registered nine-word exact C leaf. It sets bit 1 in
the `+0x98` state flag word of `D_8009D2F0`, retaining `$v1` for the retail
load order. The full rebuild, public verifier, and packed-span checks are
exact at 496 leaves.

## Matching leaf — func_8001A350 (2026-09-01)

`func_8001A350` is now a registered nine-word exact C leaf. It clears bit 1
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$v0` and
mask in `$a0` to match retail allocation. The full rebuild, public verifier,
and packed-span checks are exact at 497 leaves.

## Matching leaf — func_8001A1F0 (2026-09-01)

`func_8001A1F0` is now a registered nine-word exact C leaf. It sets bit 24
in the `+0x98` state flag word of `D_8009D2F0`, with the pointer in `$v0` and
mask in `$a0` to match retail allocation. The full rebuild, public verifier,
and packed-span checks are exact at 498 leaves.

## Matching leaf — func_8001A2F0 (2026-09-01)

`func_8001A2F0` is now a registered 15-word exact C leaf. It counts set bits
using the retail `value &= value - 1` loop, stores the count through its second
pointer slot, and returns one. The full rebuild, public verifier, and
packed-span checks are exact at 499 leaves.

## Matching leaf — func_8001A1A8 (2026-09-01)

`func_8001A1A8` is now a registered 18-word exact C leaf. It retains its
descriptor across `func_8005186C`, writes the result through the descriptor's
second pointer slot, and returns one. The full rebuild, public verifier, and
packed-span checks are exact at 500 leaves.

## Matching leaf — func_80019D44 (2026-09-01)

`func_80019D44` is now a registered 16-word exact C leaf. It dereferences four
halfword pointer slots, forwards them to `func_80037454`, and returns one. The
full rebuild, public verifier, and packed-span checks are exact at 501 leaves.

## Matching leaf — func_80017D7C (2026-09-01)

`func_80017D7C` is now a registered eight-word exact C leaf. It sets bit 0 in
`D_8009D2E8`, retaining retail's `$v0` global value and `$at` store address.
The full rebuild, public verifier, and packed-span checks are exact at 502
leaves.

## Matching leaf — func_80017D5C (2026-09-01)

`func_80017D5C` is now a registered eight-word exact C leaf. It clears bit 0
in `D_8009D2E8`, retaining retail's `$v0` global value and `$at` store
address. The full rebuild, public verifier, and packed-span checks are exact
at 503 leaves.

## Matching leaf — func_80017D3C (2026-09-01)

`func_80017D3C` is now a registered eight-word exact C leaf. It transfers the
signed second halfword through the `$v0` load/store path into
`D_8009D2F0+0x224`; an empty constrained asm operand preserves the retail
signed-load choice. The full rebuild, public verifier, and packed-span checks
are exact at 504 leaves.

## Matching leaf — func_80017D18 (2026-09-01)

`func_80017D18` is now a registered nine-word exact C leaf. It clears the
low three bits of `D_800BCF88` and sets bit 7; pinning the global address in
`$v0` retains the retail address/value register sequence. The full rebuild,
public verifier, and packed-span checks are exact at 505 leaves.

## Matching leaf — func_80017CC4 (2026-09-01)

`func_80017CC4` is now a registered nine-word exact C leaf. It tests whether
the low three bits of `D_800BCF88` equal four and writes the Boolean result to
the caller's indirect result slot. The full rebuild, public verifier, and
packed-span checks are exact at 506 leaves.

## Matching leaves — func_80017C54 and func_80017C8C (2026-09-01)

`func_80017C54` and `func_80017C8C` are now registered fourteen-word exact C
leaves. Both forward signed, signed, and unsigned halfwords to `func_800661EC`;
their only behavioral difference is the fourth argument (zero versus eight).
The full rebuild, public verifier, and packed-span checks are exact at 508
leaves.

## Matching leaf — func_80017CE8 (2026-09-01)

`func_80017CE8` is now a registered twelve-word exact C leaf. It invokes
`func_800665A0` on the `D_8009D254+0x28` state subrecord with both remaining
arguments set to minus one. The full rebuild, public verifier, and packed-span
checks are exact at 509 leaves.

## Matching leaf — func_80017B34 (2026-09-01)

`func_80017B34` is now a registered sixteen-word exact C leaf. It caps an
incoming unsigned halfword by the byte limit at `D_8009D2F0+0xF`, stores it at
`+0x12`, and sets state flag `0x200`. The full rebuild, public verifier, and
packed-span checks are exact at 510 leaves.

## Matching leaves — func_80017A78, func_80017AA4, and func_80017AC0 (2026-09-01)

`func_80017A78`, `func_80017AA4`, and `func_80017AC0` are now registered
exact C leaves for the `D_8009D2E8` flag word: a caller-supplied mask clear,
a read to an indirect result slot, and a caller-supplied OR update. The full
rebuild, public verifier, and packed-span checks are exact at 513 leaves.

## Matching leaves — func_80017A24 and func_80017A50 (2026-09-01)

`func_80017A24` and `func_80017A50` are now registered exact C leaves. They
respectively write a selected source bit to the destination slot and set that
bit in the source word using the shared three-pointer argument layout. The
full rebuild, public verifier, and packed-span checks are exact at 515 leaves.

## Matching leaves — func_80017928, func_80017948, and func_80017968 (2026-09-01)

`func_80017928`, `func_80017948`, and `func_80017968` are now registered
eight-word exact C leaves. They read the byte at `D_8009D2F0+0xD`, write that
byte from an indirect input slot, and read the unsigned halfword at `+0x24`,
respectively. The full rebuild, public verifier, and packed-span checks are
exact at 518 leaves.

## Matching leaf — func_800179F8 (2026-09-01)

`func_800179F8` is now a registered eleven-word exact C leaf. It clears the
selected indexed bit in the source word using the shared source/bit pointer
layout. The full rebuild, public verifier, and packed-span checks are exact at
519 leaves.

## Matching leaves — func_800C2B10 and func_800C2B28 (2026-09-01)

`func_800C2B10` and `func_800C2B28` are now registered six-word exact C
leaves. They return indexed addresses in `D_800E2248` at base offsets `0x8`
and `0x48`; a compiler memory barrier preserves the retail shift/load/add
schedule. The full rebuild, public verifier, and packed-span checks are exact
at 521 leaves.

## Matching leaf — func_800C2B68 (2026-09-01)

`func_800C2B68` is now a registered ten-word exact C leaf. It checks whether
the high halfword at `D_800E2248+4` equals `0x0101`. The full rebuild, public
verifier, and packed-span checks are exact at 522 leaves.

## Matching leaf — func_8008C70C (2026-09-01)

`func_8008C70C` is now a registered five-word exact C leaf. It stores the
second input word as a halfword at `D_8009D2C8+0x56`, retaining the retail
return-delay-slot store. The full rebuild, public verifier, and packed-span
checks are exact at 523 leaves.

## Matching leaf — func_8008C16C (2026-09-01)

`func_8008C16C` is now a registered eight-word exact C leaf. It clears
`D_8009D220` and stores the signed input byte shifted into the high halfword of
`D_8009D2D0`; pinning the value in `$v0` retains retail's load-delay schedule.
The full rebuild, public verifier, and packed-span checks are exact at 524
leaves.

## Matching leaf — func_8008C270 (2026-09-01)

`func_8008C270` is now a registered eight-word exact C leaf. It is the
companion signed-byte setter: it clears `D_8009D21E` and writes the shifted
byte to `D_8009D2CC`, using the same retail `$v0` load-delay schedule. The full
rebuild, public verifier, and packed-span checks are exact at 525 leaves.

## Matching leaf — func_80084B20 (2026-09-01)

`func_80084B20` is now a registered eight-word exact C leaf. It returns either
the base of `D_800A5B70` or its `+0xF0` subregion according to bits 4–7 of the
argument. The full rebuild, public verifier, and packed-span checks are exact
at 526 leaves.

## Matching leaf — func_80084B44 (2026-09-01)

`func_80084B44` is now a registered thirteen-word exact C leaf. It initializes
the three callback slots at `D_8009B73C`, `D_8009B740`, and `D_8009B744`; the
existing store-delay-slot profile preserves retail's final `sw` in the `jr`
delay slot. The full rebuild, public verifier, and packed-span checks are
exact at 527 leaves.

## Matching leaf — func_80084AE8 (2026-09-01)

`func_80084AE8` is now a registered fourteen-word exact C leaf. It maps either
of two `D_800A5B70` entry addresses to slots `0x10` and `0x20`, returning
`0xFF` when neither matches. Pinning the entry, index, and slot locals to
retail's `$v1`, `$a1`, and `$a2` preserves the loop schedule. The full rebuild,
public verifier, and packed-span checks are exact at 528 leaves.

## Matching leaf — func_8006EBE4 (2026-09-01)

`func_8006EBE4` is now a registered nine-word exact C leaf. It returns the
signed halfword `D_800B0DBC` when byte flag `D_800B0DBA` is set, otherwise
`-1`. The natural conditional-return phrasing preserves the retail load-delay
and branch-delay scheduling. The full rebuild, public verifier, and
packed-span checks are exact at 529 leaves.

## Matching leaf — func_80090574 (2026-09-01)

`func_80090574` is now a registered ten-word exact C leaf. It consumes one
stream byte, sets the `0x900` flag at state offset `0xF4`, and writes the byte
as a halfword at `+0x10E`. A zero-code memory barrier after advancing the
stream pointer preserves retail's `$v0` reuse and load order. The full rebuild,
public verifier, and packed-span checks are exact at 530 leaves.

## Matching leaf — func_8001856C (2026-09-01)

`func_8001856C` is now a registered eleven-word exact C leaf. It clears six
word fields at offsets `0x68`, `0x6C`, `0x70`, `0x78`, `0x7C`, and `0x80` of
the `D_8009D2F0` state, then returns `1`. The full rebuild, public verifier,
and packed-span checks are exact at 531 leaves.

## Matching leaf — func_8007F960 (2026-09-01)

`func_8007F960` is now a registered eleven-word exact C leaf. It invokes the
nullable `D_800B8AB8` callback with the low byte of its argument. The full
rebuild and packed-span checks are exact at 532 leaves.

## Matching leaf — func_80016DF8 (2026-09-01)

`func_80016DF8` is now a registered nine-word exact C leaf. It sets
`0x20000000` in the `+0x98` state flags of `D_8009D2F0`, then returns `1`.
The full rebuild and packed-span checks are exact at 533 leaves.

## Matching leaf — func_80016E1C (2026-09-01)

`func_80016E1C` is now a registered nine-word exact C leaf. It clears
`0x20000000` in the `+0x98` state flags of `D_8009D2F0`, then returns `1`.
The full rebuild and packed-span checks are exact at 534 leaves.

## Matching leaf — func_800172BC (2026-09-01)

`func_800172BC` is now a registered nine-word exact C leaf. It sets bit
`0x10` in the `+0x98` flags of `D_8009D2F0` and returns `0`; pinning the state
pointer to retail `$v1` preserves the original allocation. The full rebuild
and packed-span checks are exact at 535 leaves.

## Matching leaf — func_800173F4 (2026-09-01)

`func_800173F4` is now a registered seven-word exact C leaf. It copies one
word from the source pointer at argument offset `+4` to the destination
pointer at `+0`, then returns `1`. The full rebuild and packed-span checks are
exact at 536 leaves.

## Matching leaf — func_800172FC (2026-09-01)

`func_800172FC` is now a registered eight-word exact C leaf. It sets bit
`0x10` in halfword `D_8009D300[4]` and returns `0`. The Era `-O2 -G8` profile
preserves its GP-relative pointer load and retail `addu` return-zero form. The
full rebuild and packed-span checks are exact at 537 leaves.

## Matching leaf — func_800172E0 (2026-09-01)

`func_800172E0` is now a registered seven-word exact C leaf. It reads an
unsigned halfword through its argument pointer, stores it as a word at
`D_8009D300 + 0x10`, and returns `0`. The Era `-O2 -G8` profile preserves the
GP-relative state-pointer load; full rebuild and packed-span checks are exact
at 538 leaves.

## Matching leaf — func_800534CC (2026-09-01)

`func_800534CC` is now a registered six-word exact C leaf. It returns one
signed halfword from the GP-relative table pointer `D_8009D048`; era `-O2
-G8` reproduces the table load, scaled index, and halfword access. The full
rebuild, public verifier, and all packed-span checks are exact at 427 leaves.

## PE-B54K-AM — CdlReadS registration prefix (2026-08-31)

The 30-word production prefix of `func_80081314` now applies both mode bits
and registers exact guest callbacks `0x8007C214` (DMA channel 3) and
`0x800813E8` (CD callback exchange). The executable names command `0x1B`
`CdlReadS`; its 212-word low-level provider builds Pause/Setmode/Setloc/ReadS
queue records and remains untranslated. Registration invokes no callback.
The suite is 994/994 and strict production stops before queue issue at
`func_80081314_func_8007F0C8_cut`. Evidence:
`docs/evidence/pe-b54kam-read-registration/REPORT.md`.

## PE-B54K-AN — CdlReadS delivery audit (2026-08-31)

The registered CD callback enters a 583-word stream/ring state machine; its
35-word DMA3 callback changes producer-record status from the CD phase's `3`
to ready `2` before optional consumer notification. Retail also conditionally
calls that callback at the CD-handler tail under `D_800B89F4`, so the next
rung must recover the direct-tail versus DMA-dispatch rule. Direct
sector-to-ring synchronous completion would erase retail partial-record,
ring-full, DMA-in-flight, and callback ordering, so no such shortcut was
added. The next rung is a generic first-sector event contract with the exact
completion-selection rule. Evidence:
`docs/evidence/pe-b54kan-cdlreads-delivery-audit/REPORT.md`.

## PE-B54K-AO — stream completion selector (2026-08-31)

The executable/PE.IMG direct-writer census proves production
`D_800C0DB8 == 0`. That value makes the CD-handler tail skip its optional
direct call to `func_8007C214`; `D_800B89F4` remains one and suppresses CD
re-entry until the separately registered DMA3 callback publishes status 2,
notifies the consumer, and clears it. The next implementation contract can
therefore target the proven deferred-DMA production path. Evidence:
`docs/evidence/pe-b54kao-stream-completion-selector/REPORT.md`.

## PE-B54K-AP — first retail multiplexed stream sectors (2026-08-31)

Disc 1 LBA 189742, the first sector of authenticated `FMV001.STR`, is
Mode-2 Form-1 video: 2048 bytes plus EDC/ECC, with submode `0x48`. Video
chunks 0..6 are followed by a Mode-2 Form-2 XA sector (`0x64`, 2324 data
bytes), then chunks 7..8. The current disc API correctly exposes 2048 bytes
for ISO/Form-1 reads but discards the routing subheader and truncates Form-2
XA by 276 bytes, so it cannot faithfully back multiplexed `CdlReadS`. No
delivery shortcut was added; the next rung is a separate read-only Mode-2
stream-sector API with variable geometry. Evidence:
`docs/evidence/pe-b54kap-first-form2-sector/REPORT.md`.

## PE-B54K-AL — blocking CdlSetloc arm (2026-08-31)

The complete 26-word blocking command wrapper and the executable's own
command-name table prove the movie call is command 2 / `CdlSetloc`. The
complete movie CFG never reads its eight-byte stack response, so native
retains the exact four-byte location without inventing response bytes.
Unsupported commands, response consumers, absent disc, malformed BCD, and
out-of-range locations are inert. The native suite is 993/993 and strict
production stops before `func_80081314` at
`func_801924F8_801927A0_cut`. Evidence:
`docs/evidence/pe-b54kal-blocking-setloc/REPORT.md`.

## PE-B54K-AK — movie CD idle wait (2026-08-31)

The exact 8-word `CdReady`/queue-depth loop at `0x80192770..0x8019278F` is
now translated over the already-complete libcd providers. It waits for ready
state 1 and queue depth zero without writing either authority. Real Disc 1
and the independent synthetic movie fixture both retain ready/idle state,
all predecessor oracles pass, and strict production stops at
`func_801924F8_80192790_cut` before the unresolved CD command wrapper.
Evidence: `docs/evidence/pe-b54kak-cd-idle-wait/REPORT.md`.

## PE-B54K-AJ — movie stream-control initialization (2026-08-31)

The complete 33-word `func_8007C304` initializer and 7-word
`func_8007C544` setter are translated. Production supplies
`(1, signed record[+6], -1, 0, 0)`; the setter publishes the signed range and
the initializer resets only the proven callback/option/auxiliary state. No
stream frame or callback is consumed. Signed-value, option-bit, callback,
auxiliary, and halfword-width controls pass; the native suite is 992/992 and
strict production stops at the first following CD-ready call,
`func_801924F8_80192770_cut`. Evidence:
`docs/evidence/pe-b54kaj-stream-control/REPORT.md`.

## PE-B54K-AI — movie record-pool initializer (2026-08-31)

Retail entry `0x8007A214`, its worker `0x8007A244`, and the 13-word clear
helper `0x8007C444` are now translated completely. The production call
publishes `D_801D0DFC` as the pool base and `0x40` as its unsigned record
count, resets the proven associated globals, and clears only word zero of
each 32-byte record. No unproven SDK name is assigned. Synthetic footprint,
access-width, count-boundary, and zero-count controls pass; the native suite
is 991/991 and strict production stops at
`func_801924F8_80192750_cut`. Evidence:
`docs/evidence/pe-b54kai-record-pool/REPORT.md`.

## PE-B54K-AH — libpress `DecDCToutCallback` registration (2026-08-31)

The complete 9-word `func_8010C0D8` wrapper is now translated. Retail passes
the exact guest callback identity `0x80191DC8` at `0x80192738`; the wrapper
registers it through the complete DMA callback setter with channel 1. Focused
controls prove only slot 1 and its DICR enable change, with no callback
delivery or invented DMA completion. The native suite is 990/990 and strict
production stops at the later record-pool initializer. Evidence:
`docs/evidence/pe-b54kah-decdctoutcallback/REPORT.md`.

## PE-B54K-AG — generic MDEC reset/table substrate (2026-08-31)

The complete 60-word `func_8010C0FC` mode-zero/mode-one reset and its 36-word
DMA0 table-submit helper are now translated over a generic value-only MDEC
substrate. Retail's MMIO pointer table, DPCR `| 0x88`, DMA0 register values,
and both 32-word command blocks are authenticated. Real Disc 1 submits exact
quantization and scale payloads; the second submission remains pending rather
than being falsely completed. A synthetic payload control and mutation-free
invalid-mode boundary pass. Normal and fresh ASan/UBSan CTest pass, the native
suite is 989/989, and strict production stops at
`func_801924F8_80192730_cut`. Evidence:
`docs/evidence/pe-b54kag-mdec-reset/REPORT.md`.

## PE-B54K-AF — libpress `DecDCTReset` wrapper (2026-08-31)

The 38-sector payload `[0x039F,0x03C5)` is now authenticated as the retail
MDEC/libpress module. Its debug strings, 256-byte environment pair, and
channel-0/channel-1 callback wrappers establish the SDK family. The complete
13-word `func_8010BE3C` is `DecDCTReset`: mode zero calls the already-complete
`ResetCallback`, then all modes forward unchanged to `func_8010C0FC`.
Production now executes the call at `func_801924F8+0x230` and stops at that
60-word internal MDEC/DMA reset rather than treating it as a no-op. Mode-zero
and mode-one controls pass, normal and fresh ASan/UBSan CTest pass, and the
native suite is 988/988. Evidence:
`docs/evidence/pe-b54kaf-decdctreset/REPORT.md`.

## PE-B54K-AE — movie state setup (2026-08-31)

The authenticated `func_801924F8` prefix now covers 140 of 271 words through
`0x80192728`. The new 69-word block copies the returned `CdlLOC`, populates
four retail pointer fields, builds the two record-derived coordinate pairs,
selects one with the low byte of `D_800ACDDC`, and derives 16 versus 24 from
the movie-kind byte. The block contains no calls; native stops immediately
before retail's `jal 0x8010BE3C`. Real Disc 1 and a poisoned synthetic
buffer-1/kind-zero control both pass, as do normal and fresh ASan/UBSan CTest.
The native suite is 987/987 and strict execution stops at
`func_801924F8_80192728_cut`. Evidence:
`docs/evidence/pe-b54kae-1924f8-state/REPORT.md`.

## PE-B54K-AD — movie filename/search continuation (2026-08-31)

The authenticated `func_801924F8` prefix now covers 71 words through
`0x80192614`. Retail selects `\\FMV1` below record index 21 and `\\FMV2`
otherwise, appends the record suffix through the proven BIOS A(15h) `strcat`
trampoline, waits for the CD queue, and calls `DsSearchFile`. Real Disc 1
index 1 resolves `FMV001.STR;1` at LBA 189742; a synthetic index-21 control
independently proves the FMV2 branch without planting production state.
Normal and fresh ASan/UBSan CTest pass, the native suite is 986/986, and
strict real-disc execution stops honestly at
`func_801924F8_80192614_cut`. Evidence:
`docs/evidence/pe-b54kad-1924f8-filename/REPORT.md`.

## PE-B54K-AC — retail CD sector contract (2026-08-31)

The next `func_801924F8` continuation exposed and corrected an older host-unit
mismatch: retail passes sector counts through `func_8006E6A8` / `8006E6D4`
into `func_80080E34`, while the host provider had treated direct counts as
bytes. `func_8006E6D4` is now the single checked sector-to-`0x800`-byte
adaptation point; compensating shifts were removed from `func_8006E6A8` and
the `func_8006CDA4` state-7 cut. Real Disc 1 now loads the complete 133-sector
overlay `[03D2,0457)` into `[8018EFF0,801D17F0)`, verified by SHA/FNV and an
exclusive-end canary. Normal and fresh ASan/UBSan CTest pass; the 985-case
suite is unchanged and strict production still stops honestly at
`func_801924F8_80192584_cut`. Evidence:
`docs/evidence/pe-b54kac-sector-overlay/REPORT.md`.

## Grind-lane port complete — 275 matching C leaves (2026-08-21)

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

## PE-BTL147 — theater → Eve-entry dependency audit (2026-08-24)

Retail evidence is committed under `docs/evidence/pe-btl147-theater-eve-path/`.
The exhaustive 414-script scan finds the authentic `m0004i` → `m0005i` hops
and zero field-script `0x31` inbound hops to `m0360i`. Retail m0360i module 2
is the unique `persist[0] |= 4` writer; the native branch must not plant that
bit or a destination token. The current native code has no generic scene
scheduler that enters m0360i, so the next faithful rung is the event/scheduler
bridge, not another m0005i gate workaround. Current native suite: 985/985.

## PE-BTL148 — scheduler census needs retail artifact (2026-08-24)

The Disc 1 executable census is recorded at
`docs/evidence/pe-btl148-scheduler-census/REPORT.md`. All 1,011 field-script
`0x31` commands use immediate argument mode. The executable destination-state
writers are limited to ordinary immediate `0x31`, the two fixed computed
name-table handlers (`func_80015790`/`func_80015964`), system/death/menu
states, and save restoration. The computed tables omit `m0360i`, and the
package loader consumes only `D_8009D280`.

No scheduler implementation or m0360i special case is allowed yet. A retail
PCSX trace/save reaching the Day 2+ event, or the executable/overlay that
contains the missing writer, is required to close the provenance. The
standalone native frontier is separately
`PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192584_cut` after
B54K-AB completed the display-pair helper; this
does not change the scheduler evidence boundary.

## PE-BTL149 — runtime-loaded overlay / Disc 2 census (2026-08-24)

The prior Disc 1 executable census was extended to the PE.IMG ranges selected
by retail `D_8009315E..D_8009317A` and to Disc 2. Disc 2 `SLUS_006.68` and
PE.IMG are byte-identical to Disc 1, so they add no distinct code coverage.
The PE.IMG range `[0x0700,0x07B7)` contains a real state-driven chooser:
three direct stores to `D_8009D280` at raw offsets `0x383060`, `0x383230`,
and `0x383258`, with inputs including `D_800A7918`, `D_800A77FC`, and an
indirect `D_801ACA68`/`D_8019F034` dispatch. It constructs nearby computed
destination tokens but not `0xA8066048`; the full PE.IMG scan found zero raw
or `lui 0xA806`→`ori/addiu 0x6048` forms. Evidence and reproducible scanner:
`docs/evidence/pe-btl149-overlay-disc2-census/REPORT.md` and
`tools/research/pe_btl149_overlay_disc2_census.py`.

This closes the “Disc 2 may contain a different executable” hypothesis and
corrects BTL148's coverage boundary, but does not prove the indirect branch's
event input or m0360i selection. Do not implement a scheduler or special-case
m0360i. Remaining scheduler status: `SEMANTIC_IMPLEMENTATION=not_started`
and `SCHEDULER_PROVENANCE=NEEDS_ARTIFACT`. The independent native frontier is
now `func_801924F8` internal cut `0x80192584`; current suite `985/985`.

## PE-BTL150 — name-form search for m0360i (2026-08-24)

The retail alphabet at `D_800930B4` is
`0123456789abcdefghiklmnopqrstuvwxy` (no `j`). `func_8006E2D0` extracts six
5-bit fields at shifts `27,22,17,12,7,2`; `func_8006E3D4`, called by
`func_80015790` and `func_80015964`, is the inverse. Round-trip proof confirms
`0xA8066048` is exactly `m0360i`, so the BTL147 token identity was correct.

The executable and complete PE.IMG name-form scan found zero ASCII
`m0360i`/`M0360I` spellings and zero packed `0xA8066048` words in either byte
order. The generic `D_80093378` package table has the established numeric slot
359 (`D_80093378 + 359*8 = 0x80093EB0`), but that entry is package metadata and
does not provide a static name association. Evidence and scanner:
`docs/evidence/pe-btl150-name-search/REPORT.md` and
`tools/research/pe_btl150_name_search.py`.

BTL150 closes the wrong-token hypothesis and the available static name-form
lead. The unresolved boundary remains the runtime population/selection of the
indirect `D_801ACA68 -> D_8019F034` dispatch. Do not implement a scheduler or
special-case m0360i. Scheduler status remains
`SEMANTIC_IMPLEMENTATION=not_started` / `NEEDS_ARTIFACT`; the independent
native frontier is now `func_801924F8` internal cut `0x80192584`, with suite
`985/985`.

Capture guidance: watch `func_8006E3D4` callers and their six-byte inputs, then
the resulting `D_8009D280` write. Since `m0367i=0xA80663C8` and
`m0360i=0xA8066048` differ only in one 5-bit field, runtime field derivation
from a neighboring token is a concrete hypothesis to test; this is not a
native implementation claim.

## PE-B54K-B1 — func_80030894 through L4 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x80030C9C)`:
258 words total. B54K-B1 added the exact 118-word continuation
`[0x80030AC4,0x80030C9C)`, covering the bank-local fixed tile/G4/state/sprite
setup and the complete four-packet L4 loop. The first excluded instruction at
`0x80030C9C` initializes L5, so the named strict frontier is now
`func_80030894_L4_cut`.

The independent oracle verifies the exact executable SHA-1, whole-window
SHA-256, all seven `jal` sites in order, 39 selected retail words, the L4
back edge/bound, and the first excluded L5 word. Two focused tests verify all
fixed fields, the exact `4 * 28 = 0x70` L4 extent, dirty/repeat determinism,
and the unresolved-boundary stop. Normal and fresh ASan/UBSan suites pass
`930/930`; the sanitizer audit also fixed a pre-existing negative signed-shift
UB in GTE translation math at commit `469f13c`.

Evidence: `docs/evidence/pe-b54kb1-30894-l4/REPORT.md` and
`pc_port/tools/b54kb1_30894_l4_oracle.py`. Scheduler provenance remains
separately `NEEDS_ARTIFACT`: no destination token, m0360i special case, or
persist bit was added. The next available production rung is L5; the BTL151
packer capture remains the next scheduler rung.

## PE-B54K-B2 — func_80030894 through L5 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x80030D20)`:
291 words total. B54K-B2 adds the retail 33-word L5 continuation
`[0x80030C9C,0x80030D20)`. Its five packets use the machine-decoded address
`0x8009E1D0 + bank*140 + slot*28`, close exactly after `5*28 = 0x8C`
bytes, and stop before the post-L5 setup instruction at `0x80030D20`.

The independent oracle checks every one of the 33 retail words plus the first
excluded word, the sole `jal func_800370DC`, loop edge/bound, and extent. Two
tests verify all packet fields, complete dirty/repeat determinism, both end
sentinels, and the named `func_80030894_L5_cut`. Normal and rebuilt
ASan/UBSan suites pass `932/932` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kb2-30894-l5/REPORT.md` and
`pc_port/tools/b54kb2_30894_l5_oracle.py`. Scheduler provenance remains
`NEEDS_ARTIFACT`; the next artifact-free production rung begins with the
post-L5 fixed-group setup.

## PE-B54K-C — func_80030894 through L6 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x80030F6C)`:
438 words total. The 147-word C rung closes a coherent group of two G4
records, three shaded sprite records, and the three-tile L6 loop. Its exact
write set is six separate ranges; gap/end sentinels prevent a broad envelope
from hiding stray writes. L6 uses the three prologue bytes from
`D_8009CD90`, one replicated RGB value per tile.

The independent oracle verifies the whole retail-window hash, all nine calls
in order, 55 selected words, L6 edge/bound/extent, and the first excluded
`D_8009E460` materialization. Focused state and dirty/repeat tests pass;
normal and rebuilt ASan/UBSan suites are `934/934` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kc-30894-l6/REPORT.md` and
`pc_port/tools/b54kc_30894_l6_oracle.py`. The named production boundary is
`func_80030894_L6_cut`; scheduler provenance remains independently
`NEEDS_ARTIFACT`.

## PE-B54K-D — func_80030894 through L7 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x800310A4)`:
516 words total. The 78-word D rung builds one wrapped sprite, two direct
SetSprt records, one PolyF3 header, and the complete three-sprite L7 loop.
Its write set is five separate ranges; per-range end sentinels and poisoned
retail-untouched bytes guard against broad writes. L7 closes exactly after
`3*28 = 0x54` bytes.

The independent oracle verifies the whole retail-window hash, all five calls
in order, 52 selected words, L7 edge/bound/extent, and both cut-side boundary
words. Focused state and dirty/repeat tests pass; normal and rebuilt
ASan/UBSan suites are `936/936` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kd-30894-l7/REPORT.md` and
`pc_port/tools/b54kd_30894_l7_oracle.py`. The named production boundary is
`func_80030894_L7_cut`; the first excluded word initializes L8 at
`0x800310A4`. Scheduler provenance remains independently `NEEDS_ARTIFACT`.

## PE-B54K-E — func_80030894 through L8 (2026-08-29)

The production prefix now implements retail `[0x80030894,0x80031110)`:
543 words total. The 27-word E rung closes the ten-sprite L8 loop at
`D_8009E500 + bank*280 + slot*28`. The exact bank-zero span is
`[0x8009E500,0x8009E618)`; each wrapped sprite receives RGB `0x80`, while
all twelve bytes at `+0x10..+0x1B` remain untouched.

The independent oracle compares all 27 words, verifies the sole static call,
L8 edge/bound/strides/extent, and both cut-side words. Focused full-span and
dirty/repeat tests pass; normal and rebuilt ASan/UBSan suites are `938/938`
with zero diagnostics.

Evidence: `docs/evidence/pe-b54ke-30894-l8/REPORT.md` and
`pc_port/tools/b54ke_30894_l8_oracle.py`. The named production boundary is
`func_80030894_L8_cut`; the first excluded word materializes
`D_8009E768` at `0x80031110`. Scheduler provenance remains independently
`NEEDS_ARTIFACT`.

## PE-B54K-F — func_80030894 through L9 (2026-08-30)

The production prefix now implements retail `[0x80030894,0x800311EC)`:
598 words total. The 55-word F rung builds one fixed wrapped sprite at
`D_8009E768 + bank*28` and the complete four-sprite L9 loop at
`D_8009E7A0 + bank*112 + slot*28`. Its two packet ranges remain separate
across the retail gap, and L9 closes exactly after `4*28 = 0x70` bytes.

The independent oracle compares all 55 words, verifies both static calls,
L9 edge/bound/strides/extent, and both cut-side words. Focused full-union and
dirty/repeat tests pass; normal and rebuilt ASan/UBSan suites are `940/940`
with zero diagnostics.

Evidence: `docs/evidence/pe-b54kf-30894-l9/REPORT.md` and
`pc_port/tools/b54kf_30894_l9_oracle.py`. The named production boundary is
`func_80030894_L9_cut`; the first excluded word materializes
`D_8009E730` at `0x800311EC`. Scheduler provenance remains independently
`NEEDS_ARTIFACT`.

## PE-B54K-G — func_80030894 through L10 (2026-08-30)

The production prefix now implements retail `[0x80030894,0x80031320)`:
675 words total. The 77-word G rung preserves the retail call-coupled group:
fixed wrapper at `D_8009E730`, CLUT computation `(0x130,0x1F9)->0x7E53`,
fixed wrapper at `D_8009E880`, and the complete two-sprite L10 loop at
`D_8009E8B8 + bank*56 + slot*28`.

The independent oracle compares all 77 words, verifies all four calls in
order, CLUT inputs/value, L10 edge/bound/strides/extent, and both cut-side
words. Focused full-union and dirty/repeat tests pass; normal and rebuilt
ASan/UBSan suites are `942/942` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kg-30894-l10/REPORT.md` and
`pc_port/tools/b54kg_30894_l10_oracle.py`. The named production boundary is
`func_80030894_L10_cut`; the first excluded word materializes
`D_8009E928` at `0x80031320`. Scheduler provenance remains independently
`NEEDS_ARTIFACT`.

## PE-B54K-H — func_80030894 through L11 (2026-08-30)

The production prefix now implements retail `[0x80030894,0x80031438)`:
745 words total. The 70-word H rung builds the fixed sprite at
`D_8009E928 + bank*28` and the complete thirteen-sprite L11 loop at
`D_8009E960 + bank*364 + slot*28`. L11 consumes patterned descriptors
`func_8005DADC(0x6A + slot)` and preserves the retail U/V/CLUT/width/height
field mapping while leaving every XY word untouched.

The independent oracle compares all 70 words, verifies all four static calls,
descriptor/TPage constants, L11 edge/bound/strides/extent, and both cut-side
words. Focused patterned-state and dirty/repeat tests pass; normal and rebuilt
ASan/UBSan suites are `944/944` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kh-30894-l11/REPORT.md` and
`pc_port/tools/b54kh_30894_l11_oracle.py`. The named production boundary is
`func_80030894_L11_cut`; the first excluded word at `0x80031438` begins the
final 43-word TPage/fixed-sprite/outer-loop epilogue. Scheduler provenance
remains independently `NEEDS_ARTIFACT`.

## PE-B54K-I — func_80030894 complete (2026-08-30)

The final 43-word window `[0x80031438,0x800314E4)` is now native, completing
all 788 words of `func_80030894`. It builds the `16 x 16` final sprite at
`D_8009EC38 + bank*28`, increments the bank, repeats the complete bank-local
body for banks 0 and 1 through the retail back edge to `0x80030910`, then
returns through `jr ra + nop`.

The independent oracle compares all 43 new words and hashes the full body,
verifies both final calls, final packet fields, outer-loop edge/bound/extent,
and next-function boundary. Focused tests sample every second-bank packet
family and dirty/repeat the complete two-bank E-region; normal and rebuilt
ASan/UBSan suites are `946/946` with zero diagnostics.

Evidence: `docs/evidence/pe-b54ki-30894-complete/REPORT.md` and
`pc_port/tools/b54ki_30894_complete_oracle.py`. No unresolved provider remains
inside `func_80030894`. B54K-J below supersedes this rung's caller-side
frontier. Scheduler provenance remains independently `NEEDS_ARTIFACT`.

## PE-B54K-J — D_800930F0 completion/reissue gate (2026-08-30)

The exact eight-word caller window `[0x8006B0B4,0x8006B0D4)` is now native.
It polls the F0 read after `func_80030894`, clears the provider's busy bits on
completion, returns only to the F0 issue on `-1`, and skips both `718D0` and
`func_80030894` on that reissue path. The implemented `func_8006AD40` prefix
now closes at `0x394` bytes / 229 words.

The independent oracle authenticates the executable and window hash,
compares all 8 words, decodes both back edges and the sole call, and checks
both boundary words. Two focused tests prove canonical completion and
repeat/no-duplicate behavior. Normal and freshly rebuilt ASan/UBSan suites
pass `948/948` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kj-6ad40-f0-wait/REPORT.md` and
`pc_port/tools/b54kj_6ad40_f0_wait_oracle.py`. The production frontier is now
superseded by B54K-K below. Scheduler provenance remains independently
`NEEDS_ARTIFACT`; no destination token, `m0360i` branch, or persistence bit
was added.

## PE-B54K-K — D_800930E0 issue, F0 lookups, and completion (2026-08-30)

The natural 38-word caller group `[0x8006B0D4,0x8006B16C)` is now native. It
issues the five-sector E0 range into `*(D_800B0CD8+0x16C)`, resolves three
retail keys from the completed F0 archive into `+0x11C/+0x120/+0x124` exactly
once, and then consumes the live E0 completion. Timeout reissues only E0;
positive polls do not replay the lookups. The implemented `func_8006AD40`
prefix now closes at `0x42C` bytes / 267 words.

The independent oracle authenticates and compares all 38 words, all five
calls, four control-flow edges, exact keys/stores, boundaries, and prefix
arithmetic. Focused tests use a 256-sector MODE2 fixture with a real on-disc
three-key F0 archive plus an empty negative control. Normal and freshly
rebuilt ASan/UBSan suites pass `950/950` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kk-6ad40-e0-group/REPORT.md` and
`pc_port/tools/b54kk_6ad40_e0_group_oracle.py`. The production frontier is
superseded by B54K-L below. Scheduler provenance remains independently
`NEEDS_ARTIFACT`; no destination token, `m0360i` branch, or persistence bit
was added.

## PE-B54K-L — first D_80093126 issue/walk/wait (2026-08-30)

The coherent 45-word group `[0x8006B16C,0x8006B220)` is now native. It issues
the two-sector `D_80093126={0x219,0x21B}` range into
`*(D_800B0CD8+0x188)`, walks the completed E0 archive at `+0x16C` with the
retail packed count/offset and 0x14-byte entry stride exactly once, then
consumes completion. Timeout reissues only the 3126 range; positive polls
skip the completed entry walk. The implemented prefix is now 0x4E0 bytes /
312 words.

The independent oracle authenticates and compares all 45 words, issue ABI,
entry loop, retry topology, both boundaries, and the poll branch's mandatory
delay-slot `lui`. Focused tests prove a two-entry archive, ordered data
addresses, exact two-sector transfer, and a zero-count negative. Normal and
freshly rebuilt ASan/UBSan suites pass `952/952` with zero diagnostics.

Evidence: `docs/evidence/pe-b54kl-6ad40-3126-wait/REPORT.md` and
`pc_port/tools/b54kl_6ad40_3126_wait_oracle.py`. At that historical rung the
production frontier was `func_8006AD40_D_80093126_archive_cut` before retail
`0x8006B220`; B54K-M below has since completed the function.
Scheduler provenance remains independently `NEEDS_ARTIFACT`; no destination
token, `m0360i` branch, or persistence bit was added.

## PE-B54K-M readiness — historical pre-implementation audit (2026-08-30)

At this audit rung, `[0x8006B220,0x8006B35C)` was fully audited but not yet
implemented; the completion section below supersedes that disposition. The
independent oracle authenticates all 79 words, both
window hashes, the completed `+0x188` archive walk, all seven now-available
callees, display-env selection, exact state resets, bit-0 clear, normal
return, and the next-function boundary.

A temporary completion translation compiled, then was reverted after a full
trial produced `929/952`: all 23 failures were historical contracts that
intentionally require the current prefix frontier or pre-finalization repeat
behavior. The dedicated implementation rung must migrate those assertions to
full-function state checks, add positive/zero `+0x188` walks, the `0x40/0x80`
matrix, first-call-clear/second-call-guard behavior, stream-F1 whole-RAM
effects, and measure the caller's next strict frontier after its two DMA
checkpoints. Production remained at B54K-L and `952/952` at this audit rung.

Evidence:
`docs/evidence/pe-b54km-6ad40-completion-readiness/REPORT.md` and
`pc_port/tools/b54km_6ad40_completion_readiness.py`.

## PE-B54K-N — BIOS `FlushCache` adapter (2026-08-30)

`func_800726C4` is now a generic native platform provider. Retail is the
three-word BIOS A0(44h) `FlushCache` veneer, with 14 exact executable callers.
The host adapter is a deliberate no-op because native code has no emulated
R3000 instruction-cache authority. Direct strict execution and strict
`func_8006E834` integration pass without guest writes, bootstrap records, or
an unbalanced critical section. The suite is `954/954`.
The fresh ASan/UBSan suite is also `954/954` with zero diagnostics.

At B54K-N this did not move the B54K-L production frontier. B54K-M below has
since completed 6AD40 and measured `func_801909B4` after both DMA checkpoints,
confirming this static prediction.
Evidence: `docs/evidence/pe-b54kn-726c4-flushcache/REPORT.md` and
`pc_port/tools/b54kn_726c4_flushcache_oracle.py`.

## PE-B54K-O — `func_801909B4` overlay recovery (2026-08-30)

The canonical post-B54K-M boundary's retail bytes are now recovered. Retail
6E834 loads PE.IMG `[0x03D2,0x0457)` at `0x8018EFF0`, placing
`func_801909B4` at PE.IMG offset `0x1EA9C4`. Its exact range is
`[0x801909B4,0x801918F8)`, 0xF44 bytes / 977 words, with 70 direct calls to
32 targets and a normal return.

The audit found a prerequisite native discrepancy: `D_80093164` is currently
modeled as zero BSS and `D_80011614` uses bootstrap address `0x8010BD00`,
whereas retail rodata is `{0x03D2,0x0457}` and `0x8018EFF0`. Thus native
6E834 presently performs a zero-length read and does not load this overlay.
Do not claim natural 801909B4 entry until a dedicated authority/layout rung
fixes and tests those values. Production and suite remain B54K-L / `954/954`.
Evidence: `docs/evidence/pe-b54ko-1909b4-overlay-recovery/REPORT.md` and
`pc_port/tools/b54ko_1909b4_overlay_oracle.py`.

## PE-B54K-P — retail overlay authority handoff (2026-08-30)

Real-disc startup now adopts `D_80011614=0x8018EFF0` and
`D_80093164[0..3]={0x03D2,0x0457,0x04FC,0x0516}` from guest RAM only after
the boot EXE is authenticated and loaded. Range ordering and the complete
0x42800-byte destination are validated before publication. Bootstrap fixtures
retain their explicit safe defaults.

Normal and fresh ASan/UBSan suites pass `956/956`; a real-disc strict smoke
still reaches the unchanged B54K-L frontier. Evidence:
`docs/evidence/pe-b54kp-overlay-authority/REPORT.md` and
`pc_port/tools/b54kp_overlay_authority_oracle.py`.

## PE-B54K-M — complete func_8006AD40 (2026-08-30)

All 391 retail words of `func_8006AD40` are now implemented through the
normal `jr ra; nop` return. The final 79-word suffix walks the completed
`+0x188` archive, issues stream command F1, performs DrawSync/ResetGraph/
VSync/PutDispEnv/SetDispMask in retail order, applies the complete
`0x40`/`0x80` conditional-field matrix, and clears `D_800B0CD8` bit 0 last.

All 23 historical B54K-L frontier failures were migrated to full-function
contracts without deleting their earlier loop, poll, transfer, image-call,
DMA, and ordering assertions. Two new tests cover positive/zero final
archives and all four flag combinations. Normal and fresh ASan/UBSan suites
pass `958/958`; sanitizer diagnostics are zero.

The real-disc caller executes both explicit DMA checkpoints (`2/2` calls,
queries, and services; token 2 serviced). The next measured unbounded strict
frontier is `func_801909B4` from `func_8001220C`. Evidence:
`docs/evidence/pe-b54km-6ad40-complete/REPORT.md` and
`pc_port/tools/b54km_6ad40_completion_oracle.py`.

```text
FUNC_8006AD40=COMPLETE_NATIVE_TRANSLATION
PRODUCTION_REACHABILITY=blocked_at_func_801909B4
FUNC_801909B4_BYTES=STATICALLY_RECOVERED_NOT_IMPLEMENTED
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-Q — `func_801909B4` MoveImage prefix (2026-08-30)

Historical rung, superseded by B54K-R below.

The real-disc overlay now enters 149 translated retail words
`[0x801909B4,0x80190C08)`. The prefix copies two 0x5C-byte DRAWENV records
and two 0x14-byte DISPENV records into overlay storage, publishes six arena
pointers from authenticated `D_80011610`, executes translated
`func_8005E57C`, the positive path of `func_8005C1EC`, complete
`func_80042538`, and `SetDispMask(0)`, then captures the first unresolved
call.

The exact next provider is PsyQ `MoveImage` (`func_8007512C`) at
`0x80190C08`, with arguments `RECT{320,0,160,256}`, destination `(704,0)`.
The full RECT payload is recorded and the caller now honors a nested stop
before consuming the retained `-1` return. Strict real-disc execution proves
natural overlay entry and reports `func_8007512C` from `func_801909B4`.

Five focused tests, full normal, and fresh ASan/UBSan suites pass `962/962`
with zero sanitizer diagnostics. Evidence:
`docs/evidence/pe-b54kq-1909b4-moveimage-prefix/REPORT.md` and
`pc_port/tools/b54kq_1909b4_prefix_oracle.py`.

```text
FUNC_8006AD40=COMPLETE_NATIVE_TRANSLATION
FUNC_801909B4_PREFIX=149_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_8007512C_from_func_801909B4
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-R — generic MoveImage and display prefix (2026-08-30)

PsyQ `func_8007512C` is now a complete 46-word native translation. Retail
`D_80095744=0x80095704` proves that it loads `func_80076C34` from jump-table
offset `+8` and `func_80076B98` from `+0x18`; it does not route through
`func_80076C10`. The exact 18-word worker accepts only the authenticated
five-word GP0(80h) MoveImage packet. General DrawOTag linked lists remain a
named boundary.

The generic GPU authority now performs synchronous VRAM-to-VRAM copies with
retail coordinate/size masking, zero-as-maximum dimensions, both-axis wrap,
and the console-verified horizontal overlap direction. It has no overlay
special case and creates no synthetic DMA2 completion or callback.

`func_801909B4` now translates 240 words
`[0x801909B4,0x80190D74)`: the canonical `160x256` MoveImage, DrawSync/VSync
sequence, two draw/display environments, exact field stores, and black
ClearImage all execute before the one-time overlay-local call
`func_80190660`. Eight focused contracts, the independent oracle, full
normal suite, and fresh ASan/UBSan suite pass; total is `967/967` with zero
sanitizer diagnostics. Strict real-disc execution measures
`func_80190660 from func_801909B4` as the next provider.

Evidence:
`docs/evidence/pe-b54kr-moveimage-display-prefix/REPORT.md` and
`pc_port/tools/b54kr_moveimage_oracle.py`.

```text
FUNC_8007512C=COMPLETE_NATIVE_TRANSLATION
FUNC_80076B98=MOVEIMAGE_ONE_PACKET_SUBSET_ONLY
FUNC_801909B4_PREFIX=240_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80190660_from_func_801909B4
GENERAL_LINKED_LIST_DMA=UNSUPPORTED_NAMED_BOUNDARY
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-S — deterministic DrawSync DMA drain (2026-08-30)

The 26-word PsyQ `func_80074DC0` wrapper and execution-proven 79-word
`func_80077294` drain are now represented. Mode zero preserves retail queue,
DMA-busy, and GPU-ready tests; nonzero mode preserves the exact pending/status
returns. Between calls to retail wait helper `func_80077404`, the host admits
at most one already-active DMA token through the established
`PE_Port_ServiceDmaIrqCheckpoint` owner. Polling reads never evolve hardware,
and the translated pump remains the only guest-ring consumer.

The normal wait-poll half of `func_80077404` preserves deadline and poll-word
semantics. Its destructive timeout recovery and a state with no external
progress remain named boundaries. Four focused contracts prove idle/status,
direct DMA, a two-transfer queued drain, and the no-progress negative. Full
normal and fresh ASan/UBSan suites pass `971/971`.

Real-disc execution now records 27 checkpoint opportunities, 26 active-token
queries/services, then reaches the unchanged `func_80190660 from
func_801909B4` frontier. This supersedes B54K-M's historical 2/2
caller-checkpoint service trace; it does not change scheduler provenance.

Evidence: `docs/evidence/pe-b54ks-drawsync-drain/REPORT.md` and
`pc_port/tools/b54ks_drawsync_oracle.py`.

```text
FUNC_80074DC0=DRAWSYNC_WRAPPER_ADAPTED
FUNC_80077294=EXECUTION_PROVEN_PATHS_TRANSLATED
FUNC_80077404=NORMAL_POLL_TRANSLATED_TIMEOUT_RECOVERY_FENCED
DMA_CHECKPOINT_TOTAL=27_CALLS_26_ACTIVE_TOKENS
PRODUCTION_REACHABILITY=blocked_at_func_80190660_from_func_801909B4
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-T — `func_80190660` image/fade prefix (2026-08-30)

Function-hood is proven for the 213-word overlay-local `func_80190660`: one
exact-start call at `0x80190D74`, a canonical `jr ra; nop`, and real preceding
and following function boundaries. Its first 128 words
`[0x80190660,0x80190860)` now execute.

The prefix computes image records from `[0x80193278]=0x3BAC8` and anchor
`0x80193254`, yielding records `0x801CED1C` and `0x801CED50`. Their retail
RECTs `{0,480,16,1}` and `{512,256,64,64}` traverse LoadImage; DrawSync drains
the large transfer through one established checkpoint in the focused test.
It then builds both `E1000018/E1000019` draw-mode banks and paired SPRTs,
clears both environment `+0x6D` bytes, enables display, applies the exact
`old==0 ? 1 : 0` selector, and stores the selected environment pointer.

The new boundary is `jal func_80075358` at `0x80190860`. Native applies its
RGB delay-slot store and records only packet length 1 plus command word
`0xE1000018`; retail's uninitialized stack-tag bytes 0..2 and a native stack
pointer are excluded. Two focused contracts, retained B54K-R integration,
the independent oracle, full normal suite, and fresh ASan/UBSan pass
`973/973`. Real-disc framebuffer state is now 6 VSync, 4 DrawSync, 3
presentations, mask 1; the aggregate DMA checkpoint census remains 27/26.

Evidence: `docs/evidence/pe-b54kt-190660-drawprim-prefix/REPORT.md` and
`pc_port/tools/b54kt_190660_drawprim_prefix_oracle.py`.

```text
FUNC_80190660_PREFIX=128_WORDS_TRANSLATED
OVERLAY_IMAGE_RECORDS=TABLE_DERIVED_AND_EXECUTED
TRANSIENT_PACKET_POINTER=NOT_RETAINED
PRODUCTION_REACHABILITY=blocked_at_func_80075358_from_func_80190660
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-U — DrawPrim wrapper + GP0(E1h) command (2026-08-30)

Retail `func_80075358` is the 23-word DrawPrim wrapper
`[0x80075358,0x800753B4)`. It calls jump-table slot 15 / `func_80077294`
(DrawSync), re-reads the jump-table pointer, then calls slot 5 / the 16-word
`func_80076B58` worker with `packet+4` and `packet[3]`. The worker always
writes GP1(04h), DMA direction off, then writes exactly the requested GP0
words. Dirty indirect targets remain typed boundaries.

The GPU authority now implements generic GP0(E1h) draw-mode state. The first
overlay DrawPrim executes command `0xE1000018`, growing the authenticated
prefix to 130 words, `[0x80190660,0x80190868)`. The next boundary is the
second DrawPrim at `0x80190868`, whose complete four-word SPRT packet is
`64000000 00580020 78000000 00400100`. Its tpage/CLUT geometry lines up with
the two table-derived uploads from B54K-T; rasterization is not claimed yet.

Two new focused contracts, all retained B54K-T/B54K-R contracts, the
independent oracle, full normal suite, and fresh ASan/UBSan pass `975/975`.
Real-disc framebuffer state is 6 VSync, 5 DrawSync, 3 presentations, mask 1;
the aggregate DMA checkpoint census remains 27/26.

Evidence: `docs/evidence/pe-b54ku-drawprim-e1/REPORT.md` and
`pc_port/tools/b54ku_drawprim_e1_oracle.py`.

```text
FUNC_80075358=EXECUTION_PROVEN_PATH_TRANSLATED
FUNC_80076B58=COUNTED_COMMAND_WORD_PATH_TRANSLATED
GP0_E1=DRAW_MODE_STATE_IMPLEMENTED
FUNC_80190660_PREFIX=130_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80075358_sprite_at_80190868
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-V — GP0(64h) textured SPRT (2026-08-30)

The second DrawPrim now traverses the exact wrapper and a generic GP0(64h)
parser/raster path. The accepted subset is opaque, modulated, variable-size
4bpp: it sign-extends/clips destination coordinates, wraps eight-bit UV,
fetches packed nibbles and the packet-selected CLUT from VRAM, preserves zero
texture-color transparency and CLUT bit 15, and applies the retail 5-bit by
8-bit modulation rule. Unsupported depth/raw forms are mutation-free fences;
drawing-area/offset/mask state is not guessed.

Retail data proves the canonical packet is a 256x64 SPRT at `{32,88}` using
tpage `{512,256}` and CLUT `{0,480}`. The authenticated overlay prefix now
covers 183 words, `[0x80190660,0x8019093C)`, including the explicit DrawSync,
canonical optional-LoadImage bypass, VSync(0), and ResetGraph(1). PutDrawEnv
`func_80075424` at `0x8019093C` is next.

Two focused contracts, retained B54K-U/T/R contracts, the independent oracle,
the full normal suite, and fresh ASan/UBSan pass `977/977`. Real-disc
framebuffer telemetry is 7 VSync, 7 DrawSync, 3 presentations, mask 1; DMA
remains 27/26.

Evidence: `docs/evidence/pe-b54kv-textured-rectangle/REPORT.md` and
`pc_port/tools/b54kv_textured_rectangle_oracle.py`.

```text
GP0_64=OPAQUE_MODULATED_4BPP_VARIABLE_RECTANGLE_IMPLEMENTED
FUNC_80190660_PREFIX=183_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80075424_from_func_80190660
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

## PE-B54K-W — PutDrawEnv and first loop back-edge (2026-08-30)

The canonical PutDrawEnv path is translated from the complete 48-word retail
wrapper. It builds the terminal six-command DR_ENV node, dispatches it through
the existing `jtb[2]/jtb[6]` identities, then caches 0x5C bytes only after the
worker returns. The generic GPU authority now represents E2h..E6h texture
window, drawing area, offset, and mask state; the GP0(64h) raster consumes
those registers. Unsupported node shapes are rejected before GP1 mutation.

The B54K-W authenticated prefix was 192 words through PutDispEnv and the first
taken branch delay slot. B54K-X below supersedes that execution frontier while
this section remains authoritative for PutDrawEnv and GP0 environment state.

Evidence: `docs/evidence/pe-b54kw-putdrawenv/REPORT.md` and
`pc_port/tools/b54kw_putdrawenv_oracle.py`.

```text
PUTDRAWENV_CANONICAL_DR_ENV=IMPLEMENTED
GP0_E2_E6_ENVIRONMENT=IMPLEMENTED
FUNC_80190660_PREFIX=192_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80190660_loop_reentry_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=translate_func_80190660_480_frame_loop
```

## PE-B54K-X — complete overlay fade loop (2026-08-30)

Overlay-local `func_80190660` is complete: all 213 authenticated words and all
480 state-driven fade/display iterations execute. The four retail intensity
segments, parity packet banks, environment toggle, optional-upload gate,
DrawPrim/DrawSync/VSync/ResetGraph/PutDrawEnv/PutDispEnv order, final
SetDispMask(0), display-byte restore, and normal return are represented.

Canonical direct cardinality is 480 rectangles, 960 E1 commands, 480 of each
E2h..E6h environment command, 1441 DrawSync calls, 480 VSync calls, and 480
presentations. Two focused contracts include a nonzero-phase negative control.
All 981 normal tests, fresh ASan/UBSan, retained R/U/V/W oracles, and the new
independent oracle pass. Real-disc telemetry is 486 VSync, 1444 DrawSync, 483
presentations, mask 0; DMA remains 27/26. The exact next instruction is the
caller's saved-bit branch at `0x80190D7C`.

Evidence: `docs/evidence/pe-b54kx-overlay-loop/REPORT.md` and
`pc_port/tools/b54kx_overlay_loop_oracle.py`.

```text
FUNC_80190660=COMPLETE_213_WORDS
OVERLAY_FADE_LOOP=480_FRAMES_STATE_DRIVEN
PRODUCTION_REACHABILITY=blocked_at_func_801909B4_80190D7C_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_saved_bit_branch_at_80190D7C
```

## PE-B54K-Y — saved-bit arm and `func_80192CE8` prefix (2026-08-30)

The caller's authenticated `D_800B0DCD & 1` branch is now represented. Disc 1
takes the nonzero arm into `func_80192CE8(1)`; zero remains the exact
`0x80191120` structural cut. The first 69 of `func_80192CE8`'s 172 words are
translated through the initial state writes, display/reset calls, retail
issue/retry/poll loop, critical-section/cache sequence, and first
overlay-local call.

The table selects PE.IMG `[0x039F,0x03C5)`: 38 sectors into retail pointer
`0x8010BCF8`, payload SHA-256
`d0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5ba8d0b40`.
The next call is `func_80191FB8(1, &0x80122D00)`; its transient stack word is
recorded by value, never as a host pointer. The zero-arm negative control
proves no read-prefix state is touched. All 983 normal and fresh ASan/UBSan
tests pass; real-disc production stops exactly at `func_80191FB8` from
`func_80192CE8`, with DMA still 27/26.

Evidence: `docs/evidence/pe-b54ky-192ce8-prefix/REPORT.md` and
`pc_port/tools/b54ky_192ce8_prefix_oracle.py`.

```text
FUNC_80192CE8_PREFIX=69_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80191FB8_from_func_80192CE8
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_func_80191FB8
```

## PE-B54K-Z — complete `func_80191FB8` (2026-08-30)

Overlay-local `func_80191FB8` is complete: all 207 authenticated words,
one/two-source pointer layouts, `0x28`/`0xB8` environment copies, state bytes,
and both generic MoveImage calls execute. The `0x08000000` gate skips only
the second move; invalid count, null-member, and busy paths are mutation-free.
The transient caller-stack pointer list is consumed by value without assigning
a host pointer any guest identity.

All 985 normal and fresh ASan/UBSan tests pass. Production now stops at
`func_801924F8` from `func_80192CE8`; framebuffer and DMA telemetry remain
486/1445/483 and 27/26. Evidence:
`docs/evidence/pe-b54kz-191fb8-complete/REPORT.md` and
`pc_port/tools/b54kz_191fb8_oracle.py`.

```text
FUNC_80191FB8=COMPLETE_207_WORDS
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_from_func_80192CE8
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_func_801924F8
```

## PE-B54K-AA — enter `func_801924F8` (2026-08-30)

`func_801924F8` is authenticated as a real 271-word function with one exact
caller and a normal return. Its first 29 words are translated: index `<47`
selects a 20-byte record, publishes its pointer, and passes signed record byte
`+4` to the first overlay-local call. Index 47 is mutation-free. Production
now stops exactly at `func_801918F8(0, kind)` from `func_801924F8`; all 985
tests remain green. Evidence:
`docs/evidence/pe-b54kaa-1924f8-prefix/REPORT.md`.

```text
FUNC_801924F8=AUTHENTICATED_271_WORDS_PREFIX_29_WORDS
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192584_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_func_801918F8
```

## PE-B54K-AB — complete `func_801918F8` (2026-08-30)

The 155-word overlay display-pair initializer is complete. Four exact callers,
normal return, both 320-wide and 480-then-folded-to-320 paths, and all SDK
environment effects are proven. `func_801924F8` now executes both calls and
reaches exact cut `0x80192584`; 985/985 tests remain green. Evidence:
`docs/evidence/pe-b54kab-1918f8-complete/REPORT.md`.

```text
FUNC_801918F8=COMPLETE_155_WORDS
FUNC_801924F8=AUTHENTICATED_271_WORDS_PREFIX_35_WORDS
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192584_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=continue_func_801924F8_at_80192584
```

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
  FORK: (i) cc1 source patch (the maspsx model one layer deeper — the 698D4 liveness check is scoped)
  or (ii) accept the six residuals as structurally-correct-C with one-word compiler-decision deltas.
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

Detail and leaf-by-leaf narrative: git history + wiki
([Current Status](https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki/Current-Status)).
PC port remains out of scope. Redump.org cross-check still open (non-blocking).
