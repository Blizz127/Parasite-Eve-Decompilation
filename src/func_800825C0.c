/* Controller port state query: dispatch through the installed record getter
 * D_8009B738(port), then classify the returned record's state byte.
 * VRAM 0x800825C0 / file 0x72DC0 / size 0xC0.  era -O2 -G0. */
typedef struct Ctl825C0 Ctl825C0;
struct Ctl825C0 {
    unsigned char pad0[0x10];
    Ctl825C0 *unk10;
    unsigned char pad14[0x30 - 0x14];
    unsigned char *unk30;
    unsigned int unk34;
    unsigned char unk38;
    unsigned char pad39[0x49 - 0x39];
    unsigned char unk49;
};

extern Ctl825C0 *(*D_8009B738)(int port);

int func_800825C0(int port) {
    Ctl825C0 *p = D_8009B738(port);
    int s;

    if ((p->unk34 & 0xFFFF0000) || ((p != p->unk10) && (p->unk38 != 0))
        || (*p->unk30 != 0)) {
        s = p->unk49;
        switch (s) {
        case 3:
            return 1;
        case 2:
            return 1;
        case 6:
            return 4;
        }
    }
    return p->unk49;
}
