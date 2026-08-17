# PE-BTL6 — EE=13 prefix; 6CC68 is not the live wait body

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. `andi 0xFC`, `3D050`, `6698C`, `3D834`, mode 7,
and `0x55` completion were not invented.

## Live path after overlay_wait

Bit1 on `+0xE` advances `+0xEE` 0 → 11 → 12 → 13. EE=13 body
`0x8006C9F8..0x8006CC68` (156 words, SHA-256 `2f4039f7…e242`) does
**not** jal `func_8006CC68`.

```text
lw  s2, +0x158(overlay)          # package pointer
section = s2 + lw(s2+4)
walk section+0x10 directory      # same 12-byte/idB math as Writer B
  sw package+ptr → overlay+0x1C0+idB*4
sb  0, +0x10
sw  0, +0x134 / +0x138 / +0x13C
optional section+0x2C walk → +0x134
if D_8009D254 == 0:
    return sltiu(overlay_word & 2, 1)   # skips 3D050 and the clear
if D1A0 bit1: a1 = package + (lw(pkg+(section+0xC))+4 & 0x00FFFFFF)
else:         a1 = overlay+0x11C
jal 3D050(overlay+0x14, a1, overlay+0x15C, 704, …)
jal 6698C(overlay+0x14)          # 215 words, zero callees
jal 3D834(overlay+0x14, 0, 0, D_800BEA40, D_800B89F8)
andi +0xE, 0xFC; sb 0, +0xEE; return 1
```

## `func_8006CC68` is not EE=13

79 words `0x8006CC68..0x8006CDA4` (SHA-256 `262dcfc6…a74d`). TEXT has
exactly six `jal` sites, all inside `6C5BC` EE 0 / 1–7 (CD/idle).
Zero `+0xE` stores. Callees: `661A4`, `3A088`, `3AC90`, `3AF14`,
`661CC`.

## Native scope

`func_8006C5BC_ee13_prefix_cut` ports the walk / zeros / D254-D1A0 a1
select. `func_8006C5BC` at EE=13 calls that cut and still returns 1.
`3D050` (505 words, SHA-256 `50b5ff75…`), `6698C`, `3D834`, and the
clear are not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl6_ee13_oracle.py
python3 pc_port/tools/pe_btl5_overlay_wait_oracle.py
PE_TEST_FILTER=BTL ./pc_port/build/pe-native-tests
```

STOP: `func_8003D050` so the live poll can reach `andi 0xFC` without
fabrication.
