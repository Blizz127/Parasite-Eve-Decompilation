/* Akao_SetVoicePitch — key-off / pitch-mask flush (khasinski donor, cleaned).
 * VRAM 0x80089784 / file 0x79F84 / size 0x1DC.
 *
 * Composes HW key-off bits from sequencer state[+0x18] pending masks across
 * primary/secondary banks, merges D_800BCD5C, then Spu_WriteKeyOff (87728).
 * Semantic transcription; byte-match not claimed (candidate used register asm).
 */
extern unsigned int D_800BCD50;
extern unsigned int D_800BCD5C;
extern unsigned int D_800BCD60;
extern unsigned int *D_8009D2C8;
extern unsigned char D_800BA560[];
extern unsigned char D_800B8AC0[];

void func_80089724(void *tracks, unsigned int *mask_out, unsigned int mask,
                   unsigned int mask_keep); /* Spu_VoiceMaskCompose */
void func_80087728(unsigned int mask);     /* Spu_WriteKeyOff */

void func_80089784(void)
{
    unsigned int mask_out = 0u;
    unsigned int mask_keep;
    unsigned int pending_secondary;
    unsigned int pending_primary;
    unsigned int mask;
    unsigned int *state;
    unsigned int blocked;

    state = D_8009D2C8;
    blocked = D_800BCD50 | D_800BCD60;
    mask_keep = ~blocked;
    pending_secondary = state[0x1B] & state[0x20];
    mask = pending_secondary & state[0x1C];
    if (mask != 0u) {
        D_8009D2C8 = (unsigned int *)((unsigned char *)state + 0x68);
        func_80089724(D_800BA560, &mask_out, mask, mask_keep);
        state = D_8009D2C8;
        D_8009D2C8 = (unsigned int *)((unsigned char *)state - 0x68);
        pending_secondary &= ~state[2];
        state[6] &= ~state[2];
    }

    state = D_8009D2C8;
    pending_primary = state[1] & state[6];
    mask = pending_primary & state[2];
    if (mask != 0u) {
        func_80089724(D_800B8AC0, &mask_out, mask, mask_keep);
        state = D_8009D2C8;
        pending_primary &= ~state[2];
        state[6] &= ~state[2];
    }

    if (pending_secondary != 0u) {
        state = D_8009D2C8;
        D_8009D2C8 = (unsigned int *)((unsigned char *)state + 0x68);
        func_80089724(D_800BA560, &mask_out, pending_secondary, mask_keep);
        D_8009D2C8[6] = 0u;
        D_8009D2C8 = (unsigned int *)((unsigned char *)D_8009D2C8 - 0x68);
    }

    if (pending_primary != 0u) {
        func_80089724(D_800B8AC0, &mask_out, pending_primary, mask_keep);
        D_8009D2C8[6] = 0u;
    }

    mask_out |= D_800BCD5C;
    D_800BCD5C = 0u;
    if (mask_out != 0u)
        func_80087728(mask_out);
}
