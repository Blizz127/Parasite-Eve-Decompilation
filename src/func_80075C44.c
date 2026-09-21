/* func_80075C44 — VRAM 0x80075C44, size 0x28, file 0x66444-0x6646C.
 *
 * GPU primitive packer, twin of func_80075C6C but with flag word 0xE6000002:
 * writes 2 to +3, packs 0xE6000002 when a1 != 0 (else 0xE6000000) ORed with
 * the "a2 != 0" boolean into +4, and zeroes +8 in the return delay slot.
 *
 * era_o2_g0.
 */
void func_80075C44(unsigned char *a0, int a1, unsigned int a2) {
    unsigned int v1;

    *(unsigned char *)(a0 + 3) = 2;
    v1 = 0xE6000000;
    if (a1 != 0)
        v1 = 0xE6000002;
    *(unsigned int *)(a0 + 4) = v1 | (a2 != 0);
    *(unsigned int *)(a0 + 8) = 0;
}
