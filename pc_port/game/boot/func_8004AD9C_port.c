/* Original save/load file-menu page reached from func_80043DA4 command 5.
 * 37CD0.s / 3BD84.s. Every entry below is a non-matching retail function
 * transcribed instruction-for-instruction; callbacks are installed at the
 * same guest offsets as retail (+0x2C page input handler, +0x30 draw) and the
 * dispatch switches in func_80063E0C_port.c / func_800638D8_port.c resolve
 * them back to these native functions.
 *
 * The page tree is:
 *   func_8004AD9C  0x20 "save/load" window -> handler func_8004AE1C
 *   func_8004AF3C  0x21 slot-list page A   -> handler func_8004AFA4
 *   func_8004B03C  0x23 slot-list page B   -> handler func_8004B0A4
 *   func_8004B13C  0x2E slot-detail page   -> handler func_8004B394
 *   func_8004B584  0x38 modal confirm page -> handler func_8004B650
 * func_8004B5DC (the 0x38 draw callback) still depends on the unported
 * 0x8005E text/font subsystem and stays behind the loud drawing boundary. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

void func_8004AD9C(pe_addr_t owner)
{
    pe_addr_t window=func_80062D2C(0x20u,owner,0u,0u);
    pe_addr_t node=func_8006322C(0x20u,window,window);
    PE_StoreU32(window+44u,0x8004AE1Cu);
    PE_StoreU32(node+48u,0x8004FF30u);
    func_80062CB8(node);
    func_800647D0(node,4);
}

/* Confirm dispatches through jtbl_80011034 (0-indexed, six entries):
 * 0 -> func_8004AF3C, 1 -> func_8004B03C, 2 -> func_8004B13C,
 * 3 -> func_8004B584, 4 and 5 both -> the D_800C0E44 status block.
 * The status block ends with one func_800525EC and then falls through to the
 * shared tail's second func_800525EC, so cases 4/5 issue it twice. */
int32_t func_8004AE1C(pe_addr_t page,uint32_t event)
{
    pe_addr_t list=func_80062A20(page,0u);
    if (event&0x10000u) {
        int32_t selected=func_80063428(list);
        if ((uint32_t)selected<6u) {
            switch (selected) {
            case 0: func_8004AF3C(list);break;
            case 1: func_8004B03C(list);break;
            case 2: func_8004B13C(list);break;
            case 3: func_8004B584(list);break;
            default:
                func_8005D994(func_80063428(list)-4);
                func_80062F1C(page);
                func_800439D8();
                func_800525EC();
                break;
            }
        }
        func_800525EC();
        return 1;
    }
    if (event&0x40u) {
        func_80062F1C(page);
        func_800439D8();
        func_80052634();
    }
    return 1;
}

void func_8004AF3C(pe_addr_t owner)
{
    pe_addr_t window=func_80062D2C(0x21u,owner,0u,0u);
    pe_addr_t node=func_8006322C(0x21u,window,window);
    PE_StoreU32(window+44u,0x8004AFA4u);
    PE_StoreU32(node+48u,0x8004FF58u);
    func_80062CB8(node);
}

int32_t func_8004AFA4(pe_addr_t page,uint32_t event)
{
    pe_addr_t list=func_80062A20(page,0u);
    if (event&0x10000u) {
        func_800525EC();
        func_80052790((int32_t)func_80063428(list));
    } else if (event&0x40u) {
        func_80062F1C(page);
        func_80052634();
    }
    return 1;
}

void func_8004B03C(pe_addr_t owner)
{
    pe_addr_t window=func_80062D2C(0x23u,owner,0u,0u);
    pe_addr_t node=func_8006322C(0x23u,window,window);
    PE_StoreU32(window+44u,0x8004B0A4u);
    PE_StoreU32(node+48u,0x8004FF80u);
    func_80062CB8(node);
}

int32_t func_8004B0A4(pe_addr_t page,uint32_t event)
{
    pe_addr_t list=func_80062A20(page,0u);
    if (event&0x10000u) {
        func_800525EC();
        func_800649D0((int32_t)func_80063428(list));
    } else if (event&0x40u) {
        func_80062F1C(page);
        func_80052634();
    }
    return 1;
}

void func_8004B13C(pe_addr_t owner)
{
    pe_addr_t window=func_80062D2C(0x2Eu,owner,0u,0u);
    pe_addr_t node2=func_8006322C(0x2Eu,window,window);
    pe_addr_t node3;
    PE_StoreU32(window+48u,0x8004B214u);
    PE_StoreU32(window+44u,0x8004B394u);
    PE_StoreU32(window+76u,0x800922D4u);
    PE_StoreU32(window+64u,1u);
    PE_StoreU32(node2+48u,0x8004B534u);
    PE_StoreU32(node2+40u,1u);
    node3=func_8006322C(0x31u,window,window);
    PE_StoreU32(node3+48u,0x8004B55Cu);
    PE_StoreU32(node2+124u,node3);
    PE_StoreU32(node3+120u,node2);
    func_80062CB8((int32_t)PE_LoadU32(node2+68u)>=0?node2:node3);
    PE_StoreU32(0x8009CFE0u,(uint32_t)func_800614A0());
}

/* Three stat rows from the 32-bit packed meters at func_800614A0()'s backing
 * word: rows 0..2 are byte lanes 0..2, the up/down pair (0x1000/0x4000)
 * adds/subtracts 2 into the selected lane and clamps it, and confirm acts on
 * the page's second list (index 1).  Retail reaches the lanes with sllv/srav,
 * so the shift amount is masked to 5 bits exactly as the hardware does. */
