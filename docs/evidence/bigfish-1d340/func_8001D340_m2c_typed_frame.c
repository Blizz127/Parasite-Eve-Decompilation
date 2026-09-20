/* Typed compile of the m2c skeleton for func_8001D340 (0xDB40, 0x2194).
 * EVIDENCE ONLY -- not a registered match.  Generated from
 * func_8001D340_m2c_draft.c by replacing each `BASE->unkNN` with a
 * width-correct absolute access macro (widths read from the retail
 * loads/stores).  Struct-typed in spirit but expressed as byte offsets
 * because the Actor fields at 0x28/0x2A and 0x30/0x32 overlap.
 */
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef signed char    s8;
typedef short          s16;
typedef int            s32;
#ifndef NULL
#define NULL 0
#endif

#define RB(b,o)   (*(u8  *)((u8 *)(b) + (o)))
#define RH(b,o)   (*(u16 *)((u8 *)(b) + (o)))
#define RW(b,o)   (*(u32 *)((u8 *)(b) + (o)))
#define RS(b,o)   (*(s16 *)((u8 *)(b) + (o)))
#define RSW(b,o)  (*(s32 *)((u8 *)(b) + (o)))
#define RP(b,o)   (*(void **)((u8 *)(b) + (o)))
#define W16(b,o)  (*(u16 *)((u8 *)(b) + (o)))

extern u8 *D_8009D278;
extern u8 *D_8009D254;
extern u8 *D_8009D20C;
extern u32 D_8009CDDC, D_8009D1A0, D_8009D1AC, D_8009D1E8, D_8009D200;
extern u32 D_8009D28C, D_8009D2E8, D_8009D2FC, D_800B0E08, D_800BCF88;
extern u8  D_8009D1CE, D_8009D1D4, D_8009D234, D_8009D244;
extern u8  D_800B692C, D_800B692D, D_800B692E, D_800B6948, D_800B6949, D_800B694A;
extern u8 D_800B00EC;
extern u8 D_800B00ED;
extern u8 D_800B00EE;
extern u8 D_800B00F4;
extern u8 D_800B00F5;
extern u8 D_800B00F6;
extern u8 D_800B00FC;
extern u8 D_800B00FD;
extern u8 D_800B00FE;
extern u8 D_800B0104;
extern u8 D_800B0105;
extern u8 D_800B0106;
extern u8 D_800B0110;
extern u8 D_800B0111;
extern u8 D_800B0112;
extern u8 D_800B0118;
extern u8 D_800B0119;
extern u8 D_800B011A;
extern u8 D_800B0120;
extern u8 D_800B0121;
extern u8 D_800B0122;
extern u8 D_800B0128;
extern u8 D_800B0129;
extern u8 D_800B012A;
extern u8 D_800B0134;
extern u8 D_800B0135;
extern u8 D_800B0136;
extern u8 D_800B013C;
extern u8 D_800B013D;
extern u8 D_800B013E;
extern u8 D_800B0144;
extern u8 D_800B0145;
extern u8 D_800B0146;
extern u8 D_800B014C;
extern u8 D_800B014D;
extern u8 D_800B014E;
extern u8 D_800B0158;
extern u8 D_800B0159;
extern u8 D_800B015A;
extern u8 D_800B0160;
extern u8 D_800B0161;
extern u8 D_800B0162;
extern u8 D_800B0168;
extern u8 D_800B0169;
extern u8 D_800B016A;
extern u8 D_800B0170;
extern u8 D_800B0171;
extern u8 D_800B0172;
extern u8 D_800B017C;
extern u8 D_800B017D;
extern u8 D_800B017E;
extern u8 D_800B0184;
extern u8 D_800B0185;
extern u8 D_800B0186;
extern u8 D_800B018C;
extern u8 D_800B018D;
extern u8 D_800B018E;
extern u8 D_800B0194;
extern u8 D_800B0195;
extern u8 D_800B0196;
extern u8 D_800B01A0;
extern u8 D_800B01A1;
extern u8 D_800B01A2;
extern u8 D_800B01A8;
extern u8 D_800B01A9;
extern u8 D_800B01AA;
extern u8 D_800B01B0;
extern u8 D_800B01B1;
extern u8 D_800B01B2;
extern u8 D_800B01B8;
extern u8 D_800B01B9;
extern u8 D_800B01BA;

