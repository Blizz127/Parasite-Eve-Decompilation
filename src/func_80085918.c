/* VRAM 0x80085918 / file 0x76118 / size 0x34.
 * AND the complement of the indexed word into D_8009B7CC[1]. */
extern unsigned int *D_8009B7CC;
extern unsigned int D_8009B7D4[];
int func_80085918(int a0) {
    int idx = a0 & 0xFFFF;
    D_8009B7CC[1] &= ~D_8009B7D4[idx];
    return 1;
}
