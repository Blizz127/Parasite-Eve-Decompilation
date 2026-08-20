/* Hit-react handler: angle-based command select + overlay.
 * VRAM 0x8001F814 / file 0x10014 / size 0x1B0 (108 words).
 * gcc-2.7.2-psx -O2 -G8 + maspsx 2.21 --dont-expand-li.
 *
 * Jump table at 0x800106E4 for kind 6..15 (shared prefix rodata).
 * D_8009D298 is gp-relative (short); other symbols are lui (arrays).
 */
extern unsigned char D_8009D254[];
extern unsigned char D_8009D278[];
extern short D_8009D298;
extern unsigned char D_8009D29A[];
extern unsigned char D_8009D29B[];
extern unsigned int D_8009D29C[];

extern int func_800305C8(int actor, int target);
extern void func_8001A680(int actor, int cmd);
extern void func_8006DE80(int id, int a1, int x, int y, int z);

int func_8001F814(int actor) {
    unsigned char *aya;
    int kind;
    int lo;
    int s0;
    int angle;
    int hit_cmd;
    unsigned int flags;

    aya = *(unsigned char **)D_8009D254;
    s0 = 0;
    kind = aya[0x0E];
    switch (kind) {
    case 6:
    case 8:
    case 10:
    case 12:
    case 13:
    case 14:
    case 15:
        lo = aya[0x0F];
        D_8009D29A[0] = (unsigned char)kind;
        D_8009D298 = 1;
        D_8009D29B[0] = (unsigned char)lo;
        D_8009D29C[0] = *(unsigned int *)(aya + 0x14);
        break;
    case 7:
    case 9:
    case 11:
        lo = aya[0x0F];
        D_8009D29A[0] = (unsigned char)kind;
        D_8009D298 = 1;
        D_8009D29B[0] = (unsigned char)lo;
        D_8009D29C[0] = (unsigned int)lo << 16;
        break;
    }

    flags = *(unsigned int *)(*(unsigned char **)D_8009D278 + 0x4C);
    if ((flags & 0x00012000u) != 0)
        return (int)(short)s0;

    s0 = func_800305C8(actor, (int)aya);
    angle = (int)(short)s0;
    if (angle < 512)
        hit_cmd = 0;
    else if (angle < 1536)
        hit_cmd = 2;
    else if (angle < 2560)
        hit_cmd = 1;
    else
        hit_cmd = (-(int)(angle < 3584)) & 3;

    func_8001A680(*(int *)D_8009D254, hit_cmd);
    aya = *(unsigned char **)D_8009D254;
    func_8006DE80(0x46A, 0,
                  (int)*(short *)(aya + 0x2A),
                  (int)*(short *)(aya + 0x2E),
                  (int)*(short *)(aya + 0x32));
    flags = *(unsigned int *)(aya + 0x98);
    if (flags & 0x100u) {
        *(unsigned int *)(aya + 0x98) = flags & ~0x100u;
        D_8009D298 = 2;
    }
    return (int)(short)s0;
}
