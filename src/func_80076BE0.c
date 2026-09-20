/* VRAM 0x80076BE0 / file 0x673E0 / size 0x30.
 * Tag the word with 0x10000000 through D_80095854, then return the low
 * 24 bits of the word at *D_80095850. */
extern unsigned int *D_80095854;
extern unsigned int *D_80095850;

unsigned int func_80076BE0(unsigned int value) {
    *D_80095854 = value | 0x10000000u;
    return *D_80095850 & 0x00FFFFFFu;
}
