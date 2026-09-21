/* SEW17: original field item award and pickup window, native translation.
 * 194B0..194F8, 15BAC..15C7C, 532B4..5332C, 4F490..4F808,
 * 50204..50260, 51060..51084, 19540..19618, 4C34C..4C4B4 and
 * 55E14..55FB4 in the SHA-1-verified Disc 1 EXE. */
#include "psx_compat.h"
#include "pe_port_compat.h"

pe_addr_t func_800532B4(uint32_t id)
{
    if (id-256u<128u) return 0x800BEEACu+id*32u;
    if (id-1u<255u) return func_8005DB44(id-1u);
    if (id-512u<9u) return 0x8009DE64u+id*32u;
    return 0u;
}

int func_800194B0(pe_addr_t args)
{
    int result=func_80053D2C((int32_t)PE_LoadU32(PE_LoadU32(args)));
    PE_StoreU32(PE_LoadU32(args+4u),(uint32_t)result);
    return 1;
}

/* Retail 55E14..55FB4: mark items eligible for chest storage. */
void func_80055E14(void)
{
    int32_t count=(int32_t)PE_LoadU32(0x8009D064u);
    pe_addr_t bits=PE_LoadU32(0x8009D058u);
    for (int32_t i=0;i<count;i++) PE_StoreU32(bits+(uint32_t)i*4u,0u);
    for (int32_t i=0;i<(int32_t)PE_LoadU32(0x8009D050u);i++) {
        pe_addr_t record,entry;uint32_t value;
        if (i==(int8_t)PE_LoadU8(0x800C0E20u) ||
            i==(int8_t)PE_LoadU8(0x800C0E22u) || !func_80057654(i)) continue;
        record=func_8005332C(i);
        entry=PE_LoadU32(0x8009D058u)+((uint32_t)i>>5u)*4u;
        value=PE_LoadU32(entry);
        if (record && !(PE_LoadU8(record+5u)&0xE0u)) value|=1u<<((uint32_t)i&31u);
        PE_StoreU32(entry,value);
    }
}

/* Retail 4C34C..4C4B4: create the item-selection window once. */
int func_8004C34C(uint32_t item)
{
    pe_addr_t window,list;uint32_t saved;
    if (func_80062A34(1u,1u)) return 1;
    func_8005DE88();
    window=func_80062D2C(27u,0u,0u,0u);PE_StoreU32(window+48u,0x800447F0u);
    window=func_80062D2C(1u,0u,0u,0u);list=func_8006322C(1u,window,window);
    PE_StoreU32(window+44u,0x80044444u);PE_StoreU32(list+48u,0x8004F8D0u);
    func_80062CB8(list);
    PE_StoreU32(list+132u,0x80057C54u);PE_StoreU32(list+136u,0x80050260u);
    func_80055760();PE_StoreU32(0x8009CF94u,UINT32_MAX);PE_StoreU32(0x8009CF8Cu,UINT32_MAX);
    func_800647D0(list,(int32_t)func_80052F70());
    saved=PE_LoadU32(0x8009CF98u);PE_StoreU32(0x8009CF00u,0u);
    if (saved) {
        saved--;
        PE_StoreU32(list+68u,saved&1u);PE_StoreU32(list+72u,(saved>>1u)&127u);
        PE_StoreU32(list+92u,(uint32_t)((int32_t)saved>>8));
    }
    func_8004C594();PE_StoreU32(func_80062A34(2u,1u)+132u,0u);
    func_80055E14();PE_StoreU32(0x8009CF00u,1u);PE_StoreU32(0x8009CF5Cu,item);
    PE_StoreU32(0x8009CEFCu,0u);func_8005B890(0);
    return 0;
}

/* Retail opcode A9, 19540..19618: wait for selection or cancellation. */
int func_80019540(pe_addr_t args)
{
    pe_addr_t task=PE_LoadU32(0x8009D300u);
    uint32_t flags=PE_LoadU16(task+8u);
    if (!(flags&32u)) {
        PE_StoreU16(task+8u,flags|32u);func_80067CBC();
    } else {
        uint32_t result=PE_LoadU16(0x8009D2A4u);
        if (result-3u<384u || result==65535u) {
            PE_StoreU32(PE_LoadU32(args+4u),result==65535u?UINT32_MAX:result-3u);
            task=PE_LoadU32(0x8009D300u);
            PE_StoreU16(task+8u,PE_LoadU16(task+8u)&0xFFDFu);
            return 1;
        }
        (void)func_8004C34C(PE_LoadU32(PE_LoadU32(args)));
    }
    PE_StoreU32(0x8009CE00u,PE_LoadU32(0x8009CE00u)-16u);
    PE_StoreU32(PE_LoadU32(0x8009D300u)+16u,1u);
    return 0;
}

