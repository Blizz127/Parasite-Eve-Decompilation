# func_80072534 — PARKED (handwritten crt0)

**Status:** not C-matchable. Not a distinct YAML span; lives inside the
merged `asm` blob at file `0x621E4`. Retail initial PC (`asm/disc1/header.s`).

## Target

- VRAM `0x80072534`, file `0x62D34`, size `0xA8` (42 words).
- No `jr $ra`: BSS zero-fill, GP/SP setup, `jal func_800726B4`, restore
  `$ra` from `D_8009D198`, `jal func_8001220C`, then `break 0, 1`.

## Residual mechanism

Handwritten crt0. Two instructions are flagged `/* handwritten instruction */`
(`addi $v0,$v0,-8` and `addi $a0,$a0,%lo(D_80000004)`). A C front end cannot
emit `break 0, 1` or those `addi` forms. Same bucket as other crt0/syscall
wrappers (`handwritten-syscall` / handwritten startup).

## Evidence

`asm/disc1/621E4.s:892-936`.