s32 func_8001D340(s32 arg0) {
    s16 temp_v0_7;
    s16 temp_v0_8;
    s16 temp_v1_11;
    s16 temp_v1_12;
    s16 temp_v1_14;
    s16 temp_v1_9;
    s16 var_v0;
    s32 *temp_s2;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 temp_v1;
    s32 temp_v1_10;
    s32 temp_v1_2;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_7;
    s32 temp_v1_8;
    s32 var_a2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v1;
    s8 temp_v1_3;
    s8 var_a0;
    s8 var_a0_2;
    u16 temp_a1;
    u16 var_v0_2;
    u32 temp_a2;
    u8 *temp_a0_2;
    u8 temp_a0;
    u8 temp_v0_6;
    u8 temp_v1_13;
    void *temp_a1_2;
    void *temp_v1_6;
    void *var_s0;

    u8 phantom[696]; (void)phantom;
    temp_s2 = D_8009D278 + 0x4C;
    if (arg0 & 0xFF) {
        var_v1 = arg0 & 0xFF;
        if (!(D_8009D1A0 & 0x100)) {
            temp_a2 = RW(D_8009D278,0x4C);
            temp_a1 = RH(D_8009D278,0x10) + RH(D_8009D278,0x24);
            temp_v1 = temp_a2 & 0xC0;
            RH(D_8009D278,0x10) = temp_a1;
            if ((temp_v1 == 0x40) || (temp_v1 == 0x80)) {
                var_v0_2 = temp_a1 - ((RH(D_8009D278,0x24) * 2) / 5);
                goto block_7;
            }
            if (temp_a2 & 0x100) {
                var_v0_2 = temp_a1 + ((u16) RH(D_8009D278,0x24) >> 1);
block_7:
                RH(D_8009D278,0x10) = var_v0_2;
            }
            if ((RSW(D_8009D278,0x8) < RSW(D_8009D278,0x28)) && !(RW(D_8009D278,0x4C) & 0x2600)) {
                temp_v0 = RSW(D_8009D278,0x30) - 3;
                RSW(D_8009D278,0x30) = temp_v0;
                if (temp_v0 <= 0) {
                    RSW(D_8009D278,0x30) = 1;
                }
                temp_v0_2 = RSW(D_8009D278,0x2C) - ((s32) RSW(D_8009D278,0x28) / (s32) (RSW(D_8009D278,0x30) * 0x64));
                RSW(D_8009D278,0x2C) = temp_v0_2;
                if (temp_v0_2 < 0x1999) {
                    RSW(D_8009D278,0x2C) = 0x1999;
                }
                temp_v0_3 = RSW(D_8009D278,0x8) + RSW(D_8009D278,0x2C);
                RSW(D_8009D278,0x8) = temp_v0_3;
                if (temp_v0_3 >= RSW(D_8009D278,0x28)) {
                    RSW(D_8009D278,0x34) = 0xF0;
                    if (D_800B0E08 != 0) {
                        func_8006DF50(D_800B0E08, 0x455, 0, 0x80, 0x7F);
                    }
                }
            }
            var_v1 = arg0 & 0xFF;
        }
        if (var_v1 == 1) {
            temp_a0 = RB(D_8009D254,0xE);
            if (((temp_a0 == RB(D_8009D278,0x12)) || (temp_a0 == 5)) && (!(RW(RP(D_8009D278,0x6C),0x4) & 0x4000) || !(RW(D_8009D278,0x4C) & 0x4000) || ((func_80021054(temp_a0) << 0x18) <= 0))) {
                var_v0_3 = D_8009D2E8 & ~1;
            } else if ((u8) RB(D_8009D254,0xE) >= 0xEU) {
                var_v0_3 = D_8009D2E8 & ~1;
            } else {
                goto block_27;
            }
            goto block_28;
        }
    } else {
block_27:
        var_v0_3 = D_8009D2E8 | 1;
block_28:
        D_8009D2E8 = var_v0_3;
    }
    if (RSW(D_8009D278,0x8) < 0) {
        RSW(D_8009D278,0x8) = 0;
    }
    if (*temp_s2 & 0x2000) {
        temp_v1_2 = D_8009D1E8 & 3;
        switch (temp_v1_2) {                        /* switch 1; irregular */
        case 0:                                     /* switch 1 */
            var_a2 = 0x83;
            var_a0 = 1;
            *(&D_800B0158 + (D_8009CDDC * 0x48)) = 0xFF;
            *(&D_800B0159 + (D_8009CDDC * 0x48)) = 0x3D;
            *(&D_800B015A + (D_8009CDDC * 0x48)) = 0x81;
            *(&D_800B0160 + (D_8009CDDC * 0x48)) = 0x83;
            *(&D_800B0161 + (D_8009CDDC * 0x48)) = 0x13;
            *(&D_800B0162 + (D_8009CDDC * 0x48)) = 1;
            *(&D_800B0168 + (D_8009CDDC * 0x48)) = 0xFF;
            *(&D_800B0169 + (D_8009CDDC * 0x48)) = 0x3D;
            *(&D_800B016A + (D_8009CDDC * 0x48)) = 0x81;
            *(&D_800B0170 + (D_8009CDDC * 0x48)) = 0x83;
            *(&D_800B0171 + (D_8009CDDC * 0x48)) = 0x13;
            var_v0_4 = D_8009CDDC * 8;
block_40:
            *(&D_800B0172 + ((var_v0_4 + D_8009CDDC) * 8)) = var_a0;
            break;
        case 1:                                     /* switch 1 */
            var_a2 = 0xC1;
            var_a0 = 0x41;
            *(&D_800B0158 + (D_8009CDDC * 0x48)) = 0xC1;
            *(&D_800B0159 + (D_8009CDDC * 0x48)) = 0x28;
            *(&D_800B015A + (D_8009CDDC * 0x48)) = 0x41;
            *(&D_800B0160 + (D_8009CDDC * 0x48)) = 0xC1;
            *(&D_800B0161 + (D_8009CDDC * 0x48)) = 0x28;
            *(&D_800B0162 + (D_8009CDDC * 0x48)) = 0x41;
            *(&D_800B0168 + (D_8009CDDC * 0x48)) = 0xC1;
            *(&D_800B0169 + (D_8009CDDC * 0x48)) = 0x28;
            *(&D_800B016A + (D_8009CDDC * 0x48)) = 0x41;
            *(&D_800B0170 + (D_8009CDDC * 0x48)) = 0xC1;
            *(&D_800B0171 + (D_8009CDDC * 0x48)) = 0x28;
            var_v0_4 = D_8009CDDC * 8;
            goto block_40;
        case 2:                                     /* switch 1 */
            var_a2 = 0xFF;
            var_a0 = 0x81;
            *(&D_800B0158 + (D_8009CDDC * 0x48)) = 0x83;
            *(&D_800B0159 + (D_8009CDDC * 0x48)) = 0x13;
            *(&D_800B015A + (D_8009CDDC * 0x48)) = 1;
            *(&D_800B0160 + (D_8009CDDC * 0x48)) = 0xFF;
            *(&D_800B0161 + (D_8009CDDC * 0x48)) = 0x3D;
            *(&D_800B0162 + (D_8009CDDC * 0x48)) = 0x81;
            *(&D_800B0168 + (D_8009CDDC * 0x48)) = 0x83;
            *(&D_800B0169 + (D_8009CDDC * 0x48)) = 0x13;
            *(&D_800B016A + (D_8009CDDC * 0x48)) = 1;
            *(&D_800B0170 + (D_8009CDDC * 0x48)) = 0xFF;
            *(&D_800B0171 + (D_8009CDDC * 0x48)) = 0x3D;
            var_v0_4 = D_8009CDDC * 8;
            goto block_40;
        default:                                    /* switch 1 */
            var_a2 = 0xC1;
            if (temp_v1_2 == 3) {
                var_a0 = 0x41;
                *(&D_800B0158 + (D_8009CDDC * 0x48)) = 0xC1;
                *(&D_800B0159 + (D_8009CDDC * 0x48)) = 0x28;
                *(&D_800B015A + (D_8009CDDC * 0x48)) = 0x41;
                *(&D_800B0160 + (D_8009CDDC * 0x48)) = 0xC1;
                *(&D_800B0161 + (D_8009CDDC * 0x48)) = 0x28;
                *(&D_800B0162 + (D_8009CDDC * 0x48)) = 0x41;
                *(&D_800B0168 + (D_8009CDDC * 0x48)) = 0xC1;
                *(&D_800B0169 + (D_8009CDDC * 0x48)) = 0x28;
                *(&D_800B016A + (D_8009CDDC * 0x48)) = 0x41;
                *(&D_800B0170 + (D_8009CDDC * 0x48)) = 0xC1;
                *(&D_800B0171 + (D_8009CDDC * 0x48)) = 0x28;
                var_v0_4 = D_8009CDDC * 8;
                goto block_40;
            }
            break;
        }
        if (RB(D_8009D254,0xE) != 0x12) {
            func_8001A680(D_8009D254, 0x12U, var_a2);
        }
        temp_v1_3 = *(s8*)0x8009CE30;
        if (temp_v1_3 == 0x5A) {
            func_8001A680(D_8009D254, RB(D_8009D278,0x12));
            *(s8*)0x8009CE30 = 0;
            *temp_s2 &= ~0x2000;
            RSW(D_8009D278,0x8) = 0x10000;
            D_800B0158 = 0xFF;
            D_800B0159 = 0x3D;
            D_800B015A = 0x81;
            D_800B0160 = 0x83;
            D_800B0161 = 0x13;
            D_800B0162 = 1;
            D_800B0168 = 0xFF;
            D_800B0169 = 0x3D;
            D_800B016A = 0x81;
            D_800B0170 = 0x83;
            D_800B0171 = 0x13;
            D_800B0172 = 1;
            D_800B01A0 = 0xFF;
            D_800B01A1 = 0x3D;
            D_800B01A2 = 0x81;
            D_800B01A8 = 0x83;
            D_800B01A9 = 0x13;
            D_800B01AA = 1;
            D_800B01B0 = 0xFF;
            D_800B01B1 = 0x3D;
            D_800B01B2 = 0x81;
            D_800B01B8 = 0x83;
            D_800B01B9 = 0x13;
            D_800B01BA = 1;
        } else {
            *(s8*)0x8009CE30 = (s8) (temp_v1_3 + 1);
            RSW(D_8009D254,0x68) = 0;
            RSW(D_8009D254,0x6C) = 0;
            RSW(D_8009D254,0x70) = 0;
        }
    }
    if (!(D_8009D1A0 & 0x100)) {
        temp_v0_4 = RSW(D_8009D278,0x34);
        temp_v0_5 = temp_v0_4 - 1;
        if (temp_v0_4 > 0) {
            RSW(D_8009D278,0x34) = temp_v0_5;
            if (temp_v0_5 != 0) {
                temp_v1_4 = D_8009D1E8 & 3;
                switch (temp_v1_4) {                /* switch 2; irregular */
                case 0:                             /* switch 2 */
                    var_a0_2 = 0x3B;
                    *(&D_800B0134 + (D_8009CDDC * 0x48)) = 0;
                    *(&D_800B0135 + (D_8009CDDC * 0x48)) = 0x82;
                    *(&D_800B0136 + (D_8009CDDC * 0x48)) = 0x36;
                    *(&D_800B013C + (D_8009CDDC * 0x48)) = 0x4A;
                    *(&D_800B013D + (D_8009CDDC * 0x48)) = 0xFF;
                    *(&D_800B013E + (D_8009CDDC * 0x48)) = 0x3B;
                    *(&D_800B0144 + (D_8009CDDC * 0x48)) = 0;
                    *(&D_800B0145 + (D_8009CDDC * 0x48)) = 0x82;
                    *(&D_800B0146 + (D_8009CDDC * 0x48)) = 0x36;
                    *(&D_800B014C + (D_8009CDDC * 0x48)) = 0x4A;
                    *(&D_800B014D + (D_8009CDDC * 0x48)) = 0xFF;
                    var_v0_5 = D_8009CDDC * 8;
block_57:
                    *(&D_800B014E + ((var_v0_5 + D_8009CDDC) * 8)) = var_a0_2;
                    break;
                case 1:                             /* switch 2 */
                    var_a0_2 = 0x39;
                    *(&D_800B0134 + (D_8009CDDC * 0x48)) = 0x25;
                    *(&D_800B0135 + (D_8009CDDC * 0x48)) = 0xC1;
                    *(&D_800B0136 + (D_8009CDDC * 0x48)) = 0x39;
                    *(&D_800B013C + (D_8009CDDC * 0x48)) = 0x25;
                    *(&D_800B013D + (D_8009CDDC * 0x48)) = 0xC1;
                    *(&D_800B013E + (D_8009CDDC * 0x48)) = 0x39;
                    *(&D_800B0144 + (D_8009CDDC * 0x48)) = 0x25;
                    *(&D_800B0145 + (D_8009CDDC * 0x48)) = 0xC1;
                    *(&D_800B0146 + (D_8009CDDC * 0x48)) = 0x39;
                    *(&D_800B014C + (D_8009CDDC * 0x48)) = 0x25;
                    *(&D_800B014D + (D_8009CDDC * 0x48)) = 0xC1;
                    var_v0_5 = D_8009CDDC * 8;
                    goto block_57;
                case 2:                             /* switch 2 */
                    var_a0_2 = 0x36;
                    *(&D_800B0134 + (D_8009CDDC * 0x48)) = 0x4A;
                    *(&D_800B0135 + (D_8009CDDC * 0x48)) = 0xFF;
                    *(&D_800B0136 + (D_8009CDDC * 0x48)) = 0x3B;
                    *(&D_800B013C + (D_8009CDDC * 0x48)) = 0;
                    *(&D_800B013D + (D_8009CDDC * 0x48)) = 0x82;
                    *(&D_800B013E + (D_8009CDDC * 0x48)) = 0x36;
                    *(&D_800B0144 + (D_8009CDDC * 0x48)) = 0x4A;
                    *(&D_800B0145 + (D_8009CDDC * 0x48)) = 0xFF;
                    *(&D_800B0146 + (D_8009CDDC * 0x48)) = 0x3B;
                    *(&D_800B014C + (D_8009CDDC * 0x48)) = 0;
                    *(&D_800B014D + (D_8009CDDC * 0x48)) = 0x82;
                    var_v0_5 = D_8009CDDC * 8;
                    goto block_57;
                case 3:                             /* switch 2 */
                    var_a0_2 = 0x39;
                    *(&D_800B0134 + (D_8009CDDC * 0x48)) = 0x25;
                    *(&D_800B0135 + (D_8009CDDC * 0x48)) = 0xC1;
                    *(&D_800B0136 + (D_8009CDDC * 0x48)) = 0x39;
                    *(&D_800B013C + (D_8009CDDC * 0x48)) = 0x25;
                    *(&D_800B013D + (D_8009CDDC * 0x48)) = 0xC1;
                    *(&D_800B013E + (D_8009CDDC * 0x48)) = 0x39;
                    *(&D_800B0144 + (D_8009CDDC * 0x48)) = 0x25;
                    *(&D_800B0145 + (D_8009CDDC * 0x48)) = 0xC1;
                    *(&D_800B0146 + (D_8009CDDC * 0x48)) = 0x39;
                    *(&D_800B014C + (D_8009CDDC * 0x48)) = 0x25;
                    *(&D_800B014D + (D_8009CDDC * 0x48)) = 0xC1;
                    var_v0_5 = D_8009CDDC * 8;
                    goto block_57;
                }
                if (RSW(D_8009D278,0x8) < RSW(D_8009D278,0x28)) {
                    RSW(D_8009D278,0x34) = 0;
                }
                if (RSW(D_8009D278,0x34) == 0) {
                    goto block_61;
                }
            } else {
block_61:
                D_800B0134 = 0;
                D_800B0135 = 0x82;
                D_800B0136 = 0x36;
                D_800B013C = 0x4A;
                D_800B013D = 0xFF;
                D_800B013E = 0x3B;
                D_800B0144 = 0;
                D_800B0145 = 0x82;
                D_800B0146 = 0x36;
                D_800B014C = 0x4A;
                D_800B014D = 0xFF;
                D_800B014E = 0x3B;
                D_800B017C = 0;
                D_800B017D = 0x82;
                D_800B017E = 0x36;
                D_800B0184 = 0x4A;
                D_800B0185 = 0xFF;
                D_800B0186 = 0x3B;
                D_800B018C = 0;
                D_800B018D = 0x82;
                D_800B018E = 0x36;
                D_800B0194 = 0x4A;
                D_800B0195 = 0xFF;
                D_800B0196 = 0x3B;
            }
        }
        temp_v1_5 = *temp_s2;
        if (temp_v1_5 & 0x800000) {
            temp_v0_6 = D_8009D234 - 1;
            D_8009D234 = temp_v0_6;
            if ((temp_v0_6 << 0x18) <= 0) {
                *temp_s2 &= 0xFF7FFFFF;
            }
        } else if (!(temp_v1_5 & 0x180000)) {
            var_s0 = D_8009D20C;
            if (var_s0 != NULL) {
                do {
                    if (var_s0 != D_8009D254) {
                        temp_v1_6 = RP(var_s0,0x0);
                        if ((temp_v1_6 != NULL) && !(RW(var_s0,0x98) & 0x10)) {
                            temp_a0_2 = RP(temp_v1_6,0x18);
                            if ((temp_a0_2 != NULL) && (*temp_s2 & 0x4000)) {
                                temp_v1_7 = RSW(temp_v1_6,0x0);
                                if ((temp_v1_7 < 0) && ((u32) (*temp_a0_2 - 1) < 2U)) {
                                    temp_v1_8 = ((u32) temp_v1_7 >> 0x15) & 7;
                                    if (temp_v1_8 < 3) {
                                        func_8006DCE4(RW((u8*)((temp_v1_8 * 2) + temp_v1_6),0xB6), 0, RS(var_s0,0x268), RS(var_s0,0x26A), (s32) RS(var_s0,0x26C));
                                    }
                                    *temp_s2 |= 0x10000000;
                                    func_8001F4D4(var_s0);
                                    RSW(D_8009D254,0x68) = 0;
                                    RSW(D_8009D254,0x6C) = 0;
                                    RSW(D_8009D254,0x70) = 0;
                                    RSW(temp_v1_6,0x0) = (s32) (RSW(temp_v1_6,0x0) & 0x7FFFFFFF);
                                    RH(temp_v1_6,0x9C) = (u16) (RH(temp_v1_6,0x9C) + 1);
                                    *(s8*)0x8009CE34 = 0x5A;
                                    *temp_s2 |= 0x01000000;
                                }
                                temp_v1_9 = RH(D_8009D278,0xC);
                                if (RS(D_8009D278,0x1C) >= temp_v1_9) {
                                    RS(D_8009D278,0x50) = (s16) (RH(D_8009D278,0xE) - temp_v1_9);
                                    RH(D_8009D278,0x52) = (u16) RH(D_8009D254,0x210);
                                    RB(D_8009D278,0x56) = 0x1EU;
                                    RH(D_8009D278,0x54) = (u16) RH(D_8009D254,0x212);
                                    if (RS(D_8009D278,0x50) == 0) {
                                        RB(D_8009D278,0x57) = 0;
                                    } else {
                                        RB(D_8009D278,0x57) = (s8) (((u32) RW(D_8009D278,0x4C) >> 0xE) & 2);
                                    }
                                    RW(D_8009D278,0x4C) = (u32) (RW(D_8009D278,0x4C) & 0xFFFF7FFF);
                                }
                            }
                            if (RW(var_s0,0x98) & 0x02000000) {
                                temp_v1_10 = *temp_s2;
                                if (!(temp_v1_10 & 0x01000000) && (RSW(temp_v1_6,0x10) > 0)) {
                                    if (!(temp_v1_10 & 0x200)) {
                                        RH(D_8009D278,0xC) = (s16) ((u16) RH(D_8009D278,0xC) - RB(temp_v1_6,0x92));
                                    }
                                    *(s8*)0x8009CE34 = 0x5A;
                                    *temp_s2 |= 0x01000000;
                                    temp_v1_11 = RH(D_8009D278,0xC);
                                    if (RS(D_8009D278,0x1C) >= temp_v1_11) {
                                        RS(D_8009D278,0x50) = (s16) (RH(D_8009D278,0xE) - temp_v1_11);
                                        RH(D_8009D278,0x52) = (u16) RH(D_8009D254,0x210);
                                        RB(D_8009D278,0x56) = 0x1EU;
                                        RH(D_8009D278,0x54) = (u16) RH(D_8009D254,0x212);
                                        if (RS(D_8009D278,0x50) == 0) {
                                            RB(D_8009D278,0x57) = 0;
                                        } else {
                                            RB(D_8009D278,0x57) = (s8) (((u32) RW(D_8009D278,0x4C) >> 0xE) & 2);
                                        }
                                        RW(D_8009D278,0x4C) = (u32) (RW(D_8009D278,0x4C) & 0xFFFF7FFF);
                                    }
                                    RSW(D_8009D254,0x68) = 0;
                                    RSW(D_8009D254,0x6C) = 0;
                                    RSW(D_8009D254,0x70) = 0;
                                    RSW(D_8009D254,0x98) = (s32) (RSW(D_8009D254,0x98) & 0xFFF3FFFF);
                                    RH(D_8009D278,0x4A) = func_8001F814(var_s0, 0xFFF3FFFF);
                                    RB(D_8009D278,0x49) = (u8) RB(temp_v1_6,0x93);
                                    RB(D_8009D278,0x48) = 6;
                                }
                            }
                        }
                    }
                    var_s0 = RP(var_s0,0x4);
                } while (var_s0 != NULL);
            }
            temp_v1_12 = RH(D_8009D278,0xC);
            temp_v0_7 = (s16) RH(D_8009D278,0xE);
            if (temp_v1_12 < temp_v0_7) {
                if (RS(D_8009D278,0x1C) >= temp_v1_12) {
                    RS(D_8009D278,0x50) = (s16) (temp_v0_7 - temp_v1_12);
                    RH(D_8009D278,0x52) = (u16) RH(D_8009D254,0x210);
                    RB(D_8009D278,0x56) = 0x1EU;
                    RH(D_8009D278,0x54) = (u16) RH(D_8009D254,0x212);
                    if (RS(D_8009D278,0x50) == 0) {
                        RB(D_8009D278,0x57) = 0;
                    } else {
                        RB(D_8009D278,0x57) = (s8) (((u32) RW(D_8009D278,0x4C) >> 0xE) & 2);
                    }
                    RW(D_8009D278,0x4C) = (u32) (RW(D_8009D278,0x4C) & 0xFFFF7FFF);
                }
                RH(D_8009D278,0xE) = (u16) RH(D_8009D278,0xC);
            }
        }
        *temp_s2 &= ~0x4000;
        if (RB(D_8009D278,0x48) != 0) {
            temp_a0_3 = RSW(D_8009D254,0x98);
            if ((temp_a0_3 & 0xC0000) || (RB(D_8009D278,0x49) == 0)) {
                RSW(D_8009D254,0x98) = (s32) (temp_a0_3 & 0xFFF3FFFF);
                RB(D_8009D278,0x48) = 0;
                RB(D_8009D278,0x49) = 0U;
            } else {
                RSW(D_8009D254,0x28) = (s32) (RSW(D_8009D254,0x40) + (RB(D_8009D278,0x49) * func_80077CF4(RH(D_8009D278,0x4A), D_8009D254) * 0x10));
                RSW(D_8009D254,0x30) = (s32) (RSW(D_8009D254,0x48) + (RB(D_8009D278,0x49) * func_80077DC4(RH(D_8009D278,0x4A), D_8009D278) * 0x10));
                temp_v1_13 = RB(D_8009D278,0x49);
                RB(D_8009D278,0x49) = (u8) (temp_v1_13 - ((s32) temp_v1_13 / (s8) RB(D_8009D278,0x48)));
                RB(D_8009D278,0x48) = (s8) ((u8) RB(D_8009D278,0x48) - 1);
            }
        }
        func_8001F9C4();
    }
    if (RB(D_8009D278,0x56) != 0) {
        func_80032B0C(0, D_8009D278 + 0x50);
        RB(D_8009D278,0x56) = (u8) (RB(D_8009D278,0x56) - 1);
    }
    temp_v0_8 = (s16) RH(D_8009D278,0xE);
    temp_v1_14 = RH(D_8009D278,0xC);
    if (temp_v0_8 < temp_v1_14) {
        RS(D_8009D278,0x58) = (s16) (temp_v1_14 - temp_v0_8);
        RH(D_8009D278,0x5A) = (u16) RH(D_8009D254,0x210);
        RB(D_8009D278,0x5E) = 0x1EU;
        RS(D_8009D278,0x5C) = (s16) (RH(D_8009D254,0x212) - 8);
        RB(D_8009D278,0x5F) = 1;
        RH(D_8009D278,0xE) = (u16) RH(D_8009D278,0xC);
    }
    temp_a1_2 = D_8009D278;
    if (RB(temp_a1_2,0x5E) != 0) {
        func_80032B0C(0, temp_a1_2 + 0x58, temp_v0_8);
        RB(D_8009D278,0x5E) = (u8) (RB(D_8009D278,0x5E) - 1);
    }
    if (RB(temp_a1_2,0x66) != 0) {
        func_80032B0C(0, temp_a1_2 + 0x60);
        RB(D_8009D278,0x66) = (u8) (RB(D_8009D278,0x66) - 1);
    }
    temp_a0_4 = RSW(D_8009D278,0x8);
    RH(D_8009D278,0xE) = (u16) RH(D_8009D278,0xC);
    if (temp_a0_4 <= 0) {
        *temp_s2 |= 0x2000;
        if (func_80021054((u8) temp_a0_4) != D_8009D1D4) {
            if (D_8009D200 != -1) {
                func_8006F6D4(D_8009D200, 0, 0, 2, 0, 0);
                D_8009D200 = -1;
            }
            if (D_8009D2FC != -1) {
                func_8006F6D4(D_8009D2FC, 0, 0, 2, 0, 0);
                D_8009D2FC = -1;
            }
        }
        func_80021D4C();
    }
    if (D_8009D28C == 1) {
        D_800B00ED = 0x46;
        D_800B00FD = 0x46;
        D_800B0111 = 0x46;
        D_800B0121 = 0x46;
        D_800B00F4 = 0x9F;
        D_800B0104 = 0x9F;
        D_800B692C = 0x9F;
        D_800B0118 = 0x9F;
        D_800B0128 = 0x9F;
        D_800B6948 = 0x9F;
        D_800B00F6 = 0xF9;
        D_800B0106 = 0xF9;
        D_800B692E = 0xF9;
        D_800B011A = 0xF9;
        D_800B012A = 0xF9;
        D_800B694A = 0xF9;
        D_800B00EC = 0;
        D_800B00EE = 0x82;
        D_800B00F5 = 0xFF;
        D_800B00FC = 0;
        D_800B00FE = 0x82;
        D_800B0105 = 0xFF;
        D_800B692D = 0xFF;
        D_800B0110 = 0;
        D_800B0112 = 0x82;
        D_800B0119 = 0xFF;
        D_800B0120 = 0;
        D_800B0122 = 0x82;
        D_800B0129 = 0xFF;
        D_800B6949 = 0xFF;
        D_800B0134 = 0;
        D_800B0135 = 0x82;
        D_800B0136 = 0x36;
        D_800B013C = 0x4A;
        D_800B013D = 0xFF;
        D_800B013E = 0x3B;
        D_800B0144 = 0;
        D_800B0145 = 0x82;
        D_800B0146 = 0x36;
        D_800B014C = 0x4A;
        D_800B014D = 0xFF;
        D_800B014E = 0x3B;
        D_800B017C = 0;
        D_800B017D = 0x82;
        D_800B017E = 0x36;
        D_800B0184 = 0x4A;
        D_800B0185 = 0xFF;
        D_800B0186 = 0x3B;
        D_800B018C = 0;
        D_800B018D = 0x82;
        D_800B018E = 0x36;
        D_800B0194 = 0x4A;
        D_800B0195 = 0xFF;
        D_800B0196 = 0x3B;
    }
    var_v0 = RH(D_8009D278,0xC);
    if (var_v0 <= 0) {
        RSW(D_8009D254,0x68) = 0;
        RSW(D_8009D254,0x6C) = 0;
        RSW(D_8009D254,0x70) = 0;
        D_800B00ED = 0x46;
        D_800B00FD = 0x46;
        D_800B0111 = 0x46;
        D_800B0121 = 0x46;
        D_800B00F4 = 0x9F;
        D_800B0104 = 0x9F;
        D_800B692C = 0x9F;
        D_800B0118 = 0x9F;
        D_800B0128 = 0x9F;
        D_800B6948 = 0x9F;
        D_800B00EC = 0;
        D_800B00EE = 0x82;
        D_800B00F5 = 0xFF;
        D_800B00F6 = 0xF9;
        D_800B00FC = 0;
        D_800B00FE = 0x82;
        D_800B0105 = 0xFF;
        D_800B0106 = 0xF9;
        D_800B692D = 0xFF;
        D_800B692E = 0xF9;
        D_800B0110 = 0;
        D_800B0112 = 0x82;
        D_800B0119 = 0xFF;
        D_800B011A = 0xF9;
        D_800B0120 = 0;
        D_800B0122 = 0x82;
        D_800B0129 = 0xFF;
        D_800B012A = 0xF9;
        D_800B6949 = 0xFF;
        D_800B694A = 0xF9;
        D_800B0136 = 0x36;
        D_800B0146 = 0x36;
        D_800B017E = 0x36;
        D_800B018E = 0x36;
        D_800B0135 = 0x82;
        D_800B0145 = 0x82;
        D_800B017D = 0x82;
        D_800B018D = 0x82;
        D_800B013C = 0x4A;
        D_800B014C = 0x4A;
        D_800B0184 = 0x4A;
        D_800B0194 = 0x4A;
        D_800B013E = 0x3B;
        D_800B014E = 0x3B;
        D_800B0186 = 0x3B;
        D_800B0196 = 0x3B;
        D_800B0134 = 0;
        D_800B013D = 0xFF;
        D_800B0144 = 0;
        D_800B014D = 0xFF;
        D_800B017C = 0;
        D_800B0185 = 0xFF;
        D_800B018C = 0;
        D_800B0195 = 0xFF;
        D_800B0158 = 0xFF;
        D_800B0159 = 0x3D;
        D_800B015A = 0x81;
        D_800B0160 = 0x83;
        D_800B0161 = 0x13;
        D_800B0162 = 1;
        D_800B0168 = 0xFF;
        D_800B0169 = 0x3D;
        D_800B016A = 0x81;
        D_800B0170 = 0x83;
        D_800B0171 = 0x13;
        D_800B0172 = 1;
        D_800B01A0 = 0xFF;
        D_800B01A1 = 0x3D;
        D_800B01A2 = 0x81;
        D_800B01A8 = 0x83;
        D_800B01A9 = 0x13;
        D_800B01AA = 1;
        D_800B01B0 = 0xFF;
        D_800B01B1 = 0x3D;
        D_800B01B2 = 0x81;
        D_800B01B8 = 0x83;
        D_800B01B9 = 0x13;
        D_800B01BA = 1;
        if (*temp_s2 & 0x10000) {
            func_80020CE4(0x13, 0x83, 0x81, 0x3D);
        }
        func_800374E8();
        if (func_80021054() != D_8009D1D4) {
            if (D_8009D200 != -1) {
                func_8006F6D4(D_8009D200, 0, 0, 2, 0, 0);
                D_8009D200 = -1;
            }
            if (D_8009D2FC != -1) {
                func_8006F6D4(D_8009D2FC, 0, 0, 2, 0, 0);
                D_8009D2FC = -1;
            }
        }
        func_80021D4C();
        D_8009D1AC &= ~0x300;
        D_8009D28C = 3;
        D_8009D1CE = 0;
        func_8006DE80(0x46B, 0, RS(D_8009D254,0x2A), RS(D_8009D254,0x2E), (s32) RS(D_8009D254,0x32));
        D_8009D244 = 0;
        func_80062F9C();
        if (D_800BCF88 & 0x2000) {
            func_80067CBC();
        }
        D_8009D1A0 &= ~4;
        D_8009D2E8 |= 1;
        func_8001A680(D_8009D254, 0x13U, -5);
        W16(D_8009D278, 0xC) = 0;
    }
    return (s32) var_v0;
}

