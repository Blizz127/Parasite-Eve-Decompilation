/*
 * Phase 6E-B54K-Y — retail prefix of overlay-local func_80192CE8.
 *
 * Authenticated range: [0x80192CE8,0x80192DFC), 69 words, SHA-256
 * 0670dc9a913589495f623812b226d1ac665ca7984a6cbbe15efc7c200c038855.
 * The prefix marks the indexed 20-byte record, performs the first table-
 * selected PE.IMG read with the retail retry/poll CFG, balances the cache
 * critical section, and enters func_80191FB8(1, sp+0x10).  The stack word
 * contains arena + ((D_80093162-D_80093160)<<11). B54K-Z completes that
 * callee; B54K-AA enters the next call and its authenticated prefix.
 *
 * The remainder after the call at 0x80192E00 is not approximated here.
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

extern int PE_func_80191FB8_Values(int count, const pe_addr_t *sources);

int func_80192CE8(int index)
{
    uint16_t start;
    uint16_t end;
    uint32_t stream;
    int status;

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

    Bootstrap_ReturnVoid("func_80192CE8_80192E08_cut", "func_80192CE8");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return -1;
}
