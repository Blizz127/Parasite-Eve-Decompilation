/* HP clamp and copy: clamp current HP to max, copy to +0x0E.
 * VRAM 0x800293F4 / file 0x93F4 / size 0x54 (21 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Loads actor from D_8009D508 (gp+0x508), reads max HP (+0x1C)
 * and current HP (+0x0C). If max < current, stores max to +0x0C.
 * Then copies current HP to +0x0E.
 */
extern int D_8009D508;  /* actor pointer (gp+0x508) */

void func_800293F4(void) {
    int actor = *(int *)&D_8009D508;
    int max_hp = *(short *)(actor + 0x1C);
    int cur_hp = *(short *)(actor + 0x0C);
    int clamped = max_hp;
    if (max_hp < cur_hp)
        *(short *)(actor + 0x0C) = (short)clamped;
    *(short *)(actor + 0x0E) = *(short *)(actor + 0x0C);
}