int32_t func_8004B394(pe_addr_t page,uint32_t event)
{
    pe_addr_t list=func_80062A20(page,0u);
    int32_t selected=func_80063428(list);
    uint32_t lane_shift=((uint32_t)selected<<3)&31u;
    int32_t packed=func_800614A0();
    if (event&0x1000u) {
        uint32_t next=((((uint32_t)packed>>lane_shift)&0xFFu)+2u);
        if ((int32_t)next>=0xE9) next=0xE8u;
        func_800614AC((int32_t)(((uint32_t)packed&~(0xFFu<<lane_shift))|(next<<lane_shift)));
        func_8005267C();
        return 1;
    }
    if (event&0x4000u) {
        uint32_t next=((((uint32_t)packed>>lane_shift)&0xFFu)-2u);
        if ((int32_t)next<0x20) next=0x20u;
        func_800614AC((int32_t)(((uint32_t)packed&~(0xFFu<<lane_shift))|(next<<lane_shift)));
        func_8005267C();
        return 1;
    }
    if (event&0x10000u) {
        pe_addr_t sublist=func_80062A20(page,1u);
        int32_t sub_selected=func_80063428(sublist);
        if (sub_selected==1) func_800614AC(0x404040);
        if (sub_selected!=0 && sub_selected!=1) {
            PE_StoreU32(sublist+68u,0u);
            PE_StoreU32(sublist+72u,0u);
            func_80062CB8(sublist);
            PE_StoreU32(func_80062A20(page,0u)+68u,UINT32_MAX);
        } else {
            PE_StoreU32(0x800C0E44u,(uint32_t)func_800614A0());
            func_80062F1C(page);
        }
        func_800525EC();
        return 1;
    }
    if (event&0x40u) {
        func_80062F1C(page);
        func_800614AC((int32_t)PE_LoadU32(0x8009CFE0u));
        func_80052634();
    }
    return 1;
}

/* 0x2E page draw callback: three dim-able rows over two 4000/4B labelled
 * groups.  func_8005E54C takes no arguments, so retail's leftover a0 at the
 * second call site is not part of the call contract. */
void func_8004B214(pe_addr_t node)
{
    pe_addr_t list=func_80062A20(node,0u);
    int32_t selected=func_80063428(list);
    uint32_t flags;

    flags=(uint32_t)func_8005E54C()&0x1000u;
    func_8005E8A4(0x12,5);
    func_8005EB58((selected!=0||flags==0u)?1u:0u);
    func_8005EB64(0x4Au);
    func_8005E8A4(0x20,0);
    func_8005EB58((selected==1&&flags!=0u)?0u:1u);
    func_8005EB64(0x4Au);
    func_8005E8A4(0x20,0);
    func_8005EB58((selected==2&&flags!=0u)?0u:1u);
    func_8005EB64(0x4Au);

    flags=(uint32_t)func_8005E54C()&0x4000u;
    func_8005E8A4(-0x40,0x19);
    func_8005EB58((selected!=0||flags==0u)?1u:0u);
    func_8005EB64(0x4Bu);
    func_8005E8A4(0x20,0);
    func_8005EB58((selected==1&&flags!=0u)?0u:1u);
    func_8005EB64(0x4Bu);
    func_8005E8A4(0x20,0);
    func_8005EB58((selected==2&&flags!=0u)?0u:1u);
    func_8005EB64(0x4Bu);
}

void func_8004B584(pe_addr_t owner)
{
    pe_addr_t window=func_80062D2C(0x38u,owner,0u,1u);
    PE_StoreU32(window+48u,0x8004B5DCu);
    PE_StoreU32(window+44u,0x8004B650u);
    func_80062CB8(window);
    PE_StoreU32(0x8009CFE4u,(uint32_t)func_8005E884());
}

int32_t func_8004B650(pe_addr_t window,uint32_t event)
{
    if (event&0x1000u) {
        func_8005E850(0,-1);
        func_8005267C();
        return 1;
    }
    if (event&0x4000u) {
        func_8005E850(0,1);
        func_8005267C();
        return 1;
    }
    if (event&0x10000u) {
        func_80062F1C(window);
        func_800525EC();
        return 1;
    }
    if (event&0x40u) {
        func_8005E850(0,(int32_t)(PE_LoadU32(0x8009CFE4u)-(uint32_t)(int32_t)func_8005E884()));
        func_80062F1C(window);
        func_80052634();
        return 1;
    }
    return 1;
}

/* The five list-draw wrappers are matched C leaves (src/func_8004*.c,
 * byte-exact in the retail build).  Each is one func_800638D8 call whose
 * second argument is a *guest function address*; the decomp-port generator
 * would emit the bare function designator and lose that conversion, so the
 * wrappers are transcribed here with the address spelled out.  The leaf in
 * src/ remains the authority for the slot argument and the target. */
void func_8004FF30(int32_t slot) {func_800638D8((pe_addr_t)slot,0x80050C50u);}
void func_8004FF58(int32_t slot) {func_800638D8((pe_addr_t)slot,0x80050C70u);}
void func_8004FF80(int32_t slot) {func_800638D8((pe_addr_t)slot,0x80050CB4u);}
void func_8004B534(int32_t slot) {func_800638D8((pe_addr_t)slot,0x80050438u);}
void func_8004B55C(int32_t slot) {func_800638D8((pe_addr_t)slot,0x800504BCu);}
