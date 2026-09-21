/*
 * func_80075B1C — query the 0x38 display/handler slot and return its sign bit.
 *
 * VRAM 0x80075B1C / file 0x6631C / size 0x30 (12 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   lw  v0, D_80095744
 *   lw  v0, 0x38(v0)
 *   jalr v0            (no arguments)
 *   lw  ra, ...
 *   srl v0, v0, 31
 *
 * The slot at `D_80095744 + 0x38` takes no arguments (retail clears `$a0`
 * with a zero movn only because `$a0` is not live; the compiler emits the
 * plain `nop` form once the call is declared argument-less). The result is
 * an unsigned shift right by 31, i.e. the sign bit as 0/1.
 *
 * era -O2 -G0; LINK_EXACT.
 */

extern struct D44d { char pad[0x38]; unsigned int (*f)(); } *D_80095744;

unsigned int func_80075B1C(void) {
    return D_80095744->f() >> 31;
}
