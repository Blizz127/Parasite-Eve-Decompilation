/*
 * Field-menu Items sub-page input handler (installed by func_8004AF3C).
 *
 * Original: [0x8004AFA4,0x8004B03C), 0x98 bytes / 38 words, asm/disc1/37CD0.s.
 *
 *   list = func_80062A20(node, 0)
 *   if (event & 0x10000) {                 ; confirm: commit the item selection
 *       func_800525EC();                   ; menu confirm sound
 *       func_80052790(func_80063428(list)) ; boolean-state store + notify
 *       func_80062F1C(node);               ; close this sub-page
 *       return 1;
 *   }
 *   if (event & 0x40) {                    ; cancel: close unchanged
 *       func_80062F1C(node);
 *       func_80052634();                   ; menu cancel sound
 *   }
 *   return 1;
 *
 * This is the confirm path the plain route's milestone 55
 * ("normal item use restores 45 HP") travels through.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

int func_8004AFA4(pe_addr_t node, uint32_t event)
{
    pe_addr_t list = func_80062A20(node, 0u);

    if (event & 0x10000u) {
        func_800525EC();
        func_80052790(func_80063428(list));
        func_80062F1C(node);
        return 1;
    }
    if (event & 0x40u) {
        func_80062F1C(node);
        func_80052634();
    }
    return 1;
}
