/*
 * Phase 6E-B54K-Q — execution-proven positive path of func_8005C1EC.
 *
 * Retail 0x8005C1EC..0x8005C25C gates a renderer/event state word.  The
 * func_801909B4 caller passes one: the first positive call publishes one at
 * D_8009D030 and sets D_800B0CD8 bits 0xC000; later positive calls are
 * inert.  The zero path first clears D_8009D030 and then calls the still
 * untranslated event-record cleanup func_80042798.  Preserve that call as
 * a named boundary rather than inventing event closure.
 */
#include "psx_compat.h"
#include "game_port.h"

void func_8005C1EC(int enabled)
{
    if (enabled == 0) {
        PE_StoreU32(0x8009D030u, 0u);
        Bootstrap_ReturnVoid("func_80042798", "func_8005C1EC");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }

    if (PE_LoadU32(0x8009D030u) == 0u) {
        PE_StoreU32(0x8009D030u, 1u);
        PE_StoreU32(0x800B0CD8u, PE_LoadU32(0x800B0CD8u) | 0xC000u);
    }
}
