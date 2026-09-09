/* Psy-Q SpuSetVoiceAttr donor (khasinski Spu_SetVoiceAttr).
 * VRAM 0x800862F4 / file 0x76AF4 / size 0x16C.
 * Era: GCC 2.8.1-psx -O2 -G0 -mno-split-addresses, MASPSX three-word +
 * DISPATCH_FOLD=jtbl_800120FC,jtbl_8001211C.
 * Structural .text match vs retail (fade-dump oracle / EXE); j/lui immediates
 * are relocation fields resolved at link against the shared 0x800 jtbl pool.
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

typedef struct SpuVoiceWaitWork {
    volatile int i;
    volatile int value;
} SpuVoiceWaitWork;

void func_800862F4(int voice, unsigned short left, unsigned short right,
                   short leftMode, unsigned short rightMode)
{
    SpuVoiceWaitWork work;
    unsigned int leftFlags;
    unsigned int rightFlags;
    int current;

    left &= 0x7FFF;
    leftFlags = 0;
    /* Index is measured in halfword registers. */
    voice *= (int)(sizeof(SpuVoiceRegs) / sizeof(unsigned short));
    switch ((short)(leftMode - 1)) {
    case 0:
        leftFlags = 0x8000;
        break;
    case 1:
        leftFlags = 0x9000;
        break;
    case 2:
        leftFlags = 0xA000;
        break;
    case 3:
        leftFlags = 0xB000;
        break;
    case 4:
        leftFlags = 0xC000;
        break;
    case 5:
        leftFlags = 0xD000;
        break;
    case 6:
        leftFlags = 0xE000;
        break;
    }
    leftMode = (short)(right & 0x7FFF);
    rightFlags = 0;
    ((SpuVoiceRegs *)((unsigned short *)D_8009B3FC + voice))->volume_left =
        left | leftFlags;
    switch ((short)(rightMode - 1)) {
    case 0:
        rightFlags = 0x8000;
        break;
    case 1:
        rightFlags = 0x9000;
        break;
    case 2:
        rightFlags = 0xA000;
        break;
    case 3:
        rightFlags = 0xB000;
        break;
    case 4:
        rightFlags = 0xC000;
        break;
    case 5:
        rightFlags = 0xD000;
        break;
    case 6:
        rightFlags = 0xE000;
        break;
    }
    ((SpuVoiceRegs *)((unsigned short *)D_8009B3FC + voice))->volume_right =
        (unsigned short)leftMode | rightFlags;
    work.value = 1;
    asm volatile("" : : : "memory");
    *(int *)&work.i = 0;
    while (work.i < 2) {
        current = work.value;
        work.value = (((current << 1) + current) << 2) + current;
        work.i = work.i + 1;
    }
}
