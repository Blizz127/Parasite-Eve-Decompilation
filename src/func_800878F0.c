/* Akao_WriteVoiceParam — publish pending voice+0xF0 overlay to SPU HW.
 * VRAM 0x800878F0 / file 0x780F0 / size 0x1B8.
 *
 * khasinski Akao_SpuVoiceRegisters.c donor (register-asm pins stripped).
 * Flag bits match AKAO_VOICE_PARAM_* (volume/pitch/start/loop/ADSR*).
 * Semantic; byte-match not claimed.
 */
void func_800877BC(unsigned int index, unsigned int pitch); /* SetPitch */
void func_80087798(unsigned int index, int left, int right); /* SetVolume */
void func_800877D4(unsigned int index, unsigned int addr); /* SetStartAddress */
void func_800877F0(unsigned int index, unsigned int addr); /* SetRepeatAddress */
void func_8008788C(int voice, unsigned int rate, unsigned int mode); /* SustainRate */
void func_8008780C(int voice, unsigned int rate, unsigned int mode); /* Attack */
void func_800878C0(int voice, unsigned int rate, unsigned int mode); /* Release */
void func_8008783C(int voice, unsigned int rate); /* DecayRate */
void func_80087864(int voice, unsigned int level); /* SustainLevel */

void func_800878F0(unsigned int voice_index, void *params)
{
    unsigned char *p = (unsigned char *)params;
    unsigned int flags;
    unsigned int cur;

    flags = *(unsigned int *)(p + 4);
    if (flags == 0u)
        return;

    if (flags & 0x10u) {
        func_800877BC(voice_index, *(unsigned short *)(p + 0x1Cu));
        cur = *(unsigned int *)(p + 4) & ~0x10u;
        *(unsigned int *)(p + 4) = cur;
        if (cur == 0u)
            return;
    }
    if (flags & 0x3u) {
        func_80087798(voice_index, *(short *)(p + 0x28), *(short *)(p + 0x2A));
        cur = *(unsigned int *)(p + 4) & ~0x3u;
        *(unsigned int *)(p + 4) = cur;
        if (cur == 0u)
            return;
    }
    if (flags & 0x80u) {
        func_800877D4(voice_index, *(unsigned int *)(p + 8));
        cur = *(unsigned int *)(p + 4) & ~0x80u;
        *(unsigned int *)(p + 4) = cur;
        if (cur == 0u)
            return;
    }
    if (flags & 0x10000u) {
        func_800877F0(voice_index, *(unsigned int *)(p + 0xCu));
        cur = *(unsigned int *)(p + 4) & ~0x10000u;
        *(unsigned int *)(p + 4) = cur;
        if (cur == 0u)
            return;
    }
    if (flags & 0x2200u) {
        func_8008788C(voice_index, *(unsigned short *)(p + 0x24),
                      *(unsigned int *)(p + 0x14));
        cur = *(unsigned int *)(p + 4) & ~0x2200u;
        *(unsigned int *)(p + 4) = cur;
        if (cur == 0u)
            return;
    }
    if (flags & 0x900u) {
        func_8008780C(voice_index, *(unsigned short *)(p + 0x1Eu),
                      *(unsigned int *)(p + 0x10));
        cur = *(unsigned int *)(p + 4) & ~0x900u;
        *(unsigned int *)(p + 4) = cur;
        if (cur == 0u)
            return;
    }
    if (flags & 0x4400u) {
        func_800878C0(voice_index, *(unsigned short *)(p + 0x26),
                      *(unsigned int *)(p + 0x18));
        cur = *(unsigned int *)(p + 4) & ~0x4400u;
        *(unsigned int *)(p + 4) = cur;
        if (cur == 0u)
            return;
    }
    if (flags & 0x9000u) {
        func_8008783C(voice_index, *(unsigned short *)(p + 0x20));
        func_80087864(voice_index, *(unsigned short *)(p + 0x22));
    }
    *(unsigned int *)(p + 4) = 0u;
}
