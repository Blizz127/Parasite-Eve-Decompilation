/* Complete original input handler 8003EB04..8003F074,348 words.
 * SHA256 c6a27aa96c180ee441866ac86dfc995683fb48c2366dfd864f0bbd03aeb4660b.
 * Controller setup, hold counters, priority masks, analog axes and edges. */
#include "pe_port_compat.h"
#include "game_port.h"
#define W(a) PE_LoadU32(a)
#define S(a,v) PE_StoreU32(a,v)
#define CHECK() do {if(PE_Port_StopEpoch()!=epoch)return;} while(0)
void func_8003EB04(void)
{
    uint32_t epoch=PE_Port_StopEpoch();
    int state=func_800825C0(0);CHECK();
    if(!state) {
        uint32_t flags=W(0x8009D1A0u);
        if(!(flags&0x4001u)) {
            S(0x8009D1F4u,4);S(0x8009D26Cu,4);S(0x8009D1E4u,0);
            S(0x8009D1A0u,flags|0x4000u);return;
        }
        S(0x8009D1A0u,flags|0x4000u);
    }
    uint32_t type=PE_LoadU16(0x800BE9A0u)&0xF000u;
    if(type!=0x4000u && type!=0x7000u) {
        S(0x8009D26Cu,0);S(0x8009D1F4u,0);S(0x8009D1E4u,0);return;
    }
    if(W(0x8009D1A0u)&0x4000u) {
        state=func_800825C0(0);CHECK();
        if(state==1) {func_80082974(0,0x8009D1C0u,2);CHECK();}
        else if(state==2)S(0x8009D1A0u,W(0x8009D1A0u)&~0x4000u);
        else if(state==6) {
            uint32_t modes=func_80082680(0,2,0);CHECK();
            if(modes) {
                if(!(W(0x8009D1A0u)&0x8000u)) {
                    func_8008292C(0,1,0);CHECK();
                    S(0x8009D1A0u,W(0x8009D1A0u)|0x8000u);
                } else {
                    func_800828F4(0,0x800921F8u);CHECK();
                    S(0x8009D1A0u,W(0x8009D1A0u)&~0x4000u);
                }
            }
        }
    }
    uint32_t raw=~(uint32_t)PE_LoadU16(0x800BE9A2u),mapped=raw&0x9FFFu;
    S(0x8009D238u,W(0x8009D26Cu));
    if(raw&0x2000u)mapped|=0x4000u;if(raw&0x4000u)mapped|=0x2000u;
    S(0x8009D26Cu,0);
    for(unsigned i=0;i<32;i++) {
        pe_addr_t counter=0x800A7770u+i*4u;
        if(mapped&W(0x800A76F0u+i*4u)) {
            uint32_t held=W(0x8009D26Cu),count=W(counter)+1u;
            S(0x8009D26Cu,held|(1u<<i));S(counter,count);
        } else S(counter,0);
    }
    uint32_t flags=W(0x8009D1A0u);
    if(flags&1u) {
        uint32_t held=W(0x8009D26Cu);
        if(held&0x80000000u) {
            uint32_t pressed=((held^W(0x8009D2D4u))&held)&0x7000007Eu;
            if(pressed) {
                uint32_t index=W(0x8009D2A8u),expected=W(0x80092200u+index*4u);
                if((pressed&expected)==expected) {
                    S(0x8009D2A8u,index+1u);
                    if(index+1u==9u) {S(0x8009D280u,0xAA108448u);S(0x8009D1A0u,flags|0x12000u);}
                } else S(0x8009D2A8u,0);
            }
        } else S(0x8009D2A8u,0);
        S(0x8009D2D4u,W(0x8009D26Cu));
    }
    flags=W(0x800B0CD8u);
    if(flags&0x400u)S(0x8009D26Cu,W(0x8009D26Cu)&0x40FFDC7Fu);
    if(flags&0x200u) {
        uint32_t held=W(0x8009D26Cu)&0x60FFDB04u;S(0x8009D26Cu,held);
        if((int8_t)PE_LoadU8(0x800B0DBFu)!=1)S(0x8009D26Cu,held&0xDFFFFCFFu);
    }
    uint32_t held=W(0x8009D26Cu);
    if(held&0x10000080u)S(0x8009D26Cu,held&0x7FFFFFFFu);
    held=W(0x8009D26Cu);
    if(held&0x40000401u)S(0x8009D26Cu,held&0x6FFFFF7Fu);
    held=W(0x8009D26Cu);
    if(held&0x20000300u)S(0x8009D26Cu,held&0x2FFFFB7Eu);
    if((PE_LoadU16(0x800BE9A0u)&0xF000u)==0x7000u) {
        S(0x8009D26Cu,W(0x8009D26Cu)&~0x79u);
        pe_addr_t menu=func_80062A34(1,0);CHECK();
        for(unsigned axis=0;axis<2;axis++) {
            unsigned value=PE_LoadU8(0x800BE9A7u-axis);
            uint32_t negative=axis?0x40u:8u,positive=axis?0x10u:0x20u;
            held=W(0x8009D26Cu);
            if(menu) {
                if(value<20)S(0x8009D26Cu,held|negative);
                else if(value>=231)S(0x8009D26Cu,held|positive);
            } else if(value<90) {
                S(0x8009D26Cu,held|negative);
                if(value<20)S(0x8009D26Cu,held|negative|1u);
            } else if(value>=161) {
                S(0x8009D26Cu,held|positive);
                if(value>=231)S(0x8009D26Cu,held|positive|1u);
            }
        }
    }
    held=W(0x8009D26Cu);uint32_t old=W(0x8009D238u),changed=held^old;
    S(0x8009D1F4u,changed&held);S(0x8009D1E4u,changed&old);
}
#undef W
#undef S
#undef CHECK
