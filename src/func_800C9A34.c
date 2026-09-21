/* Family template: decay values[2], advance values[3], latch state[1].
 * VRAM 0x800C9A34 / file 0xBA234 / size 0x3C. */
void func_800C9A34(void *unused, signed char *state, unsigned short *values) {
    values[2] -= 8;
    values[3] += 0x28;
    if ((short)values[2] < 20) {
        values[2] = 0;
        state[1] = 2;
    }
}
