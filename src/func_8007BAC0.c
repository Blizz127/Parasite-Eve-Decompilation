extern unsigned char *D_8009B290;
extern unsigned char *D_8009B27C;
extern unsigned char *D_8009B280;
extern unsigned char *D_8009B284;
extern unsigned char *D_8009B288;

int func_8007BAC0(void) {
    unsigned char sp[4];

    if (*(unsigned short *)(D_8009B290 + 0x1B8) == 0) {
        if (*(unsigned short *)(D_8009B290 + 0x1BA) == 0) {
            *(unsigned short *)(D_8009B290 + 0x180) = 0x3FFF;
            *(unsigned short *)(D_8009B290 + 0x182) = 0x3FFF;
        }
    }
    *(unsigned short *)(D_8009B290 + 0x1B0) = 0x3FFF;
    *(unsigned short *)(D_8009B290 + 0x1B2) = 0x3FFF;
    *(unsigned short *)(D_8009B290 + 0x1AA) = 0xC001;
    sp[2] = 0x80;
    sp[0] = 0x80;
    sp[3] = 0;
    sp[1] = 0;
    *D_8009B27C = 2;
    *D_8009B284 = sp[0];
    *D_8009B288 = sp[1];
    *D_8009B27C = 3;
    *D_8009B280 = sp[2];
    *D_8009B284 = sp[3];
    *D_8009B288 = 0x20;
    return 0;
}
