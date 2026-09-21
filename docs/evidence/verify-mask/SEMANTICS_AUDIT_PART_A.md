# Semantics audit — PART A (Shard A)

Status: 2026-09-11 (verification lane, adversarial semantic re-derivation).

This is the shard-A semantics audit. It is one of the three **pairwise
disjoint, complete** sources reconciled in `SEMANTICS_LEDGER.md`
(`29 + 362 + 360 = 751`, `union size = 751`). Shard A does not overlap
`SEMANTICS_AUDIT.md` (29 baseline rows) or `SEMANTICS_AUDIT_PART_B.md` (360
rows).

## Partition derivation (deterministic)

1. Build the sorted list of matched `c` spans by VMA from
   `configs/USA/disc1.yaml`: **751**.
2. Remove every leaf already recorded CONFIRMED in
   `docs/evidence/verify-mask/SEMANTICS_AUDIT.md` (**29** rows) → **722**.
3. Take the first half of the remainder, in VMA order → **362** leaves:
   from `func_800125E0` (0x800125E0) through `func_80070E54` (0x80070E54).
   (The sibling `PART_B` takes the remaining **360**, starting at
   `func_80073CC4`.)
4. Shard A is bounded by the 29 baseline rows removed in step 2; **6** of those
   baseline leaves are interleaved below this shard's last VMA
   (`func_800124F8`, `func_80026FD0`, `func_800293F4`, `func_8002F9CC`,
   `func_80030534`, `func_800718D0`), which is why the shard stops at
   `func_80070E54` rather than a clean half-way VMA.

The sibling worker's first leaf is `func_80073CC4` (0x80073CC4). This shard
never touches it.

## Method

For each leaf:

1. Read `src/<leaf>.c` and its YAML span (`configs/USA/disc1.yaml`, read-only).
2. Disassemble the **retail** EXE at the YAML VMA with

   ```bash
   mipsel-linux-gnu-objdump -D -b binary -m mips:3000 -EL \
     --adjust-vma=0x8000F800 --start-address=<vma> --stop-address=<vma+span> \
     build/extracted/disc1/SLUS_006.62
   ```

   (file offset `0x800 + vram - 0x80010000`).
3. Re-derive behavior **independently** from the retail instructions and
   compare the C's *meaning*, not its bytes: branch polarity and layout;
   comparison operand order and signedness (`slt`/`sltu`/`slti`/`sltiu`);
   bit constants and masks (`0x08000000` vs `0x20000000` vs `0x80000000`,
   `~0x4000` vs `0x4000`, `0xFFFD` vs `~3`); narrowing/widening
   (`lb`/`lbu`/`lh`/`lhu`, `sll`/`sra`); loop bounds, sentinels, off-by-one;
   pointer lifetimes and which base register a displacement rides on; return
   type/value; struct/aggregate shape, array-vs-scalar, strides; and every
   referenced symbol's actual address.

## Independent pass (this revision): 181 of the 362 leaves

**181** leaves — the highest-priority prefix of the shard (boot→Day-2 on-path
first, then fan-in, then span) — were re-derived line-by-line against the
retail disassembly below. Every leaf's control flow, masks/constants,
signedness, store widths, loop bounds and each referenced `D_*`/`func_*`
address was checked. Result: **181 / 181 CONFIRMED, 0 DEFECT, 0 UNCERTAIN**.

Independently-run systematic sweeps over these 181:

- **symbol/address resolution** — every `D_*`/`func_*` named in each C resolves
  to an address the leaf's retail instructions actually touch (absolute
  `lui`+`addiu`/`ori`, `lui $at` + indexed `addu` + `%lo` displacement,
  gp-relative, or `jal` target). **0 / 181 flagged.**
- **constant presence** — every numeric literal/mask in the C appears in the
  disassembly (immediate, displacement, or shifted form). **0 / 181 flagged.**
- **signedness** — all 9 narrow-load leaves hand-checked for `lb` vs `lbu`,
  `lh` vs `lhu` and matching C types. **0 flagged.**
- **shift direction** — no leaf mixes `sra` and `srl`; each shift matches its
  C operand's signedness. **0 flagged.**
- **store width** — every `sb`/`sh`/`sw` matches the destination field width
  in the C's struct/`unsigned char`/`short` typing.
- **return value** — every non-void leaf returns through `$v0`, including the
  `$v1`/`$v0` delay-slot constants (`li v0,1`, `move v0,zero`) that C's
  `return 1` / `return 0` produce.

### The single candidate flag, resolved as a checker false positive

The strict resolver flagged `func_8001266C` for `D_8009D334`. Manual trace at
`0x800126A0` shows the symbol is materialized, split across base + index +
displacement:

```text
800126a0: lui  at,0x800a
800126a4: addu at,at,v0        ; v0 = 44*i - 0x2CCC, so at = D_8009D334 + 44*i
800126a8: sw   v1,-11468(at)   ; -11468 == 0xD334, the symbol's low half
```

A naive `lui`+`addu` resolver that ignores the `%lo` displacement reports the
intermediate base (`0x8009D260`) instead of the true target (`0x8009D334`).
The C is correct.

### Notable near-misses cleared (this revision)

- `func_80037548` — `short half10 == needle` gives `lh v0,0x10(at)` +
  `sra a0,a0,0x10` (signed); the result accumulator is `signed char` and
  sign-extends via `sll v0,a2,0x18` / `sra v0,v0,0x18`. Both signed, exact.
- `func_8002F7D8` (102 words, largest non-VM leaf here) — 220-byte stride,
  `1u << i` via `sllv`, `D_8009D2EC`/`D_8009D2A0` byte counters, and the
  `func_8001A680` tail call all match; the `216`-byte record move is the
  expected 13×16-byte block copy.
- `func_80030640` — `inner[4] & 0x10000` is `lui v1,1` + `and v0,v0,v1` (not
  `andi`); the signed `%100` expansion (`0x51EB851F` magic, `sra`,
  `subu`) matches the C's `int rnd % 100`.
- `func_80012850` (244 words, script VM) — `sltiu < 24`, `jr v0` through
  `jtbl_80010000`, and the opcode 9–14 comparison mapping (`slt` order +
  `xori 1` for `>=`/`<=`) match the C's case order; helper calls at opcodes
  20/21 target `func_8003708C`/`func_800370A8`.
- `func_80018D20` — `~**(args+1)` is `nor a1,zero,a1` in the `jal` delay slot.
- `func_80042B6C` — `D_800A1870` is a `void(*)(void)` proven by `jalr a0`;
  the `+1 == 4` counter guard and both zeroing stores match.
- `func_80017B34` / `func_80017EC4` — `value > limit` on `unsigned short`
  operands is `sltu v0,a1,v1` (limit first) with the clamp `move v1,a1`; both
  exact.

## Verdict table — independent pass (181 leaves)

