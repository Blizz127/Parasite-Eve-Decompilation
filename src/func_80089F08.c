/* SpuGetVoiceEnvelope — read SPU ENVX for one HW voice.
 * VRAM 0x80089F08 / file 0x7A708 / size 0x1C.
 *
 * khasinski psyq/libspu/s_gvex.c shape using D_8009B3FC (retail base ptr).
 * Semantic; byte-match not claimed (era pins may differ).
 */
typedef struct SpuVoiceRegs {
    volatile unsigned short volume_left;
    volatile unsigned short volume_right;
    volatile unsigned short pitch;
    volatile unsigned short start_address;
    volatile unsigned short adsr_low;
    volatile unsigned short adsr_high;
    volatile unsigned short envelope;
    volatile unsigned short repeat_address;
} SpuVoiceRegs;

typedef struct SpuRegs {
    SpuVoiceRegs voice[24];
} SpuRegs;

extern SpuRegs *D_8009B3FC;

void func_80089F08(unsigned int voice_index, short *out)
{
    *out = (voice_index + D_8009B3FC->voice)->envelope;
}
