# PE-B54I — GPU primitive-builder leaves + add/sort wrappers ported (native)

## Goal: 100% Retail Accuracy to the Sewers (continuation)

Predecessor rung PE-B54G parked `func_8006AD40` at retail `0x8006B060`
with `func_80030894` (788 words, boot GPU-primitive builder) as the
blocking wall. The B54D audit listed twelve direct callees; five SET
leaves were ported by PE-GPU1 and four getters classified.

**IMPORTANT — predecessor cleanup:** an uncommitted, context-exhausted
session (self-styled "PE-B54H") left nine speculative port files, an
oracle, and two status docs claiming "9 of 10 functions PORTED". None of
it was wired into the build, none contained verified word decodes, and
several contained contradictory duplicate implementations (e.g. two
`GetTPage` variants with a wrong "a1 means Y=16" theory; a `func_80037140`
with a placeholder "(Additional ROM instructions would go here)"). All
of it was deleted and redone from the retail words. Claims in those files
were false: nothing was ported before B54I.

## What B54I ports (11 functions, all word-exact against the SHA-1 image)

| symbol | words | file offset | semantics |
| --- | --- | --- | --- |
| func_80077A64 | 15 | 0x68264 | GetTPage(tp,abr,x,y): `((tp&3)<<7)\|((abr&3)<<5)\|((y&0x100)>>4)\|((x&0x3FF)>>6)\|((y&0x200)<<2)`; retail call (0,1,256,480)→0x34 |
| func_80077AA4 | 6 | 0x682A4 | CLUT builder variant: `((y<<6)\|((x>>a4)&0x3F))&0xFFFF`; retail call (0x130,0x1F8)→0x7E13; the sra writes **$a0**, y term survives |
| func_80077B04 | 10 | 0x68304 | SetSemiTrans: code byte p+7, set/clear bit 1 |
| func_80077B34 | 10 | 0x68334 | SetShadeTex: code byte p+7, set/clear bit 0 |
| func_80077C04 | 5+1 | 0x68404 | setSprt header: p[3]=4, p[7]=0x64; **delay-slot store at 0x80077C14 is part of the function** |
| func_80077C84 | 10+1 | 0x68484 | DR draw-mode: p[3]=1; `word=(0xE1000000\|(a2?0x200:0))\|((a3&0x9FF)\|(a1?0x400:0))`; sw at p+4 lands in the jr delay slot (0x80077CAC); returns word |
| func_80077CB4 | 13 | 0x684B4 | length-budget append: `len=head[3]+tail[3]+1`; `len<17` → head[3]=len, `*(u32*)tail=0`, ret 0; else ret −1, **no stores** |
| func_8005DADC | 7+1 | 0x4E2DC | `*(u32*)0x800A8030 + 0x800A8028 + (a0<<3)`; final add is the jr delay slot (0x8005DAF8) |
| func_800370DC | 25 | 0x278DC | add/sort wrapper (sprite twin): 77C84(p,0,1,mode) → 77C04(p+8) → 77CB4(p,p+8) → fail: 719E4(−1) |
| func_80037140 | 25 | 0x27940 | add/sort wrapper (tile twin): 77C84(p,0,1,mode) → **77C44** (SetTile) → 77CB4 → fail: 719E4(−1) |
| func_800719E4 | 3 | 0x621E4 | **BIOS B(38h) CD-mode trampoline** (`li $t2,0xB0; jr; li $t1,0x38`), sibling of the ported A(28h) bzero trampoline 0x80071A24 — NOT the "large function" the dead session speculated |

Canonical wrapper effects (success path, retail mode value 0x34):

- func_800370DC: p[3]=6, `*(u32*)(p+4)=0xE1000234`, `*(u32*)(p+8)=0`, p[15]=0x64
- func_80037140: p[3]=5, same word, `*(u32*)(p+8)=0`, p[15]=0x60

## Key discoveries