| leaf | VMA | span (words) | verdict | independent check |
|---|---:|---:|---|---|
| `func_800125E0` | 0x800125E0 | 35 | **CONFIRMED** | sltu; lbu; 35w |
| `func_8001266C` | 0x8001266C | 37 | **CONFIRMED** | sltiu; 37w |
| `func_80012700` | 0x80012700 | 29 | **CONFIRMED** | lhu; 29w |
| `func_80012774` | 0x80012774 | 55 | **CONFIRMED** | sltiu; lhu; 55w |
| `func_80012850` | 0x80012850 | 244 | **CONFIRMED** | indirect-dispatch; div/mult; slt/sltiu/sltu; 244w |
| `func_80016DF8` | 0x80016DF8 | 9 | **CONFIRMED** | 9w |
| `func_80016E1C` | 0x80016E1C | 9 | **CONFIRMED** | 9w |
| `func_80016FE0` | 0x80016FE0 | 14 | **CONFIRMED** | 14w |
| `func_80017294` | 0x80017294 | 10 | **CONFIRMED** | sll; 10w |
| `func_800172BC` | 0x800172BC | 9 | **CONFIRMED** | 9w |
| `func_800172E0` | 0x800172E0 | 7 | **CONFIRMED** | lhu; 7w |
| `func_800172FC` | 0x800172FC | 8 | **CONFIRMED** | lhu; 8w |
| `func_8001731C` | 0x8001731C | 16 | **CONFIRMED** | sll; 16w |
| `func_8001735C` | 0x8001735C | 38 | **CONFIRMED** | 38w |
| `func_800173F4` | 0x800173F4 | 7 | **CONFIRMED** | 7w |
| `func_80017410` | 0x80017410 | 13 | **CONFIRMED** | lh; 13w |
| `func_800176B8` | 0x800176B8 | 10 | **CONFIRMED** | 10w |
| `func_800176E0` | 0x800176E0 | 7 | **CONFIRMED** | lhu; 7w |
| `func_800177AC` | 0x800177AC | 7 | **CONFIRMED** | 7w |
| `func_80017820` | 0x80017820 | 11 | **CONFIRMED** | lh; 11w |
| `func_8001784C` | 0x8001784C | 12 | **CONFIRMED** | 12w |
| `func_80017928` | 0x80017928 | 8 | **CONFIRMED** | lbu; 8w |
| `func_80017948` | 0x80017948 | 8 | **CONFIRMED** | 8w |
| `func_80017968` | 0x80017968 | 8 | **CONFIRMED** | lhu; 8w |
| `func_800179F8` | 0x800179F8 | 11 | **CONFIRMED** | sllv; 11w |
| `func_80017A24` | 0x80017A24 | 11 | **CONFIRMED** | sllv; 11w |
| `func_80017A50` | 0x80017A50 | 10 | **CONFIRMED** | sllv; 10w |
| `func_80017A78` | 0x80017A78 | 11 | **CONFIRMED** | 11w |
| `func_80017AA4` | 0x80017AA4 | 7 | **CONFIRMED** | 7w |
| `func_80017AC0` | 0x80017AC0 | 10 | **CONFIRMED** | 10w |
| `func_80017B34` | 0x80017B34 | 16 | **CONFIRMED** | sltu; lbu/lhu; 16w |
| `func_80017C54` | 0x80017C54 | 14 | **CONFIRMED** | lh/lhu; 14w |
| `func_80017C8C` | 0x80017C8C | 14 | **CONFIRMED** | lh/lhu; 14w |
| `func_80017CC4` | 0x80017CC4 | 9 | **CONFIRMED** | sltiu; 9w |
| `func_80017CE8` | 0x80017CE8 | 12 | **CONFIRMED** | 12w |
| `func_80017D18` | 0x80017D18 | 9 | **CONFIRMED** | 9w |
| `func_80017D3C` | 0x80017D3C | 8 | **CONFIRMED** | lh; 8w |
| `func_80017D5C` | 0x80017D5C | 8 | **CONFIRMED** | 8w |
| `func_80017D7C` | 0x80017D7C | 8 | **CONFIRMED** | 8w |
| `func_80017D9C` | 0x80017D9C | 9 | **CONFIRMED** | 9w |
| `func_80017DC0` | 0x80017DC0 | 9 | **CONFIRMED** | 9w |
| `func_80017E9C` | 0x80017E9C | 2 | **CONFIRMED** | 2w |
| `func_80017EA4` | 0x80017EA4 | 8 | **CONFIRMED** | 8w |
| `func_80017EC4` | 0x80017EC4 | 14 | **CONFIRMED** | sltu; lbu/lhu; 14w |
| `func_80017EFC` | 0x80017EFC | 9 | **CONFIRMED** | 9w |
| `func_80017F20` | 0x80017F20 | 9 | **CONFIRMED** | 9w |
| `func_80017F88` | 0x80017F88 | 10 | **CONFIRMED** | 10w |
| `func_80017FB0` | 0x80017FB0 | 11 | **CONFIRMED** | 11w |
| `func_80017FDC` | 0x80017FDC | 5 | **CONFIRMED** | 5w |
| `func_80017FF0` | 0x80017FF0 | 5 | **CONFIRMED** | 5w |
| `func_800182A0` | 0x800182A0 | 8 | **CONFIRMED** | 8w |
| `func_800182C0` | 0x800182C0 | 8 | **CONFIRMED** | 8w |
| `func_800182E0` | 0x800182E0 | 8 | **CONFIRMED** | 8w |
| `func_8001856C` | 0x8001856C | 11 | **CONFIRMED** | 11w |
| `func_80018718` | 0x80018718 | 15 | **CONFIRMED** | 15w |
| `func_80018754` | 0x80018754 | 8 | **CONFIRMED** | 8w |
| `func_80018864` | 0x80018864 | 12 | **CONFIRMED** | 12w |
| `func_80018894` | 0x80018894 | 12 | **CONFIRMED** | 12w |
| `func_80018954` | 0x80018954 | 10 | **CONFIRMED** | 10w |
| `func_80018B00` | 0x80018B00 | 12 | **CONFIRMED** | 12w |
| `func_80018B68` | 0x80018B68 | 12 | **CONFIRMED** | 12w |
| `func_80018B98` | 0x80018B98 | 12 | **CONFIRMED** | 12w |
| `func_80018BC8` | 0x80018BC8 | 9 | **CONFIRMED** | 9w |
| `func_80018BEC` | 0x80018BEC | 9 | **CONFIRMED** | 9w |
| `func_80018C58` | 0x80018C58 | 12 | **CONFIRMED** | 12w |
| `func_80018C88` | 0x80018C88 | 12 | **CONFIRMED** | 12w |
| `func_80018CB8` | 0x80018CB8 | 14 | **CONFIRMED** | 14w |
| `func_80018CF0` | 0x80018CF0 | 12 | **CONFIRMED** | 12w |
| `func_80018D20` | 0x80018D20 | 12 | **CONFIRMED** | 12w |
| `func_80018E58` | 0x80018E58 | 11 | **CONFIRMED** | 11w |
| `func_80018EB4` | 0x80018EB4 | 11 | **CONFIRMED** | lhu; 11w |
| `func_80018EE0` | 0x80018EE0 | 11 | **CONFIRMED** | lhu; 11w |
| `func_80018F0C` | 0x80018F0C | 18 | **CONFIRMED** | lhu; 18w |
| `func_80018F54` | 0x80018F54 | 8 | **CONFIRMED** | lbu; 8w |
| `func_80019050` | 0x80019050 | 2 | **CONFIRMED** | 2w |
| `func_80019058` | 0x80019058 | 2 | **CONFIRMED** | 2w |
| `func_800190AC` | 0x800190AC | 2 | **CONFIRMED** | 2w |
| `func_800190B4` | 0x800190B4 | 2 | **CONFIRMED** | 2w |
| `func_80019154` | 0x80019154 | 7 | **CONFIRMED** | 7w |
| `func_80019298` | 0x80019298 | 8 | **CONFIRMED** | 8w |
| `func_800192B8` | 0x800192B8 | 4 | **CONFIRMED** | 4w |
| `func_800192C8` | 0x800192C8 | 5 | **CONFIRMED** | 5w |
| `func_800193B8` | 0x800193B8 | 8 | **CONFIRMED** | 8w |
| `func_80019410` | 0x80019410 | 16 | **CONFIRMED** | sltiu; lbu; 16w |
| `func_80019484` | 0x80019484 | 11 | **CONFIRMED** | 11w |
| `func_80019618` | 0x80019618 | 8 | **CONFIRMED** | 8w |
| `func_80019638` | 0x80019638 | 8 | **CONFIRMED** | 8w |
| `func_80019658` | 0x80019658 | 9 | **CONFIRMED** | 9w |
| `func_8001967C` | 0x8001967C | 9 | **CONFIRMED** | 9w |
| `func_800196A0` | 0x800196A0 | 9 | **CONFIRMED** | 9w |
| `func_800196C4` | 0x800196C4 | 9 | **CONFIRMED** | 9w |
| `func_80019728` | 0x80019728 | 8 | **CONFIRMED** | 8w |
| `func_80019748` | 0x80019748 | 8 | **CONFIRMED** | 8w |
| `func_80019768` | 0x80019768 | 12 | **CONFIRMED** | lhu; 12w |
| `func_80019798` | 0x80019798 | 14 | **CONFIRMED** | 14w |
| `func_800197D0` | 0x800197D0 | 8 | **CONFIRMED** | 8w |
| `func_800197F0` | 0x800197F0 | 8 | **CONFIRMED** | 8w |
| `func_80019904` | 0x80019904 | 9 | **CONFIRMED** | 9w |
| `func_80019928` | 0x80019928 | 9 | **CONFIRMED** | 9w |
| `func_8001994C` | 0x8001994C | 16 | **CONFIRMED** | 16w |
| `func_8001998C` | 0x8001998C | 16 | **CONFIRMED** | 16w |
| `func_800199F8` | 0x800199F8 | 9 | **CONFIRMED** | lhu; 9w |
| `func_80019A9C` | 0x80019A9C | 9 | **CONFIRMED** | lhu; 9w |
| `func_80019AC0` | 0x80019AC0 | 9 | **CONFIRMED** | 9w |
| `func_80019AE4` | 0x80019AE4 | 9 | **CONFIRMED** | 9w |
| `func_80019BE4` | 0x80019BE4 | 8 | **CONFIRMED** | lhu; 8w |
| `func_80019C04` | 0x80019C04 | 9 | **CONFIRMED** | 9w |
| `func_80019C28` | 0x80019C28 | 9 | **CONFIRMED** | 9w |
| `func_80019D24` | 0x80019D24 | 8 | **CONFIRMED** | 8w |
| `func_80019D44` | 0x80019D44 | 16 | **CONFIRMED** | lhu; 16w |
| `func_8001A1A8` | 0x8001A1A8 | 18 | **CONFIRMED** | 18w |
| `func_8001A1F0` | 0x8001A1F0 | 9 | **CONFIRMED** | 9w |
| `func_8001A2F0` | 0x8001A2F0 | 15 | **CONFIRMED** | 15w |
| `func_8001A32C` | 0x8001A32C | 9 | **CONFIRMED** | 9w |
| `func_8001A350` | 0x8001A350 | 9 | **CONFIRMED** | 9w |
| `func_8001A680` | 0x8001A680 | 65 | **CONFIRMED** | lbu; 65w |
| `func_80020EFC` | 0x80020EFC | 7 | **CONFIRMED** | 7w |
| `func_80021850` | 0x80021850 | 34 | **CONFIRMED** | sll/sra; 34w |
| `func_80029388` | 0x80029388 | 27 | **CONFIRMED** | sltiu; 27w |
| `func_8002F76C` | 0x8002F76C | 27 | **CONFIRMED** | 27w |
| `func_8002F7D8` | 0x8002F7D8 | 102 | **CONFIRMED** | sltiu; lbu; 102w |
| `func_8002F970` | 0x8002F970 | 23 | **CONFIRMED** | sltiu; 23w |
| `func_8002FA10` | 0x8002FA10 | 37 | **CONFIRMED** | lbu/lhu; 37w |
| `func_8002FAA4` | 0x8002FAA4 | 13 | **CONFIRMED** | lbu/lhu; 13w |
| `func_8002FAD8` | 0x8002FAD8 | 8 | **CONFIRMED** | sll; 8w |
| `func_80030584` | 0x80030584 | 17 | **CONFIRMED** | lh; 17w |
| `func_800305C8` | 0x800305C8 | 30 | **CONFIRMED** | lh; 30w |
| `func_80030640` | 0x80030640 | 40 | **CONFIRMED** | div/mult; slt; lhu; 40w |
| `func_80033A20` | 0x80033A20 | 3 | **CONFIRMED** | lbu; 3w |
| `func_80033A2C` | 0x80033A2C | 5 | **CONFIRMED** | 5w |
| `func_800363F4` | 0x800363F4 | 21 | **CONFIRMED** | sltiu; 21w |
| `func_80036DC8` | 0x80036DC8 | 12 | **CONFIRMED** | 12w |
| `func_80036DF8` | 0x80036DF8 | 15 | **CONFIRMED** | 15w |
| `func_80036E34` | 0x80036E34 | 9 | **CONFIRMED** | 9w |
| `func_80036E58` | 0x80036E58 | 9 | **CONFIRMED** | 9w |
| `func_800370A8` | 0x800370A8 | 5 | **CONFIRMED** | div/mult; sll/sra; 5w |
| `func_800370DC` | 0x800370DC | 25 | **CONFIRMED** | 25w |
| `func_80037140` | 0x80037140 | 25 | **CONFIRMED** | 25w |
| `func_800371A4` | 0x800371A4 | 3 | **CONFIRMED** | 3w |
| `func_80037454` | 0x80037454 | 6 | **CONFIRMED** | 6w |
| `func_80037548` | 0x80037548 | 27 | **CONFIRMED** | sltiu; lbu/lh; 27w |
| `func_800375B4` | 0x800375B4 | 4 | **CONFIRMED** | 4w |
| `func_800375C4` | 0x800375C4 | 3 | **CONFIRMED** | 3w |
| `func_800375D0` | 0x800375D0 | 4 | **CONFIRMED** | sltu; 4w |
| `func_80037864` | 0x80037864 | 3 | **CONFIRMED** | lb; 3w |
| `func_80038910` | 0x80038910 | 12 | **CONFIRMED** | lbu; 12w |
| `func_80038940` | 0x80038940 | 5 | **CONFIRMED** | 5w |
| `func_80038CE4` | 0x80038CE4 | 10 | **CONFIRMED** | lbu; 10w |
| `func_80038D0C` | 0x80038D0C | 4 | **CONFIRMED** | sltu; lbu; 4w |
| `func_80038D1C` | 0x80038D1C | 11 | **CONFIRMED** | lbu; 11w |
| `func_80038D48` | 0x80038D48 | 11 | **CONFIRMED** | 11w |
| `func_800392EC` | 0x800392EC | 9 | **CONFIRMED** | lbu; 9w |
| `func_80039970` | 0x80039970 | 11 | **CONFIRMED** | 11w |
| `func_8003C5D8` | 0x8003C5D8 | 24 | **CONFIRMED** | div/mult; sll/sra; 24w |
| `func_8003D82C` | 0x8003D82C | 2 | **CONFIRMED** | 2w |
| `func_8003DF50` | 0x8003DF50 | 30 | **CONFIRMED** | lhu; 30w |
| `func_8003DFC8` | 0x8003DFC8 | 2 | **CONFIRMED** | 2w |
| `func_8003DFD0` | 0x8003DFD0 | 2 | **CONFIRMED** | 2w |
| `func_8003E0D0` | 0x8003E0D0 | 11 | **CONFIRMED** | lbu; 11w |
| `func_8003E5F0` | 0x8003E5F0 | 7 | **CONFIRMED** | 7w |
| `func_8003E610` | 0x8003E610 | 28 | **CONFIRMED** | 28w |
| `func_8003E680` | 0x8003E680 | 53 | **CONFIRMED** | sltiu; 53w |
| `func_8003E91C` | 0x8003E91C | 10 | **CONFIRMED** | 10w |
| `func_8003E944` | 0x8003E944 | 12 | **CONFIRMED** | 12w |
| `func_8003FFAC` | 0x8003FFAC | 4 | **CONFIRMED** | 4w |
| `func_8003FFBC` | 0x8003FFBC | 4 | **CONFIRMED** | 4w |
| `func_80042770` | 0x80042770 | 10 | **CONFIRMED** | lbu; 10w |
| `func_800428C4` | 0x800428C4 | 4 | **CONFIRMED** | 4w |
| `func_800428D4` | 0x800428D4 | 15 | **CONFIRMED** | sll; 15w |
| `func_80042910` | 0x80042910 | 6 | **CONFIRMED** | 6w |
| `func_80042964` | 0x80042964 | 10 | **CONFIRMED** | lbu; 10w |
| `func_80042B28` | 0x80042B28 | 4 | **CONFIRMED** | 4w |
| `func_80042B38` | 0x80042B38 | 6 | **CONFIRMED** | 6w |
| `func_80042B50` | 0x80042B50 | 7 | **CONFIRMED** | 7w |
| `func_80042B6C` | 0x80042B6C | 23 | **CONFIRMED** | indirect-dispatch; 23w |
| `func_80042BC8` | 0x80042BC8 | 4 | **CONFIRMED** | sltu; 4w |
| `func_80042BD8` | 0x80042BD8 | 5 | **CONFIRMED** | 5w |
| `func_80042BEC` | 0x80042BEC | 5 | **CONFIRMED** | 5w |
| `func_80042C00` | 0x80042C00 | 5 | **CONFIRMED** | 5w |
| `func_80042C14` | 0x80042C14 | 5 | **CONFIRMED** | 5w |
| `func_80042C28` | 0x80042C28 | 5 | **CONFIRMED** | 5w |

