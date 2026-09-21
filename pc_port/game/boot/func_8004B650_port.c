/*
 * Field-menu modal sub-page input handler (installed by func_8004B584 into
 * window+0x2C).
 *
 * Original: [0x8004B650,0x8004B70C), 0xBC bytes / 47 words, asm/disc1/3BD84.s.
 *
 *   if (event & 0x1000) { func_8005E850(0, -1); func_8005267C(); return 1; }
 *   if (event & 0x4000) { func_8005E850(0,  1); func_8005267C(); return 1; }
 *   if (event & 0x10000) { func_80062F1C(node); func_800525EC(); return 1; }
 *   if (event & 0x40) {
 *       func_8005E850(0, D_8009D264 - func_8005E884());  ; alarm-timer delta
 *       func_80062F1C(node); func_80052634(); return 1;
 *   }
 *   return 1;
 *
 * D_8009D264 is the Alarm/timer snapshot func_8004B584 published when the modal
 * window was constructed (the same word func_8004B6CC reads).  func_8005E884()
 * returns the current signed alarm byte, so the cancel arm scrolls the modal by
 * the elapsed delta.  This was the named boundary `PE_MenuInputCallback_8004B650`.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

int func_8004B650(pe_addr_t node, uint32_t event)
{
    if (event & 0x1000u) {
        func_8005E850(0, -1);
        func_8005267C();
        return 1;
    }
    if (event & 0x4000u) {
        func_8005E850(0, 1);
        func_8005267C();
        return 1;
    }
    if (event & 0x10000u) {
        func_80062F1C(node);
        func_800525EC();
        return 1;
    }
    if (event & 0x40u) {
        int32_t delta = (int32_t)(PE_LoadU32(0x8009D264u) -
                                  (uint32_t)(int32_t)func_8005E884());
        func_8005E850(0, delta);
        func_80062F1C(node);
        func_80052634();
        return 1;
    }
    return 1;
}
