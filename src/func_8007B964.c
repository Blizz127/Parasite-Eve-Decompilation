/* VRAM 0x8007B964 / file 0x6C164 / size 0x88.
 * Six staged byte writes to SPU-style pointer globals; returns 0. */
extern unsigned char *D_8009B27C;
extern unsigned char *D_8009B280;
extern unsigned char *D_8009B284;
extern unsigned char *D_8009B288;

int func_8007B964(unsigned char *src) {
    *D_8009B27C = 2;
    *D_8009B284 = src[0];
    *D_8009B288 = src[1];
    *D_8009B27C = 3;
    *D_8009B280 = src[2];
    *D_8009B284 = src[3];
    *D_8009B288 = 0x20;
    return 0;
}
