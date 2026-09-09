/* Akao_StepSampleLoader / seq bytecode interpreter (khasinski name).
 * VRAM 0x8008E8D0 / file 0x7F0D0 / size 0x800 (to SeqOp_SetVoiceInstrument).
 *
 * Called from Akao_Tick when voice gate +0x56 hits 0. Fetches seq opcodes from
 * track->pc, dispatches via D_8009C8F0 (ops >= 0xA0) / D_8009CCF0 (FC sub-ops),
 * and programs note duration into +0x56/+0x58. Semantic transcription from
 * retail disassembly; byte-match not claimed (handler callees mostly still asm).
 */
extern unsigned short D_8009B8DC[]; /* note length table (12 entries) */
extern void (*D_8009C8F0[])(void *voice, unsigned int mask); /* opcode >= 0xA0 */
extern void (*D_8009CCF0[])(void *voice, unsigned int mask); /* FC sub-ops */

extern unsigned int D_8009D2C8;
extern unsigned int D_800BCD54;
extern unsigned int D_800BCD58;
extern unsigned int D_800BCD5C;

unsigned int func_8008E2DC(void *voice); /* Akao_LookupSampleBankByte */
void func_8008E7F4(void *voice, unsigned int note_mod); /* Akao_SetNotePitchBounded */
unsigned int func_8008E840(unsigned int note, unsigned int key, int detune);
void func_8008F0D0(void *voice, void *instrument, unsigned int sample_header);
void func_80089B28(void); /* mark audio dirty */

static unsigned int note_length_from_opcode(unsigned int opcode)
{
    /* retail: opcode % 14 via magic 0xBA2E8BA3, index D_8009B8DC */
    return D_8009B8DC[opcode - (opcode / 14u) * 14u];
}

static void apply_gate_duration(unsigned char *voice, unsigned int len,
                                unsigned int art_byte, int force_full)
{
    unsigned short gate = (unsigned short)len;
    unsigned short *t56 = (unsigned short *)(voice + 0x56);
    unsigned short *t58 = (unsigned short *)(voice + 0x58);

    *t56 = gate;
    if (!force_full && art_byte < 0x8Fu) {
        if (art_byte >= 0x84u ||
            ((*(unsigned short *)(voice + 0x84) & 5u) != 0u)) {
            /* keep full gate as key-off timer too */
        } else {
            gate = (unsigned short)(gate - 2u);
        }
    } else if (!force_full) {
        if (((*(unsigned short *)(voice + 0x84) & 5u) == 0u))
            gate = (unsigned short)(gate - 2u);
    }
    *t58 = gate;
}

