/* func_80083790 — VRAM 0x80083790, size 0x38, file 0x73F90-0x73FC8.
 *
 * Packer: takes (obj[0xE3] + 1) right-shifted by one and scaled by 4, adds
 * the 5*obj[0xE9] span rounded up to a 0x10 boundary, then adds the 32-bit
 * little-endian value at obj+0xEC.
 *
 * era_o2_g0. The shifted expression must stay signed: declaring the shifted
 * value as `int` keeps cc1 on `sra` (retail 0x00021043). An unsigned type
 * makes cc1 prove the byte range non-negative and emit `srl` (0x00021042).
 */
int func_80083790(unsigned char *obj) {
    int a;
    int b;
    int v0;
    int v1;
    unsigned int c;

    a = obj[0xE3];
    b = obj[0xE9];
    c = *(unsigned int *)(obj + 0xEC);
    v0 = a + 1;
    v0 = v0 >> 1;
    v0 = v0 << 2;
    v1 = b << 2;
    v1 = v1 + b;
    v1 = v1 + 3;
    v1 = v1 & 0xFFC;
    v1 = v1 + 4;
    v0 = v0 + v1;
    return v0 + c;
}
