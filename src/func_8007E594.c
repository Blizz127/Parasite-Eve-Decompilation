/*
 * func_8007E594 — clear a 0x18-byte object: word at 0, then bytes 4..8.
 *
 * VRAM 0x8007E594 / file 0x6ED94 / size 0x30 (12 words).
 * ROM: asm/disc1/6E6C0.s.
 *
 * Retail stores the leading word, then byte 4, then walks bytes 5..8 with a
 * down-counting index and a parallel pointer q = a0 + 3 (so q[5] is a0[8] and
 * the pointer decrements toward a0[5]), then three trailing words at
 * 0xC/0x10/0x14. The parallel `q` is load-bearing: indexing a0 directly makes
 * cc1 emit `addu v0,v1,a0` and reorder the store, and an ascending byte loop
 * grows the frame past 0x30.
 */
void func_8007E594(unsigned char *a0) {
    int i;
    unsigned char *q;

    *(unsigned int *)a0 = 0;
    a0[4] = 0;
    i = 3;
    q = a0 + 3;
    do {
        q[5] = 0;
        i--;
        q--;
    } while (i >= 0);
    *(unsigned int *)(a0 + 0xC) = 0;
    *(unsigned int *)(a0 + 0x10) = 0;
    *(unsigned int *)(a0 + 0x14) = 0;
}