void func_8004F490(uint32_t item)
{
    pe_addr_t record=func_800532B4(item),window,list,help;
    PE_StoreU32(0x8009CF58u,record);
    if (PE_LoadU8(record+6u)<10u) {
        window=func_80062D2C(5u,0u,0u,0u);list=func_8006322C(5u,window,window);
        PE_StoreU32(window+48u,0x8004F644u);PE_StoreU32(window+44u,0x8004F730u);
        PE_StoreU32(window+52u,128u);func_80063158(window,68,20);
        PE_StoreU32(list+48u,0x80050204u);func_80064C20(list);
        PE_StoreU32(list+112u,0xFFFFFFFFu);func_80062CB8(window);
        window=func_80062D2C(6u,0u,0u,0u);PE_StoreU32(window+52u,128u);
        func_80063158(window,68,20);list=func_8006322C(6u,window,window);
        PE_StoreU32(list+48u,0x8005022Cu);func_80064C20(list);
        PE_StoreU32(list+60u,62u);
        func_800647D0(list,PE_LoadU8(PE_LoadU32(0x8009CF58u)+20u));
        PE_StoreU32(0x8009CF18u,PE_LoadU8(PE_LoadU32(0x8009CF58u)+6u)!=9u);
    } else {
        window=func_80062D2C(55u,0u,0u,0u);
        PE_StoreU32(window+48u,0x8004F7D8u);PE_StoreU32(window+44u,0x8004F730u);
        func_80062CB8(window);
    }
    help=func_80062D2C(19u,0u,0u,0u);PE_StoreU32(help+48u,0x8004F798u);
    func_8005DE88();
}

int func_80015BAC(pe_addr_t args)
{
    pe_addr_t task;
    uint16_t flags;
    if (PE_LoadU32(0x800B0CD8u)&0x1000u) return 1;
    task=PE_LoadU32(0x8009D300u);flags=PE_LoadU16(task+8u);
    if (!(flags&32u)) {
        PE_StoreU16(task+8u,flags|32u);PE_StoreU32(task+16u,1u);
        PE_StoreU32(0x8009CE00u,PE_LoadU32(0x8009CE00u)-12u);
        return 0;
    }
    func_80067CBC();func_8004F490(PE_LoadU32(PE_LoadU32(args)));
    PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)|0x9000u);
    task=PE_LoadU32(0x8009D300u);D_8009D1A0|=4u;
    PE_StoreU16(task+8u,PE_LoadU16(task+8u)&0xFFDFu);
    return 1;
}

void func_8004F644(void)
{
    pe_addr_t record;
    func_8004551C(PE_LoadU32(0x8009CF58u));record=PE_LoadU32(0x8009CF58u);
    if (!record) return;
    func_8005E8A4(42,-12);func_8005FDF0(PE_LoadU8(record+9u));
    func_8005E8A4(5,0);func_8005FF28((int16_t)PE_LoadU16(record+18u));
    func_8005E8A4(-45,-14);func_8005FDF0(PE_LoadU8(record+8u));
    func_8005E8A4(5,0);func_8005FF28((int16_t)PE_LoadU16(record+16u));
    func_8005E8A4(-45,-14);func_8005FDF0(PE_LoadU8(record+7u));
    func_8005E8A4(5,0);func_8005FF28((int16_t)PE_LoadU16(record+14u));
    func_8005E8A4(-45,-10);func_8005EB64(135u);
    func_8005E8A4(25,0);func_8005EB64(136u);
}

int func_8004F730(pe_addr_t window,uint32_t event)
{
    (void)window;
    if (event&0x10040u) {
        if (PE_LoadU32(0x8009CF38u)) {
            PE_StoreU32(0x8009CF38u,0u);
            PE_MenuCommitResult(PE_LoadU8(PE_LoadU32(0x8009CF58u)+4u)+3u);
        } else func_800512AC(9,0u);
        func_800525EC();
    }
    return 1;
}

void func_8004F798(void)
{
    func_8005E8A4(4,4);
    func_8005F27C(func_8005DCEC(PE_LoadU8(PE_LoadU32(0x8009CF58u)+4u)-1u));
}
void func_8004F7D8(void)
{ func_8005E8A4(6,6);func_80053648(PE_LoadU32(0x8009CF58u)); }
void func_80051060(void)
{ func_80053648(PE_LoadU32(0x8009CF58u)); }
void func_80050204(pe_addr_t list)
{ func_800638D8(list,0x80051060u); }
void func_8005022C(pe_addr_t list)
{
    PE_StoreU32(0x8009CEF4u,list);
    PE_StoreU32(0x8009CF20u,PE_LoadU32(0x8009CF58u));
    func_800638D8(list,0x80050AD8u);
}
