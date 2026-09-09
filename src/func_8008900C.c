/* Akao_StepVoiceNote — assign HW voices / key-on bits for pending tracks.
 * VRAM 0x8008900C / file 0x7980C / size 0x20C.
 *
 * Retail jals: Akao_SetVoiceKeyOff (0x80088344), Akao_WriteVoiceParam
 * (0x800878F0). Software envelope table at 0x800B002C (8 bytes / HW voice)
 * allocates free slots when the restart bit is clear.
 * Semantic transcription from retail; byte-match not claimed.
 */
extern unsigned int *D_8009D2C8;
extern unsigned int D_8009D2C4;
extern unsigned int D_8009D2B8;
extern short D_800B002C[]; /* envelope mirror — s16 + pad, stride 8 */

void func_80088344(void *voice, unsigned int voice_mask, unsigned int track_index);
void func_800878F0(unsigned int hw_index, void *voice_param);

void func_8008900C(void *voices, unsigned int active_mask,
                   unsigned int restart_mask, unsigned int *key_on_mask)
{
    unsigned char *voice = (unsigned char *)voices;
    unsigned char *voice_tail = voice + 0x38;
    unsigned int bit = 1u;
    unsigned int track = 0u;
    unsigned int gated = active_mask & D_8009D2C8[4]; /* +0x10 */
    const unsigned int pending_cluster = 0x1FF93u;

    while (active_mask != 0u) {
        if ((active_mask & bit) != 0u) {
            unsigned int pending;

            func_80088344(voice, bit, track);
            pending = *(unsigned int *)(voice_tail + 0xBCu); /* voice+0xF4 */
            if (pending != 0u) {
                if ((gated & bit) != 0u) {
                    if ((restart_mask & bit) != 0u) {
                        *key_on_mask |= 1u << track;
                        *(unsigned int *)(voice_tail + 0xB8u) = track; /* +0xF0 */
                        *(unsigned int *)(voice_tail + 0xBCu) |= pending_cluster;
                    } else {
                        unsigned int hw = 0u;
                        short *envelope = D_800B002C;

                        for (;;) {
                            if (envelope[0] == 0) {
                                *(unsigned int *)(voice_tail + 0xBCu) |=
                                    pending_cluster;
                                *key_on_mask |= 1u << hw;
                                *(unsigned int *)(voice_tail + 0xB8u) = hw;
                                envelope[0] = 0x7FFF;
                                D_8009D2C4 |= 0x100u;
                                break;
                            }
                            hw += 1u;
                            envelope += 4;
                            if (hw == 24u) {
                                *(unsigned int *)(voice_tail + 0xB8u) = 24u;
                                D_8009D2C8[0] |= 1u;
                                break;
                            }
                        }
                    }
                }

                if ((D_8009D2B8 & bit) != 0u) {
                    *(short *)(voice_tail + 0xE2u) = 0; /* +0x11A */
                    *(short *)(voice_tail + 0xE0u) = 0; /* +0x118 */
                }

                {
                    unsigned int hw = *(unsigned int *)(voice_tail + 0xB8u);
                    if (hw < 24u)
                        func_800878F0(hw, voice + 0xF0u);
                }
            }
            active_mask &= ~bit;
        }
        bit <<= 1;
        voice_tail += 0x11Cu;
        voice += 0x11Cu;
        track += 1u;
    }
}