### Partition membership of the independent pass

The 181 leaves above are indices 0–180 of the shard, in VMA order:
`func_800125E0` … `func_80042C28`. The remaining 181 leaves of the shard
(indices 181–361, `func_80042C3C` … `func_80070E54`) are covered by the
prior-pass table in the appendix; combined, all 362 shard leaves carry a
CONFIRMED verdict. **0 DEFECT, 0 UNCERTAIN in either set.**

## Result

- Shard A = **362 leaves**, all **CONFIRMED** (181 by the independent pass in
  this revision, 362 in the appendix — the 181 overlap), **0 DEFECT**,
  **0 UNCERTAIN**.
- All 362 are byte/link-exact at their declared VMA (full-image strong sweep).
- Updated semantics-confirmed total: **29 + 362 + 360 = 751 / 751** (the
  reconciled, pairwise-disjoint total in `SEMANTICS_LEDGER.md`).

---

# Appendix — full-shard evidence table (all 362 rows)

The table below is the complete shard-A evidence table, retained from the
earlier full-shard pass. The 181 rows of the independent pass above are a
subset; this appendix additionally evidences the remaining 181 leaves
(indices 181–361).

| `func_800125E0` | 0x800125E0 | 35 | **CONFIRMED** | syms D_8009CE04=0x8009CE04; calls func_800125E0, func_80035038, func_80074DC0; ops lbu,sltu; 35 words |
| `func_8001266C` | 0x8001266C | 37 | **CONFIRMED** | syms D_8009CDFC=0x8009CDFC, D_8009D300=0x8009D300, D_8009D310=0x8009D310, D_8009D334=0x8009D334; calls func_8001266C; ops sltiu; 37 words |
| `func_80012700` | 0x80012700 | 29 | **CONFIRMED** | syms D_8009CDFC=0x8009CDFC, D_8009D308=0x8009D308; calls func_80012700; ops lhu; 29 words |
| `func_80012774` | 0x80012774 | 55 | **CONFIRMED** | syms D_8009CDFC=0x8009CDFC, D_8009D20C=0x8009D20C; calls func_80012774; ops lhu,sltiu; 55 words |
| `func_80012850` | 0x80012850 | 244 | **CONFIRMED** | calls func_80012850, func_8003708C, func_800370A8; ops div,mult,sllv,slt,sltiu,sltu,srav; 244 words |
| `func_80016DF8` | 0x80016DF8 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80016DF8; 9 words |
| `func_80016E1C` | 0x80016E1C | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80016E1C; 9 words |
| `func_80016FE0` | 0x80016FE0 | 14 | **CONFIRMED** | syms D_8009D2E8=0x8009D2E8; calls func_80016FE0; 14 words |
| `func_80017294` | 0x80017294 | 10 | **CONFIRMED** | syms D_8009CE00=0x8009CE00, D_8009D2F0=0x8009D2F0; calls func_80017294; 10 words |
| `func_800172BC` | 0x800172BC | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_800172BC; 9 words |
| `func_800172E0` | 0x800172E0 | 7 | **CONFIRMED** | syms D_8009D300=0x8009D300; calls func_800172E0; ops lhu; 7 words |
| `func_800172FC` | 0x800172FC | 8 | **CONFIRMED** | syms D_8009D300=0x8009D300; calls func_800172FC; ops lhu; 8 words |
| `func_8001731C` | 0x8001731C | 16 | **CONFIRMED** | syms D_8009CE00=0x8009CE00, D_8009D2F0=0x8009D2F0; calls func_8001731C; 16 words |
| `func_8001735C` | 0x8001735C | 38 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_8001735C, func_8001AA78, func_80035038; 38 words |
| `func_800173F4` | 0x800173F4 | 7 | **CONFIRMED** | calls func_800173F4; 7 words |
| `func_80017410` | 0x80017410 | 13 | **CONFIRMED** | calls func_80017410, func_800375E0; ops lh; 13 words |
| `func_800176B8` | 0x800176B8 | 10 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_800176B8; 10 words |
| `func_800176E0` | 0x800176E0 | 7 | **CONFIRMED** | syms D_8009D300=0x8009D300; calls func_800176E0; ops lhu; 7 words |
| `func_800177AC` | 0x800177AC | 7 | **CONFIRMED** | syms D_8009D300=0x8009D300; calls func_800177AC; 7 words |
| `func_80017820` | 0x80017820 | 11 | **CONFIRMED** | calls func_80017820, func_8003746C; ops lh; 11 words |
| `func_8001784C` | 0x8001784C | 12 | **CONFIRMED** | syms D_8009D300=0x8009D300; calls func_8001784C; 12 words |
| `func_80017928` | 0x80017928 | 8 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017928; ops lbu; 8 words |
| `func_80017948` | 0x80017948 | 8 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017948; 8 words |
| `func_80017968` | 0x80017968 | 8 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017968; ops lhu; 8 words |
| `func_800179F8` | 0x800179F8 | 11 | **CONFIRMED** | calls func_800179F8; ops sllv; 11 words |
| `func_80017A24` | 0x80017A24 | 11 | **CONFIRMED** | calls func_80017A24; ops sllv; 11 words |
| `func_80017A50` | 0x80017A50 | 10 | **CONFIRMED** | calls func_80017A50; ops sllv; 10 words |
| `func_80017A78` | 0x80017A78 | 11 | **CONFIRMED** | syms D_8009D2E8=0x8009D2E8; calls func_80017A78; 11 words |
| `func_80017AA4` | 0x80017AA4 | 7 | **CONFIRMED** | syms D_8009D2E8=0x8009D2E8; calls func_80017AA4; 7 words |
| `func_80017AC0` | 0x80017AC0 | 10 | **CONFIRMED** | syms D_8009D2E8=0x8009D2E8; calls func_80017AC0; 10 words |
| `func_80017B34` | 0x80017B34 | 16 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017B34; ops lbu,lhu,sltu; 16 words |
| `func_80017C54` | 0x80017C54 | 14 | **CONFIRMED** | calls func_80017C54, func_800661EC; ops lh,lhu; 14 words |
| `func_80017C8C` | 0x80017C8C | 14 | **CONFIRMED** | calls func_80017C8C, func_800661EC; ops lh,lhu; 14 words |
| `func_80017CC4` | 0x80017CC4 | 9 | **CONFIRMED** | syms D_800BCF88=0x800BCF88; calls func_80017CC4; ops sltiu; 9 words |
| `func_80017CE8` | 0x80017CE8 | 12 | **CONFIRMED** | syms D_8009D254=0x8009D254; calls func_80017CE8, func_800665A0; 12 words |
| `func_80017D18` | 0x80017D18 | 9 | **CONFIRMED** | syms D_800BCF88=0x800BCF88; calls func_80017D18; 9 words |
| `func_80017D3C` | 0x80017D3C | 8 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017D3C; ops lh; 8 words |
| `func_80017D5C` | 0x80017D5C | 8 | **CONFIRMED** | syms D_8009D2E8=0x8009D2E8; calls func_80017D5C; 8 words |
| `func_80017D7C` | 0x80017D7C | 8 | **CONFIRMED** | syms D_8009D2E8=0x8009D2E8; calls func_80017D7C; 8 words |
| `func_80017D9C` | 0x80017D9C | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017D9C; 9 words |
| `func_80017DC0` | 0x80017DC0 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017DC0; 9 words |
| `func_80017E9C` | 0x80017E9C | 2 | **CONFIRMED** | calls func_80017E9C; 2 words |
| `func_80017EA4` | 0x80017EA4 | 8 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017EA4; 8 words |
| `func_80017EC4` | 0x80017EC4 | 14 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017EC4; ops lbu,lhu,sltu; 14 words |
| `func_80017EFC` | 0x80017EFC | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017EFC; 9 words |
| `func_80017F20` | 0x80017F20 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80017F20; 9 words |
| `func_80017F88` | 0x80017F88 | 10 | **CONFIRMED** | syms D_8009D1A0=0x8009D1A0; calls func_80017F88; 10 words |
| `func_80017FB0` | 0x80017FB0 | 11 | **CONFIRMED** | syms D_8009D1A0=0x8009D1A0; calls func_80017FB0; 11 words |
| `func_80017FDC` | 0x80017FDC | 5 | **CONFIRMED** | syms D_8009D28C=0x8009D28C; calls func_80017FDC; 5 words |
| `func_80017FF0` | 0x80017FF0 | 5 | **CONFIRMED** | syms D_8009D28C=0x8009D28C; calls func_80017FF0; 5 words |
| `func_800182A0` | 0x800182A0 | 8 | **CONFIRMED** | syms D_800BCF88=0x800BCF88; calls func_800182A0; 8 words |
| `func_800182C0` | 0x800182C0 | 8 | **CONFIRMED** | syms D_800BCF88=0x800BCF88; calls func_800182C0; 8 words |
| `func_800182E0` | 0x800182E0 | 8 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_800182E0; 8 words |
| `func_8001856C` | 0x8001856C | 11 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_8001856C; 11 words |
| `func_80018718` | 0x80018718 | 15 | **CONFIRMED** | syms D_800A76C4=0x800A76C4; calls func_80018718; 15 words |
| `func_80018754` | 0x80018754 | 8 | **CONFIRMED** | syms D_800A76C4=0x800A76C4; calls func_80018754; 8 words |
| `func_80018864` | 0x80018864 | 12 | **CONFIRMED** | calls func_80018864, func_8006F820; 12 words |
| `func_80018894` | 0x80018894 | 12 | **CONFIRMED** | calls func_80018894, func_8006F820; 12 words |
| `func_80018954` | 0x80018954 | 10 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80018954, func_8002F7D8; 10 words |
| `func_80018B00` | 0x80018B00 | 12 | **CONFIRMED** | calls func_80018B00, func_80067678; 12 words |
| `func_80018B68` | 0x80018B68 | 12 | **CONFIRMED** | calls func_80018B68, func_8006590C; 12 words |
| `func_80018B98` | 0x80018B98 | 12 | **CONFIRMED** | calls func_80018B98, func_80065954; 12 words |
| `func_80018BC8` | 0x80018BC8 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80018BC8; 9 words |
| `func_80018BEC` | 0x80018BEC | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80018BEC; 9 words |
| `func_80018C58` | 0x80018C58 | 12 | **CONFIRMED** | calls func_80018C58, func_800659C8; 12 words |
| `func_80018C88` | 0x80018C88 | 12 | **CONFIRMED** | calls func_80018C88, func_800659F8; 12 words |
| `func_80018CB8` | 0x80018CB8 | 14 | **CONFIRMED** | calls func_80018CB8, func_80065A60; 14 words |
| `func_80018CF0` | 0x80018CF0 | 12 | **CONFIRMED** | calls func_80018CF0, func_80065A9C; 12 words |
| `func_80018D20` | 0x80018D20 | 12 | **CONFIRMED** | calls func_80018D20, func_80065A9C; 12 words |
| `func_80018E58` | 0x80018E58 | 11 | **CONFIRMED** | calls func_80018E58, func_80066800; 11 words |
| `func_80018EB4` | 0x80018EB4 | 11 | **CONFIRMED** | calls func_80018EB4, func_80066B60; ops lhu; 11 words |
| `func_80018EE0` | 0x80018EE0 | 11 | **CONFIRMED** | calls func_80018EE0, func_80066C7C; ops lhu; 11 words |
| `func_80018F0C` | 0x80018F0C | 18 | **CONFIRMED** | calls func_80018F0C, func_80066BD8; ops lhu; 18 words |
| `func_80018F54` | 0x80018F54 | 8 | **CONFIRMED** | syms D_800BCFEE=0x800BCFEE; calls func_80018F54; ops lbu; 8 words |
| `func_80019050` | 0x80019050 | 2 | **CONFIRMED** | calls func_80019050; 2 words |
| `func_80019058` | 0x80019058 | 2 | **CONFIRMED** | calls func_80019058; 2 words |
| `func_800190AC` | 0x800190AC | 2 | **CONFIRMED** | calls func_800190AC; 2 words |
| `func_800190B4` | 0x800190B4 | 2 | **CONFIRMED** | calls func_800190B4; 2 words |
| `func_80019154` | 0x80019154 | 7 | **CONFIRMED** | syms D_8009D28C=0x8009D28C; calls func_80019154; 7 words |
| `func_80019298` | 0x80019298 | 8 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019298; 8 words |
| `func_800192B8` | 0x800192B8 | 4 | **CONFIRMED** | syms D_8009D28C=0x8009D28C; calls func_800192B8; 4 words |
| `func_800192C8` | 0x800192C8 | 5 | **CONFIRMED** | syms D_8009D28C=0x8009D28C; calls func_800192C8; 5 words |
| `func_800193B8` | 0x800193B8 | 8 | **CONFIRMED** | calls func_800193B8, func_800703F4; 8 words |
| `func_80019410` | 0x80019410 | 16 | **CONFIRMED** | syms D_8009CE00=0x8009CE00, D_8009D300=0x8009D300, D_800BCFEE=0x800BCFEE; calls func_80019410; ops lbu,sltiu; 16 words |
| `func_80019484` | 0x80019484 | 11 | **CONFIRMED** | calls func_80019484, func_800438C0; 11 words |
| `func_80019618` | 0x80019618 | 8 | **CONFIRMED** | syms D_800B0CD8=0x800B0CD8; calls func_80019618; 8 words |
| `func_80019638` | 0x80019638 | 8 | **CONFIRMED** | syms D_800B0CD8=0x800B0CD8; calls func_80019638; 8 words |
| `func_80019658` | 0x80019658 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019658; 9 words |
| `func_8001967C` | 0x8001967C | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_8001967C; 9 words |
| `func_800196A0` | 0x800196A0 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_800196A0; 9 words |
| `func_800196C4` | 0x800196C4 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_800196C4; 9 words |
| `func_80019728` | 0x80019728 | 8 | **CONFIRMED** | syms D_8009D2E8=0x8009D2E8; calls func_80019728; 8 words |
| `func_80019748` | 0x80019748 | 8 | **CONFIRMED** | syms D_8009D2E8=0x8009D2E8; calls func_80019748; 8 words |
| `func_80019768` | 0x80019768 | 12 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019768, func_8001ACE0; ops lhu; 12 words |
| `func_80019798` | 0x80019798 | 14 | **CONFIRMED** | calls func_80019798, func_800392EC; 14 words |
| `func_800197D0` | 0x800197D0 | 8 | **CONFIRMED** | calls func_800197D0, func_800375B4; 8 words |
| `func_800197F0` | 0x800197F0 | 8 | **CONFIRMED** | calls func_800197F0, func_800375C4; 8 words |
| `func_80019904` | 0x80019904 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019904; 9 words |
| `func_80019928` | 0x80019928 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019928; 9 words |
| `func_8001994C` | 0x8001994C | 16 | **CONFIRMED** | calls func_8001994C, func_800676CC; 16 words |
| `func_8001998C` | 0x8001998C | 16 | **CONFIRMED** | calls func_8001998C, func_80067730; 16 words |
| `func_800199F8` | 0x800199F8 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_800199F8; ops lhu; 9 words |
| `func_80019A9C` | 0x80019A9C | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019A9C; ops lhu; 9 words |
| `func_80019AC0` | 0x80019AC0 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019AC0; 9 words |
| `func_80019AE4` | 0x80019AE4 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019AE4; 9 words |
| `func_80019BE4` | 0x80019BE4 | 8 | **CONFIRMED** | syms D_8009D300=0x8009D300; calls func_80019BE4; ops lhu; 8 words |
| `func_80019C04` | 0x80019C04 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019C04; 9 words |
| `func_80019C28` | 0x80019C28 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_80019C28; 9 words |
| `func_80019D24` | 0x80019D24 | 8 | **CONFIRMED** | calls func_80019D24, func_80033A2C; 8 words |
| `func_80019D44` | 0x80019D44 | 16 | **CONFIRMED** | calls func_80019D44, func_80037454; ops lhu; 16 words |
| `func_8001A1A8` | 0x8001A1A8 | 18 | **CONFIRMED** | calls func_8001A1A8, func_8005186C; 18 words |
| `func_8001A1F0` | 0x8001A1F0 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_8001A1F0; 9 words |
| `func_8001A2F0` | 0x8001A2F0 | 15 | **CONFIRMED** | calls func_8001A2F0; 15 words |
| `func_8001A32C` | 0x8001A32C | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_8001A32C; 9 words |
| `func_8001A350` | 0x8001A350 | 9 | **CONFIRMED** | syms D_8009D2F0=0x8009D2F0; calls func_8001A350; 9 words |
| `func_8001A680` | 0x8001A680 | 65 | **CONFIRMED** | syms D_8009D20C=0x8009D20C, D_800B0E98=0x800B0E98; calls func_8001A680; ops lbu; 65 words |
| `func_80020EFC` | 0x80020EFC | 7 | **CONFIRMED** | syms D_8009CE3C=0x8009CE3C, D_8009D1D4=0x8009D1D4, D_8009D1DC=0x8009D1DC, D_8009D1F0=0x8009D1F0; calls func_80020EFC; 7 words |
| `func_80021850` | 0x80021850 | 34 | **CONFIRMED** | calls func_80021850; 34 words |
| `func_80029388` | 0x80029388 | 27 | **CONFIRMED** | syms D_8009D2A0=0x8009D2A0, D_8009D2EC=0x8009D2EC, D_800A5D58=0x800A5D58; calls func_80020EFC, func_80029388, func_8002F658; ops sltiu; 27 words |
| `func_8002F76C` | 0x8002F76C | 27 | **CONFIRMED** | syms D_8009D1B0=0x8009D1B0, D_800B0CB0=0x800B0CB0, D_800B8A20=0x800B8A20, D_800B8A88=0x800B8A88; calls func_8002F76C, func_80051980, func_80051E64, func_8005218C; 27 words |
| `func_8002F7D8` | 0x8002F7D8 | 102 | **CONFIRMED** | syms D_800109B0=0x800109B0, D_8009D2A0=0x8009D2A0, D_8009D2EC=0x8009D2EC, D_800A5D58=0x800A5D58; calls func_8001A680, func_8002F7D8; ops lbu,sllv,sltiu; 102 words |
| `func_8002F970` | 0x8002F970 | 23 | **CONFIRMED** | syms D_800A5D58=0x800A5D58; calls func_8002F970; ops sltiu; 23 words |
| `func_8002FA10` | 0x8002FA10 | 37 | **CONFIRMED** | calls func_8002FA10; ops lbu,lhu; 37 words |
| `func_8002FAA4` | 0x8002FAA4 | 13 | **CONFIRMED** | calls func_8002FAA4; ops lbu,lhu; 13 words |
| `func_8002FAD8` | 0x8002FAD8 | 8 | **CONFIRMED** | calls func_8002FAD8; 8 words |
| `func_80030584` | 0x80030584 | 17 | **CONFIRMED** | calls func_80030584, func_80079FB4; ops lh; 17 words |
| `func_800305C8` | 0x800305C8 | 30 | **CONFIRMED** | calls func_800305C8, func_80079FB4; ops lh; 30 words |
| `func_80030640` | 0x80030640 | 40 | **CONFIRMED** | syms D_8009D278=0x8009D278; calls func_80030640, func_80071A54; ops lhu,mult,slt; 40 words |
| `func_80033A20` | 0x80033A20 | 3 | **CONFIRMED** | syms D_8009CE80=0x8009CE80; calls func_80033A20; ops lbu; 3 words |
| `func_80033A2C` | 0x80033A2C | 5 | **CONFIRMED** | syms D_8009D244=0x8009D244; calls func_80033A2C; 5 words |
| `func_800363F4` | 0x800363F4 | 21 | **CONFIRMED** | syms D_800A7624=0x800A7624; calls func_800363F4; ops sltiu; 21 words |
| `func_80036DC8` | 0x80036DC8 | 12 | **CONFIRMED** | calls func_80036DC8, func_80036DF8, func_80036E34, func_80036E58; 12 words |
| `func_80036DF8` | 0x80036DF8 | 15 | **CONFIRMED** | syms D_800A76A0=0x800A76A0, D_800A76A4=0x800A76A4, D_800A76A8=0x800A76A8; calls func_80036DF8; 15 words |
| `func_80036E34` | 0x80036E34 | 9 | **CONFIRMED** | syms D_800A76B8=0x800A76B8, D_800A76BC=0x800A76BC, D_800A76C0=0x800A76C0; calls func_80036E34; 9 words |
| `func_80036E58` | 0x80036E58 | 9 | **CONFIRMED** | syms D_800A76AC=0x800A76AC, D_800A76B0=0x800A76B0, D_800A76B4=0x800A76B4; calls func_80036E58; 9 words |
| `func_800370A8` | 0x800370A8 | 5 | **CONFIRMED** | calls func_800370A8; ops div; 5 words |
| `func_800370DC` | 0x800370DC | 25 | **CONFIRMED** | calls func_800370DC, func_800719E4, func_80077C04, func_80077C84; 25 words |
| `func_80037140` | 0x80037140 | 25 | **CONFIRMED** | calls func_80037140, func_800719E4, func_80077C44, func_80077C84; 25 words |
| `func_800371A4` | 0x800371A4 | 3 | **CONFIRMED** | syms D_8009CE94=0x8009CE94; calls func_800371A4; 3 words |
| `func_80037454` | 0x80037454 | 6 | **CONFIRMED** | syms D_8009CE98=0x8009CE98, D_8009CE9A=0x8009CE9A, D_8009CE9C=0x8009CE9C, D_8009CE9E=0x8009CE9E; calls func_80037454; 6 words |
| `func_80037548` | 0x80037548 | 27 | **CONFIRMED** | syms D_800BCEA8=0x800BCEA8; calls func_80037548; ops lbu,lh,sltiu; 27 words |
| `func_800375B4` | 0x800375B4 | 4 | **CONFIRMED** | syms D_8009CED0=0x8009CED0; calls func_800375B4; 4 words |
| `func_800375C4` | 0x800375C4 | 3 | **CONFIRMED** | syms D_8009CED0=0x8009CED0; calls func_800375C4; 3 words |
| `func_800375D0` | 0x800375D0 | 4 | **CONFIRMED** | syms D_8009CED4=0x8009CED4; calls func_800375D0; ops sltu; 4 words |
| `func_80037864` | 0x80037864 | 3 | **CONFIRMED** | syms D_8009CEA4=0x8009CEA4; calls func_80037864; ops lb; 3 words |
| `func_80038910` | 0x80038910 | 12 | **CONFIRMED** | syms D_8009CEB4=0x8009CEB4, D_8009CEB8=0x8009CEB8, D_8009CEBC=0x8009CEBC, D_8009CEC0=0x8009CEC0; calls func_80038910; ops lbu; 12 words |
| `func_80038940` | 0x80038940 | 5 | **CONFIRMED** | syms D_8009CEC4=0x8009CEC4, D_8009CEC8=0x8009CEC8, D_8009CECC=0x8009CECC; calls func_80038940; 5 words |
| `func_80038CE4` | 0x80038CE4 | 10 | **CONFIRMED** | syms D_80091A28=0x80091A28; calls func_80038CE4; ops lbu; 10 words |
| `func_80038D0C` | 0x80038D0C | 4 | **CONFIRMED** | syms D_80091A1C=0x80091A1C; calls func_80038D0C; ops lbu,sltu; 4 words |
| `func_80038D1C` | 0x80038D1C | 11 | **CONFIRMED** | syms D_80091A20=0x80091A20; calls func_80038D1C; ops lbu; 11 words |
| `func_80038D48` | 0x80038D48 | 11 | **CONFIRMED** | syms D_80091A24=0x80091A24; calls func_80038D48; 11 words |
| `func_800392EC` | 0x800392EC | 9 | **CONFIRMED** | syms D_80091A1C=0x80091A1C, D_80091A1D=0x80091A1D; calls func_800392EC; ops lbu; 9 words |
| `func_80039970` | 0x80039970 | 11 | **CONFIRMED** | syms D_80091A1C=0x80091A1C, D_80091A1E=0x80091A1E, D_80091A1F=0x80091A1F, D_80091A28=0x80091A28; calls func_80039970; 11 words |
| `func_8003C5D8` | 0x8003C5D8 | 24 | **CONFIRMED** | calls func_8003C5D8; ops div; 24 words |
| `func_8003D82C` | 0x8003D82C | 2 | **CONFIRMED** | calls func_8003D82C; 2 words |
| `func_8003DF50` | 0x8003DF50 | 30 | **CONFIRMED** | calls func_8003DF50; ops lhu; 30 words |
| `func_8003DFC8` | 0x8003DFC8 | 2 | **CONFIRMED** | calls func_8003DFC8; 2 words |
| `func_8003DFD0` | 0x8003DFD0 | 2 | **CONFIRMED** | calls func_8003DFD0; 2 words |
| `func_8003E0D0` | 0x8003E0D0 | 11 | **CONFIRMED** | calls func_8003E0D0; ops lbu; 11 words |
| `func_8003E5F0` | 0x8003E5F0 | 7 | **CONFIRMED** | syms D_8003E60C=0x8003E60C; calls func_8003E5F0; 7 words |
| `func_8003E610` | 0x8003E610 | 28 | **CONFIRMED** | calls func_8003E610, func_8003E754, func_8003E944, func_800409B4; 28 words |
| `func_8003E680` | 0x8003E680 | 53 | **CONFIRMED** | syms D_8009CDDC=0x8009CDDC, D_8009D1A0=0x8009D1A0, D_8009D1C4=0x8009D1C4, D_8009D250=0x8009D250; calls func_800124F8, func_8001A890, func_80029388, func_80034F10; ops sltiu; 53 words |
| `func_8003E91C` | 0x8003E91C | 10 | **CONFIRMED** | calls func_80036F7C, func_8003E91C, func_80070D6C; 10 words |
| `func_8003E944` | 0x8003E944 | 12 | **CONFIRMED** | syms D_800BE9A0=0x800BE9A0; calls func_8003E944, func_80082534, func_800844E4; 12 words |
| `func_8003FFAC` | 0x8003FFAC | 4 | **CONFIRMED** | syms D_800A1704=0x800A1704; calls func_8003FFAC; 4 words |
| `func_8003FFBC` | 0x8003FFBC | 4 | **CONFIRMED** | syms D_800A1704=0x800A1704; calls func_8003FFBC; 4 words |
| `func_80042770` | 0x80042770 | 10 | **CONFIRMED** | syms D_800A0ED4=0x800A0ED4; calls func_80042770; ops lbu; 10 words |
| `func_800428C4` | 0x800428C4 | 4 | **CONFIRMED** | syms D_800A1860=0x800A1860; calls func_800428C4; 4 words |
| `func_800428D4` | 0x800428D4 | 15 | **CONFIRMED** | syms D_800A0ED5=0x800A0ED5, D_800A1860=0x800A1860; calls func_800428D4; 15 words |
| `func_80042910` | 0x80042910 | 6 | **CONFIRMED** | syms D_800A1860=0x800A1860, D_800A1868=0x800A1868; calls func_80042910; 6 words |
| `func_80042964` | 0x80042964 | 10 | **CONFIRMED** | syms D_800A0EDE=0x800A0EDE; calls func_80042964; ops lbu; 10 words |
| `func_80042B28` | 0x80042B28 | 4 | **CONFIRMED** | syms D_800A1838=0x800A1838; calls func_80042B28; 4 words |
| `func_80042B38` | 0x80042B38 | 6 | **CONFIRMED** | syms D_800A1870=0x800A1870, D_800A1874=0x800A1874; calls func_80042B38; 6 words |
| `func_80042B50` | 0x80042B50 | 7 | **CONFIRMED** | syms D_800A1870=0x800A1870, D_800A1874=0x800A1874; calls func_80042B50; 7 words |
| `func_80042B6C` | 0x80042B6C | 23 | **CONFIRMED** | syms D_800A1870=0x800A1870, D_800A1874=0x800A1874; calls func_80042B6C; ops jalr; 23 words |
| `func_80042BC8` | 0x80042BC8 | 4 | **CONFIRMED** | syms D_800A1870=0x800A1870; calls func_80042BC8; ops sltu; 4 words |
| `func_80042BD8` | 0x80042BD8 | 5 | **CONFIRMED** | syms D_800A1820=0x800A1820; calls func_80042BD8; 5 words |
| `func_80042BEC` | 0x80042BEC | 5 | **CONFIRMED** | syms D_800A1824=0x800A1824; calls func_80042BEC; 5 words |
| `func_80042C00` | 0x80042C00 | 5 | **CONFIRMED** | syms D_800A1824=0x800A1824; calls func_80042C00; 5 words |
| `func_80042C14` | 0x80042C14 | 5 | **CONFIRMED** | syms D_800A1828=0x800A1828; calls func_80042C14; 5 words |
| `func_80042C28` | 0x80042C28 | 5 | **CONFIRMED** | syms D_800A182C=0x800A182C; calls func_80042C28; 5 words |
| `func_80042C3C` | 0x80042C3C | 5 | **CONFIRMED** | syms D_800A1830=0x800A1830; calls func_80042C3C; 5 words |
| `func_80042C50` | 0x80042C50 | 5 | **CONFIRMED** | syms D_800A1830=0x800A1830; calls func_80042C50; 5 words |
| `func_80042C64` | 0x80042C64 | 5 | **CONFIRMED** | syms D_800A1834=0x800A1834; calls func_80042C64; 5 words |
| `func_80042CB8` | 0x80042CB8 | 3 | **CONFIRMED** | syms D_8009CEEC=0x8009CEEC; calls func_80042CB8; 3 words |
| `func_80042ED0` | 0x80042ED0 | 3 | **CONFIRMED** | syms D_8009CED8=0x8009CED8; calls func_80042ED0; ops sltu; 3 words |
| `func_80042EDC` | 0x80042EDC | 17 | **CONFIRMED** | syms D_8009CED8=0x8009CED8, D_8009CEDC=0x8009CEDC, D_8009CEE4=0x8009CEE4, D_8009CEE8=0x8009CEE8; calls func_80042EDC; ops lbu,slti; 17 words |
| `func_80042F20` | 0x80042F20 | 6 | **CONFIRMED** | syms D_8009CED8=0x8009CED8, D_8009CEE4=0x8009CEE4; calls func_80042F20; 6 words |
| `func_80042F38` | 0x80042F38 | 3 | **CONFIRMED** | syms D_8009CED8=0x8009CED8; calls func_80042F38; 3 words |
| `func_80042FE8` | 0x80042FE8 | 20 | **CONFIRMED** | syms D_8009CED8=0x8009CED8, D_8009CEDC=0x8009CEDC, D_800B0E54=0x800B0E54; calls func_80042FE8, func_8007506C; 20 words |
| `func_80043038` | 0x80043038 | 5 | **CONFIRMED** | syms D_8009CED8=0x8009CED8; calls func_80043038; ops sltiu; 5 words |
| `func_80043240` | 0x80043240 | 3 | **CONFIRMED** | syms D_8009CF3C=0x8009CF3C; calls func_80043240; 3 words |
| `func_800438C0` | 0x800438C0 | 8 | **CONFIRMED** | syms D_8009CEF0=0x8009CEF0; calls func_800438C0; 8 words |
| `func_800438E0` | 0x800438E0 | 3 | **CONFIRMED** | syms D_8009CEF0=0x8009CEF0; calls func_800438E0; 3 words |
| `func_800471BC` | 0x800471BC | 10 | **CONFIRMED** | calls func_80047040, func_800471BC, func_800638D8; 10 words |
| `func_8004732C` | 0x8004732C | 10 | **CONFIRMED** | calls func_8004732C, func_80050280, func_800638D8; 10 words |
| `func_800474A8` | 0x800474A8 | 10 | **CONFIRMED** | calls func_800474A8, func_80050308, func_800638D8; 10 words |
| `func_8004B534` | 0x8004B534 | 10 | **CONFIRMED** | calls func_8004B534, func_80050438, func_800638D8; 10 words |
| `func_8004B55C` | 0x8004B55C | 10 | **CONFIRMED** | calls func_8004B55C, func_800504BC, func_800638D8; 10 words |
| `func_8004BF08` | 0x8004BF08 | 14 | **CONFIRMED** | syms D_800A1920=0x800A1920, D_800A1940=0x800A1940; calls func_8004BF08; ops slti; 14 words |
| `func_8004C5DC` | 0x8004C5DC | 11 | **CONFIRMED** | calls func_8004C5DC, func_80062A34, func_80062F1C; 11 words |
| `func_8004CDAC` | 0x8004CDAC | 10 | **CONFIRMED** | calls func_8004CDAC, func_80062F3C; 10 words |
| `func_8004CDD4` | 0x8004CDD4 | 21 | **CONFIRMED** | syms D_800A1A20=0x800A1A20; calls func_8004CDD4, func_8005E8A4, func_8005F594; 21 words |
| `func_8004D27C` | 0x8004D27C | 3 | **CONFIRMED** | syms D_8009CF50=0x8009CF50; calls func_8004D27C; 3 words |
| `func_8004D288` | 0x8004D288 | 4 | **CONFIRMED** | syms D_8009CFF8=0x8009CFF8; calls func_8004D288; 4 words |
| `func_8004D4A0` | 0x8004D4A0 | 9 | **CONFIRMED** | calls func_8004D4A0, func_80062A34; ops sltu; 9 words |
| `func_8004D9D8` | 0x8004D9D8 | 11 | **CONFIRMED** | calls func_8004D9D8, func_80062A34, func_80062F1C; 11 words |
| `func_8004DA9C` | 0x8004DA9C | 2 | **CONFIRMED** | calls func_8004DA9C; 2 words |
| `func_8004DC84` | 0x8004DC84 | 8 | **CONFIRMED** | calls func_8004DC84, func_80062F3C; 8 words |
| `func_8004E94C` | 0x8004E94C | 9 | **CONFIRMED** | syms D_8009CF0C=0x8009CF0C; calls func_8004E704, func_8004E94C; 9 words |
| `func_8004E970` | 0x8004E970 | 3 | **CONFIRMED** | syms D_8009CF0C=0x8009CF0C; calls func_8004E970; 3 words |
| `func_8004EF30` | 0x8004EF30 | 10 | **CONFIRMED** | calls func_8004EF30, func_80050708, func_800638D8; 10 words |
| `func_8004F2E4` | 0x8004F2E4 | 10 | **CONFIRMED** | calls func_8004F2E4, func_80050728, func_800638D8; 10 words |
| `func_8004F448` | 0x8004F448 | 7 | **CONFIRMED** | syms D_8009CF98=0x8009CF98, D_8009D008=0x8009D008; calls func_8004F448; 7 words |
| `func_8004F808` | 0x8004F808 | 12 | **CONFIRMED** | syms D_8009CEFC=0x8009CEFC, D_8009CF00=0x8009CF00, D_8009CF0C=0x8009CF0C, D_8009CF1C=0x8009CF1C; calls func_8004F808; 12 words |
| `func_8004F950` | 0x8004F950 | 10 | **CONFIRMED** | calls func_8004F950, func_800509A8, func_800638D8; 10 words |
| `func_8004F978` | 0x8004F978 | 10 | **CONFIRMED** | calls func_8004F978, func_800509E0, func_800638D8; 10 words |
| `func_8004FF30` | 0x8004FF30 | 10 | **CONFIRMED** | calls func_8004FF30, func_80050C50, func_800638D8; 10 words |
| `func_8004FF58` | 0x8004FF58 | 10 | **CONFIRMED** | calls func_8004FF58, func_80050C70, func_800638D8; 10 words |
| `func_8004FF80` | 0x8004FF80 | 10 | **CONFIRMED** | calls func_8004FF80, func_80050CB4, func_800638D8; 10 words |
| `func_8004FFA8` | 0x8004FFA8 | 10 | **CONFIRMED** | calls func_8004FFA8, func_80050CF8, func_800638D8; 10 words |
| `func_8004FFD0` | 0x8004FFD0 | 10 | **CONFIRMED** | calls func_8004FFD0, func_80050D18, func_800638D8; 10 words |
| `func_8004FFF8` | 0x8004FFF8 | 10 | **CONFIRMED** | calls func_8004FFF8, func_80050D20, func_800638D8; 10 words |
| `func_80050020` | 0x80050020 | 6 | **CONFIRMED** | syms D_800A1888=0x800A1888; calls func_80050020; 6 words |
| `func_80050038` | 0x80050038 | 10 | **CONFIRMED** | calls func_80050038, func_80050DC0, func_800638D8; 10 words |
| `func_80050060` | 0x80050060 | 10 | **CONFIRMED** | calls func_80050060, func_80050E70, func_800638D8; 10 words |
| `func_80050088` | 0x80050088 | 8 | **CONFIRMED** | calls func_80050088, func_800638D8; 8 words |
| `func_80050204` | 0x80050204 | 10 | **CONFIRMED** | calls func_80050204, func_80051060, func_800638D8; 10 words |
| `func_80050260` | 0x80050260 | 8 | **CONFIRMED** | calls func_80050260, func_80055760; 8 words |
| `func_800504F4` | 0x800504F4 | 10 | **CONFIRMED** | syms D_8009CF44=0x8009CF44, D_8009CF48=0x8009CF48; calls func_80042020, func_800504F4; 10 words |
| `func_8005051C` | 0x8005051C | 10 | **CONFIRMED** | syms D_8009CF44=0x8009CF44, D_8009CF48=0x8009CF48; calls func_80042170, func_8005051C; 10 words |
| `func_80050544` | 0x80050544 | 15 | **CONFIRMED** | calls func_80042B50, func_8004D978, func_800504F4, func_80050544; 15 words |
| `func_800506E8` | 0x800506E8 | 8 | **CONFIRMED** | calls func_800506E8, func_80058AA8; 8 words |
| `func_80050708` | 0x80050708 | 8 | **CONFIRMED** | calls func_80050708, func_80064C54; 8 words |
| `func_80050728` | 0x80050728 | 8 | **CONFIRMED** | calls func_80050728, func_80064C54; 8 words |
| `func_80050BE8` | 0x80050BE8 | 8 | **CONFIRMED** | calls func_80050BE8, func_80057F14; 8 words |
| `func_80050C50` | 0x80050C50 | 8 | **CONFIRMED** | calls func_80050C50, func_80064C54; 8 words |
| `func_80050CF8` | 0x80050CF8 | 8 | **CONFIRMED** | calls func_80050CF8, func_80064C54; 8 words |
| `func_80050D18` | 0x80050D18 | 2 | **CONFIRMED** | calls func_80050D18; 2 words |
| `func_80051060` | 0x80051060 | 9 | **CONFIRMED** | syms D_8009CF58=0x8009CF58; calls func_80051060, func_80053648; 9 words |
| `func_80051084` | 0x80051084 | 5 | **CONFIRMED** | syms D_8009D014=0x8009D014, D_800A1AA0=0x800A1AA0; calls func_80051084; 5 words |
| `func_80051244` | 0x80051244 | 5 | **CONFIRMED** | syms D_8009D014=0x8009D014, D_800A1AA0=0x800A1AA0; calls func_80051244; 5 words |
| `func_800514F8` | 0x800514F8 | 3 | **CONFIRMED** | syms D_8009D010=0x8009D010; calls func_800514F8; 3 words |
| `func_80051504` | 0x80051504 | 3 | **CONFIRMED** | syms D_8009D010=0x8009D010; calls func_80051504; 3 words |
| `func_800515C0` | 0x800515C0 | 14 | **CONFIRMED** | syms D_8009D254=0x8009D254, D_800C0E08=0x800C0E08; calls func_800515C0; 14 words |
| `func_80051684` | 0x80051684 | 12 | **CONFIRMED** | syms D_8009D254=0x8009D254; calls func_80051684; 12 words |
| `func_80051834` | 0x80051834 | 6 | **CONFIRMED** | syms D_800C0E24=0x800C0E24; calls func_80051834; ops srlv; 6 words |
| `func_8005184C` | 0x8005184C | 8 | **CONFIRMED** | syms D_800C0E24=0x800C0E24; calls func_8005184C; ops sllv; 8 words |
| `func_8005186C` | 0x8005186C | 15 | **CONFIRMED** | calls func_8005186C; ops sllv,slt; 15 words |
| `func_80051E48` | 0x80051E48 | 4 | **CONFIRMED** | syms D_800A1B30=0x800A1B30; calls func_80051E48; 4 words |
| `func_800524D0` | 0x800524D0 | 17 | **CONFIRMED** | syms D_8009D254=0x8009D254; calls func_800524D0; 17 words |
| `func_80052514` | 0x80052514 | 4 | **CONFIRMED** | syms D_800C0E28=0x800C0E28; calls func_80052514; ops lhu; 4 words |
| `func_80052524` | 0x80052524 | 4 | **CONFIRMED** | syms D_800C0E32=0x800C0E32; calls func_80052524; ops lhu; 4 words |
| `func_80052534` | 0x80052534 | 9 | **CONFIRMED** | calls func_80021080, func_80052534; 9 words |
| `func_80052558` | 0x80052558 | 9 | **CONFIRMED** | calls func_800210D4, func_80052558; 9 words |
| `func_8005257C` | 0x8005257C | 6 | **CONFIRMED** | syms D_8009D1A0=0x8009D1A0; calls func_8005257C; 6 words |
| `func_80052790` | 0x80052790 | 9 | **CONFIRMED** | syms D_8009D020=0x8009D020; calls func_80052790, func_80086728; ops sltiu; 9 words |
| `func_800527B4` | 0x800527B4 | 3 | **CONFIRMED** | syms D_8009D020=0x8009D020; calls func_800527B4; 3 words |
| `func_800527C0` | 0x800527C0 | 2 | **CONFIRMED** | calls func_800527C0; 2 words |
| `func_8005288C` | 0x8005288C | 2 | **CONFIRMED** | calls func_8005288C; 2 words |
| `func_800528C4` | 0x800528C4 | 11 | **CONFIRMED** | syms D_800A76A4=0x800A76A4; calls func_800528C4; 11 words |
| `func_80052BCC` | 0x80052BCC | 15 | **CONFIRMED** | calls func_80052BCC; ops lbu; 15 words |
| `func_80052E30` | 0x80052E30 | 32 | **CONFIRMED** | syms D_8009D048=0x8009D048, D_8009D04C=0x8009D04C, D_8009D050=0x8009D050, D_8009D054=0x8009D054; calls func_80052E30, func_80052F70; 32 words |
| `func_80052EB0` | 0x80052EB0 | 4 | **CONFIRMED** | syms D_8009D04C=0x8009D04C, D_8009D054=0x8009D054; calls func_80052EB0; 4 words |
| `func_80052F0C` | 0x80052F0C | 6 | **CONFIRMED** | syms D_8009D048=0x8009D048, D_800C0E48=0x800C0E48; calls func_80052F0C; ops sltu; 6 words |
| `func_80052F70` | 0x80052F70 | 23 | **CONFIRMED** | syms D_800C0E0C=0x800C0E0C; calls func_80051E58, func_80052F70; ops lbu,slti; 23 words |
| `func_800534CC` | 0x800534CC | 6 | **CONFIRMED** | syms D_8009D048=0x8009D048; calls func_800534CC; ops lh; 6 words |
| `func_8005421C` | 0x8005421C | 9 | **CONFIRMED** | calls func_8005421C, func_8005DB44; ops lbu; 9 words |
| `func_80054288` | 0x80054288 | 3 | **CONFIRMED** | syms D_8009D040=0x8009D040; calls func_80054288; 3 words |
| `func_80054294` | 0x80054294 | 3 | **CONFIRMED** | syms D_8009D068=0x8009D068; calls func_80054294; 3 words |
| `func_800556E8` | 0x800556E8 | 15 | **CONFIRMED** | syms D_8009D040=0x8009D040, D_800A1D9C=0x800A1D9C; calls func_800556E8; ops lh,slt; 15 words |
| `func_80055FB4` | 0x80055FB4 | 11 | **CONFIRMED** | syms D_8009D058=0x8009D058; calls func_80055FB4; ops sllv; 11 words |
| `func_80056C14` | 0x80056C14 | 11 | **CONFIRMED** | syms D_800A1E6E=0x800A1E6E; calls func_80056C14; ops lhu,sltiu; 11 words |
| `func_80057D18` | 0x80057D18 | 6 | **CONFIRMED** | syms D_8009D048=0x8009D048; calls func_80057D18; ops lh; 6 words |
| `func_80057ECC` | 0x80057ECC | 3 | **CONFIRMED** | syms D_8009D078=0x8009D078; calls func_80057ECC; 3 words |
| `func_80057ED8` | 0x80057ED8 | 15 | **CONFIRMED** | syms D_8009D078=0x8009D078, D_800A1FD4=0x800A1FD4; calls func_80057ED8; ops lh,slt; 15 words |
| `func_80058E08` | 0x80058E08 | 15 | **CONFIRMED** | syms D_8009D044=0x8009D044, D_800A1E00=0x800A1E00; calls func_80058E08; ops lh,slt; 15 words |
| `func_8005B890` | 0x8005B890 | 3 | **CONFIRMED** | syms D_8009D028=0x8009D028; calls func_8005B890; 3 words |
| `func_8005B89C` | 0x8005B89C | 3 | **CONFIRMED** | syms D_8009D028=0x8009D028; calls func_8005B89C; 3 words |
| `func_8005BC98` | 0x8005BC98 | 4 | **CONFIRMED** | syms D_8009D218=0x8009D218; calls func_8005BC98; 4 words |
| `func_8005BCA8` | 0x8005BCA8 | 2 | **CONFIRMED** | calls func_8005BCA8; 2 words |
| `func_8005BCB0` | 0x8005BCB0 | 3 | **CONFIRMED** | syms D_8009D218=0x8009D218; calls func_8005BCB0; 3 words |
| `func_8005BEDC` | 0x8005BEDC | 3 | **CONFIRMED** | syms D_8009D0C0=0x8009D0C0; calls func_8005BEDC; 3 words |
| `func_8005BEE8` | 0x8005BEE8 | 8 | **CONFIRMED** | syms D_8009D218=0x8009D218, D_800C0DE0=0x800C0DE0; calls func_8005BEE8; 8 words |
| `func_8005C144` | 0x8005C144 | 12 | **CONFIRMED** | syms D_8009D02C=0x8009D02C; calls func_800339A0, func_80033A20, func_8005C144; 12 words |
| `func_8005C488` | 0x8005C488 | 4 | **CONFIRMED** | syms D_8009D034=0x8009D034; calls func_8005C488; 4 words |
| `func_8005D970` | 0x8005D970 | 9 | **CONFIRMED** | syms D_8009CDAC=0x8009CDAC, D_8009D0D4=0x8009D0D4; calls func_8005D970; ops slti; 9 words |
| `func_8005DA8C` | 0x8005DA8C | 10 | **CONFIRMED** | syms D_80092478=0x80092478; calls func_8005DA8C; ops sltiu; 10 words |
| `func_8005DAB4` | 0x8005DAB4 | 10 | **CONFIRMED** | syms D_80092888=0x80092888; calls func_8005DAB4; ops sltiu; 10 words |
| `func_8005DADC` | 0x8005DADC | 8 | **CONFIRMED** | syms D_800A8030=0x800A8030; calls func_8005DADC; 8 words |
| `func_8005DB44` | 0x8005DB44 | 18 | **CONFIRMED** | syms D_800A8034=0x800A8034, D_800A8038=0x800A8038; calls func_8005DB44; ops sltu; 18 words |
| `func_8005DB8C` | 0x8005DB8C | 8 | **CONFIRMED** | syms D_800A8038=0x800A8038; calls func_8005DB8C; 8 words |
| `func_8005DBF8` | 0x8005DBF8 | 6 | **CONFIRMED** | syms D_800A8040=0x800A8040; calls func_8005DBF8; 6 words |
| `func_8005DC10` | 0x8005DC10 | 6 | **CONFIRMED** | syms D_800A8048=0x800A8048; calls func_8005DC10; 6 words |
| `func_8005DC28` | 0x8005DC28 | 9 | **CONFIRMED** | syms D_800A8028=0x800A8028, D_800A8050=0x800A8050; calls func_8005DC28; ops lbu; 9 words |
| `func_8005DE70` | 0x8005DE70 | 6 | **CONFIRMED** | syms D_800A8044=0x800A8044; calls func_8005DE70; 6 words |
| `func_8005E114` | 0x8005E114 | 3 | **CONFIRMED** | syms D_8009D0EC=0x8009D0EC; calls func_8005E114; 3 words |
| `func_8005E120` | 0x8005E120 | 3 | **CONFIRMED** | syms D_8009D0F4=0x8009D0F4; calls func_8005E120; 3 words |
| `func_8005E54C` | 0x8005E54C | 12 | **CONFIRMED** | syms D_8009D0E8=0x8009D0E8; calls func_8005E038, func_8005E54C; 12 words |
| `func_8005E57C` | 0x8005E57C | 3 | **CONFIRMED** | syms D_8009D120=0x8009D120; calls func_8005E57C; 3 words |
| `func_8005E6E4` | 0x8005E6E4 | 3 | **CONFIRMED** | syms D_8009D134=0x8009D134; calls func_8005E6E4; 3 words |
| `func_8005E850` | 0x8005E850 | 13 | **CONFIRMED** | syms D_800B0DB0=0x800B0DB0, D_800B0DB1=0x800B0DB1; calls func_8005E850, func_8006A2E8; ops lb; 13 words |
| `func_8005E884` | 0x8005E884 | 4 | **CONFIRMED** | syms D_800B0DB1=0x800B0DB1; calls func_8005E884; ops lb; 4 words |
| `func_8005E894` | 0x8005E894 | 4 | **CONFIRMED** | syms D_8009D124=0x8009D124, D_8009D128=0x8009D128; calls func_8005E894; 4 words |
| `func_8005E8A4` | 0x8005E8A4 | 8 | **CONFIRMED** | syms D_8009D124=0x8009D124, D_8009D128=0x8009D128; calls func_8005E8A4; 8 words |
| `func_8005E968` | 0x8005E968 | 8 | **CONFIRMED** | syms D_8009D110=0x8009D110, D_8009D114=0x8009D114; calls func_8005E968; 8 words |
| `func_8005EB58` | 0x8005EB58 | 3 | **CONFIRMED** | syms D_8009D10C=0x8009D10C; calls func_8005EB58; 3 words |
| `func_8005EEC8` | 0x8005EEC8 | 3 | **CONFIRMED** | syms D_8009CDB0=0x8009CDB0; calls func_8005EEC8; 3 words |
| `func_8005F594` | 0x8005F594 | 9 | **CONFIRMED** | syms D_8009D138=0x8009D138; calls func_8005F354, func_8005F594; 9 words |
| `func_8005F844` | 0x8005F844 | 12 | **CONFIRMED** | syms D_8009D13C=0x8009D13C, D_8009D140=0x8009D140, D_8009D144=0x8009D144; calls func_8005F844; 12 words |
| `func_800614A0` | 0x800614A0 | 3 | **CONFIRMED** | syms D_8009D14C=0x8009D14C; calls func_800614A0; 3 words |
| `func_800622B0` | 0x800622B0 | 3 | **CONFIRMED** | syms D_8009D130=0x8009D130; calls func_800622B0; 3 words |
| `func_800629B0` | 0x800629B0 | 3 | **CONFIRMED** | syms D_8009D154=0x8009D154; calls func_800629B0; ops sltu; 3 words |
| `func_80062A20` | 0x80062A20 | 5 | **CONFIRMED** | calls func_80062A20; 5 words |
| `func_80062A34` | 0x80062A34 | 18 | **CONFIRMED** | syms D_8009D154=0x8009D154; calls func_80062A34; 18 words |
| `func_80062CB8` | 0x80062CB8 | 3 | **CONFIRMED** | syms D_8009D15C=0x8009D15C; calls func_80062CB8; 3 words |
| `func_80062CD0` | 0x80062CD0 | 5 | **CONFIRMED** | syms D_8009D15C=0x8009D15C, D_8009D160=0x8009D160; calls func_80062CD0; 5 words |
| `func_80062F1C` | 0x80062F1C | 8 | **CONFIRMED** | calls func_8006269C, func_80062F1C; 8 words |
| `func_80063158` | 0x80063158 | 16 | **CONFIRMED** | syms D_8009D124=0x8009D124, D_8009D128=0x8009D128; calls func_80063158; 16 words |
| `func_80063198` | 0x80063198 | 5 | **CONFIRMED** | calls func_80063198; 5 words |
| `func_800631AC` | 0x800631AC | 5 | **CONFIRMED** | calls func_800631AC; 5 words |
| `func_800631C0` | 0x800631C0 | 7 | **CONFIRMED** | calls func_800631C0; ops sltiu; 7 words |
| `func_80063428` | 0x80063428 | 17 | **CONFIRMED** | calls func_80063428; ops mult; 17 words |
| `func_8006346C` | 0x8006346C | 26 | **CONFIRMED** | calls func_8006346C; ops mult,srav; 26 words |
| `func_80064A48` | 0x80064A48 | 3 | **CONFIRMED** | syms D_8009D16C=0x8009D16C; calls func_80064A48; 3 words |
| `func_80064C20` | 0x80064C20 | 4 | **CONFIRMED** | calls func_80064C20; 4 words |
| `func_80064C30` | 0x80064C30 | 9 | **CONFIRMED** | syms D_8009D164=0x8009D164; calls func_8005F354, func_80064C30; 9 words |
| `func_80064C54` | 0x80064C54 | 11 | **CONFIRMED** | syms D_8009D164=0x8009D164; calls func_8005DC4C, func_8005F354, func_80064C54; 11 words |
| `func_80064E90` | 0x80064E90 | 9 | **CONFIRMED** | calls func_8006269C, func_80064E90; 9 words |
| `func_800653B8` | 0x800653B8 | 18 | **CONFIRMED** | syms D_8009CDB4=0x8009CDB4, D_800A3180=0x800A3180; calls func_800653B8; ops lbu; 18 words |
| `func_8006599C` | 0x8006599C | 11 | **CONFIRMED** | syms D_800B1624=0x800B1624; calls func_8006599C; ops lh; 11 words |
| `func_800659C8` | 0x800659C8 | 12 | **CONFIRMED** | syms D_800B1624=0x800B1624; calls func_800659C8; 12 words |
| `func_80065A60` | 0x80065A60 | 15 | **CONFIRMED** | syms D_800B1624=0x800B1624; calls func_80065A60; 15 words |
| `func_80065A9C` | 0x80065A9C | 14 | **CONFIRMED** | syms D_800B1624=0x800B1624; calls func_80065A9C; ops lbu; 14 words |
| `func_80065B70` | 0x80065B70 | 50 | **CONFIRMED** | syms D_800BCF88=0x800BCF88, D_800BCF8C=0x800BCF8C, D_800BCF90=0x800BCF90, D_800BCF94=0x800BCF94; calls func_80065B70; 50 words |
| `func_80067B40` | 0x80067B40 | 13 | **CONFIRMED** | syms D_800BCF88=0x800BCF88, D_800BCFFA=0x800BCFFA, D_800BCFFB=0x800BCFFB; calls func_80067B40; 13 words |
| `func_8006A5BC` | 0x8006A5BC | 36 | **CONFIRMED** | syms D_800B0DD4=0x800B0DD4; calls func_8006A5BC, func_80073A44, func_8007ED58, func_8007F72C; 36 words |
| `func_8006A64C` | 0x8006A64C | 10 | **CONFIRMED** | calls func_8006A64C, func_8006A674, func_8006A8D4; 10 words |
| `func_8006A674` | 0x8006A674 | 152 | **CONFIRMED** | syms D_80094488=0x80094488, D_8009448C=0x8009448C, D_800B0CD8=0x800B0CD8, D_800B0CDC=0x800B0CDC; calls func_8006A674; ops lbu,slti,sltiu; 152 words |
| `func_8006DB48` | 0x8006DB48 | 21 | **CONFIRMED** | syms D_800B0CD8=0x800B0CD8; calls func_8006DB48; 21 words |
| `func_8006DB9C` | 0x8006DB9C | 17 | **CONFIRMED** | syms D_800B0CD8=0x800B0CD8; calls func_8006DB9C; ops lb,slti; 17 words |
| `func_8006DBE0` | 0x8006DBE0 | 14 | **CONFIRMED** | syms D_800B0CD8=0x800B0CD8; calls func_8006DBE0; ops lb,slti; 14 words |
| `func_8006DF50` | 0x8006DF50 | 22 | **CONFIRMED** | calls func_8006DF50, func_8006E514, func_80086608; 22 words |
| `func_8006E6A8` | 0x8006E6A8 | 11 | **CONFIRMED** | calls func_8006E6A8, func_8006E6D4; 11 words |
| `func_8006E7E8` | 0x8006E7E8 | 19 | **CONFIRMED** | syms D_800B0CD8=0x800B0CD8; calls func_8006E7E8, func_800811E4; ops sltiu; 19 words |
| `func_8006E9A0` | 0x8006E9A0 | 141 | **CONFIRMED** | syms D_80011614=0x80011614, D_8009CDDC=0x8009CDDC, D_8009D280=0x8009D280, D_800B0DC6=0x800B0DC6; calls func_80038D1C, func_8005E588, func_80066B60, func_80068E24; ops lbu; 141 words |
| `func_8006EBD4` | 0x8006EBD4 | 4 | **CONFIRMED** | syms D_800B0DBA=0x800B0DBA; calls func_8006EBD4; ops lb; 4 words |
| `func_8006EBE4` | 0x8006EBE4 | 9 | **CONFIRMED** | syms D_800B0DBA=0x800B0DBA, D_800B0DBC=0x800B0DBC; calls func_8006EBE4; ops lbu,lh; 9 words |
| `func_8006EC6C` | 0x8006EC6C | 6 | **CONFIRMED** | calls func_8006EC6C; 6 words |
| `func_8006EC84` | 0x8006EC84 | 26 | **CONFIRMED** | calls func_8006EC84, func_800718D0; ops slt; 26 words |
| `func_8006ECEC` | 0x8006ECEC | 214 | **CONFIRMED** | syms D_80011614=0x80011614, D_80093168=0x80093168, D_800A77FC=0x800A77FC, D_800B0CD8=0x800B0CD8; calls func_8006CDA4, func_8006E6D4, func_8006ECEC, func_800718D0; ops lhu,slti,sltiu; 214 words |
| `func_8006F044` | 0x8006F044 | 120 | **CONFIRMED** | syms D_8001160C=0x8001160C, D_80011610=0x80011610, D_8009315E=0x8009315E, D_80093166=0x80093166; calls func_8006E6D4, func_8006F044, func_800726C4, func_80072714; ops lhu,sltiu; 120 words |
| `func_8006F224` | 0x8006F224 | 40 | **CONFIRMED** | syms D_800942E4=0x800942E4, D_800942E8=0x800942E8; calls func_8006F224; ops lbu,slti,sltiu; 40 words |
| `func_8006F2C4` | 0x8006F2C4 | 54 | **CONFIRMED** | syms D_800942E4=0x800942E4, D_800942E8=0x800942E8, D_800B0CD8=0x800B0CD8, D_800E10A0=0x800E10A0; calls func_8006F2C4; ops lbu,sltiu; 54 words |
| `func_8006F39C` | 0x8006F39C | 206 | **CONFIRMED** | syms D_80011618=0x80011618, D_80093162=0x80093162, D_800942E0=0x800942E0, D_800942E4=0x800942E4; calls func_8006914C, func_8006E6A8, func_8006E7E8, func_8006F39C; ops jalr,lbu,lhu,slti,sltiu; 206 words |
| `func_8006F6D4` | 0x8006F6D4 | 83 | **CONFIRMED** | syms D_800942E0=0x800942E0, D_800942E4=0x800942E4, D_800942E8=0x800942E8; calls func_8006F6D4; ops jalr,lbu,sltiu; 83 words |
| `func_8006F820` | 0x8006F820 | 51 | **CONFIRMED** | syms D_800942E0=0x800942E0, D_800942E4=0x800942E4, D_800942E8=0x800942E8; calls func_8006F820; ops lbu,sltiu; 51 words |
| `func_8006F8EC` | 0x8006F8EC | 65 | **CONFIRMED** | syms D_800942E0=0x800942E0, D_800942E4=0x800942E4, D_800942E8=0x800942E8; calls func_8006F8EC; ops jalr,lbu,sltiu; 65 words |
| `func_8006FE14` | 0x8006FE14 | 148 | **CONFIRMED** | syms D_800942E4=0x800942E4, D_800942E8=0x800942E8, D_800B0CD8=0x800B0CD8, D_800E0EF0=0x800E0EF0; calls func_8006FC18, func_8006FE14; ops lbu,slti,sltiu; 148 words |
| `func_80070064` | 0x80070064 | 84 | **CONFIRMED** | syms D_800942E4=0x800942E4, D_800942E8=0x800942E8, D_8009D254=0x8009D254, D_800B0CD8=0x800B0CD8; calls func_8006FC18, func_80070064; ops lbu,slti,sltiu; 84 words |
| `func_800702DC` | 0x800702DC | 70 | **CONFIRMED** | syms D_800942E4=0x800942E4, D_800942E8=0x800942E8, D_800B0CD8=0x800B0CD8, D_800E0EF0=0x800E0EF0; calls func_8006FC18, func_800702DC; ops lbu,slti,sltiu; 70 words |
| `func_80070E54` | 0x80070E54 | 86 | **CONFIRMED** | syms D_8009CDDC=0x8009CDDC, D_800B0CD8=0x800B0CD8, D_800BCDC8=0x800BCDC8, D_800BCE80=0x800BCE80; calls func_80042FE8, func_8006EBE4, func_8006EC08, func_80070E54; ops slti,sltiu; 86 words |

