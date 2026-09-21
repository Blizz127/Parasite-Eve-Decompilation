/*
 * Field-menu Equipment list2 draw: tier-2 wrapper (hand adapter).
 *
 * Original: matched C leaf src/func_8004B55C.c, [0x8004B55C,0x8004B584),
 * 10 words (configs/USA/disc1.yaml:703):
 *
 *     void func_8004B55C(int slot) { func_800638D8(slot, func_800504BC); }
 *
 * Hand adapter for the guest identity 0x800504BC that func_8004B13C stores
 * into list2+0x30.  0x800504BC has its own generated native port
 * (pc_port/game/decomp/func_800504BC_port.c).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8004B55C(pe_addr_t slot)
{
    func_800638D8(slot, 0x800504BCu);
}
