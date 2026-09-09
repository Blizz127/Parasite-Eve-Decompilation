/* Akao_ProcessVoiceQueue — tick-start key/mask prep (khasinski donor, cleaned).
 * VRAM 0x80089328 / file 0x79B28 / size 0x3FC (sibling Spu_VoiceMaskCompose
 * at 0x80089724 shares the ~0x45C span through Akao_SetVoicePitch @ 0x80089784).
 *
 * Retail jals: Akao_UpdateVoiceEnvelopes (0x80089250), Akao_StepVoiceNote
 * (0x8008900C), Akao_SetVoiceKeyOn (0x80088980), Akao_WriteVoiceParam
 * (0x800878F0), plus volume/ADSR/key-on flush leaves.
 * Semantic transcription; byte-match not claimed.
 */
extern unsigned int D_800BCD50; /* g_SpuActiveVoiceMask */
extern unsigned int D_800BCD54;
extern unsigned int D_800BCD58;
extern unsigned int D_800BCD60;
extern unsigned short D_800BCD78;
extern unsigned int D_8009D2C4;
extern unsigned int *D_8009D2C8;
extern unsigned char D_800BA560[]; /* secondary 24-voice table */
extern unsigned char D_800B8AC0[]; /* primary 24-voice table */
extern unsigned char D_800BC000[]; /* stream/CD 12-voice table */
extern unsigned int D_800C0DD0;
extern unsigned int D_800C0DD4;
extern unsigned int D_800C0DD8;

void func_80089250(unsigned int blocked_mask); /* Akao_UpdateVoiceEnvelopes */
void func_8008900C(void *voices, unsigned int active_mask,
                   unsigned int restart_mask, unsigned int *key_on_mask);
void func_80088980(void *voice, unsigned int voice_mask); /* Akao_SetVoiceKeyOn */
void func_800878F0(unsigned int hw_index, void *voice_param);
void func_80089F28(short left, short right); /* Akao_SetMasterVolume */
void func_80089EB8(unsigned int value);      /* SpuSetNoiseClock / 1AA field */
void func_80089B48(void);                    /* Akao_SetVoiceAdsr */
void func_80089980(void);                    /* Akao_SetVoiceVolume */
void func_80089D10(void);                    /* Akao_SetVoiceStartAddr */
void func_80087744(unsigned int mask);       /* Spu_WriteReverbEnable */
void func_80087760(unsigned int mask);       /* Spu_WriteNoiseEnable */
void func_8008777C(unsigned int mask);       /* Spu_WriteFmEnable */
void func_8008770C(unsigned int mask);       /* Spu_WriteKeyOn */

void func_80089328(void)
{
    unsigned int key_on_mask = 0u;
    unsigned int blocked_mask;
    unsigned int secondary_pending;
    unsigned int secondary_restart;
    unsigned int primary_pending;
    unsigned int primary_restart;
    unsigned int flags;
    unsigned int *state;
    unsigned int bit;
    unsigned char *voice;
    unsigned char *voice_tail;

    blocked_mask = D_800BCD50 | D_800BCD60;

    state = D_8009D2C8;
    if (((state[1] & state[4]) | (state[0x1B] & state[0x1E])) != 0u)
        func_80089250(blocked_mask);

    state = D_8009D2C8;
    secondary_pending =
        (state[0x1B] & state[0x1F]) & ~(state[0x1D] & blocked_mask);
    secondary_restart =
        (secondary_pending & state[0x1D]) & ~blocked_mask;
    if ((secondary_pending & state[0x1C]) != 0u) {
        D_8009D2C8 = (unsigned int *)((unsigned char *)state + 0x68);
        func_8008900C(D_800BA560, secondary_pending & state[0x1C],
                      secondary_restart, &key_on_mask);

        state = D_8009D2C8;
        D_8009D2C8 = (unsigned int *)((unsigned char *)state - 0x68);
        secondary_pending &= ~state[2];
        state[4] &= ~state[2];
    }

    state = D_8009D2C8;
    primary_pending =
        (state[1] & state[5]) &
        ~(state[3] & (secondary_restart | blocked_mask));
    primary_restart =
        (primary_pending & state[3]) & ~(secondary_restart | blocked_mask);
    if ((primary_pending & state[2]) != 0u) {
        func_8008900C(D_800B8AC0, primary_pending & state[2], primary_restart,
                      &key_on_mask);

        state = D_8009D2C8;
        primary_pending &= ~state[2];
        state[4] &= ~state[2];
    }

    if (secondary_pending != 0u) {
        state = D_8009D2C8;
        D_8009D2C8 = (unsigned int *)((unsigned char *)state + 0x68);
        func_8008900C(D_800BA560, secondary_pending,
                      secondary_restart & ~primary_restart, &key_on_mask);
        D_8009D2C8[4] = 0u;
        D_8009D2C8 = (unsigned int *)((unsigned char *)D_8009D2C8 - 0x68);
    }

    if (primary_pending != 0u) {
        func_8008900C(D_800B8AC0, primary_pending, primary_restart,
                      &key_on_mask);
        D_8009D2C8[4] = 0u;
    }

    primary_pending = D_800BCD50 & D_800BCD58;
    if (primary_pending != 0u) {
        bit = 0x1000u;
        voice = D_800BC000;
        voice_tail = voice + 0x38;
        key_on_mask |= D_800BCD54;

        while (primary_pending != 0u) {
            if ((primary_pending & bit) != 0u) {
                func_80088980(voice, bit);
                if (*(unsigned int *)(voice_tail + 0xBC) != 0u) {
                    func_800878F0(*(unsigned int *)(voice_tail + 0xB8),
                                 voice + 0xF0);
                }
                primary_pending &= ~bit;
            }
            bit <<= 1;
            voice_tail += 0x11C;
            voice += 0x11C;
        }

        D_800BCD54 = 0u;
    }

    flags = D_8009D2C4;
    if ((flags & 0x80u) != 0u) {
        short vol = *(short *)((unsigned char *)D_8009D2C8 + 0x42);
        func_80089F28(vol, vol);
        D_8009D2C4 &= ~0x80u;
    }

    if ((flags & 0x10u) != 0u) {
        if (D_800BCD50 != 0u)
            func_80089EB8(D_800BCD78);
        else
            func_80089EB8(*(unsigned short *)((unsigned char *)D_8009D2C8 + 0x5A));
        D_8009D2C4 &= ~0x10u;
    }

    if ((flags & 0x100u) != 0u) {
        func_80089B48();
        func_80089980();
        func_80089D10();
        func_80087744(D_800C0DD0);
        func_80087760(D_800C0DD4);
        func_8008777C(D_800C0DD8);
        D_8009D2C4 &= ~0x100u;
    }

    if (key_on_mask != 0u)
        func_8008770C(key_on_mask);
}

/* Spu_VoiceMaskCompose — VRAM 0x80089724 / size 0x60.
 * Maps logical track bits → hardware voice bits via voice[+0xF0]. */
void func_80089724(void *tracks, unsigned int *mask_out, unsigned int mask,
                   unsigned int mask_keep)
{
    unsigned int bit = 1u;
    unsigned char *track = (unsigned char *)tracks;

    while (mask != 0u) {
        if ((mask & bit) != 0u) {
            unsigned int idx = *(unsigned int *)(track + 0xF0);
            if (idx < 24u)
                *mask_out |= 1u << idx;
        }
        mask &= ~bit;
        track += 0x11C;
        bit <<= 1;
    }

    *mask_out &= mask_keep;
}
