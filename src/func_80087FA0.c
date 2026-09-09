/* Spu_TickVoiceEnvelopes — stream-bank voice slide/LFO tick.
 * VRAM 0x80087FA0 / file 0x787A0 / size 0x3A4.
 *
 * Stream/CD voice table updater from Akao_Tick (8DB7C). Sibling of
 * func_80087AA8 with fewer slide channels and D_800BCD6C/D_800BCD74 mask
 * toggles instead of the primary state +0x34/+0x3C fields.
 * Semantic transcription; byte-match not claimed.
 */
extern unsigned int D_8009D2C4;
extern unsigned int D_800BCD6C;
extern unsigned int D_800BCD74;

void func_80089960(void);
void func_80089CF0(void);

void func_80087FA0(void *voice, unsigned int voice_mask)
{
    unsigned int old_value;
    unsigned int new_value;
    int value;
    int scaled;
    short *table;

    if (*(unsigned short *)((char *)voice + 0x72) != 0) {
        old_value = *(unsigned int *)((char *)voice + 0x44);
        new_value = old_value + *(unsigned int *)((char *)voice + 0x48);
        *(unsigned short *)((char *)voice + 0x72) -= 1;
        if ((new_value & 0xFFE00000u) != (old_value & 0xFFE00000u)) {
            *(unsigned int *)((char *)voice + 0xF4) |= 3u;
        }
        *(unsigned int *)((char *)voice + 0x44) = new_value;
    }

    if (*(unsigned short *)((char *)voice + 0xBA) != 0) {
        *(unsigned short *)((char *)voice + 0xBA) -= 1;
        if (*(unsigned short *)((char *)voice + 0xBA) == 0) {
            D_800BCD6C ^= voice_mask;
            D_8009D2C4 |= 0x10u;
            func_80089960();
        }
    }

    if (*(unsigned short *)((char *)voice + 0xBC) != 0) {
        *(unsigned short *)((char *)voice + 0xBC) -= 1;
        if (*(unsigned short *)((char *)voice + 0xBC) == 0) {
            D_800BCD74 ^= voice_mask;
            func_80089CF0();
        }
    }

    if (*(unsigned short *)((char *)voice + 0x96) != 0) {
        *(unsigned short *)((char *)voice + 0x96) -= 1;
        *(unsigned short *)((char *)voice + 0x94) +=
            *(unsigned short *)((char *)voice + 0x98);
        value = (*(unsigned short *)((char *)voice + 0x94) & 0x7F00) >> 8;
        if ((*(unsigned short *)((char *)voice + 0x94) & 0x8000) != 0) {
            scaled = (value * (int)*(unsigned int *)((char *)voice + 0x30)) >> 7;
        } else {
            unsigned int base = *(unsigned int *)((char *)voice + 0x30);
            scaled = (value * (int)(((base << 4) - base) >> 8)) >> 7;
        }
        *(unsigned short *)((char *)voice + 0x92) = (unsigned short)scaled;

        if (*(unsigned short *)((char *)voice + 0x8E) != 1) {
            table = *(short **)((char *)voice + 0x1C);
            if (table[0] == 0 && table[1] == 0) {
                table += table[2];
            }
            value = (*(short *)((char *)voice + 0x92) * table[0]) >> 16;
            if (value != *(short *)((char *)voice + 0xE8)) {
                *(unsigned short *)((char *)voice + 0xE8) = (unsigned short)value;
                *(unsigned int *)((char *)voice + 0xF4) |= 0x10u;
                if (value >= 0) {
                    *(unsigned short *)((char *)voice + 0xE8) =
                        (unsigned short)(value << 1);
                }
            }
        }
    }

    if (*(unsigned short *)((char *)voice + 0xA8) != 0) {
        *(unsigned short *)((char *)voice + 0xA8) -= 1;
        *(unsigned short *)((char *)voice + 0xA6) +=
            *(unsigned short *)((char *)voice + 0xAA);
        if (*(unsigned short *)((char *)voice + 0xA2) != 1) {
            table = *(short **)((char *)voice + 0x20);
            if (table[0] == 0 && table[1] == 0) {
                table += table[2];
            }
            value = ((*(short *)((char *)voice + 0x46) *
                      (*(unsigned short *)((char *)voice + 0x6C) >> 8)) >>
                     7) *
                    (*(unsigned short *)((char *)voice + 0xA6) >> 8);
            value = ((value << 9) >> 16) * table[0];
            value >>= 15;
            if (value != *(short *)((char *)voice + 0xEA)) {
                *(unsigned short *)((char *)voice + 0xEA) = (unsigned short)value;
                *(unsigned int *)((char *)voice + 0xF4) |= 3u;
            }
        }
    }

    if (*(unsigned short *)((char *)voice + 0xB6) != 0) {
        *(unsigned short *)((char *)voice + 0xB6) -= 1;
        *(unsigned short *)((char *)voice + 0xB4) +=
            *(unsigned short *)((char *)voice + 0xB8);
        if (*(unsigned short *)((char *)voice + 0xB0) != 1) {
            table = *(short **)((char *)voice + 0x24);
            if (table[0] == 0 && table[1] == 0) {
                table += table[2];
            }
            value = ((*(unsigned short *)((char *)voice + 0xB4) >> 8) * table[0]) >>
                    15;
            if (value != *(short *)((char *)voice + 0xEC)) {
                *(unsigned short *)((char *)voice + 0xEC) = (unsigned short)value;
                *(unsigned int *)((char *)voice + 0xF4) |= 3u;
            }
        }
    }

    if (*(unsigned short *)((char *)voice + 0x7A) != 0) {
        old_value = *(unsigned int *)((char *)voice + 0x34);
        new_value = old_value + *(unsigned int *)((char *)voice + 0x4C);
        *(unsigned short *)((char *)voice + 0x7A) -= 1;
        if ((new_value & 0xFFFF0000u) != (old_value & 0xFFFF0000u)) {
            *(unsigned int *)((char *)voice + 0xF4) |= 0x10u;
        }
        *(unsigned int *)((char *)voice + 0x34) = new_value;
    }
}
