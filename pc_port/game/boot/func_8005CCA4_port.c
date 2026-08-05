/*
 * Phase 6E-B28 — func_8005CCA4: resource-table initialization.
 * Full implementation with local Bootstrap_ReturnVoid declaration.
 */
#include "psx_compat.h"

extern unsigned int func_80052F70(void);
extern pe_addr_t func_8005DB8C(int idx);
extern pe_addr_t func_8005DBAC(int arg);
extern int func_800438C0(int arg);
extern void Bootstrap_ReturnVoid(const char *symbol, const char *caller);

static void B28_func_80053D2C(int arg) {
    Bootstrap_ReturnVoid("func_80053D2C", "func_8005CCA4");
}

#define GA_E28    0x800C0E28u
#define GA_E06    0x800C0E06u
#define GA_E08    0x800C0E08u
#define GA_E0C    0x800C0E0Cu
#define GA_E24    0x800C0E24u
#define GA_EAC    0x800C0EACu
#define GA_1EAC   0x800C1EACu
#define GA_2024   0x800C2024u
#define GA_E00    0x800C0E00u
#define GA_E0A    0x800C0E0Au
#define GA_E22    0x800C0E22u
#define GA_E20    0x800C0E20u
#define GA_E40    0x800C0E40u
#define GA_E48    0x800C0E48u
#define GP_2D8    0x8009D058u
#define GP_2E0    0x8009D060u
#define GP_2E8    0x8009D068u
#define GP_2F4    0x8009D074u
#define GA_1E6E   0x800A1E6Eu
#define GA_1E8E   0x800A1E8Eu
#define GA_1EAE   0x800A1EAEu

void func_8005CCA4(void)
{
    pe_addr_t s0, s1, s2, s3;
    int i;

    s1 = GA_E28;
    for (i = 0; i < 7; i++) {
        pe_addr_t val = func_8005DB8C(i);
        PE_StoreU16(s1, (uint16_t)(val & 0xFFFFu));
        s1 += 2u;
    }

    s0 = func_8005DBAC(0);

    {
        uint16_t hw = (s0 != 0u) ? PE_LoadU16(s0) : 0u;
        PE_StoreU16(GA_E08, hw);
        PE_StoreU16(GA_E06, hw);
    }
    {
        uint8_t b = (s0 != 0u) ? PE_LoadU8(s0 + 7u) : 0u;
        PE_StoreU32(GA_E24, 1u);
        PE_StoreU16(GA_1EAE, 0u);
        PE_StoreU16(GA_1E8E, 0u);
        PE_StoreU16(GA_1E6E, 0u);
        PE_StoreU32(GP_2D8, s1);
        PE_StoreU8(GA_E0C, b);
    }

    s2 = 2u;
    s3 = 0x8009D05Cu;
    {
        unsigned int ret = func_80052F70();
        PE_StoreU32(GP_2E0, ret);
        PE_StoreU32(GP_2E8, s3);
        PE_StoreU32(GP_2F4, s2);
    }

    {
        uint8_t b7 = (s0 != 0u) ? PE_LoadU8(s0 + 7u) : 0u;
        if (b7 >= 0x33u) b7 = 0x32u;
        PE_StoreU8(GA_E0C, b7);
        if (PE_LoadU32(GP_2D8) != s1) {
            unsigned int ret = func_80052F70();
            PE_StoreU32(GP_2E0, ret);
        }
    }

    PE_StoreU32(GP_2D8, s1);
    {
        unsigned int ret = func_80052F70();
        PE_StoreU32(GP_2E0, ret);
        PE_StoreU32(GP_2E8, s3);
        PE_StoreU32(GP_2F4, s2);
    }

    {
        pe_addr_t p = GA_E48;
        int count = 0x31;
        while (count >= 0) {
            PE_StoreU16(p, 0u);
            count--;
            p -= 2u;
        }
    }

    B28_func_80053D2C(0x44);
    B28_func_80053D2C(0x96);
    B28_func_80053D2C(0x3F);
    B28_func_80053D2C(1);
    B28_func_80053D2C(6);

    s0 = 0u;
    {
        pe_addr_t a1 = GA_EAC;
        pe_addr_t end = a1 + 0x1000u;
        pe_addr_t v1 = a1 + 5u;
        while (a1 < end) {
            uint8_t b0 = PE_LoadU8(a1);
            if (b0 != 0u) {
                uint8_t b6 = PE_LoadU8(v1 + 1u);
                if (b6 != 9u) {
                    uint8_t b5 = PE_LoadU8(v1);
                    if ((b5 & 0x10u) != 0u) {
                        pe_addr_t a2 = GA_1EAC;
                        if (a1 < a2) {
                            pe_addr_t base = a2 - 0x1000u;
                            int idx = (int)((a1 - base) >> 5) + 0x100;
                            pe_addr_t v = a2 + 0xD4u;
                            pe_addr_t a2end = a2 + 0x178u;
                            while (v < a2end) {
                                int16_t hw = (int16_t)PE_LoadU16(v);
                                if (hw == (int16_t)idx) {
                                    if (v < GA_2024) PE_StoreU16(v, 0u);
                                    break;
                                }
                                v += 2u;
                            }
                            s0 = (pe_addr_t)idx;
                        }
                        break;
                    }
                }
            }
            a1 += 0x20u;
            v1 += 0x20u;
        }
    }

    if (s0 != 0u) B28_func_80053D2C(0);

    {
        pe_addr_t a1 = GA_EAC;
        pe_addr_t end = a1 + 0x1000u;
        pe_addr_t v1 = a1 + 5u;
        while (a1 < end) {
            uint8_t b0 = PE_LoadU8(a1);
            if (b0 != 0u) {
                uint8_t b6 = PE_LoadU8(v1 + 1u);
                if (b6 == 9u) {
                    uint8_t b5 = PE_LoadU8(v1);
                    if ((b5 & 0x10u) != 0u) {
                        pe_addr_t a2 = GA_1EAC;
                        if (a1 < a2) {
                            pe_addr_t base = a2 - 0x1000u;
                            int idx = (int)((a1 - base) >> 5) + 0x100;
                            pe_addr_t v = a2 + 0xD4u;
                            pe_addr_t a2end = a2 + 0x178u;
                            while (v < a2end) {
                                int16_t hw = (int16_t)PE_LoadU16(v);
                                if (hw == (int16_t)idx) {
                                    if (v < GA_2024) PE_StoreU16(v, 0u);
                                    break;
                                }
                                v += 2u;
                            }
                            s0 = (pe_addr_t)idx;
                        }
                        break;
                    }
                }
            }
            a1 += 0x20u;
            v1 += 0x20u;
        }
    }

    if (s0 != 0u) B28_func_80053D2C(0);

    PE_StoreU8(GA_E22, (uint8_t)(s0 != 0u ? 1u : 0u));
    PE_StoreU8(GA_E20, 0u);
    PE_StoreU16(GA_E40, 0x3Du);
    func_800438C0(0x3D);
    PE_StoreU32(GA_E00, 0u);
    PE_StoreU8(GA_E0A, 0u);

    Bootstrap_ReturnVoid("func_80042C78", "func_8005CCA4");
}
