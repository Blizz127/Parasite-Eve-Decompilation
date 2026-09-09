/* SPU_StepReverbLoad — quarter-rate reverb/pitch slide stepper.
 * VRAM 0x8008D844 / file 0x7E044 / size 0x338.
 *
 * khasinski still carries this span as asm; semantic transcription from retail
 * disassembly. Runs every 4th Akao_Tick: global pitch slides + Seq_ApplyGlobalPitch
 * (8D7D0), bank pitch slides (dirty via 8AB9C), stream voice pan/pitch slides.
 * Byte-match not claimed.
 */
extern unsigned short D_8009CDEC;
extern short D_8009D2A2;
extern unsigned int D_8009D2B4;
extern unsigned int D_8009D284;
extern short D_8009D220;
extern unsigned int D_8009D2D0;
extern unsigned int D_8009D214;
extern short D_8009D21E;
extern unsigned int D_8009D2CC;
extern unsigned int D_8009D210;
extern unsigned int *D_8009D2C8;
extern unsigned int D_800BCD50;
extern unsigned char D_800B8AC0[];
extern unsigned char D_800BA560[];
extern unsigned char D_800BC000[];

void func_8008D7D0(void); /* Seq_ApplyGlobalPitch */
void func_8008AB9C(void *tracks); /* mark active voices volume-dirty */

void func_8008D844(void)
{
    unsigned short tick;
    unsigned int *state;
    unsigned int old_value;
    unsigned int new_value;
    unsigned int mask;
    unsigned int bit;
    unsigned char *voice;
    unsigned int i;
    unsigned short duration;

    tick = (unsigned short)(D_8009CDEC + 1u);
    D_8009CDEC = tick;
    if ((tick & 3u) != 0u)
        return;

    if (D_8009D2A2 != 0) {
        D_8009D2A2 = (short)(D_8009D2A2 - 1);
        D_8009D2B4 += D_8009D284;
        func_8008D7D0();
    }

    if (D_8009D220 != 0) {
        D_8009D220 = (short)(D_8009D220 - 1);
        D_8009D2D0 += D_8009D214;
    }

    if (D_8009D21E != 0) {
        D_8009D21E = (short)(D_8009D21E - 1);
        old_value = D_8009D2CC;
        new_value = old_value + D_8009D210;
        if ((new_value & 0x00FF0000u) != (old_value & 0x00FF0000u)) {
            voice = D_800B8AC0 + 0xF4;
            for (i = 0u; i < 24u; i++) {
                *(unsigned int *)voice |= 0x10u;
                voice += 0x11C;
            }
        }
        D_8009D2CC = new_value;
    }

    state = D_8009D2C8;
    if (state[1] != 0u) {
        duration = *(unsigned short *)((unsigned char *)state + 0x50);
        if (duration != 0u) {
            old_value = state[0x12]; /* +0x48 */
            new_value = old_value + state[0x13]; /* +0x4C */
            *(unsigned short *)((unsigned char *)state + 0x50) =
                (unsigned short)(duration - 1u);
            if ((new_value & 0x007F0000u) != (old_value & 0x007F0000u))
                func_8008AB9C(D_800B8AC0);
            /* reload — AB9C may have used D_8009D2C8 */
            state = D_8009D2C8;
            state[0x12] = new_value;
        }
    }

    state = D_8009D2C8;
    D_8009D2C8 = (unsigned int *)((unsigned char *)state + 0x68);
    if (state[0x1B] != 0u) { /* +0x6C secondary active */
        duration = *(unsigned short *)((unsigned char *)state + 0xB8);
        if (duration != 0u) {
            old_value = *(unsigned int *)((unsigned char *)state + 0xB0);
            new_value = old_value +
                        *(unsigned int *)((unsigned char *)state + 0xB4);
            *(unsigned short *)((unsigned char *)state + 0xB8) =
                (unsigned short)(duration - 1u);
            if ((new_value & 0x007F0000u) != (old_value & 0x007F0000u))
                func_8008AB9C(D_800BA560);
            state = D_8009D2C8; /* now secondary base */
            state[0x12] = new_value; /* secondary+0x48 == primary+0xB0 */
        }
    }
    D_8009D2C8 = (unsigned int *)((unsigned char *)D_8009D2C8 - 0x68);

    mask = D_800BCD50;
    bit = 0x1000u;
    voice = D_800BC000 + 0x3C;
    while (mask != 0u) {
        if ((mask & bit) != 0u) {
            duration = *(unsigned short *)(voice + 0x38);
            if (duration != 0u) {
                short cur = *(short *)(voice + 0x9C);
                short delta = *(short *)(voice + 0x9E);
                short sum;
                *(unsigned short *)(voice + 0x38) =
                    (unsigned short)(duration - 1u);
                sum = (short)(cur + delta);
                if (((unsigned short)sum & 0xFF00u) !=
                    ((unsigned short)cur & 0xFF00u))
                    *(unsigned int *)(voice + 0xB8) |= 3u;
                *(short *)(voice + 0x9C) = sum;
            }

            duration = *(unsigned short *)(voice + 0x3C);
            if (duration != 0u) {
                unsigned short cur = *(unsigned short *)(voice + 0x3A);
                short delta = *(short *)(voice + 0xA0);
                unsigned short sum;
                *(unsigned short *)(voice + 0x3C) =
                    (unsigned short)(duration - 1u);
                sum = (unsigned short)(cur + (unsigned short)delta);
                if ((sum & 0xFF00u) != (cur & 0xFF00u))
                    *(unsigned int *)(voice + 0xB8) |= 3u;
                *(unsigned short *)(voice + 0x3A) = sum;
            }

            duration = *(unsigned short *)(voice + 0x34);
            if (duration != 0u) {
                old_value = *(unsigned int *)(voice + 0x00);
                new_value = old_value + *(unsigned int *)(voice + 0x04);
                *(unsigned short *)(voice + 0x34) =
                    (unsigned short)(duration - 1u);
                if ((new_value & 0xFF00u) != (old_value & 0xFF00u))
                    *(unsigned int *)(voice + 0xB8) |= 0x10u;
                *(unsigned int *)(voice + 0x00) = new_value;
            }

            mask ^= bit;
        }
        voice += 0x11C;
        bit <<= 1;
    }
}