void func_8008E8D0(void *voice_arg, unsigned int mask)
{
    unsigned char *voice = (unsigned char *)voice_arg;
    unsigned int opcode;
    unsigned int art;
    unsigned char *pc;
    unsigned int loops = 0u;

    for (;;) {
        pc = *(unsigned char **)voice;
        opcode = *pc;
        *(unsigned char **)voice = pc + 1;

        if (opcode < 0xA0u)
            break;

        if (opcode == 0xFCu) {
            unsigned int sub = pc[1];
            *(unsigned char **)voice = pc + 2;
            D_8009CCF0[sub](voice, mask);
        } else if (opcode == 0xCAu) {
            if ((*(unsigned int *)(voice + 0x38) & 0x200000u) != 0u) {
                D_800BCD5C |= mask;
                opcode = 0xA0u;
            }
            D_8009C8F0[opcode](voice, mask);
        } else {
            D_8009C8F0[opcode](voice, mask);
        }

        if (opcode < 0xA0u)
            break;
        if (opcode == 0xA0u)
            break;
        if (++loops > 0x100u)
            return;
    }

    /* --- opcode < 0xA0 note / 0xA0 rest path --- */
    if (opcode == 0xA0u) {
        if (*(unsigned short *)(voice + 0x54) == 0u) {
            unsigned int *st = (unsigned int *)D_8009D2C8;
            if ((st[5] & mask) != 0u &&
                *(unsigned int *)(voice + 0xF0) < 24u) {
                st[6] |= mask;
            }
        }
        return;
    }

    art = func_8008E2DC(voice) & 0xFFu;

    {
        short leg = *(short *)(voice + 0xD2);
        if (leg != 0) {
            *(unsigned short *)(voice + 0x58) = (unsigned short)leg;
            *(unsigned short *)(voice + 0x56) = (unsigned short)leg;
        }
    }

    if (*(unsigned short *)(voice + 0x56) != 0u) {
        /* gate already programmed by a prior legamento path */
        if (art < 0x8Fu) {
            if (art < 0x84u &&
                ((*(unsigned short *)(voice + 0x84) & 5u) == 0u)) {
                unsigned short k = *(unsigned short *)(voice + 0x58);
                *(unsigned short *)(voice + 0x58) = (unsigned short)(k - 2u);
            }
        }
    } else {
        unsigned int len = note_length_from_opcode(opcode);
        apply_gate_duration(voice, len, art, 0);
    }

    {
        unsigned int pending = *(unsigned int *)(voice + 0xF4) | 0x4000u;
        *(unsigned int *)(voice + 0xF4) = pending;
        *(unsigned short *)(voice + 0xD0) = *(unsigned short *)(voice + 0x56);
    }

    if (art < 0x8Fu) {
        *(unsigned int *)(voice + 0x38) &= ~0x40u;
    } else {
        *(unsigned int *)(voice + 0x38) |= 0x40u;
    }

    /* Rest-class opcodes 0x84..0x8E: key-off style, skip pitch program */
    if (opcode >= 0x8Fu) {
        /* fall through to full note */
    } else if (opcode >= 0x84u) {
        if (*(unsigned short *)(voice + 0x54) == 0u) {
            unsigned int *st = (unsigned int *)D_8009D2C8;
            if ((st[5] & mask) != 0u &&
                *(unsigned int *)(voice + 0xF0) < 24u)
                st[6] |= mask;
        }
        *(unsigned short *)(voice + 0x82) = 0;
        *(unsigned short *)(voice + 0xE8) = 0;
        *(unsigned short *)(voice + 0xEA) = 0;
        *(unsigned short *)(voice + 0x84) =
            (unsigned short)(*(unsigned short *)(voice + 0x84) & 0xFFFDu);
        return;
    }

    /* Full note-on: instrument + pitch + key-on mask */
    {
        unsigned int flags = *(unsigned int *)(voice + 0x38);
        unsigned int note_mod = note_length_from_opcode(opcode); /* reused rem */

        /* recompute note class index (opcode % 14) like retail s1 after div */
        note_mod = opcode - (opcode / 14u) * 14u;

        if (flags & 8u) {
            /* alternate bank / layered voice path (uses +0x14 program) */
            unsigned char *prog;
            unsigned int *st = (unsigned int *)D_8009D2C8;
            unsigned int note;
            int octave_bias = 0;

            st[4] |= mask;
            if ((st[5] & mask) != 0u &&
                *(unsigned int *)(voice + 0xF0) < 24u)
                st[6] |= mask;

            /* secondary index via %12 on note_mod */
            {
                unsigned int rem = note_mod - (note_mod / 12u) * 12u;
                prog = *(unsigned char **)(voice + 0x14) + rem * 6u;
            }
            if ((*(unsigned int *)st & 0x100u) != 0u)
                octave_bias = -0x30; /* bit inverted in retail; see disasm */

            note = prog[0];
            if (note >= 0x20u)
                note = (unsigned int)((int)note - octave_bias);
            /* simplified: match PlayNote instrument select */
            {
                unsigned int np = *(unsigned short *)(voice + 0x5A);
                unsigned char *table =
                    (unsigned char *)0x800B2900 + (note << 6);
                if ((unsigned int)prog[0] >= 0x20u &&
                    (*(unsigned int *)st & 0x100u))
                    note = prog[0] + 0x30u;
                else
                    note = prog[0];
                if (np != note) {
                    *(unsigned short *)(voice + 0x5A) = (unsigned short)note;
                    func_8008F0D0(voice, table, *(unsigned int *)table);
                }
                {
                    unsigned int pitch = func_8008E840(
                        *(unsigned short *)(voice + 0x5A), prog[1],
                        *(short *)(voice + 0xE0));
                    unsigned int vol = *(unsigned short *)(voice + 0x6A);
                    unsigned int expr =
                        ((unsigned int)prog[3] << 8) | prog[2];
                    *(unsigned int *)(voice + 0x44) = (vol * expr) << 2;
                    {
                        unsigned int pan = (prog[4] + 0x40u) & 0xFFu;
                        *(unsigned short *)(voice + 0x76) =
                            (unsigned short)(pan << 8);
                    }
                    if (prog[5] != 0u)
                        st[0xE] |= mask; /* +0x38 dirty */
                    else
                        st[0xE] &= ~mask;
                    func_80089B28();
                    *(unsigned int *)(voice + 0x30) = pitch;
                }
            }
        } else {
            /* primary note path */
            unsigned int nbase = note_mod;
            nbase += (*(unsigned short *)(voice + 0x7C) * 6u);

            if ((*(unsigned short *)(voice + 0x84) & 2u) == 0u) {
                if (*(unsigned short *)(voice + 0x54) == 0u) {
                    if (flags & 0x1000u)
                        func_8008E7F4(voice, nbase);
                    {
                        unsigned int *st = (unsigned int *)D_8009D2C8;
                        st[4] |= mask;
                        if ((st[5] & mask) != 0u &&
                            *(unsigned int *)(voice + 0xF0) < 24u)
                            st[6] |= mask;
                    }
                } else {
                    D_800BCD54 |= mask;
                }
                *(unsigned short *)(voice + 0x7A) = 0;
            }

            {
                unsigned int key = nbase;
                if (*(unsigned short *)(voice + 0x82) != 0u &&
                    *(unsigned short *)(voice + 0x80) != 0u) {
                    /* vibrato/portamento blend — keep host-simple */
                    key = *(unsigned short *)(voice + 0x80);
                } else {
                    key = nbase + (unsigned int)*(short *)(voice + 0xDE);
                    *(unsigned short *)(voice + 0xE2) = (unsigned short)key;
                }
                {
                    unsigned int pitch = func_8008E840(
                        *(unsigned short *)(voice + 0x5A), key,
                        *(short *)(voice + 0xE0));
                    *(unsigned int *)(voice + 0x30) = pitch;
                }
            }
        }

        if (*(unsigned short *)(voice + 0x54) == 0u) {
            unsigned int *st = (unsigned int *)D_8009D2C8;
            st[5] |= mask;
        } else {
            D_800BCD58 |= mask;
        }

        *(unsigned int *)(voice + 0xF4) |= 0x13u;
    }
}
