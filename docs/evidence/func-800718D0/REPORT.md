# func_800718D0 — 0x74 bytes, LINK_EXACT

Retail VRAM `0x800718D0` (file offset `0x620D0`, span size `0x74`), in
`asm/disc1/617AC.s`. Era toolchain `-O2 -G0` (default profile `era_o2_g0`).

## Retail semantics

8-byte-aligned record walker that records the returned tail pointer in `$s1`
across two `func_8007506C` calls:

```c
extern void func_8007506C(int *a0, int *a1);

int *func_800718D0(int *a0) {
    int *s0 = 0;
    int *v0;
    if (a0[1] & 8) {
        s0 = a0 + 2;
        v0 = (int *)((char *)s0 + a0[2]);
    } else {
        v0 = a0 + 2;
    }
    func_8007506C(v0 + 1, v0 + 3);
    if (s0)
        func_8007506C(s0 + 1, s0 + 3);
    return v0 + 3;
}
```

`a0[1]` is an 8-byte-granular length/flags word (`& 8` selects the extended
form); `a0[2]` is the extension offset. The second `func_8007506C` is skipped
when `s0 == 0` (i.e. the `a0[1]&8` bit is clear) — the `move $s0,$zero` in the
`beqz` delay slot seeds that.

**Load-bearing levers:**
- Keep the byte arithmetic `.a0[1] & 8` — computing `a0[2]` first and testing
  it puts the two loads in the wrong order.
- `s0` must be the live pointer whose nullness gates the second call (not a
  boolean), so cc1 keeps it in `$s0` and the return value in `$s1`.
- Return `v0 + 3` directly; introducing a local for the tail makes cc1
  re-materialize it after the guards.

## Commands and results

```
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu"
export PATH="$PWD/tools/mipsel-host/bin:$PATH"
tools/analysis/era_leaf_match.sh src/func_800718D0.c 0x800718D0 0x74 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_800718D0.c 0x800718D0 0x74 -O2 -G0
```

Object-level: `ROM .text 116 bytes  C .text 128 bytes` — the 12-byte surplus is
the assembler's trailing alignment pad; the 3 object "mismatches" are the
`j .L` and two `jal func_8007506C` relocation slots. Link-level at the retail
VMA:

```
linked .text 128 bytes, target 0x74, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Registration

- YAML carve in `configs/USA/disc1.yaml`: the `0x617AC asm` run is split into
  prefix `0x617AC asm` (0x124), `0x618D0 c func_800718D0` (0x74), resume
  `0x61944 asm`.
- No `disc1_build_profiles.json` assignment (default profile load-bearing).
- `tools/analysis/profile_necessity.py` covers it as default-exact.
