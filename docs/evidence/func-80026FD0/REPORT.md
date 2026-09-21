# func_80026FD0 — 0x28 bytes, LINK_EXACT (resolves the PARK)

Retail VRAM `0x80026FD0` (file offset `0x177D0`, span size `0x28`), in
`asm/disc1/120D8.s`. Era toolchain `-O2 -G8` (profile `era_o2_g8`).

This leaf was PARKED earlier in the session for an apparent `lb`-vs-`lbu`
frontend divergence; the park was **wrong** and is now resolved. The `lb` was
achievable all along — the bug was that the earlier draft declared the gate as
plain `char` (unsigned, so `lbu`) and the store targets through the wrong
symbols.

## Retail semantics

```c
extern signed char D_8009D2B0[16];     /* out of the -G8 window => absolute */
extern unsigned char D_8009CE68;       /* gp-relative, 0xF8($gp) */
extern signed char D_8009CE6C;         /* gp-relative, 0xFC($gp) */

void func_80026FD0(void) {
    if (D_8009D2B0[0]) {
        D_8009CE68 = 0x80;             /* -> li $v0,0x80 ; sb 0xF8($gp) */
        D_8009CE6C = -8;               /* -> li $v0,-8   ; sb 0xFC($gp) */
    }
}
```

`D_8009D2B0` loads `lui $v0,0x800A` / `lb $v0,-0x2D50($v0)` — **absolute**, so
it is outside the `-G8` small-data window; declaring it `signed char[16]` (any
size > 1) makes cc1 use the absolute base while still emitting `lb`.
`D_8009CE68`/`D_8009CE6C` are at gp `0xF8`/`0xFC`, i.e. `0x8009CD70 + 0xF8` and
`+0xFC`.

## Load-bearing typing

- The gate must be `signed char` (→ `lb`, sign-extending). `char`/`unsigned char`
  give `lbu`. Isolated probe:
  `extern signed char A; if (A) ...` → `lb`; `extern char B; if (B) ...` → `lbu`.
- The two store values have **different signedness**: `0x80` is an `unsigned
  char` store (`li $v0,0x80`), `-8` is a `signed char` store (`li $v0,-8`).
  Both give `sb`, but an `unsigned char = 0xF8` would emit `li $v0,0xF8`.
- `D_8009D2B0` must be an array (>1 byte) so the `-G8` build keeps the load
  absolute; a scalar would be gp-relative.

## Commands and results

```
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu"
export PATH="$PWD/tools/mipsel-host/bin:$PATH"
tools/analysis/era_leaf_match.sh src/func_80026FD0.c 0x80026FD0 0x28 -O2 -G8
python3 tools/analysis/era_link_check.py src/func_80026FD0.c 0x80026FD0 0x28 -O2 -G8
```

Object-level: `ROM .text 40 bytes  C .text 48 bytes` — the 8-byte surplus is the
assembler's trailing alignment pad; the 4 object "mismatches" are the
`lui`/`%lo` relocations for `D_8009D2B0` plus the two gp-relative `sb` offsets
(`a3820000` → `a38200f8`/`a38200fc` resolved at link). Link-level at the retail
VMA:

```
linked .text 48 bytes, target 0x28, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- YAML carve in `configs/USA/disc1.yaml`: split the `0x120D8` asm run into
  prefix `0x120D8 asm` (0x56F8), `0x177D0 c func_80026FD0` (0x28), resume
  `0x177F8 asm`.
- `disc1_build_profiles.json`: added `func_80026FD0` to `era_o2_g8`.
  `tools/analysis/profile_necessity.py` → 473/473 clean (default `-O2 -G0`
  does **not** reproduce it).
