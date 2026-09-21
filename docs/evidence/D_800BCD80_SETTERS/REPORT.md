# D_800BCD80 state-setter family — 40 new matching C leaves

**Outcome:** 40 leaves moved from `asm` to `c`, all `LINK_EXACT` at the retail
VMA, all on the boot → end-of-Day-2 route. This is the single largest batch of
this lane: the matched-C count went **695 → 735**.

All of them share one shape: store a command byte into `D_800BCD80`, store 0–4
arguments into `D_800BCD84`/`88`/`8C`/`90`, then `jal func_8008CBA8`. They differ
only in the command byte, the argument masks, and the argument count. Era rung
is the default **`-O2 -G0`** for every one of them; no maspsx patch and no
explicit `assignments` entry was needed (`profile_necessity.py` proves the
default reproduces all 40).

## The 40 leaves

| file offset | VRAM | size | cmd | argument stores |
|---|---|---|---|---|
| `0x76C64` | `0x80086464` | `0x34` | `0x10` | `84 = a0` |
| `0x76C98` | `0x80086498` | `0x34` | `0x11` | `84 = a0` |
| `0x76CCC` | `0x800864CC` | `0x2C` | `0x40` | — |
| `0x76CF8` | `0x800864F8` | `0x70` | `0x19`,`0xC0` | two calls, see below |
| `0x76D68` | `0x80086568` | `0x3C` | `0x12` | `84 = a0`, `88 = a1` |
| `0x76DA4` | `0x800865A4` | `0x64` | `0x20` | `84=a0&0x3FF`, `88=a1&0xFFFFFF`, `8C=a2&0xFF`, `90=a3&0x7F` |
| `0x76EF0` | `0x800866F0` | `0x38` | `0x30` | `84 = a0 & 0x3FF` |
| `0x76F28` | `0x80086728` | `0x48` | switch | `0x80`/`0x81`/`0x82` |
| `0x76F70` | `0x80086770` | `0x40` | `0x90` | `84 = a0 & 0xFFFFFF` |
| `0x76FB0` | `0x800867B0` | `0x34` | `0x92` | `84 = a0` |
| `0x76FE4` | `0x800867E4` | `0x48` | switch | `0x99`/`0x9B`/`0x9D` |
| `0x7702C` | `0x8008682C` | `0x48` | switch | `0x98`/`0x9A`/`0x9C` |
| `0x77074` | `0x80086874` | `0x38` | `0xA8` | `84 = a0 & 0x7F` |
| `0x770AC` | `0x800868AC` | `0x44` | `0xA9` | `84=a0&0xFF`, `88=a1&0x7F` |
| `0x770F0` | `0x800868F0` | `0x58` | `0xA0` | `84=a0&0xFFFF`, `88=a1&0xFFFFFF`, `8C=a2&0x7F` |
| `0x77148` | `0x80086948` | `0x64` | `0xA1` | `84=a0&0xFFFF`, `88=a1&0xFFFFFF`, `8C=a2&0xFF`, `90=a3&0x7F` |
| `0x771AC` | `0x800869AC` | `0x38` | `0xAA` | `84 = a0 & 0xFF` |
| `0x771E4` | `0x800869E4` | `0x44` | `0xAB` | `84=a0&0xFF`, `88=a1&0xFF` |
| `0x77228` | `0x80086A28` | `0x58` | `0xA2` | `84=a0&0x3FF`, `88=a1&0xFFFFFF`, `8C=a2&0xFF` |
| `0x77280` | `0x80086A80` | `0x64` | `0xA3` | `84=a0&0x3FF`, `88=a1&0xFFFFFF`, `8C=a2&0xFF`, `90=a3&0xFF` |
| `0x772E4` | `0x80086AE4` | `0x38` | `0xAC` | `84 = a0 & 0xFF` |
| `0x7731C` | `0x80086B1C` | `0x44` | `0xAD` | `84=a0&0xFF`, `88=a1&0xFF` |
| `0x77360` | `0x80086B60` | `0x58` | `0xA4` | `84=a0&0xFFFF`, `88=a1&0xFFFFFF`, `8C=a2&0xFF` |
| `0x773B8` | `0x80086BB8` | `0x64` | `0xA5` | `84=a0&0xFFFF`, `88=a1&0xFFFFFF`, `8C=a2&0xFF`, `90=a3&0xFF` |
| `0x7741C` | `0x80086C1C` | `0x40` | `0xC0` | `84=a1&0x7F`, `90=a0` |
| `0x7745C` | `0x80086C5C` | `0x48` | `0xC1` | `84=a1`, `88=a2&0x7F`, `90=a0` |
| `0x774A4` | `0x80086CA4` | `0x54` | `0xC2` | `84=a1`, `88=a2&0x7F`, `8C=a3&0x7F`, `90=a0` |
| `0x774F8` | `0x80086CF8` | `0x34` | `0xC8` | `84 = a0` |
| `0x7752C` | `0x80086D2C` | `0x3C` | `0xC9` | `84=a0`, `88=a1` |
| `0x77568` | `0x80086D68` | `0x44` | `0xCA` | `84=a0`, `88=a1`, `8C=a2` |
| `0x775AC` | `0x80086DAC` | `0x38` | `0xD0` | `84 = a0 & 0xFF` |
| `0x775E4` | `0x80086DE4` | `0x40` | `0xD1` | `84=a0`, `88=a1&0xFF` |
| `0x77624` | `0x80086E24` | `0x4C` | `0xD2` | `84=a0`, `88=a1&0xFF`, `8C=a2&0xFF` |
| `0x77670` | `0x80086E70` | `0x38` | `0xD4` | `84 = a0 & 0xFF` |
| `0x776A8` | `0x80086EA8` | `0x40` | `0xD5` | `84=a0`, `88=a1&0xFF` |
| `0x776E8` | `0x80086EE8` | `0x4C` | `0xD6` | `84=a0`, `88=a1&0xFF`, `8C=a2&0xFF` |
| `0x77734` | `0x80086F34` | `0x38` | `0xD8` | `84 = a0 & 0xFF` |
| `0x7776C` | `0x80086F6C` | `0x40` | `0xD9` | `84=a0`, `88=a1&0xFF` |
| `0x777AC` | `0x80086FAC` | `0x4C` | `0xDA` | `84=a0`, `88=a1&0xFF`, `8C=a2&0xFF` |
| `0x77890` | `0x80087090` | `0x50` | — | call-poll, see below |

