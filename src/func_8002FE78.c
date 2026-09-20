/* VRAM 0x8002FE78 / file 0x20678 / size 0x100. */
typedef struct {
    int f0;
    short f4;
    unsigned short f6;
    int f8;
    short fC;
    char gap0[2];
    unsigned short f10;
    unsigned char f12;
    char pad0[0x1C - 0x13];
    short f1C;
    unsigned short f1E;
    unsigned short f20;
    unsigned short f22;
    char pad1[0x28 - 0x24];
    int f28;
    char pad2[0x4C - 0x2C];
    union {
        unsigned int f4C;
        struct {
            unsigned int lo6 : 6;
            unsigned int mid2 : 2;
            unsigned int b8 : 1;
            unsigned int b9 : 1;
            unsigned int hi19 : 19;
            unsigned int b29 : 1;
            unsigned int top2 : 2;
        } bits;
    } u;
} Unk;

extern Unk **D_8009D254;

int func_8002FE78(unsigned int idx) {
    Unk *p = *D_8009D254;
    int r = -0x3E8;

    switch (idx & 0xFF) {
    case 0:
        r = p->f0;
        break;
    case 1:
        r = p->f4;
        break;
    case 2:
        r = p->f6;
        break;
    case 3:
        r = p->f8;
        break;
    case 4:
        r = p->fC;
        break;
    case 6:
        r = p->f10;
        break;
    case 7:
        r = p->f12;
        break;
    case 10:
        r = p->f1C;
        break;
    case 11:
        r = p->f1E;
        break;
    case 12:
        r = p->f20;
        break;
    case 13:
        r = p->f28;
        break;
    case 14:
        r = p->f22;
        break;
    case 20:
        r = p->u.bits.b9;
        break;
    case 21:
        r = p->u.bits.mid2;
        break;
    case 22:
        r = p->u.bits.b29;
        break;
    }
    return r;
}
