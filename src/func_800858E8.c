/* VRAM 0x800858E8 / file 0x760E8 / size 0x30.
 * OR the indexed word into D_8009B7CC[1]; return idx < 3. */
extern unsigned int *D_8009B7CC;
extern unsigned int D_8009B7D4[];
int func_800858E8(int a0) {
    int idx = a0 & 0xFFFF;
    D_8009B7CC[1] |= D_8009B7D4[idx];
    return idx < 3;
}
