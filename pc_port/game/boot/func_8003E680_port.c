/*
 * Phase 6D-S — Boot subsystem-init dispatcher.
 *
 * Phase 6E-B6: callback registration now goes through the REAL
 * func_80073D24 (guest-backed VBlank slot table, pe_libetc.c).  The
 * PE_Callback_Bind of guest 0x8003E91C to its host stub is host plumbing
 * (idempotent), the counterpart of retail passing the guest address.
 * All remaining BOOTSTRAP_RET callees go through the centralized
 * Bootstrap_* policy.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern unsigned int D_8009D1C4, D_8009D250;
extern void func_80070D10(void);
extern unsigned int func_80070D6C(void);
extern void func_8003E974(void);
extern void func_80036DC8(void);
extern void func_8003E91C(void);
extern void func_800371A4(int);
extern void func_80029388(void);
extern void func_8005BCA8(void);
extern void func_80068D28(void);
extern void func_800124F8(void);
extern void func_8001A890(void);
extern void func_80034F10(void);
extern void func_8006536C(void);

void func_8003E680(void)
{
    unsigned int i;
    D_8009D1C4 = 0;
    D_8009D280 = 0;
    D_8009D1A0 = 0;
    D_8009D250 = 0;
    PE_StoreU32(0x8009CDDCu,0);
    func_80070D10();
    for (i = 0; i < 0x7D0; i++) {
        func_80070D6C();
    }
    func_8003E974();
    func_80036DC8();

    /* func_80073D24(0) then func_80073D24(&func_8003E91C) — real libetc
     * VBlank callback slot-4 writes (Phase 6E-B6); both returns discarded,
     * matching retail.  The bind is host plumbing (idempotent): it maps
     * the guest address 0x8003E91C to its host implementation so the
     * dispatcher can resolve it. */
    PE_Callback_Bind(0x8003E91Cu, func_8003E91C);
    (void)func_80073D24(0u);
    (void)func_80073D24(0x8003E91Cu);

    func_800371A4(0);
    func_80029388();
    func_8005BCA8();
    func_80068D28();
    func_800124F8();
    func_8001A890();
    func_80034F10();
    func_8006536C();
    func_80038D1C();
}
