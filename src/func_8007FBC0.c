/* Phase 5EF-pilot: first delay-slot $at sw setter — D_800A36A0 = cb.
 * VRAM 0x8007FBC0 / file 0x703C0 / size 0xC.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li
 * + MASPSX_FILL_STORE_DELAY_SLOT=1 (LOCAL PATCH: sw fills the jr delay slot).
 *
 * Type provenance: D_800A36A0 = void (*)(void). READY-FROM-READER jalr proof
 * in asm/disc1/704BC.s (beqz-guarded lw of D_800A36A0 then jalr $v0 — the
 * value is invoked as a callback; same pattern as D_800A1870 in 5EI).
 *
 * ROM: lui $at,%hi(D_800A36A0); jr $ra; sw $a0,%lo(D_800A36A0)($at)
 * (words 3C01800A 03E00008 AC2436A0 — sw in the return delay slot.)
 */
typedef void (*func_800A36A0_t)(void);

extern func_800A36A0_t D_800A36A0;

void func_8007FBC0(func_800A36A0_t cb) {
    D_800A36A0 = cb;
}
