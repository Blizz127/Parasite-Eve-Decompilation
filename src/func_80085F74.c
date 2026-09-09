/* Psy-Q SpuSetCommonAttr donor (khasinski SPU_WriteVoiceRegs).
 * VRAM 0x80085F74 / file 0x76774 / size 0x37C.
 * Era: GCC 2.8.1-psx -O2 -G0 -mno-split-addresses, MASPSX three-word +
 * DISPATCH_FOLD=jtbl_800120BC,jtbl_800120DC.
 * Historical name writes common (master/CD/external) registers, not per-voice.
 * Structural .text match vs retail (fade-dump oracle / EXE).
 */
typedef struct SpuCommonSettings {
    unsigned int mask;
    unsigned short left, right;
    short leftMode, rightMode;
    short currentLeft, currentRight;
    short cdLeft, cdRight;
    int cdReverb, cdMix;
    short externalLeft, externalRight;
    int externalReverb, externalMix;
} SpuCommonSettings;

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
    volatile unsigned short master_volume_left;
    volatile unsigned short master_volume_right;
    volatile unsigned short reverb_volume_left;
    volatile unsigned short reverb_volume_right;
    unsigned char pad_188[0x1E];
    volatile unsigned short trans_addr;
    volatile unsigned short transfer_fifo;
    volatile unsigned short spucnt;
    unsigned char pad_1AC[2];
    volatile unsigned short transfer_status;
    volatile unsigned short cd_volume_left;
    volatile unsigned short cd_volume_right;
    volatile unsigned short external_volume_left;
    volatile unsigned short external_volume_right;
} SpuRegs;

extern SpuRegs *D_8009B3FC;

void func_80085F74(SpuCommonSettings *attr)
{
    unsigned short left = 0;
    register unsigned short right asm("$8");
    unsigned int mask;
    int all;
    unsigned int mode;

    asm volatile("" : "+r"(left));
    mask = attr->mask;
    all = mask == 0;
    right = 0;
    if (all || (mask & 1)) {
        if (all || (mask & 4)) {
            switch (attr->leftMode) {
            case 1:
                mode = 0x8000;
                break;
            case 2:
                mode = 0x9000;
                break;
            case 3:
                mode = 0xA000;
                break;
            case 4:
                mode = 0xB000;
                break;
            case 5:
                mode = 0xC000;
                break;
            case 6:
                mode = 0xD000;
                break;
            case 7:
                mode = 0xE000;
                break;
            case 0:
            default:
                left = attr->left;
                mode = 0;
                break;
            }
        } else {
            left = attr->left;
            mode = 0;
        }
        if (mode) {
            int value = (short)attr->left;
            left = value > 0x7F ? 0x7F : value < 0 ? 0 : attr->left;
        }
        D_8009B3FC->master_volume_left = (left & 0x7FFF) | mode;
    }
    if (all || (mask & 2)) {
        if (all || (mask & 8)) {
            switch (attr->rightMode) {
            case 1:
                mode = 0x8000;
                break;
            case 2:
                mode = 0x9000;
                break;
            case 3:
                mode = 0xA000;
                break;
            case 4:
                mode = 0xB000;
                break;
            case 5:
                mode = 0xC000;
                break;
            case 6:
                mode = 0xD000;
                break;
            case 7:
                mode = 0xE000;
                break;
            case 0:
            default:
                right = attr->right;
                mode = 0;
                break;
            }
        } else {
            right = attr->right;
            mode = 0;
        }
        if (mode) {
            right = (short)attr->right > 0x7F ? 0x7F
                  : (short)attr->right < 0     ? 0
                                               : attr->right;
        }
        D_8009B3FC->master_volume_right = (right & 0x7FFF) | mode;
    }
    if (all || (mask & 0x40))
        D_8009B3FC->cd_volume_left = attr->cdLeft;
    if (all || (mask & 0x80))
        D_8009B3FC->cd_volume_right = attr->cdRight;
    if (all || (mask & 0x400))
        D_8009B3FC->external_volume_left = attr->externalLeft;
    if (all || (mask & 0x800))
        D_8009B3FC->external_volume_right = attr->externalRight;
    if (all || (mask & 0x100)) {
        if (!attr->cdReverb)
            D_8009B3FC->spucnt &= ~4;
        else
            D_8009B3FC->spucnt |= 4;
    }
    if (all || (mask & 0x200)) {
        if (!attr->cdMix)
            D_8009B3FC->spucnt &= ~1;
        else
            D_8009B3FC->spucnt |= 1;
    }
    if (all || (mask & 0x1000)) {
        if (!attr->externalReverb)
            D_8009B3FC->spucnt &= ~8;
        else
            D_8009B3FC->spucnt |= 8;
    }
    if (all || (mask & 0x2000)) {
        if (!attr->externalMix)
            D_8009B3FC->spucnt &= ~2;
        else
            D_8009B3FC->spucnt |= 2;
    }
    asm volatile("" : : "r"(right));
}
