/* VRAM 0x8009071C / file 0x80F1C / size 0x38.
 * Ring-buffer insert of *a0 into +4+4*idx, then clear +0x62+2*idx. */
void func_8009071C(unsigned char *a0) {
    unsigned int idx = (*(unsigned short *)(a0 + 0xCE) + 1) & 3;
    int v1 = *(int *)a0;
    *(unsigned short *)(a0 + 0xCE) = (unsigned short)idx;
    *(int *)(a0 + 4 + (idx << 2)) = v1;
    idx = *(unsigned short *)(a0 + 0xCE);
    *(unsigned short *)(a0 + 0x62 + (idx << 1)) = 0;
}
