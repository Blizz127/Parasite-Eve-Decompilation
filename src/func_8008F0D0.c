/* SeqOp_SetVoiceInstrument — apply instrument ADSR/sample to a track.
 * VRAM 0x8008F0D0 / file 0x7F8D0 / size 0xA8.
 * khasinski seq_op.c donor; semantic/near-exact for host + matching pipeline.
 */
void func_8008F0D0(unsigned char *track, unsigned char *instrument,
                   unsigned int sample_header)
{
    unsigned int flags;
    unsigned int update;

    *(unsigned int *)(track + 0xF8) = sample_header;
    *(unsigned int *)(track + 0xFC) = *(unsigned int *)(instrument + 4);
    *(unsigned short *)(track + 0x10E) = instrument[8];
    *(unsigned short *)(track + 0x110) = instrument[9];
    *(unsigned short *)(track + 0x112) = instrument[10];
    *(unsigned short *)(track + 0x114) = instrument[11];
    *(unsigned int *)(track + 0x100) = instrument[13];
    *(unsigned int *)(track + 0x104) = instrument[14];

    flags = *(unsigned int *)(track + 0x38);
    if (flags & 0x200u) {
        update = *(unsigned int *)(track + 0xF4);
        *(unsigned int *)(track + 0xF4) = update | 0x0001BB80u;
    } else {
        *(unsigned short *)(track + 0x116) = instrument[12];
        update = *(unsigned int *)(track + 0xF4);
        *(unsigned int *)(track + 0xF4) = update | 0x0001FF80u;
        *(unsigned int *)(track + 0x108) = instrument[15];
    }
}
