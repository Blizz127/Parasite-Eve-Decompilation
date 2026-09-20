/* Family template: decay values[2], advance values[3], latch state[1].
 * VRAM 0x800CBC68 / file 0xBC468 / size 0x3C. */
void func_800CBC68(void *unused, signed char *state, unsigned short *values) {
    values[2] -= 8;
    values[3] += 0xA;
    if ((short)values[2] < 20) {
        values[2] = 0;
        state[1] = 2;
    }
}
