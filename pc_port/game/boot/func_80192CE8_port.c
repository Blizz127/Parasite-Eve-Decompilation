/*
 * Overlay-local func_80192CE8 — complete CFG including the post-E08 media
 * loop. Retail [0x80192CE8,0x80192F98), 172 words, SHA-256
 * ed89408bffde43742781c89f1d16b7a98c52dc165b67cafd62cfdcf9319e03e7.
 *
 * Prefix through jal func_801924F8 matches B54K-Y/Z/AA. Post-E08
 * [0x80192E08,0x80192F98) SHA-256
 * 77218c9c335f6b4a24416a39d21d09876d7bc9f74778fefb26d28d501dbfd01a
 * drives pad -> 80192934 -> clear/abort -> 70E54 until DBA clears, then
 * clears D_800B0CD8 bit 0x200. Evidence: docs/evidence/pe-92ce8-post-e08.
 *
 * 80192934 body beyond its DBA<2 gate / C89C Stage-1b cut is drafted
 * separately; this loop calls it without re-wiring live C89C into 924F8.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

#define GA_READ_DEST       0x8001160Cu
#define GA_ARENA           0x80011610u
#define GA_READ_TABLE      0x8009315Eu
#define GA_PEIMG_LBA       0x800B0DD8u
#define GA_OVERLAY_FLAGS   0x800B0CD8u
#define GA_RECORD_BASE     0x801D0E04u
#define GA_DBA             0x800B0DBAu
#define GA_DBC             0x800B0DBCu
#define GA_PAD_HELD        0x8009D26Cu

extern int PE_func_80191FB8_Values(int count, const pe_addr_t *sources);
extern int func_80192934(void);
extern void func_8007A2A4(void);
extern int func_80080DC4(int command, pe_addr_t param, pe_addr_t response);

static void pe_92ce8_clear_stream(void)
{
    PE_StoreU32(0x801D0DE8u, 0u);
    PE_StoreU32(0x801D0DECu, 0u);
    PE_StoreU32(0x801D0DFCu, 0u);
    PE_StoreU32(0x801D0DF8u, 0u);
    PE_StoreU32(0x801D0DF0u, 0u);
    PE_StoreU32(0x801D0DF4u, 0u);
    PE_StoreU8(GA_DBA, 0u);
}

int func_80192CE8(int index)
{
    uint16_t start;
    uint16_t end;
    uint32_t stream;
    int status;
    int s3 = 0;
    int16_t frame_count;

    PE_StoreU32(GA_OVERLAY_FLAGS,
                PE_LoadU32(GA_OVERLAY_FLAGS) | 0x200u);
    PE_StoreU8(GA_RECORD_BASE + (uint32_t)index * 20u, 1u);

    func_80074D28(0); /* SetDispMask */
    (void)func_80074DC0(0); /* DrawSync */
    (void)func_80074A44(1); /* ResetGraph light path */

retry_issue:
    start = PE_LoadU16(GA_READ_TABLE);
    end = PE_LoadU16(GA_READ_TABLE + 2u);
    do {
        status = func_8006E6A8(
            (int)(PE_LoadU32(GA_PEIMG_LBA) + start),
            PE_LoadU32(GA_READ_DEST),
            (int)((uint32_t)end - (uint32_t)start));
    } while (status == -1);

    for (;;) {
        status = func_8006E7E8();
        if (status == 0)
            break;
        if (status == -1)
            goto retry_issue;
    }

    (void)func_80072714();
    func_800726C4();
    func_80072724();

    stream = PE_LoadU32(GA_ARENA) +
             ((uint32_t)(PE_LoadU16(0x80093162u) -
                         PE_LoadU16(0x80093160u)) << 11);
    (void)PE_func_80191FB8_Values(1, &stream);
    if (PE_Port_ShouldStop())
        return -1;

    (void)func_801924F8((int16_t)index);
    if (PE_Port_ShouldStop())
        return -1;

    /* ---- post-E08 media loop [0x80192E08,0x80192F98) ---- */
    if (PE_LoadU8(GA_DBA) == 0u)
        goto epilogue;

    for (;;) {
        frame_count = (int16_t)PE_LoadU16(GA_DBC);
        if (frame_count <= 0)
            break;

        func_8003EB04();
        if (PE_Port_ShouldStop())
            return -1;

        status = func_80192934();
        if (PE_Port_ShouldStop())
            return -1;

        /* Retail tests the low byte of the return via sll 24. */
        if (((uint32_t)status << 24) == 0u) {
            pe_92ce8_clear_stream();
        } else if (PE_LoadU32(GA_PAD_HELD) & 0x20000004u) {
            PE_StoreU8(GA_DBA, (uint8_t)(PE_LoadU8(GA_DBA) - 1u));
            func_800870F0(0u);
            if (PE_Port_ShouldStop())
                return -1;
            func_8010C0D8(0u);
            if (PE_Port_ShouldStop())
                return -1;
            func_8007A2A4();
            if (PE_Port_ShouldStop())
                return -1;
            (void)func_80080DC4(9, 0u, 0u);
            if (PE_Port_ShouldStop())
                return -1;
            pe_92ce8_clear_stream();
            if (frame_count < 1400) {
                func_80073A44(0); /* VSync */
                func_80074D28(0); /* SetDispMask */
                s3 = 1;
            }
        }

        func_80070E54();
        if (PE_Port_ShouldStop())
            return -1;
        if (PE_LoadU8(GA_DBA) == 0u)
            break;
    }

epilogue:
    PE_StoreU32(GA_OVERLAY_FLAGS,
                PE_LoadU32(GA_OVERLAY_FLAGS) & ~0x200u);
    return s3;
}
