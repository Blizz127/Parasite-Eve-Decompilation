/*
 * Phase 6D-S — Boot subsystem-init dispatcher.
 *
 * Callback registration now uses PE_Callback_Reset / PE_Callback_Register
 * instead of (int)(uintptr_t).  All BOOTSTRAP_RET callees go through the
 * centralized Bootstrap_* policy.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern unsigned int D_8009D1C4, D_8009D280, D_8009D1A0, D_8009D250;
extern int D_8009CDDC;
extern void func_80070D10(void);
extern void func_80070D6C(void);
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
    D_8009CDDC = 0;
    func_80070D10();
    for (i = 0; i < 0x7D0; i++) {
        func_80070D6C();
    }
    func_8003E974();
    func_80036DC8();

    /* Reset callback, then register func_8003E91C — retail calls
     * func_80073D24(0) then func_80073D24(&func_8003E91C); the order log
     * records the retail callee name for both invocations while the real
     * work goes through the full-width host-safe callback registry. */
    PE_Callback_Reset();
    Bootstrap_ReturnVoid("func_80073D24", "func_8003E680");
    PE_Callback_Register(func_8003E91C);
    Bootstrap_ReturnVoid("func_80073D24", "func_8003E680");

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