Not matched, left `asm`: only `func_80086608` (`0x9C`, 39 insn), a guarded
variant whose callee `func_80085084` return-value test and four-way
callee-saved shuffle still leave 8 words of register-home residual.

## Also matched this session (4 more leaves)

| leaf | span | size | era / profile | shape |
|---|---|---:|---|---|
| `func_80080950` | `0x71150` | `0x48` | `-O2 -G0` (default) | guarded byte copy, count 4 |
| `func_80080998` | `0x71198` | `0x48` | `-O2 -G0` (default) | guarded byte copy, count 8 |
| `func_8005E850` | `0x4F050` | `0x34` | `-O2 -G0` (default) | `func_8006A2E8(D_800B0DB0+a0, D_800B0DB1+a1)` |
| `func_80056C14` | `0x47414` | `0x2C` | `era_o2_g0_symbol_at_temp` (patch 5) | bound-checked halfword getter, `a0 < 3` |

`func_80080950`/`80080998` are the same shape with a different loop bound, and
use the same `do { } while (i < N)` body-order lever as the setter family's
`func_80087090`. `func_80056C14` is the early-return-polarity lever again:
`if (a0 >= 3) return 0; return D_800A1E6E[a0 * 16];` gives retail's
compare→branch-over-fallthrough, and patch 5 supplies the 3-word `$at` load.