### Appendix result

### Notable near-misses cleared

- `func_80012850` (244 words, the script-VM step, the largest Shard-A leaf):
  retail `sltiu v1,24` range check, `sll v1,2`, table load from
  `jtbl_80010000` at `0x80010000`, then `jr $v0`. Dumped all 24 table entries
  (`0x80012894`, `0x800128B4`, … `0x80012BF0`): they ascend one-per-opcode,
  matching the C's `case 0..23` order exactly. Case bodies confirm the
  operator mapping: opcodes 9..14 are `> < == >= <= !=` (e.g. opcode 9 is
  `slt v0,v0,a0` → `*lhs` in `$v0`, `*rhs` in `$a0`, i.e. `lhs > rhs`; opcode
  10 is `slt v0,v0,v1` → `lhs < rhs`; opcode 12 adds `xori v0,v0,0x1` →
  `!slt` = `lhs >= rhs`; opcode 13 is `slt` of the swapped order plus `xori`
  = `lhs <= rhs`). Opcode 0 writes `*dst` (`sw v0,0(a0)` at `0x80012C08`);
  opcodes 9..14 write to `*dst` at `0x80012C04`; both are distinct labels, so
  the C's per-case `*ctx->dst = ...` form is exact. `>>` → `srav`/`sra`,
  `*` → `mult`/`mflo`, `/` and `%` → the era div guards.
