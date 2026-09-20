/*
 * func_8006DED4 — VRAM 0x8006DED4, size 0x7C, file 0x5E6D4-0x5E750.
 *
 * Packs the three halfword coordinates into a local SVECTOR-style struct,
 * asks func_8006DFA8 to project them into pan/volume, then queues the sound
 * via func_8006DF50(package, id, key, pan, volume). era -O2 -G0.
 *
 * Local layout mirrors retail: shorts at +0x18/+0x1A/+0x1C, ints at
 * +0x20/+0x24 relative to the caller's frame.
 */
struct SVec {
    short x;
    short y;
    short z;
    int pan;
    int volume;
};

void func_8006DFA8(short *pos, int *pan, int *volume);

int func_8006DF50(unsigned char *package, int id, int key, int pan, int volume);

int func_8006DED4(unsigned char *package, int id, int key, short x, short y, short z) {
    struct SVec s;
    s.x = x;
    s.y = y;
    s.z = z;
    func_8006DFA8(&s.x, &s.pan, &s.volume);
    return func_8006DF50(package, id, key, s.pan, s.volume);
}
