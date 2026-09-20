/* Family template: decay values[2], advance values[3], latch state[1].
 * VRAM 0x800CD5B0 / file 0xBDDB0 / size 0x3C. */
void func_800CD5B0(void *unused, signed char *state, unsigned short *values) {
    values[2] -= 0xA;
    values[3] += 0x3C;
    if ((short)values[2] < 20) {
        values[2] = 0;
        state[1] = 2;
    }
}