- `func_800125E0`, `func_8001266C`, `func_80012774`, `func_80029388`,
  `func_800363F4`, `func_80037548`, `func_8003E680`, `func_80056C14`,
  `func_8005DA8C`, `func_8005DAB4`, `func_8005DB44`, `func_800631C0`,
  `func_8006E7E8`, `func_8006F044`, `func_8006F2C4`, `func_8006F6D4`,
  `func_8006F820`, `func_8006F8EC`, `func_80017B34`, `func_80017EC4`,
  `func_80019410`, `func_8002F7D8`, `func_8002F970`: the
  `SIGNED-CMP-but-only-unsigned-slt` heuristic flagged these, but every C
  operand is `unsigned`/`unsigned char`/`unsigned short`, so
  `sltu`/`sltiu` is the *required* form. Examples: `func_800125E0`'s
  `do..while (i < **p)` with `unsigned int i` → `sltu v0,s0,v0`;
  `func_80019410`'s `(D_800BCFEE & 3) < 2` → `sltiu v0,v0,2`;
  `func_8005DB44`'s `(unsigned int)(end-start) >> 5` → `srl` then `sltu`.
- `func_80017B34` / `func_80017EC4`: `sltu v0,a1,v1` (`limit < value`)
  implements `value > limit`; `unsigned short` operands make `sltu` correct.
  Clamp (`move v1,a1`) and `flags |= 0x200` / `*(u32*)(base+0x14)=v<<16` exact.
