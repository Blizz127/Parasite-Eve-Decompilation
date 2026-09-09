/* Original menu button mapping and edge/repeat event producer (4E92C.s).
 * Events retain their guest free-list and FIFO ownership. */
#include "psx_compat.h"
#include "pe_port_compat.h"

uint32_t func_8005E038(void)
{
    static const uint32_t input[]={8u,32u,64u,16u,0x20000000u,0x40000000u,0x10000000u,
        0x80000000u,0x04000000u,0x08000000u,0x01000000u,0x02000000u,2u,4u};
    static const uint32_t output[]={0x1000u,0x4000u,0x8000u,0x2000u,0x20u,0x40u,0x10u,
        0x80u,4u,8u,1u,2u,0x100u,0x800u};
    uint32_t pad=PE_LoadU32(0x8009D26Cu),result=0u;unsigned i;
    for (i=0;i<14;i++) if (pad&input[i]) result|=output[i];
    return result;
}

void func_8005E114(int32_t value) { PE_StoreU32(0x8009D0ECu,(uint32_t)value); }

static void menu_input_event(uint32_t type,uint32_t buttons)
{
    pe_addr_t node=PE_LoadU32(0x8009D0DCu),tail;
    if (!node) return;
    tail=PE_LoadU32(0x8009D0E4u);PE_StoreU32(0x8009D0DCu,PE_LoadU32(node));PE_StoreU32(node,0u);
    if (tail) PE_StoreU32(tail,node);
    else PE_StoreU32(0x8009D0E0u,node); /* 527C0's inconsistent-queue diagnostic is an empty leaf. */
    PE_StoreU32(0x8009D0E4u,node);PE_StoreU32(node+4u,type);PE_StoreU32(node+8u,buttons);
}

void func_8005E12C(int32_t fast_repeat)
{
    uint32_t buttons=func_8005E038(),released,pressed,repeating=0u;
    int32_t timer;
    if (!PE_LoadU32(0x8009D0E8u)) {
        if (!buttons) PE_StoreU32(0x8009D0E8u,1u);
        return;
    }
    released=PE_LoadU32(0x8009D0F0u)&~buttons;
    if (released) menu_input_event(4u,released);
    if (PE_LoadU32(0x8009D0F0u)!=buttons) PE_StoreU32(0x8009D0F8u,16u);
    timer=(int32_t)(PE_LoadU32(0x8009D0F8u)-2u);PE_StoreU32(0x8009D0F8u,(uint32_t)timer);
    if (timer<0 && ((fast_repeat && timer<-90) || !((uint32_t)timer&3u))) {
        PE_StoreU32(0x8009D0F0u,0u);repeating=1u;
    }
    PE_StoreU32(0x8009D0F4u,timer<-299?8u:1u);
    pressed=buttons&~PE_LoadU32(0x8009D0F0u);
    if (pressed && !(repeating && (pressed&64u))) menu_input_event(repeating?2u:1u,pressed);
    PE_StoreU32(0x8009D0F0u,buttons);
}

static void menu_sound(uint32_t id)
{
    pe_addr_t package=PE_LoadU32(0x800B0E08u);
    if (package) (void)func_8006DF50(package,id,0x100u,128u,127u);
}

void func_800525EC(void) { menu_sound(0x44Cu); }
void func_80052634(void) { menu_sound(0x44Du); }
void func_8005267C(void) { menu_sound(0x44Eu); }
void func_800526C4(void) { menu_sound(0x44Fu); }
