# `func_80074D28` — boot-spine scene-transition trampoline

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(`LINK_EXACT`, 0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x65528,0x655C0)` = 38 words. VRAM `[0x80074D28,0x80074DC0)`.
- Boot-spine tail, immediately after matched `func_80074CB8` / parked
  `func_80074CC8`; the whole `0x80074xxx` cluster reads the `D_8009574E`
  subsystem block and dispatches through `D_80095744`/`D_80095748`.

## Semantics (retail bytes)

```text
sp -= 0x20; save s1, ra, s0
s1 = &D_8009574E; s0 = a0
if (D_8009574E[0] >= 2)
    D_80095748(0x80011870, s0)            ; debug/log vector
if (s0 == 0)
    func_80077A28(&D_8009574E + 0x6A, -1, 0x14)   ; clear 0x14 bytes
arg = (s0 == 0) ? 0x3000001 : 0x3000000
D_80095744->f(0x10)(arg)                  ; renderer slot
restore; return
```

The `s0 == 0` test is shared: its `beqz` delay slot materializes the default
`0x3000001`, and the non-zero path reloads `lui a0,0x300`.

## Levers

1. **`asm` register pins on `s0`/`s1`.** Retail saves `$s1` (the
   `D_8009574E` base) before `$s0` and uses `$s1` for the byte load; without
   pins cc1 picks `$s0` for the base, reorders the saved registers, and hoists
   the `lbu` above the frame setup. Pinning `register … asm("$16")` /
   `asm("$17")` fixes both the allocation and the frame order.
2. **Symbol, not literal, for the log argument.** Writing `(char *)0x80011870`
   makes cc1 fold the address and emit a single `lui a0,0x8001`, while retail
   emits `lui`/`addiu` — declare `extern char D_80011870` and pass `&D_80011870`.
3. **Ternary with the non-zero case first** (`s0 ? 0x3000000 : 0x3000001`)
   reproduces the shared `beqz` / delay-slot `ori a0,a0,1` / fall-through `lui`
   shape. The logically equivalent `s0 == 0 ? 0x3000001 : 0x3000000` ordering
   perturbs the delay-slot materialization and fails at link level — the
   operand order is load-bearing.
4. **`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`** (maspsx patch 3, same knob used by
   `func_800811E4`) moves the closing `addiu sp,sp,0x20` into the `jr $31` slot.

## Exact residual accounting

Best C without patch 3: `MISMATCHES=11` — all relocation fields
(`lui/addiu`/`lw` against `D_8009574E`, `D_80011870`, `D_80095748`,
`D_80095744`, `func_80077A28`) plus the swapped epilogue pair. With patch 3 the
only differences left at object level are the relocation fields
(`MISMATCHES=9`), all resolved by the defsym link below.

Rungs tried: `-O1 -G0` (30), `-O1 -G0 -fschedule-insns2` (27),
`-O2 -G0 -fno-delayed-branch` (27). Object shapes without the `asm` pins and
the symbol form bottom out at 21–36 mismatches with an unordered frame.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  tools/analysis/era_leaf_match.sh src/func_80074D28.c 0x80074D28 0x98 -O2 -G0
```

## Link-level proof

```text
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
python3 tools/analysis/era_link_check.py src/func_80074D28.c 0x80074D28 0x98 -O2 -G0
linked .text 160 bytes, target 0x98, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

All five undefined symbols (`D_8009574E`, `D_80011870`, `D_80095748`,
`D_80095744`, `func_80077A28`) resolve to the retail addresses retail actually
touches, so the remaining object-level differences are pure HI16/LO16 fields.

## Registration

- Source `src/func_80074D28.c`.
- YAML carve `[0x65528, c, func_80074D28]`, resume `asm` at `0x655C0`
  (previously the `0x654C8` asm region ran to `0x66950`).
- Profile: `era_o2_g0_fill_epilogue` (`-O2 -G0` +
  `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`).
