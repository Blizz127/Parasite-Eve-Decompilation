# func_8006EC84

- **VRAM:** 0x8006EC84
- **File offset / size:** 0x5F484 / 0x68 (26 words), unit 5F484
- **Branch:** agent/wave4-b (wave-4 leaf slice B)
- **Flags / profile:** `era_o2_g0` (`-O2 -G0`), aspsx 2.21, no frame environment
- **YAML carve:** `[0x5F484, c, func_8006EC84]` + `[0x5F4EC, asm]`, splitting the
  former single `[0x5F484, asm]` run (which also held the still-unmatched
  func_8006ECEC).

## Result

- Fresh `bash scripts/build_us.sh` in `pe-mipsel`:
  `cand SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b` = retail, **EXACT MATCH**.
- `bash scripts/verify_us.sh`: `VERIFY_US=PASS`; **matching-C count 865**
  (was 864).
- Plan SHA-256 `914aaffac2c0435856e26f81cc04ba640fd9db60e1761f0bfe82c4a13503cdae`.

## Source

```c
extern void func_800718D0(int);
void func_8006EC84(int base, int count) {
    int i;
    for (i = 0; i < count; i++)
        func_800718D0(base + ((int *)base)[(short)i]);
}
```

## Notes / divergences

- The function walks `count` (a1) slots of the bank at `base` (a0), reading a
  byte offset from `((int *)base)[i]` and dispatching `base + offset` to
  func_800718D0.
- The decisive lever is the **`(short)i` narrowing on the index**: retail
  computes the scaled index as `sll $v0,$s0,16 / sra $v0,14` (sign-extend the
  low 16 bits, then scale by 4). With a plain `int i`, cc1 strength-reduces the
  whole access into an advancing pointer (`lw 0($s1); addiu $s1,$s1,4`) and
  adds a fourth saved register — 28 words instead of 26. The narrowing makes
  cc1 emit the retail `sll/sra` pair and keeps the loop variable in `$s0` with
  the direct `addiu $s0,$s0,1` / `slt $v0,$s0,$s2` control.
- `try_leaf.py src/func_8006EC84.c 0x5F484 0x68` reported
  `WORDS MATCH (+8 pad bytes, trimmed by the build)` before the authority run.

## Parked neighbours in this unit

`func_8006A9E4`, `func_8006B35C`, `func_8006BD68`, `func_8006ECEC` remain
unmatched; see `docs/ai_context/parked_blockers.json` and the wave4-b handoff.
