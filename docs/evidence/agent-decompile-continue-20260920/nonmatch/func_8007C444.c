/* VRAM 0x8007C444 / file 0x6CC44 / size 0x34.
 * Zero `count` 32-byte records starting at record index `base`; the pointer
 * global D_800C0DC8 is reloaded every iteration (retail does). */
extern unsigned char *D_800C0DC8;

void func_8007C444(int base, int count) {
    int i;
    for (i = 0; i < count; i++)
        *(unsigned int *)(D_800C0DC8 + (base + i) * 32) = 0;
}
