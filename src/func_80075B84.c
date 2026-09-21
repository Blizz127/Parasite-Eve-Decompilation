struct XY {
    short x;
    short y;
    short w;
    short h;
};

extern int func_80076170(short a0, short a1);
extern int func_80076208(short a0, short a1);

void func_80075B84(unsigned char *a0, struct XY *a1) {
    a0[3] = 2;
    *(int *)(a0 + 4) = func_80076170(a1->x, a1->y);
    *(int *)(a0 + 8) = func_80076208(a1->x + a1->w - 1, a1->y + a1->h - 1);
}
