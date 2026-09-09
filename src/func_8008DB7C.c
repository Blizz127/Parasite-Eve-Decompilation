/* Akao_Tick — music/score sequencer (khasinski Akao_Tick).
 * VRAM 0x8008DB7C / file 0x7E37C / size 0x6C0.
 *
 * Semantic transcription from retail disassembly for host score sequencing.
 * Byte-match not claimed (khasinski still carries this span as asm). The
 * 8E1F0..8E208 command-drain tail is the final gated func_8008CA84 call.
 */
extern unsigned char D_8009D2D2;
extern unsigned int D_8009D2C4;
extern unsigned int D_8009D2C8;
extern unsigned int D_8009D2DC;
extern unsigned int D_8009D22C;
extern unsigned int D_8009D268;
extern unsigned int D_800BCD50;
extern unsigned short D_800BCD66;
extern unsigned int D_800BCD68;
extern unsigned int D_800BCD5C;
extern unsigned int D_800BCD58;

extern unsigned char D_800B8AC0[]; /* primary 24-voice table */
extern unsigned char D_800BA560[]; /* secondary 24-voice table */
extern unsigned char D_800BC000[]; /* stream/CD 12-voice table */

void func_80089328(void);
void func_8008E8D0(void *voice, unsigned int mask);
void func_80087AA8(void *voice, unsigned int mask);
void func_80087FA0(void *voice, unsigned int mask);
void func_8008D820(unsigned int *dst, unsigned int *src, unsigned int nbytes);
void func_8008CA84(void);
void func_8008D844(void);
void func_80089784(void);

static unsigned int scale_tempo(unsigned int tempo, unsigned int factor)
{
    unsigned int product;

    if (factor == 0)
        return tempo;
    product = tempo * factor;
    if (factor < 0x80u)
        return tempo + (product >> 7);
    return product >> 8;
}

static void tick_primary_or_secondary_voices(unsigned char *base,
                                             unsigned int mask)
{
    unsigned int bit = 1u;
    unsigned char *voice = base;

    while (mask != 0u) {
        if (mask & bit) {
            unsigned short *t0 = (unsigned short *)(voice + 0x56);
            unsigned short *t1 = (unsigned short *)(voice + 0x58);
            unsigned short v0 = (unsigned short)(*t0 - 1u);
            unsigned short v1;

            *t0 = v0;
            v1 = (unsigned short)(*t1 - 1u);
            *t1 = v1;
            if (v0 == 0u) {
                func_8008E8D0(voice, bit);
            } else if (v1 == 0u) {
                unsigned int *st = (unsigned int *)D_8009D2C8;
                st[6] |= bit;
                st[5] &= ~bit;
            }
            func_80087AA8(voice, bit);
            mask ^= bit;
        }
        voice += 0x11Cu;
        bit <<= 1;
    }
}

static void tick_primary_slides(int mark_dirty)
{
    unsigned int *st = (unsigned int *)D_8009D2C8;
    unsigned short slide = *(unsigned short *)((char *)st + 0x52);

    if (slide != 0u) {
        *(unsigned short *)((char *)st + 0x52) = (unsigned short)(slide - 1u);
        st[8] += st[9];
        st = (unsigned int *)D_8009D2C8;
    }

    slide = *(unsigned short *)((char *)st + 0x58);
    if (slide != 0u) {
        *(unsigned short *)((char *)st + 0x58) = (unsigned short)(slide - 1u);
        st[0x10] += st[0x11];
        if (mark_dirty)
            D_8009D2C4 |= 0x80u;
        st = (unsigned int *)D_8009D2C8;
    }

    {
        unsigned short period = *(unsigned short *)((char *)st + 0x60);
        if (period != 0u) {
            unsigned short count = *(unsigned short *)((char *)st + 0x62);
            count = (unsigned short)(count + 1u);
            *(unsigned short *)((char *)st + 0x62) = count;
            if (count == period) {
                unsigned short a = *(unsigned short *)((char *)st + 0x5E);
                unsigned short b = *(unsigned short *)((char *)st + 0x5C);
                *(unsigned short *)((char *)st + 0x62) = 0;
                a = (unsigned short)(a + 1u);
                *(unsigned short *)((char *)st + 0x5E) = a;
                if (a == b) {
                    unsigned short c = *(unsigned short *)((char *)st + 0x64);
                    *(unsigned short *)((char *)st + 0x5E) = 0;
                    c = (unsigned short)(c + 1u);
                    *(unsigned short *)((char *)st + 0x64) = c;
                    if (D_8009D22C != 0u)
                        D_8009D22C -= 1u;
                }
            }
        }
    }
}

