/* Family template: decay values[2], advance values[3], latch state[1].
 * VRAM 0x800CBC2C / file 0xBC42C / size 0x3C. */
void func_800CBC2C(void *unused, signed char *state, unsigned short *values) {
    values[2] -= 8;
    values[3] += 0x78;
    if ((short)values[2] < 20) {
        values[2] = 0;
        state[1] = 2;
    }
}