1. **The "a1=1 means Y=16" GetTPage theory was wrong.** The ROM ABI is
   exactly Psy-Q GetTPage(tp, abr, x, y); the audit's first-call vector
   (0,1,256,480)→0x34 falls out of the literal shifts with no
   reinterpretation.
2. **func_800719E4 is a 3-word BIOS trampoline**, not an unfindable large
   function. Twelve executable-wide jal sites: eleven `a0=-1` fail paths
   (the 0x80037xxx wrapper family + siblings) and one `a0=1` CD mode set
   at 0x8006E758 already collapsed in pe_libcd.c with recorded
   justification. B(38h) has no guest-visible effect; all twelve callers
   discard $v0. The fail path is unreachable with retail boot data (first
   append computes len 6/5 against the 17-word budget).
3. **Three functions complete in their jr delay slots** (77C04 store at
   0x80077C14, 77C84 store at 0x80077CAC, 5DADC add at 0x8005DAF8) — the
   same family as the 5EF matching-lane delay-slot findings.
4. **func_80030894 is now 100% callee-unblocked**: every jal target is a
   real native port (this rung + PE-GPU1) or an already-translated
   dependency. The wall is now only the 788-word body itself.

## Files

```text
pc_port/game/boot/func_80077A64_port.c        # GetTPage
pc_port/game/boot/func_80077AA4_port.c        # CLUT builder
pc_port/game/boot/func_80077B04_port.c        # SetSemiTrans
pc_port/game/boot/func_80077B34_port.c        # SetShadeTex
pc_port/game/boot/func_80077C04_port.c        # setSprt header
pc_port/game/boot/func_80077C84_port.c        # DR draw-mode word
pc_port/game/boot/func_80077CB4_port.c        # length-budget append
pc_port/game/boot/func_8005DADC_port.c        # guest table lookup
pc_port/game/boot/func_800370DC_port.c        # wrapper (sprite twin)
pc_port/game/boot/func_80037140_port.c        # wrapper (tile twin)
pc_port/platform/func_800719E4_port.c         # BIOS B(38h) trampoline
pc_port/tools/b54i_gpu_leaves_oracle.py       # independent oracle
pc_port/tests/test_native.c                   # focused B54I test
```

## Gates

```text
native=580/580 (579 baseline + 1 focused B54I)
asan_ubsan=580/580 in toolbox jk2026-dev, zero sanitizer diagnostics
exe_oracles=52/52 (every exe-argument oracle in pc_port/tools)
pe_gpu1_oracle=18 checks PASS (retained)
b54i_oracle=30 checks PASS (new)
matching_exe=unchanged by construction (no src/, configs/, asm/ edits;
             SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b)
frontier=unchanged: named func_8006AD40_prefix_cut @ retail 0x8006B060,
         asserted in-suite by the retained B54G tests
bootstrap_disc_synthetic=behavior identical to HEAD (both stop at the
         pre-existing PE_LoadU32(0) FATAL in this disc-less checkout;
         verified against a clean HEAD worktree build)
real_disc=not runnable in this checkout (rom/image/ empty)
func_80030894_entered=NO (asserted in the B54I test)
```

## Reproduce

```sh
cmake --build pc_port/build -j8 --target pe-native-tests &&
  ./pc_port/build/pe-native-tests            # 580/580
toolbox run -c jk2026-dev bash -lc '
  cmake --build pc_port/build-san -j8 --target pe-native-tests &&
  ./pc_port/build-san/pe-native-tests'       # 580/580 ASan+UBSan
python3 pc_port/tools/b54i_gpu_leaves_oracle.py   # 30 checks
python3 pc_port/tools/pe_gpu1_header_leaves_oracle.py  # 18 checks
```

## Next rung

PE-B54J: translate `func_80030894` itself (788 words / 0xC50, sole
caller `func_8006AD40` @ `0x8006B0AC`). Every direct callee is now
native; the audit's seven "unresolved" entries are all ported by
GPU1/B54I. Recommend a read-only prefix audit first (establish register
flow, the 12-word stack frame at `0x80030894`, and the packet destination
regions) before any production C.
