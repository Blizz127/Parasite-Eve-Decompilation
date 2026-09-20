/*
 * func_8006F224 — find a free id slot for a command id.
 * VRAM 0x8006F224 / file 0x5FA24 / size 0xA0 (40 words).
 *
 * Rejects ids >= 0xC0 with -1. Ids 0x46..0x54 use the D_800942E8 arena
 * (stride 0x10C) and map to slot i+0xB; every other id uses D_800942E4
 * (stride 0xA0C) with slot i. Returns the first slot whose in-use byte is 0,
 * or -1 when the 0xB-entry arena is full. Both arenas are pointer globals
 * (retail loads the base with lui+lw). era -O2 -G0.
 */
extern unsigned char *D_800942E4;
extern unsigned char *D_800942E8;

int func_8006F224(int id)
{
    int i;
    int result = -1;
    unsigned char *p;

    if ((unsigned int)id >= 0xC0)
        return -1;

    if ((unsigned int)(id - 0x46) < 0xF) {
        p = D_800942E8;
        for (i = 0; i < 0xB; i++) {
            if (p[0] == 0) {
                result = i + 0xB;
                break;
            }
            p += 0x10C;
        }
    } else {
        p = D_800942E4;
        for (i = 0; i < 0xB; i++) {
            if (p[0] == 0) {
                result = i;
                break;
            }
            p += 0xA0C;
        }
    }
    return result;
}
