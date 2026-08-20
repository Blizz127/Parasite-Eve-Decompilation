/* Hit-react handler: angle-based command select + overlay.
 * VRAM 0x8001F814 / file 0xF814 / size 0x1AC (107 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Reads aya actor from D_8009D254, checks kind in [6,15],
 * dispatches via jump table to store kind/lo to D_8009D29A/B/C,
 * computes angle via func_800305C8, selects hit_cmd by quadrant,
 * calls func_8001A680 (command setup) then func_8006DE80 (overlay).
 */
extern unsigned int D_8009D254;   /* aya actor pointer */
extern unsigned int D_8009D278;   /* attack record pointer */
extern short        D_8009D298;   /* hit flag (16-bit) */
extern unsigned char D_8009D29A;  /* kind byte */
extern unsigned char D_8009D29B;  /* lo byte */
extern unsigned int D_8009D29C;   /* hit param (32-bit) */

extern int func_800305C8(int actor, int target);
extern void func_8001A680(int actor, int cmd);
extern void func_8006DE80(int id, int a1, int x, int y, int z);

int func_8001F814(int actor) {
    int aya;
    int kind;
    int lo;
    unsigned int rec;
    unsigned int flags;
    int s0;
    int angle;
    int hit_cmd;

    aya = *(int *)&D_8009D254;
    if (aya == 0)
        return 0;

    kind = *(unsigned char *)(aya + 0x0E);
    if ((unsigned int)(kind - 6) < 10) {
        lo = *(unsigned char *)(aya + 0x0F);
        D_8009D29A = (unsigned char)kind;
        D_8009D298 = 1;
        D_8009D29B = (unsigned char)lo;
        if (kind == 7 || kind == 9 || kind == 11)
            D_8009D29C = (unsigned int)lo << 16;
        else
            D_8009D29C = *(unsigned int *)(aya + 0x14);
    }

    rec = *(unsigned int *)&D_8009D278;
    flags = (rec != 0) ? *(unsigned int *)(rec + 0x4C) : 0;
    if ((flags & 0x00012000) != 0 || actor == 0)
        return 0;

    s0 = func_800305C8(actor, aya);
    angle = (int)(short)s0;
    if (angle < 512)
        hit_cmd = 0;
    else if (angle < 1536)
        hit_cmd = 2;
    else if (angle < 2560)
        hit_cmd = 1;
    else if (angle < 3584)
        hit_cmd = 3;
    else
        hit_cmd = 0;

    func_8001A680(*(int *)&D_8009D254, hit_cmd);
    aya = *(int *)&D_8009D254;
    if (aya != 0) {
        func_8006DE80(0x46A, 0,
                      (int)(short)*(unsigned short *)(aya + 0x2A),
                      (int)(short)*(unsigned short *)(aya + 0x2E),
                      (int)(short)*(unsigned short *)(aya + 0x32));
        flags = *(unsigned int *)(aya + 0x98);
        if (flags & 0x100) {
            *(unsigned int *)(aya + 0x98) = flags & ~0x100u;
            D_8009D298 = 2;
        }
    }
    return (int)(short)s0;
}
