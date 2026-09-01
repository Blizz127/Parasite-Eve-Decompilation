# `func_80084FC4` — exact root-counter checkpoint

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0` with the
established `MASPSX_FILL_STORE_DELAY_SLOT=1` gate. Integrated as matching-C
leaf 349 and Tier-2 probe rung 8.

## Function hood and retail span

- File `[0x757C4,0x757E4)`, VRAM `[0x80084FC4,0x80084FE4)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x80084FDC`, with the final global store as
  its live delay slot at `0x80084FE0`.
- Seven unique exact direct callers exist at `0x80082E4C`, `0x80083068`,
  `0x80083328`, `0x80083464`, `0x8008424C`, `0x800843CC`, and
  `0x80084444`.
- The preceding real `func_80084F8C` returns at file `0x757B0/0x757B4`;
  three explicit zero alignment words occupy `0x757B8..0x757C0`.
- The following real `func_80084FE4` begins immediately at file `0x757E4`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`. The preceding padding remains asm and
is not part of this body.

## Retail body

```text
757C4 80084FC4 801F023C  lui  v0,0x1F80
757C8 80084FC8 20114234  ori  v0,v0,0x1120
757CC 80084FCC 00004294  lhu  v0,0(v0)
757D0 80084FD0 0C80013C  lui  at,%hi(D_800BD02C)
757D4 80084FD4 2CD024AC  sw   a0,%lo(D_800BD02C)(at)
757D8 80084FD8 0A80013C  lui  at,%hi(D_800A76D0)
757DC 80084FDC 0800E003  jr   ra
757E0 80084FE0 D07622AC  sw   v0,%lo(D_800A76D0)(at)
```

## Screens

| Screen | Result |
|---|---|
| Callee buckets | no calls or frame |
| Stage-0 globals | `D_800A76D0` is the sampled-counter baseline read by `func_80084FE4` and later elapsed checks; `D_800BD02C` is the caller-supplied limit read beside that baseline; another proven setup path writes both together |
| Coloring pressure | sampled halfword naturally stays in `$v0`; limit stays in `$a0` |
| `$v0` liveness | `$v0` owns both the MMIO address and loaded sample, then remains live to the return-delay store |
| Address retention | no symbolic address survives an access; two transient `$at` materializations |
| MMIO | volatile 16-bit read at `0x1F801120`, the PSX root-counter 2 count register; following code also reads its mode at `0x1F801128` to account for wrap |
| `-O` signal | frame-free volatile read and ordered stores are era `-O2`; fixed address is materialized as `lui/ori` |
| Loop/back-edge | none |
| Relocations | two normalized `HI16/LO16` global pairs |

## Minimal C and flags

```c
extern unsigned int D_800BD02C;
extern unsigned int D_800A76D0;

void func_80084FC4(unsigned int limit) {
    unsigned short count = *(volatile unsigned short *)0x1F801120;

    D_800BD02C = limit;
    D_800A76D0 = count;
}
```

Compile: era cc1, `-O2 -G0`, plus the existing default-off store-delay gate.
Without the gate cc1 emits the same first seven operations followed by the
last store, `jr ra`, and `nop` (nine content words). The gate moves only that
independent final store into the return delay slot; no toolchain code changed.

## Single-leaf object and ROM comparison

```text
00000000 <func_80084FC4>:
   0: 3c021f80  lui v0,0x1f80
   4: 34421120  ori v0,v0,0x1120
   8: 94420000  lhu v0,0(v0)
   c: 3c010000  lui at,0              R_MIPS_HI16 D_800BD02C
  10: ac240000  sw  a0,0(at)          R_MIPS_LO16 D_800BD02C
  14: 3c010000  lui at,0              R_MIPS_HI16 D_800A76D0
  18: 03e00008  jr  ra
  1c: ac220000  sw  v0,0(at)          R_MIPS_LO16 D_800A76D0

ROM: 3c021f80 34421120 94420000 3c01800c ac24d02c 3c01800a 03e00008 ac2276d0
C:   3c021f80 34421120 94420000 3c01800c ac24d02c 3c01800a 03e00008 ac2276d0
RELOCS_NORMALIZED=two HI16/LO16 pairs
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0x74FB0,0x75884)` = `0x08D4`:

```text
asm prefix: 0x757C4 - 0x74FB0 = 0x0814
C leaf:     0x757E4 - 0x757C4 = 0x0020
asm resume: 0x75884 - 0x757E4 = 0x00A0
closure:    0x0814 + 0x0020 + 0x00A0 = 0x08D4
```

## Packed span and gates

```text
757B4: 00000000 = 00000000  preceding return delay
757B8: 00000000 = 00000000  alignment
757BC: 00000000 = 00000000  alignment
757C0: 00000000 = 00000000  alignment
757C4: 3c021f80 = 3c021f80  leaf 1
757C8: 34421120 = 34421120  leaf 2
757CC: 94420000 = 94420000  leaf 3
757D0: 3c01800c = 3c01800c  leaf 4
757D4: ac24d02c = ac24d02c  leaf 5
757D8: 3c01800a = 3c01800a  leaf 6
757DC: 03e00008 = 03e00008  leaf 7
757E0: ac2276d0 = ac2276d0  leaf 8 / return delay
757E4: 3c021f80 = 3c021f80  following function entry
757E8: 34421120 = 34421120  following function word 2
757EC: 94630000 = 94630000  following function word 3
757F0: 3c02800a = 3c02800a  following function word 4
PACKED_SPAN=EXACT
```

```text
Compare:  EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 349
```
