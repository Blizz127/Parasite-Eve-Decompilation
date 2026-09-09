/* Akao_RemoveVoice + Akao_UpdateVoiceEnvelopes (khasinski voice_envelopes.c).
 * RemoveVoice VRAM 0x80089218 / file 0x79A18 / size 0x38.
 * UpdateVoiceEnvelopes VRAM 0x80089250 / file 0x79A50 / size 0xD8.
 *
 * Polls func_80089F08 (ENVX); frees HW slots whose envelope has decayed
 * to 0 via RemoveVoice on primary (0x800B8AC0) and secondary (+0x1AA0) tables.
 * Semantic; byte-match not claimed.
 */
extern unsigned int *D_8009D2C8;
extern unsigned char D_800B8AC0[];
extern short D_800B002C[]; /* envelope mirror, stride 8 */

void func_80089F08(unsigned int voice_index, short *out); /* SpuGetVoiceEnvelope */

void func_80089218(void *voices, unsigned int hw_index)
{
    unsigned int i = 0u;
    unsigned int *slot = (unsigned int *)((unsigned char *)voices + 0xF0u);

    do {
        if (*slot == hw_index)
            *slot = 24u;
        i += 1u;
        slot = (unsigned int *)((unsigned char *)slot + 0x11Cu);
    } while (i < 24u);
}

void func_80089250(unsigned int blocked_mask)
{
    unsigned int voice_index = 0u;
    unsigned int bit = 1u;
    unsigned char *voice_base = D_800B8AC0;
    short *envelope = D_800B002C;
    unsigned int protected_mask;
    unsigned int *state = D_8009D2C8;

    protected_mask =
        ((state[1] & state[3]) | (state[0x1B] & state[0x1D])) | blocked_mask;

    do {
        if ((protected_mask & (bit << voice_index)) != 0u) {
            *envelope = 0x7FFF;
        } else {
            func_80089F08(voice_index, envelope);
            if (*envelope == 0) {
                func_80089218(voice_base, voice_index);
                func_80089218(voice_base + 0x1AA0, voice_index);
            }
        }
        voice_index += 1u;
        envelope += 4;
    } while (voice_index < 24u);
}
