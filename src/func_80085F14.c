/* Selector: store the argument and a 0/1 equality flag.
 * VRAM 0x80085F14 / file 0x76714 / size 0x30. */
extern int D_8009B38C;
extern int D_8009B418;

int func_80085F14(int a0) {
    int v0;

    switch (a0) {
    case 0:
        v0 = 0;
        break;
    case 1:
        v0 = 1;
        break;
    default:
        v0 = 0;
        break;
    }
    D_8009B38C = a0;
    D_8009B418 = v0;
    return v0;
}
