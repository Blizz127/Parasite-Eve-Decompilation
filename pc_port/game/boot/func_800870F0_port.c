/*
 * Phase 6E-MV1a — func_800870F0 (42 words, 0x800870F0..0x80087198):
 * movie-frame scaler latch.  Pure byte stores, then a call to the
 * untranslated func_8007A88C (no asm in tree) — the CDQ1-anticipated
 * boundary.  Flag clear ([D_8009D2C0] & 2 == 0): [D1CA] = [D1C8] =
 * a0, [D1CB] = [D1C9] = 0.  Flag set: all four bytes hold
 * ((2903 * a0) >> 13) & 0xFF — the sll/addu/subu/srl chain is a
 * linear combination, so one 32-bit multiply-and-shift matches
 * retail exactly (wrapping included).  The beqz delay slot stores
 * $ra (executes on both arms); 7A88C's return is ignored (void).
 */

#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_bootstrap.h"

#define MV_D2C0 0x8009D2C0u
#define MV_D1C8 0x8009D1C8u
#define MV_D1C9 0x8009D1C9u
#define MV_D1CA 0x8009D1CAu
#define MV_D1CB 0x8009D1CBu

void func_8007A88C(pe_addr_t p)
{
    (void)p;
    Bootstrap_ReturnVoid("func_8007A88C", "func_800870F0");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

void func_800870F0(uint32_t a0)
{
    if ((PE_LoadU32(MV_D2C0) & 2u) == 0u) {
        uint8_t v = (uint8_t)a0;
        PE_StoreU8(MV_D1CA, v);
        PE_StoreU8(MV_D1C8, v);
        PE_StoreU8(MV_D1CB, 0u);
        PE_StoreU8(MV_D1C9, 0u);
    } else {
        uint8_t v = (uint8_t)(((uint32_t)(2903u * a0)) >> 13);
        PE_StoreU8(MV_D1CB, v);
        PE_StoreU8(MV_D1C9, v);
        PE_StoreU8(MV_D1CA, v);
        PE_StoreU8(MV_D1C8, v);
    }
    func_8007A88C(MV_D1C8);
}
