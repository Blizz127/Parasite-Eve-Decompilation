# Wave-5 slice B — executed-path functions (highest-value queue)

**Worktree** `/tmp/pe-agent-w9`, branch `agent/wave5-b`, base `74d7dd41`
(parent status: 870 matching C leaves, executed-path C-share 37.18%).
**Never merged here** — parent merges after independent verification.

## Baseline gate (re-run first, before any edit)

```
bash scripts/split_us.sh                                  # host, needs splat
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-w9 && \
  mkdir -p build/asm/disc1 build/src && bash scripts/build_us.sh && bash scripts/verify_us.sh'
```

Result: `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (870 registered C leaves)`,
plan `1268 spans = 870 c + 396 asm + 2 rodata`, `VERIFY_US=PASS`.

A fresh worktree lacks the git-ignored era toolchain; `tools/era/maspsx`
was copied as a **real directory** (not a symlink, which would make
maspsx's `sys.path[0]` resolve into the main tree) and
`gcc-2.7.2-psx`, `m2c`, `m2c-venv` were symlinked. Disc 1 extracted with
`PE_IMAGE_DIR="/home/blizz/.attic-decomp/rom/image/Parasite Eve (USA) (Disc 1)"
bash scripts/extract_us.sh 1` (md5 cb095240…, sha1 452fb033…).

## Landed leaves — 870 -> 873

Final fresh `scripts/build_us.sh` + `scripts/verify_us.sh`:

```
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (873 registered C leaves)
plan 1271 spans = 873 c + 396 asm + 2 rodata
VERIFY_US=PASS
```

| function | file | size | profile | commit |
| --- | --- | --- | --- | --- |
| `func_80085644` | 0x75E44 | 0xE4 | `era_o2_g0` (default) | `eff11fb6` |
| `func_8005E038` | 0x4E838 | 0xDC | `era_o2_g0` (default) | `eff11fb6` |
| `func_8005F27C` | 0x4FA7C | 0xD8 | `era_o2_g8` (registered) | `b9c4dc6f` |

### func_80085644 — CD/stream boot (75E44)

Straight-line bring-up (`func_8007D15C`, `func_80085A04(4, D_800B6958)`,
`func_800850F4(D_8009B7FC, 0x20)`, …) followed by three ready-poll loops
over the `0xF2000002` command word, the `func_8009CDE0` event object and
`func_80072704`. Only needs `-O2 -G0`: D_8009B7FC and D_8009CDE0 sit inside
gp range but retail addresses them absolutely, so `-G8` would change the
load form. Candidate is 240 bytes vs retail 228; the trailing 12 zero bytes
are trimmed by the build. No profile registration needed.

### func_8005E038 — menu pad-state bit expander (4E838)

Pure unrolled if-chain over `D_8009D26C` producing the 14-bit menu mask.
`era -O2 -G0`. Two cc1 colours had to be reproduced:

* cc1 keeps the loaded pad in `$a0` but, at the `0x08000000` test, needs a
  second live copy in `$a1` because `$a0` is still required by the final
  `& 4` test. A plain `w = v` makes cc1 emit exactly the retail
  `addu $a1,$a0,$zero` in that branch delay slot (and use `$a1` for the
  0x01000000 / 0x02000000 tests). A no-copy source stays in `$a0` and is
  off by one word for the whole tail.
* the `w & 2` and `v & 4` AND results must land in `$v0` instead of being
  coalesced with their source register, so they are pinned through
  `register int y asm("$2")`. Without the `$v0` result home the AND writes
  `$a1`/`$a0` and 10-18 words differ.

### func_8005F27C — D_8009D12C decoder-stack push/walk/pop (4FA7C)

Registered **`era_o2_g8`** (append `func_8005F27C` to
`assignments.era_o2_g8` in `configs/USA/disc1_build_profiles.json`).
Retail homes the push cursor in `$a1`, the pop cursor in `$v1`, the two
saved words in `$v0`/`$a0`; the string walk keeps the loaded byte in `$a0`.
The three gp words are D_8009D12C (0x3BC), D_8009D124 (0x3B4) and
D_8009D128 (0x3B8); `D_800A22B0`/`D_800A2270` are the absolute stack-limit
bases. With the pins the candidate matches at 216 bytes with 8 trailing
zero bytes.

## Parked (see `docs/ai_context/parked_blockers.json`)

| function | file | closest | blocker |
| --- | --- | --- | --- |
| `func_800754E4` | 0x65CE4 | 7 words | single cc1 scheduling difference: retail materialises the call's `$a2 = 0x40` before the `a1+0x1C` read-modify-write, cc1 puts it after; `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` fixes the epilogue. |
| `func_80057D30` | 0x48530 | 12 words | needs the 3-word `sb $0,%lo(D_800C0EAC)($at)` store (`MASPSX_THREE_WORD_SYMBOL_STORE=1`) **and** the 2-word `lb $v0,%lo(D_800C0E22)($v0)` load, but the latter needs a scalar declaration while the absolute `&D_800C0E48` pointer compare needs the incomplete-array form; both cc1 shapes cannot coexist in one unit. |
| `func_8007FA2C` | 0x7022C | 11 words | block-1/block-2 register homes pin cleanly (`$v0` base, `$a0` first-loop cursor, `$v0` second-loop cursor, `$v1` counter) but the D_8009B56C base must stay in `$s0` across `func_80080B44`; pinning it shifts the immediate-store order, leaving the struct block mis-scheduled. |
| `func_8005F1A0` | 0x4F9A0 | 17 words | retail keeps the loaded byte in `$a0`, the masked value in `$v1` and the pending-escape word in `$a1`; cc1 swaps char/masked (`$v1`/`$a0`). Pinning the char to `$a0` makes the masked `andi` write `$a0` too. |
| `func_800CE78C` | 0xBEF8C | 26 words | retail saves nine words (`$s7..$s0`+`$ra`, frame 0x40); cc1's natural colouring coalesces two loop counters and emits an eight-word frame, moving every `$sp` offset by 4 and swapping the counter homes. |
| `func_80018080` | 0x8880 | 12 words | the middle `w = v` copy is scheduled into the wrong delay slot in the wrong register (`$v1` at the `j` slot instead of `$a1` at the `beqz` slot). |
| `func_8007C13C` | 0x6C93C | 28 words | cc1 keeps the saved low bits in `$s1` and rematerialises `&D_8009B295`, retail keeps `D_8009B27C`-bits in `$s2`, the string base in `$s1` and the base-1 in `$s3`. |
| `func_80076354` | 0x66B54 | 30 words | `$s0`/`$s1` swap (arg1 vs the 0x01000000 mask) plus a nop-vs-`addiu` scheduling difference at the second `sw zero`. |
| `func_800518A8` | 0x420A8 | 32 words | cc1 inserts a spurious `nop` after the `lhu 0xC($s0)`/absolute `sh` pair (`lui $at` cannot be hoisted), shifting every subsequent delay slot and the epilogue. |

Stop condition reached (well over four consecutive parks).

## Levers that worked

* **Operand/result register pins** (`register T x asm("$N")`) are the
  decisive lever when cc1's natural colouring is stable but differs from
  retail by one or two homes: `func_8005E038` (`$4`/`$5`/`$2`) and
  `func_8005F27C` (`$5`/`$3`/`$2`/`$4`).
* **A no-op local copy forces cc1's live-range split**: `w = v` reproduces
  retail's redundant `addu $a1,$a0,$zero` when the original is still live
  for a later test.
* **`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`** moves the stack restore into the
  `jr $ra` slot (took `func_800754E4` from 11 to 9 words; not sufficient
  alone).
* **`MASPSX_THREE_WORD_SYMBOL_STORE=1`** reproduces the indexed
  `sb $0,%lo(SYM)($at)` form (`func_80057D30` store side).

## Toolchain hygiene

`tools/era/maspsx` is a real copy, not a symlink (maspsx's `sys.path[0]`
must not resolve into the main tree). All trial sources live in the
git-ignored `build/trials/`; the temporary `try_leaf` debug driver was
deleted before the final commits. Only
`configs/USA/disc1.yaml`, `configs/USA/disc1_build_profiles.json`,
`docs/generated/DISC1_MATCHING_STATUS.md` and the three `src/func_*.c`
files are tracked changes.
