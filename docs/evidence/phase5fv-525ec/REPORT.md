# Phase 5FV — matching C leaf `func_800525EC`

Date: 2026-09-18

```text
IMPLEMENTED=0x800525EC..0x80052634 exclusive
WORDS=18
BYTES=0x48
SOURCE=src/func_800525EC.c
PROFILE=era_o2_g0 (-O2 -G0, era cc1) — the YAML default, no per-leaf assignment
YAML=[0x42DEC, c, func_800525EC]
CARVE=mid-42D94: asm prefix 0x58, C 0x48, asm resumes 0x42E34
LINK_CHECK=LINK_EXACT (tools/analysis/era_link_check.py, word mismatches=0, pad 0)
PREFLIGHT=disc1_preflight: PASS (deep, 798 c / 349 asm / 2 rodata)
GATE=EXACT_REBUILD_GATE=PASS
GATE_SHA1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b (orig == cand)
GATE_PLAN=aaaa8873cd47da8e4c3aa5ef6cd704a59e3e175fe905b895a725fafd52353f6b
GATE_SWEEP=VERIFY_SWEEP=PASS leaves=798
PLANTED_STATE=NO
```

## What the function is

Menu sound helper. It loads `D_800B0E08` (`$gp`-relative in the source tree, but
this unit is `-G0` so the address is materialised), and when the package pointer
is non-zero calls `func_8006DF50(package, 0x44C, 0x100, 0x80, 0x7F)`. The 5th
argument goes on the stack; the 4th is materialised in the `jal` delay slot.

Retail instruction list (`asm/disc1/42D94.s` before the carve):

```text
addiu sp,sp,-0x20      lui   a0,%hi(D_800B0E08)   addiu a0,a0,%lo(...)  sw ra,0x18(sp)
lw    v0,0x0(a0)       nop                        beqz  v0,.L80052624   addiu v0,zero,0x7F
addiu a1,zero,0x44C    addiu a2,zero,0x100        sw    v0,0x10(sp)     lw a0,0x0(a0)
jal   func_8006DF50    addiu a3,zero,0x80         lw    ra,0x18(sp)     addiu sp,sp,0x20
jr    ra               nop
```

## Codegen finding (the whole difficulty of this leaf)

Retail materialises `&D_800B0E08` **once** into `$a0` and then reads through that
register **twice** (test read into `$v0`, reload into `$a0` for the call). The
natural C spellings both miss it, and the sweep is the evidence:

| source form | result (`era_link_check.py`) |
| --- | --- |
| `extern int D_800B0E08;` | 16 word mismatches — GCC folds the two reads into one load |
| `extern volatile int D_800B0E08;` | 11 mismatches — the address is materialised twice (`lui $v0` for the test, `lui $a0` for the call) |
| `volatile int *p = &D_800B0E08;` at `-O1 -G0` | 7 mismatches |
| `volatile int *p = &D_800B0E08;` at `-O2 -G0` | **0 — LINK_EXACT** |

Taking the address once into `p` keeps a single materialisation, and the
`volatile` pointee keeps both reads from being commoned. `-O2 -G0` is the era
schedule that leaves `sw ra` after the two address instructions, as retail does.

Reproduce with (no YAML edit needed — the check compiles `src/<name>.c` and links
it at the retail VMA):

```sh
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
python3 tools/analysis/era_link_check.py src/func_800525EC.c 0x800525EC 0x48 -O2 -G0
```

## Registration notes for the next leaf

Two staging rules surfaced while running the full gate; both are mechanical:

1. a new YAML `c` source must be **git-tracked** (`git add`) before
   `scripts/verify_us.sh` will accept it — the gate fails with
   `staging gap — a YAML C source is not git-tracked yet`;
2. `docs/generated/DISC1_MATCHING_STATUS.md` must be current
   (`python3 tools/build/disc1_plan.py --write-status`) — the span count in it
   moves with every carve.

`docs/generated/NATIVE_CANDIDATE_PRIORITY.md` likewise needs
`tools/progress/port_priority.py --write-status`; `func_800525EC` left the
candidate pool and the top candidate is now `func_8005267C`, its twin
(same 18-word shape, sound id `0x44D`).
