/*
 * func_80077AC4 — pack the low 24 bits of one side into the high byte of the
 * other, both directions (16-bit colour/palette key swap).
 *
 * VRAM 0x80077AC4 / file 0x682C4 / size 0x3C (15 words).
 *
 * Retail (asm/disc1/682BC.s):
 *   *a1 = (*a1 & 0xFF000000) | (*a0 & 0x00FFFFFF);
 *   *a0 = (*a0 & 0xFF000000) | ((unsigned int)a1 & 0x00FFFFFF);
 *   (return is the *a0 store in the jr delay slot)
 *
 * The two mask constants sit in fixed registers retail: `0x00FFFFFF` in `$6`
 * (a2) and `0xFF000000` in `$7` (a3), materialized first and reused by both
 * statements. Without pins cc1 mirrors the two `lui/ori` pairs (`$7` then `$6`)
 * and swaps the and-operands; pinning the constants with `asm("$6")`/`asm("$7")`
 * reproduces retail byte-for-byte.
 *
 * era -O2 -G0; LINK_EXACT.
 * ROM: asm/disc1/682BC.s @ file 0x682C4, 15 words (0x3C bytes).
 */

void func_80077AC4(unsigned int *a0, unsigned int *a1) {
    register unsigned int lo asm("$6") = 0x00FFFFFFu;
    register unsigned int hi asm("$7") = 0xFF000000u;

    *a1 = (*a1 & hi) | (*a0 & lo);
    *a0 = (*a0 & hi) | ((unsigned int)a1 & lo);
}
