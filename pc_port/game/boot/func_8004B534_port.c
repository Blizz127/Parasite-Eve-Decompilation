/*
 * Field-menu Equipment list1 draw: tier-2 wrapper (hand adapter).
 *
 * Original: matched C leaf src/func_8004B534.c, [0x8004B534,0x8004B55C),
 * 10 words (configs/USA/disc1.yaml:700):
 *
 *     void func_8004B534(int slot) { func_800638D8(slot, func_80050438); }
 *
 * Hand adapter for the guest identity 0x80050438 that func_8004B13C stores
 * into list1+0x30.  The per-cell draw 0x80050438 itself is still a named
 * boundary (PE_MenuDrawCell).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8004B534(pe_addr_t slot)
{
    func_800638D8(slot, 0x80050438u);
}