static void tick_stream_voices(unsigned int mask)
{
    unsigned int bit = 0x1000u;
    unsigned char *voice = D_800BC000;

    while (mask != 0u) {
        if (mask & bit) {
            int run = 1;
            if ((D_8009D2DC & 2u) != 0u) {
                unsigned int flags = *(unsigned int *)(voice + 0x2C);
                if ((flags & 0x2000000u) == 0u)
                    run = 0;
            }
            if (run) {
                unsigned short *t0 = (unsigned short *)(voice + 0x56);
                unsigned short *t1 = (unsigned short *)(voice + 0x58);
                unsigned int *ctr = (unsigned int *)(voice + 0x50);
                unsigned short v0 = (unsigned short)(*t0 - 1u);
                unsigned short v1 = (unsigned short)(*t1 - 1u);

                *t0 = v0;
                *t1 = v1;
                *ctr += 1u;
                if (v0 == 0u) {
                    func_8008E8D0(voice, bit);
                } else if (v1 == 0u) {
                    D_800BCD5C |= bit;
                    D_800BCD58 &= ~bit;
                }
                func_80087FA0(voice, bit);
            }
            mask ^= bit;
        }
        voice += 0x11Cu;
        bit <<= 1;
    }
}

void func_8008DB7C(void)
{
    unsigned int *st;
    unsigned int active;
    unsigned int tempo;
    unsigned int acc;

    func_80089328();

    st = (unsigned int *)D_8009D2C8;
    active = st[1];
    if (active != 0u) {
        tempo = *(unsigned short *)((char *)st + 0x22);
        tempo = scale_tempo(tempo, D_8009D2D2);
        acc = st[0xA] + tempo;
        st[0xA] = acc;
        if ((acc & 0xFFFF0000u) == 0u && (D_8009D2DC & 4u) == 0u)
            goto after_primary;
        st[0xA] = acc & 0xFFFFu;
        do {
            tick_primary_or_secondary_voices(D_800B8AC0,
                                             ((unsigned int *)D_8009D2C8)[1]);
            tick_primary_slides(1);
        } while (D_8009D22C != 0u);
    }

after_primary:
    st = (unsigned int *)D_8009D2C8;
    if (st[0x1B] != 0u) {
        unsigned int *primary = st;
        unsigned int secondary = (unsigned int)primary + 0x68u;
        unsigned int t;

        t = *(unsigned short *)((char *)primary + 0x8A);
        D_8009D2C8 = secondary;
        st = (unsigned int *)secondary;
        t = scale_tempo(t, D_8009D2D2);
        acc = st[0xA] + t;
        st[0xA] = acc;
        if (!((acc & 0xFFFF0000u) == 0u && (D_8009D2DC & 4u) == 0u)) {
            st[0xA] = acc & 0xFFFFu;
            tick_primary_or_secondary_voices(D_800BA560, st[1]);
            tick_primary_slides(0);
        }
        D_8009D2C8 = D_8009D2C8 - 0x68u;
    }

    st = (unsigned int *)D_8009D2C8;
    if (st[1] == 0u && st[7] == 0u && st[0x1B] != 0u) {
        func_8008D820((unsigned int *)((char *)st + 0x68), st, 0x68u);
        func_8008D820((unsigned int *)D_800BA560, (unsigned int *)D_800B8AC0,
                      0x1AA0u);
        st = (unsigned int *)D_8009D2C8;
        *(unsigned short *)((char *)st + 0xBC) = 0;
        st[0x1B] = 0;
    }

    active = D_800BCD50;
    if (active != 0u) {
        acc = D_800BCD68 + D_800BCD66;
        D_800BCD68 = acc;
        if ((acc & 0xFFFF0000u) == 0u && (D_8009D2DC & 4u) == 0u)
            goto drain;
        D_800BCD68 = acc & 0xFFFFu;
        tick_stream_voices(active);
    }

drain:
    /* 8E1F0..8E208: drain commands when the audio lock is clear. */
    if (D_8009D268 == 0u)
        func_8008CA84();
    func_8008D844();
    func_80089784();
}
