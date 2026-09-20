extern unsigned char *D_800E2368;
extern unsigned char *D_800F33E0;

void func_800D401C(int a0)
{
    unsigned char *s0 = D_800E2368;
    unsigned short *s1 = (unsigned short *)(s0 + 0x20);
    int a1 = 0;
    unsigned short s2;
    unsigned int v0, v1;
    unsigned int a2;

    for (a1 = 0; a1 < 8; a1++) {
        if (*s1 == 0xFFFFu)
            break;
        s1 = (unsigned short *)((unsigned char *)s1 + 0xC);
    }
    if (a1 == 8)
        return;

    *(unsigned char *)(s0 + 0x0C) = (unsigned char)(*(unsigned char *)(s0 + 0x0C) + 1);
    *s1 = (unsigned short)a0;
    s1[1] = 0;

    v1 = (unsigned int)(a0 << 1);
    v0 = *(unsigned int *)(s0 + 0x80);
    v0 += v1;
    s2 = *(unsigned short *)(v0 + 0x20);

    v0 = *(unsigned short *)(s0 + 0x10);
    a1 = *(int *)(s0 + 4);
    v0 += s2;
    *(unsigned short *)(s0 + 0x10) = (unsigned short)v0;
    if ((v0 & 0xFFFFu) >= 0x97Du) {
        a1 = (int)(s0 + 0x84);
        *(unsigned short *)(s0 + 0x10) = s2;
        *(unsigned int *)(s0 + 4) = (unsigned int)a1;
    }

    v0 = (unsigned int)(a1 + s2);
    *(unsigned int *)(s1 + 2) = v0;

    v0 = *s1;
    *(unsigned int *)(s1 + 2) = (unsigned int)a1;
    v1 = *(unsigned int *)(s0 + 0x80);
    a2 = *(unsigned int *)(s0 + 8);
    v0 = (v0 << 2) + v1;
    {
        int (*fn)(int) = *(int (**)(int))v0;
        D_800F33E0 = (unsigned char *)s1;
        a1 = fn(0);
    }
    v0 = (unsigned int)(s2 + a1);
    if (a1 == 0)
        *(unsigned int *)(s1 + 2) = 0;

    v1 = *(unsigned int *)(s0 + 4);
    a2 = *(unsigned short *)(s0 + 0x10);
    v1 += v0;
    a2 += (unsigned int)a1;
    *(unsigned int *)(s0 + 4) = v1;
    *(unsigned short *)(s0 + 0x10) = (unsigned short)a2;
}