Not matched, left `asm` with a precise PARK note: `func_80084F8C` (11 words,
`D_80084F8C`) — retail keeps the `(lbu +0x46) != 0xFF` boolean in `$v0`
(the return-register home) while cc1 2.7.2 keeps it in `$v1` and adds a tail
`move $v0,$v1`; every source spelling tried (ternary, nested `if`, value local,
`!=`/`==` polarity, `-O1`) gives `$v1`.


## Durable levers found in this batch

1. **A real `switch` beats an if/else chain** for the three command-byte
   selectors (`func_8008682C`/`86728`/`867E4`). Retail emits
   `li $v0,1` / `beq $a0,$v0` / `li $v0,2` / `beq $a0,$v0` with the **default
   constant in the `j` delay slot**; every hand-written `if`/`else` form folds
   the compare chain or loses the default in the delay slot. The switch form is
   byte-exact on the first try.

2. **`int *p = &D_800BCD80;` for a two-call setter** (`func_800864F8`). Retail
   keeps the `D_800BCD80` base in the callee-saved `$s1`
   (`lui $s1,%hi` / `addiu $s1,$s1,%lo`) across the first `jal func_8008CBA8`
   and stores both command bytes through it. A bare `D_800BCD80 = …` re-loads
   the base after the call. This is the same pointer-local form the already
   matched sibling `func_80086FF8` uses. Only the *second* call's return value
   is discarded — `r = func_8008CBA8(); ... func_8008CBA8(); return r;`.

3. **Argument-store order can be inverted from the source order**
   (`func_80086CA4`): retail performs the `andi a2,a2,0x7F` /
   `andi a3,a3,0x7F` immediately after the command byte (before `sw $ra`), and
   stores the `a0` argument to `D_800BCD90` **last**, after `88`/`8C`. Writing
   `D_800BCD84 = a1; D_800BCD88 = a2&0x7F; D_800BCD8C = a3&0x7F; D_800BCD90 = a0;`
   reproduces it; any other order does not.

4. **`func_80087090` is a call-poll `do`-while**: `while (f(a0,a1) == 1)`, with
   the `1` materialized once into `$s2` before the loop body, and the `a0`
   argument re-set in the branch delay slot (`addu $a0,$s0,$zero`).

5. **The deep-size preflight caught a real gap in this batch.** The first carve
   declared `func_80086464` with span `0x76C64 → 0x76CCC` (`0x68`), but its
   compiled `.text` is `0x40`. The extra `0x28` was the real, previously
   unmatched `func_80086498` (`0x11`/`a0` setter). `disc1_preflight.py --deep`
   flagged it by name and size and the remedy line named the correct end; that
   function was then matched and carved. This is the same defect class as the
   withdrawn `func_800906B4`; the guard worked as designed.

## Verification

Per-leaf link check (all 40, at the retail VMA):

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/<leaf>.c <vram> <size> -O2 -G0
```

Every one prints `linked .text N bytes, target 0x…, word mismatches=0,
nonzero_pad=0` and `LINK_EXACT`. Example (`func_8008682C`):

```
linked .text 80 bytes, target 0x48, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

Per-leaf deep span-size check (`tools/analysis/check_leaf.sh`) passed for every
carve; the full gate outputs are recorded in `ACTIVE_HANDOFF.md`:

```
disc1_preflight: PASS (deep, 735 c / 327 asm / 2 rodata)
PUBLIC_VERIFY=PASS  matching-C count: 735 (from YAML)
EXACT_REBUILD_GATE=PASS plan=60916571… yaml=581845e1… spans=[735 c, 327 asm, 2 rodata]
  sha1_orig=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  sha1_cand=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
profile-necessity: 570/570 era leaves clean; 0 hard defect(s); 0 redundant
```

Route coverage after the batch: matched C `341/979` funcs, `5988` words;
real remaining on-path asm `562` funcs, `66291` words; non-C-matchable
unchanged at `76` funcs / `5414` words.
