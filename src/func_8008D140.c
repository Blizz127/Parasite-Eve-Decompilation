typedef struct {
    unsigned int flags;
    unsigned short vals[32];
} D140;

extern unsigned short *D_8009B3FC;

void func_8008D140(D140 *p) {
    unsigned int flags = p->flags;
    unsigned int zero = (flags == 0);
    if (zero || (flags & 1u))
        D_8009B3FC[0xE0 + 0] = p->vals[0];
    if (zero || (flags & 2u))
        D_8009B3FC[0xE0 + 1] = p->vals[1];
    if (zero || (flags & 4u))
        D_8009B3FC[0xE0 + 2] = p->vals[2];
    if (zero || (flags & 8u))
        D_8009B3FC[0xE0 + 3] = p->vals[3];
    if (zero || (flags & 16u))
        D_8009B3FC[0xE0 + 4] = p->vals[4];
    if (zero || (flags & 32u))
        D_8009B3FC[0xE0 + 5] = p->vals[5];
    if (zero || (flags & 64u))
        D_8009B3FC[0xE0 + 6] = p->vals[6];
    if (zero || (flags & 128u))
        D_8009B3FC[0xE0 + 7] = p->vals[7];
    if (zero || (flags & 256u))
        D_8009B3FC[0xE0 + 8] = p->vals[8];
    if (zero || (flags & 512u))
        D_8009B3FC[0xE0 + 9] = p->vals[9];
    if (zero || (flags & 1024u))
        D_8009B3FC[0xE0 + 10] = p->vals[10];
    if (zero || (flags & 2048u))
        D_8009B3FC[0xE0 + 11] = p->vals[11];
    if (zero || (flags & 4096u))
        D_8009B3FC[0xE0 + 12] = p->vals[12];
    if (zero || (flags & 8192u))
        D_8009B3FC[0xE0 + 13] = p->vals[13];
    if (zero || (flags & 16384u))
        D_8009B3FC[0xE0 + 14] = p->vals[14];
    if (zero || (flags & 32768u))
        D_8009B3FC[0xE0 + 15] = p->vals[15];
    if (zero || (flags & 65536u))
        D_8009B3FC[0xE0 + 16] = p->vals[16];
    if (zero || (flags & 131072u))
        D_8009B3FC[0xE0 + 17] = p->vals[17];
    if (zero || (flags & 262144u))
        D_8009B3FC[0xE0 + 18] = p->vals[18];
    if (zero || (flags & 524288u))
        D_8009B3FC[0xE0 + 19] = p->vals[19];
    if (zero || (flags & 1048576u))
        D_8009B3FC[0xE0 + 20] = p->vals[20];
    if (zero || (flags & 2097152u))
        D_8009B3FC[0xE0 + 21] = p->vals[21];
    if (zero || (flags & 4194304u))
        D_8009B3FC[0xE0 + 22] = p->vals[22];
    if (zero || (flags & 8388608u))
        D_8009B3FC[0xE0 + 23] = p->vals[23];
    if (zero || (flags & 16777216u))
        D_8009B3FC[0xE0 + 24] = p->vals[24];
    if (zero || (flags & 33554432u))
        D_8009B3FC[0xE0 + 25] = p->vals[25];
    if (zero || (flags & 67108864u))
        D_8009B3FC[0xE0 + 26] = p->vals[26];
    if (zero || (flags & 134217728u))
        D_8009B3FC[0xE0 + 27] = p->vals[27];
    if (zero || (flags & 268435456u))
        D_8009B3FC[0xE0 + 28] = p->vals[28];
    if (zero || (flags & 536870912u))
        D_8009B3FC[0xE0 + 29] = p->vals[29];
    if (zero || (flags & 1073741824u))
        D_8009B3FC[0xE0 + 30] = p->vals[30];
    if (zero || (flags & 2147483648u))
        D_8009B3FC[0xE0 + 31] = p->vals[31];
}
