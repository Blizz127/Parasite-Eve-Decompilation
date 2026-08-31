/*
 * Phase 6E-B54K-AF — bounded libpress DecDCTReset wrapper.
 *
 * Retail function: [0x8010BE3C,0x8010BE70), 13 words, SHA-256
 * d29b6f4ff5d6c611a9193862caffbce26eae005f5cca46747f4fc780ffdc4f9d.
 * The mode-zero arm calls ResetCallback, then every mode forwards unchanged
 * to the internal MDEC reset routine at 0x8010C0FC. That hardware routine is
 * deliberately retained as the next exact boundary.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

void func_8010BE3C(int mode)
{
    if (mode == 0)
        func_80073C94();

    Bootstrap_ReturnVoid1("func_8010C0FC", "func_8010BE3C",
                          (uint32_t)mode);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
