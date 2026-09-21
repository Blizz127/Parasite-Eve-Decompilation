/* ==== func_8001D340 (DB40.s) ==== */
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

    temp_s2 = D_8009D278 + 0x4C;
    if (arg0 & 0xFF) {
        var_v1 = arg0 & 0xFF;
        if (!(D_8009D1A0 & 0x100)) {
            temp_a2 = D_8009D278->unk4C;
            temp_a1 = D_8009D278->unk10 + D_8009D278->unk24;
            temp_v1 = temp_a2 & 0xC0;
            D_8009D278->unk10 = temp_a1;
            if ((temp_v1 == 0x40) || (temp_v1 == 0x80)) {
                var_v0_2 = temp_a1 - ((D_8009D278->unk24 * 2) / 5);
                goto block_7;
            }
            if (temp_a2 & 0x100) {
                var_v0_2 = temp_a1 + ((u16) D_8009D278->unk24 >> 1);
block_7:
                D_8009D278->unk10 = var_v0_2;
            }
            if ((D_8009D278->unk8 < D_8009D278->unk28) && !(D_8009D278->unk4C & 0x2600)) {
                temp_v0 = D_8009D278->unk30 - 3;
                D_8009D278->unk30 = temp_v0;
                if (temp_v0 <= 0) {
                    D_8009D278->unk30 = 1;
                }
                temp_v0_2 = D_8009D278->unk2C - ((s32) D_8009D278->unk28 / (s32) (D_8009D278->unk30 * 0x64));
                D_8009D278->unk2C = temp_v0_2;
                if (temp_v0_2 < 0x1999) {
                    D_8009D278->unk2C = 0x1999;
                }
                temp_v0_3 = D_8009D278->unk8 + D_8009D278->unk2C;
                D_8009D278->unk8 = temp_v0_3;
                if (temp_v0_3 >= D_8009D278->unk28) {
                    D_8009D278->unk34 = 0xF0;
                    if (D_800B0E08 != 0) {
                        func_8006DF50(D_800B0E08, 0x455, 0, 0x80, 0x7F);
                    }
                }
            }
            var_v1 = arg0 & 0xFF;
        }
        if (var_v1 == 1) {
            temp_a0 = D_8009D254->unkE;
            if (((temp_a0 == D_8009D278->unk12) || (temp_a0 == 5)) && (!(D_8009D278->unk6C->unk4 & 0x4000) || !(D_8009D278->unk4C & 0x4000) || ((func_80021054(temp_a0) << 0x18) <= 0))) {
                var_v0_3 = D_8009D2E8 & ~1;
            } else if ((u8) D_8009D254->unkE >= 0xEU) {
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
    if (D_8009D278->unk8 < 0) {
        D_8009D278->unk8 = 0;
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
        if (D_8009D254->unkE != 0x12) {
            func_8001A680(D_8009D254, 0x12U, var_a2);
        }
        temp_v1_3 = saved_reg_gp->unkC0;
        if (temp_v1_3 == 0x5A) {
            func_8001A680(D_8009D254, D_8009D278->unk12);
            saved_reg_gp->unkC0 = 0;
            *temp_s2 &= ~0x2000;
            D_8009D278->unk8 = 0x10000;
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
            saved_reg_gp->unkC0 = (s8) (temp_v1_3 + 1);
            D_8009D254->unk68 = 0;
            D_8009D254->unk6C = 0;
            D_8009D254->unk70 = 0;
        }
    }
    if (!(D_8009D1A0 & 0x100)) {
        temp_v0_4 = D_8009D278->unk34;
        temp_v0_5 = temp_v0_4 - 1;
        if (temp_v0_4 > 0) {
            D_8009D278->unk34 = temp_v0_5;
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
                if (D_8009D278->unk8 < D_8009D278->unk28) {
                    D_8009D278->unk34 = 0;
                }
                if (D_8009D278->unk34 == 0) {
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
                        temp_v1_6 = var_s0->unk0;
                        if ((temp_v1_6 != NULL) && !(var_s0->unk98 & 0x10)) {
                            temp_a0_2 = temp_v1_6->unk18;
                            if ((temp_a0_2 != NULL) && (*temp_s2 & 0x4000)) {
                                temp_v1_7 = temp_v1_6->unk0;
                                if ((temp_v1_7 < 0) && ((u32) (*temp_a0_2 - 1) < 2U)) {
                                    temp_v1_8 = ((u32) temp_v1_7 >> 0x15) & 7;
                                    if (temp_v1_8 < 3) {
                                        func_8006DCE4(((temp_v1_8 * 2) + temp_v1_6)->unkB6, 0, var_s0->unk268, var_s0->unk26A, (s32) var_s0->unk26C);
                                    }
                                    *temp_s2 |= 0x10000000;
                                    func_8001F4D4(var_s0);
                                    D_8009D254->unk68 = 0;
                                    D_8009D254->unk6C = 0;
                                    D_8009D254->unk70 = 0;
                                    temp_v1_6->unk0 = (s32) (temp_v1_6->unk0 & 0x7FFFFFFF);
                                    temp_v1_6->unk9C = (u16) (temp_v1_6->unk9C + 1);
                                    saved_reg_gp->unkC4 = 0x5A;
                                    *temp_s2 |= 0x01000000;
                                }
                                temp_v1_9 = D_8009D278->unkC;
                                if (D_8009D278->unk1C >= temp_v1_9) {
                                    D_8009D278->unk50 = (s16) (D_8009D278->unkE - temp_v1_9);
                                    D_8009D278->unk52 = (u16) D_8009D254->unk210;
                                    D_8009D278->unk56 = 0x1EU;
                                    D_8009D278->unk54 = (u16) D_8009D254->unk212;
                                    if (D_8009D278->unk50 == 0) {
                                        D_8009D278->unk57 = 0;
                                    } else {
                                        D_8009D278->unk57 = (s8) (((u32) D_8009D278->unk4C >> 0xE) & 2);
                                    }
                                    D_8009D278->unk4C = (u32) (D_8009D278->unk4C & 0xFFFF7FFF);
                                }
                            }
                            if (var_s0->unk98 & 0x02000000) {
                                temp_v1_10 = *temp_s2;
                                if (!(temp_v1_10 & 0x01000000) && (temp_v1_6->unk10 > 0)) {
                                    if (!(temp_v1_10 & 0x200)) {
                                        D_8009D278->unkC = (s16) ((u16) D_8009D278->unkC - temp_v1_6->unk92);
                                    }
                                    saved_reg_gp->unkC4 = 0x5A;
                                    *temp_s2 |= 0x01000000;
                                    temp_v1_11 = D_8009D278->unkC;
                                    if (D_8009D278->unk1C >= temp_v1_11) {
                                        D_8009D278->unk50 = (s16) (D_8009D278->unkE - temp_v1_11);
                                        D_8009D278->unk52 = (u16) D_8009D254->unk210;
                                        D_8009D278->unk56 = 0x1EU;
                                        D_8009D278->unk54 = (u16) D_8009D254->unk212;
                                        if (D_8009D278->unk50 == 0) {
                                            D_8009D278->unk57 = 0;
                                        } else {
                                            D_8009D278->unk57 = (s8) (((u32) D_8009D278->unk4C >> 0xE) & 2);
                                        }
                                        D_8009D278->unk4C = (u32) (D_8009D278->unk4C & 0xFFFF7FFF);
                                    }
                                    D_8009D254->unk68 = 0;
                                    D_8009D254->unk6C = 0;
                                    D_8009D254->unk70 = 0;
                                    D_8009D254->unk98 = (s32) (D_8009D254->unk98 & 0xFFF3FFFF);
                                    D_8009D278->unk4A = func_8001F814(var_s0, 0xFFF3FFFF);
                                    D_8009D278->unk49 = (u8) temp_v1_6->unk93;
                                    D_8009D278->unk48 = 6;
                                }
                            }
                        }
                    }
                    var_s0 = var_s0->unk4;
                } while (var_s0 != NULL);
            }
            temp_v1_12 = D_8009D278->unkC;
            temp_v0_7 = (s16) D_8009D278->unkE;
            if (temp_v1_12 < temp_v0_7) {
                if (D_8009D278->unk1C >= temp_v1_12) {
                    D_8009D278->unk50 = (s16) (temp_v0_7 - temp_v1_12);
                    D_8009D278->unk52 = (u16) D_8009D254->unk210;
                    D_8009D278->unk56 = 0x1EU;
                    D_8009D278->unk54 = (u16) D_8009D254->unk212;
                    if (D_8009D278->unk50 == 0) {
                        D_8009D278->unk57 = 0;
                    } else {
                        D_8009D278->unk57 = (s8) (((u32) D_8009D278->unk4C >> 0xE) & 2);
                    }
                    D_8009D278->unk4C = (u32) (D_8009D278->unk4C & 0xFFFF7FFF);
                }
                D_8009D278->unkE = (u16) D_8009D278->unkC;
            }
        }
        *temp_s2 &= ~0x4000;
        if (D_8009D278->unk48 != 0) {
            temp_a0_3 = D_8009D254->unk98;
            if ((temp_a0_3 & 0xC0000) || (D_8009D278->unk49 == 0)) {
                D_8009D254->unk98 = (s32) (temp_a0_3 & 0xFFF3FFFF);
                D_8009D278->unk48 = 0;
                D_8009D278->unk49 = 0U;
            } else {
                D_8009D254->unk28 = (s32) (D_8009D254->unk40 + (D_8009D278->unk49 * func_80077CF4(D_8009D278->unk4A, D_8009D254) * 0x10));
                D_8009D254->unk30 = (s32) (D_8009D254->unk48 + (D_8009D278->unk49 * func_80077DC4(D_8009D278->unk4A, D_8009D278) * 0x10));
                temp_v1_13 = D_8009D278->unk49;
                D_8009D278->unk49 = (u8) (temp_v1_13 - ((s32) temp_v1_13 / (s8) D_8009D278->unk48));
                D_8009D278->unk48 = (s8) ((u8) D_8009D278->unk48 - 1);
            }
        }
        func_8001F9C4();
    }
    if (D_8009D278->unk56 != 0) {
        func_80032B0C(0, D_8009D278 + 0x50);
        D_8009D278->unk56 = (u8) (D_8009D278->unk56 - 1);
    }
    temp_v0_8 = (s16) D_8009D278->unkE;
    temp_v1_14 = D_8009D278->unkC;
    if (temp_v0_8 < temp_v1_14) {
        D_8009D278->unk58 = (s16) (temp_v1_14 - temp_v0_8);
        D_8009D278->unk5A = (u16) D_8009D254->unk210;
        D_8009D278->unk5E = 0x1EU;
        D_8009D278->unk5C = (s16) (D_8009D254->unk212 - 8);
        D_8009D278->unk5F = 1;
        D_8009D278->unkE = (u16) D_8009D278->unkC;
    }
    temp_a1_2 = D_8009D278;
    if (temp_a1_2->unk5E != 0) {
        func_80032B0C(0, temp_a1_2 + 0x58, temp_v0_8);
        D_8009D278->unk5E = (u8) (D_8009D278->unk5E - 1);
    }
    if (temp_a1_2->unk66 != 0) {
        func_80032B0C(0, temp_a1_2 + 0x60);
        D_8009D278->unk66 = (u8) (D_8009D278->unk66 - 1);
    }
    temp_a0_4 = D_8009D278->unk8;
    D_8009D278->unkE = (u16) D_8009D278->unkC;
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
    var_v0 = D_8009D278->unkC;
    if (var_v0 <= 0) {
        D_8009D254->unk68 = 0;
        D_8009D254->unk6C = 0;
        D_8009D254->unk70 = 0;
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
        func_8006DE80(0x46B, 0, D_8009D254->unk2A, D_8009D254->unk2E, (s32) D_8009D254->unk32);
        D_8009D244 = 0;
        func_80062F9C();
        if (D_800BCF88 & 0x2000) {
            func_80067CBC();
        }
        D_8009D1A0 &= ~4;
        D_8009D2E8 |= 1;
        func_8001A680(D_8009D254, 0x13U, -5);
        var_v0 = (s16) D_8009D278;
        var_v0->unkC = 0;
    }
    return (s32) var_v0;
}

