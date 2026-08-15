# Adjacent TXT0 unknowns

## D_800B1628 / D_800B162C

Slot-7 stream pointers. `0x8003F244` reads them:

```text
if (D_800B0CD8 & 0x40000000)
    a0 = lw D_800B162C     # 0x8003F260
else
    a0 = lw D_800B1628     # 0x8003F270
jal func_800371B0
```

USA boot ORs the language bit, so first-play uses 162C.

Exhaustive SLUS_006.62 scan (lui 0x800B + sw/sh/sb 0x1628/162C;
lui+addiu then sw 0(reg); literal data words 0x800B1628/162C):
**no writer**. Both words are 0 in the EXE.

```text
d800b1628_writer=UNKNOWN_NOT_IN_SLUS
```

Likely an overlay or a bulk copy whose destination is not an
immediate. Not answered by the font site.

## D_80091694

ROM default `10 48 30 00…` (Aya). FA draws this buffer.

Runtime writer is **`func_80052594`** (B27, 22 words). It copies
from `$a0` until `0xFF` or 8 bytes and stores the count at
`D_8009169D`.

Callers:

| PC | Parent |
|---|---|
| `0x8005D898` | `func_8005D6F4` (boot streaming dispatcher) |
| `0x8004E28C` | `func_8004DD64` |
| `0x8004E428` | `func_8004DD64` |
| `0x8004E6A8` | `func_8004DD64` |
| `0x8005C46C` | `func_8005C46C` |

```text
d80091694_runtime_writer=func_80052594
```

Not the font site. ROM Aya remains until one of those calls runs.

## Not answered here

`0x4B` glyph identity. State-2 advance icon. FB 00–03/08/09.
