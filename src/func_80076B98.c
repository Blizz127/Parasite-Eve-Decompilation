/* func_80076B98 - VRAM 0x80076B98, file 0x67398, size 0x48.
 * Publish 0x04000002 through D_80095854, a0 through D_80095858, 0 through
 * D_8009585C, and 0x01000401 through D_80095860.  The final store into the
 * D_80095860 pointer fills the jr delay slot. */
extern unsigned int D_80095854, D_80095858, D_8009585C, D_80095860;

void func_80076B98(unsigned int a0) {
    *(unsigned int *)D_80095854 = 0x04000002u;
    *(unsigned int *)D_80095858 = a0;
    *(unsigned int *)D_8009585C = 0u;
    *(unsigned int *)D_80095860 = 0x01000401u;
}