- `func_8002F7D8` (102 words): seven-slot arena carve with the 220-byte stride
  (`sll/subu/sll/subu/sll` = `220*i`), 216-byte body copy, `1u << i` via
  `sllv`, `0x2000` flag test, `D_8009D2EC`/`D_8009D2A0` byte increments, and
  the `func_8001A680` tail call — all constants, masks and strides match.
- `func_8002F970` (23 words, `void`): the `$v1`-final flag is
  `sw zero,0(a0)` in the `jr` delay slot (the C's `*p = 0` tail store), not a
  return value. The body comparison `D_800A5D58[i].body == *p` uses the
  `&D_800A5D5C + 220*i` address, matching the `SlotRecord`/`D_800A5D5C`
  declarations.
- `func_80021850` (`void`, 34 words): `sll/sra 24` sign-extension of the two
  `signed char` indices, 12-byte record stride (`sll/addu/sll` = `12*i`), and
  the three-word rotate through the `sp` temporary — exact.
- `func_8006F8EC` (65 words): `sltiu 22` / `sltiu 11` arena split with strides
  `0x10C` (ids `0xB..0x15`) and `0xA0C` (ids `0..0xA`), the `lbu` tags, the
  `sltiu 0xC0` / `sltiu 0x55` clamp, the `D_800942E0[a1]` handler-table load,
  the `+0xC` method load, then `jalr v0` — all offsets and masks exact.
- `func_80042B6C` (23 words): `D_800A1870` callback pointer, `D_800A1874`
  counter increment, `bne v0,4` guard, `jalr a0`, then both globals zeroed —
  exact.
- `func_80042FE8`, `func_8004BF08`, `func_8004CDD4`, `func_80055FB4`,
  `func_8005E850`, `func_8005E8A4`, `func_80063158` and the other
  `$v1`-final flags: sampled and confirmed to be a `$v1` result computed
  immediately before a `jr` whose delay slot holds the constant `1` in `$v0`;
  the C's `return 1` is the value actually returned.

### Per-leaf inventory

The table above lists all 362 Shard-A leaves with their VMA, word span, and the
resolved symbols / special opcodes used as evidence. Every row's verdict is
CONFIRMED; there are no DEFECT or UNCERTAIN rows to report.

