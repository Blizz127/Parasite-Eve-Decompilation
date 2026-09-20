/* Original card-operation dispatch.  State 2's directory enumeration and
 * entry scan are native (over the host libcard file API); the slot-list UI
 * continuation (func_8004D4C4/func_8004D298) needs the PS1 low-memory
 * kernel/menu substrate and stays an explicit boundary.  Other transfer/retry
 * paths beyond their first callee remain unported.
 * Boundary payload: known-register mask, then fifth argument when present.
 * Unknown guest-stack arguments are not assigned fabricated guest addresses. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static void operation_boundary(const char *caller,pe_addr_t target,uint32_t a0,uint32_t a1,
                               uint32_t a2,uint32_t a3,uint32_t known,uint32_t fifth)
{
    uint32_t payload[2]={known,fifth};
    (void)Bootstrap_ReturnInt4Indirect("card operation unresolved call",caller,0,
        target,a0,a1,a2,a3,payload,sizeof(payload));
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

/* Live-path formatter scratch. Same dedicated band func_80042020 uses; the
 * two never run concurrently. */
#define GA_FMT_SP     0x801FF600u
#define GA_CARD_BASE  0x800A0ED4u
#define GA_SAVE_NAME  0x8009EE70u

static void card_io_fail(pe_addr_t record);

/* Retail retry/failure tail shared by every state's first-callee failure
 * (0x80041754 / .L80041798 / .L80041FBC / .L80041FF8): decrement the retry
 * count and return; once exhausted run the record's abort handler and set the
 * terminal state. */
static void card_io_fail(pe_addr_t record)
{
    uint16_t c = PE_LoadU16(record + 0x16u);
    PE_StoreU16(record + 0x16u, (uint16_t)(c - 1u));
    if (c > 0u)
        return;
    if (!(PE_LoadU8(record) & 1u)) {
        func_80040F80(record);
        return;
    }
    func_80040F80(record);
    if (PE_Port_ShouldStop())
        return;
    {
        uint8_t m = PE_LoadU8(record + 7u);
        if (m == 1u)
            func_8004CE28(0x3Du, 0x3Fu);
        else if (m == 2u)
            func_8004CC50(0x3Eu, 0u);
        PE_StoreU8(record + 7u, 0u);
        PE_StoreU8(record + 1u, 12u);
    }
}

/* Live continuation after the name formatter, per retail return address:
 *   state 3  @0x800416D8  open(name,1)   -> state 8 (header refresh)
 *   state 4  @0x80041834  open(name,10200) + close -> state 6
 *   state 5  @0x80041908  open(name,1)   -> state 7 (load)
 *   state 6  @0x800419D4  open(name,2)   -> state 9 (write 0x2000 block)
 *   state 11 @0x80041E98  open(name,1) + close + erase(name) -> state 4
 * Open modes are the retail constants (1 existing, 2 create/truncate,
 * 0x10200 create-and-close). */
static void card_after_format(uint32_t state, pe_addr_t record,
                              uint32_t selected, uint32_t index)
{
    int handle;

    switch (state) {
    case 3u:
        handle = func_80072734(GA_SAVE_NAME, 1);
        if (handle < 0) { card_io_fail(record); return; }
        PE_StoreU32(record + 0xCu, (uint32_t)handle);
        PE_StoreU8(record + 1u, 8u);
        PE_StoreU32(record + 0x18u, 0x800A1720u + (index << 7));
        PE_StoreU16(record + 0x14u, 0x80u);
        PE_StoreU16(record + 0x16u, 0x1Eu);
        if (PE_LoadU8(record + selected * 0x44u + 0x1Cu) == 1u)
            (void)func_80072744(handle, 0x100, 0);
        return;
    case 4u:
        handle = func_80072734(GA_SAVE_NAME, 0x10200);
        if (handle < 0) { card_io_fail(record); return; }
        PE_StoreU32(record + 0xCu, (uint32_t)handle);
        (void)func_80072774(handle);
        PE_StoreU32(record + 0xCu, 0xFFFFFFFFu);
        PE_StoreU8(record + 1u, 6u);
        PE_StoreU16(record + 0x16u, 0x0Au);
        return;
    case 5u:
        handle = func_80072734(GA_SAVE_NAME, 1);
        if (handle < 0) { card_io_fail(record); return; }
        PE_StoreU32(record + 0xCu, (uint32_t)handle);
        PE_StoreU16(record + 0x16u, 0x1Eu);
        PE_StoreU8(record + 1u, 7u);
        return;
    case 6u:
        handle = func_80072734(GA_SAVE_NAME, 2);
        if (handle < 0) { card_io_fail(record); return; }
        PE_StoreU32(record + 0xCu, (uint32_t)handle);
        PE_StoreU8(record + 1u, 9u);
        PE_StoreU32(record + 0x18u, 0x8009EED0u);
        PE_StoreU16(record + 0x16u, 0x1Eu);
        return;
    case 11u: {
        pe_addr_t format = PE_LoadU32(0x80092224u);
        uint32_t variant = PE_LoadU8(record + selected * 0x44u + 69u) + 0x30u;
        uint32_t saved[9];
        handle = func_80072734(GA_SAVE_NAME, 1);
        if (handle < 0) { card_io_fail(record); return; }
        PE_StoreU32(record + 0xCu, (uint32_t)handle);
        (void)func_80072774(handle);
        for (unsigned i = 0; i < 9u; i++) saved[i] = 0u;
        saved[2] = (record > GA_CARD_BASE) ? 1u : 0u;
        saved[3] = 1u;
        saved[8] = 0x80041F04u;
        PE_StoreU32(GA_FMT_SP + 16u, selected + 0x41u);
        (void)PE_FormatterFrame(GA_SAVE_NAME, format,
                                (record > GA_CARD_BASE) ? 1u : 0u, variant,
                                GA_FMT_SP, saved);
        if (PE_Port_ShouldStop())
            return;
        if (func_800727A4(GA_SAVE_NAME) == 0) { card_io_fail(record); return; }
        PE_StoreU8(record + 1u, 4u);
        PE_StoreU16(record + 0x16u, 0x0Au);
        return;
    }
    default:
        return;
    }
}

/* Live formatter call (retail func_80071A84) then the state continuation. */
static void card_live_format(uint32_t state, pe_addr_t record,
                             uint32_t selected, uint32_t index)
{
    pe_addr_t format = PE_LoadU32(0x80092224u);
    uint32_t variant = PE_LoadU8(record + selected * 0x44u + 69u) + 0x30u;
    uint32_t saved[9];

    for (unsigned i = 0; i < 9u; i++) saved[i] = 0u;
    saved[0] = record;
    saved[2] = index;
    if (state == 4u || state == 5u || state == 6u) saved[1] = 1u;
    if (state == 11u) { saved[2] = (record > GA_CARD_BASE) ? 1u : 0u; saved[3] = 1u; }
    saved[8] = state == 3u ? 0x800416D8u : state == 4u ? 0x80041834u :
               state == 5u ? 0x80041908u : state == 6u ? 0x800419D4u : 0x80041E98u;
    PE_StoreU32(GA_FMT_SP + 16u, selected + 0x41u);
    (void)PE_FormatterFrame(GA_SAVE_NAME, format,
                            (record > GA_CARD_BASE) ? 1u : 0u, variant,
                            GA_FMT_SP, saved);
    if (PE_Port_ShouldStop())
        return;
    card_after_format(state, record, selected, index);
}

/* Live read loop (states 7/8 @0x80041A58/0x80041B18): read min(len,0x80) for
 * state 8 and min(len,0x400) for state 7, advancing buf/len. */
static void card_live_read(pe_addr_t record, uint32_t state, uint32_t index)
{
    int32_t len, cap, a2, r;
    int handle;
    pe_addr_t buf;

    (void)index;
    if (PE_LoadU8(record) != 1u) { func_80040F80(record); return; }
    len = (int16_t)PE_LoadU16(record + 0x14u);
    cap = (state == 8u) ? 0x80 : 0x400;
    a2 = (len < cap + 1) ? len : cap;
    handle = (int)PE_LoadU32(record + 0xCu);
    buf = PE_LoadU32(record + 0x18u);
    r = func_80072754(handle, buf, a2);
    if (r > 0) {
        int32_t rem = (int16_t)(uint16_t)((uint16_t)PE_LoadU16(record + 0x14u)
                                          - (uint16_t)r);
        PE_StoreU32(record + 0x18u, (uint32_t)(buf + (uint32_t)r));
        PE_StoreU16(record + 0x14u, (uint16_t)rem);
        if (rem > 0)
            return;
        if (state == 8u) { PE_StoreU8(record + 1u, 10u); return; }
        (void)func_80072774(handle);
        PE_StoreU32(record + 0xCu, 0xFFFFFFFFu);
        PE_StoreU8(record + 1u, 12u);
        PE_StoreU8(record + 7u, 0u);
        PE_StoreU32(0x800A1854u, 0u);
        PE_StoreU32(0x800A1838u, 0u);
        /* func_80042264 is unported; keep the honest boundary for the load
         * tail rather than inventing its menu effect. */
        operation_boundary("func_80041108", 0x80042264u, 0u, 0u, 0u, 0u, 0u, 0u);
        return;
    }
    card_io_fail(record);
}

/* Live write loop (state 9 @0x80041C0C): write the assembled 0x2000-byte
 * block in 0x400-byte chunks, then close and refresh the slot list. */
static void card_live_write(pe_addr_t record)
{
    int32_t len, a2, r;
    int handle;
    pe_addr_t buf;

    if (PE_LoadU8(record) != 1u) { func_80040F80(record); return; }
    len = (int16_t)PE_LoadU16(record + 0x14u);
    a2 = (len < 0x401) ? len : 0x400;
    handle = (int)PE_LoadU32(record + 0xCu);
    buf = PE_LoadU32(record + 0x18u);
    r = func_80072764(handle, buf, a2);
    if (r > 0) {
        int32_t rem = (int16_t)(uint16_t)((uint16_t)PE_LoadU16(record + 0x14u)
                                          - (uint16_t)r);
        PE_StoreU32(record + 0x18u, (uint32_t)(buf + (uint32_t)r));
        PE_StoreU16(record + 0x14u, (uint16_t)rem);
        if (rem > 0)
            return;
        (void)func_80072774(handle);
        PE_StoreU32(record + 0xCu, 0xFFFFFFFFu);
        PE_StoreU8(record + 1u, 3u);
        PE_StoreU8(record + 7u, 0u);
        PE_StoreU32(0x800A1854u, 0u);
        func_8004D9D8();
        if (PE_Port_ShouldStop())
            return;
        func_8004CC50(0x53u, 0u);
        return;
    }
    card_io_fail(record);
}

/* Live load-close (state 10 @0x80041D04): close the file, copy the loaded
 * header fields into the slot entry, then advance the walk cursor. */
static void card_live_close(pe_addr_t record, uint32_t index)
{
    pe_addr_t entry, buf;
    uint32_t selected;

    if (PE_LoadU8(record) != 1u) { func_80040F80(record); return; }
    (void)func_80072774((int)PE_LoadU32(record + 0xCu));
    PE_StoreU32(record + 0xCu, 0xFFFFFFFFu);
    selected = PE_LoadU8(record + 3u);
    entry = record + selected * 0x44u + 0x1Cu;
    buf = 0x800A1720u + (index << 7);
    if (PE_LoadU8(entry) == 1u) {
        PE_StoreU16(entry + 0x24u, PE_LoadU16(buf + 0x28u));
        PE_StoreU16(entry + 0x26u, PE_LoadU16(buf + 0x26u));
        PE_StoreU8(entry + 0x28u, PE_LoadU8(buf + 0x2Au));
        PE_StoreU32(entry + 0x0Cu, PE_LoadU32(buf + 0x08u));
        PE_StoreU32(entry + 0x10u, PE_LoadU32(buf + 0x0Cu));
        PE_StoreU32(entry + 0x20u, PE_LoadU32(buf + 0x64u));
        PE_StoreU8(entry + 0x2Au, PE_LoadU8(buf + 0x2Bu));
        PE_StoreU16(entry + 0x2Cu, PE_LoadU16(buf + 0x5Cu));
        PE_StoreU16(entry + 0x2Eu, PE_LoadU16(buf + 0x5Eu));
        PE_StoreU32(entry + 0x04u, PE_LoadU32(buf + 0x00u));
        PE_StoreU32(entry + 0x08u, PE_LoadU32(buf + 0x04u));
        PE_StoreU32(entry + 0x14u, PE_LoadU32(buf + 0x10u));
        PE_StoreU32(entry + 0x18u, PE_LoadU32(buf + 0x14u));
        PE_StoreU32(entry + 0x1Cu, PE_LoadU32(buf + 0x18u));
    }
    PE_StoreU8(record + 1u, 1u);
    {
        uint8_t cursor = (uint8_t)(PE_LoadU8(record + 6u) + 1u);
        uint8_t limit = (uint8_t)(PE_LoadU8(record + 2u) << 1);
        uint8_t st;
        PE_StoreU8(record + 6u, cursor);
        st = (cursor < limit) ? 3u : 12u;
        PE_StoreU8(record + 1u, st);
        if (st == 12u)
            PE_StoreU32(0x800A1838u, 0u);
    }
}

/* Optional explicit guest context; NULL retains the unresolved live boundary. */
static void card_operation(uint32_t index,pe_addr_t frame,const uint32_t *incoming)
{
    pe_addr_t record=0x800A0ED4u+index*0x418u;
    uint32_t state=PE_LoadU8(record+1u),selected;
    if(state==0u || state>=15u)return;
    if(state==13u) {
        if(PE_LoadU8(record)!=5u) {
            PE_StoreU32(0x800A1864u,0xFFFFFFFFu);PE_StoreU8(record+1u,0u);return;
        }
        goto ready;
    }
    if(state==14u) {
        if(incoming) {
            uint32_t saved[9];for(unsigned i=0;i<8;i++)saved[i]=incoming[16u+i];
            saved[0]=record;saved[2]=index;saved[8]=0x80041284u;
            unsigned epoch=PE_Port_StopEpoch();
            (void)PE_FormatterFrame(frame+24u,0x80010F60u,index,incoming[7],frame,saved);
            if(PE_Port_StopEpoch()!=epoch)return;
            operation_boundary("PE_CardOperationFrame",0x80072784u,frame+24u,0u,0u,0u,1u,0u);return;
        }
        /* a0 is the unresolved original frame's sp+18h, not a RAM scratch. */
        operation_boundary("func_80041108",0x80071A84u,0u,0x80010F60u,index,0u,6u,0u);return;
    }
    if(PE_LoadU8(record)!=1u) {
        if(incoming) {
            uint32_t cleanup[32];for(unsigned i=0;i<32;i++)cleanup[i]=incoming[i];
            cleanup[16]=record;cleanup[18]=index;cleanup[31]=0x80042000u;
            if(state>=4u && state<=10u)cleanup[17]=PE_LoadU8(record);
            if(state==11u)cleanup[19]=PE_LoadU8(record);
            PE_CardCleanupFrame(record,frame,cleanup);
        } else func_80040F80(record);
        return;
    }
    switch(state) {
    case 1:
    ready: {
        if(PE_LoadU8(record+8u)!=4u)return;
        uint32_t other=PE_LoadU8(0x800A0EDCu+(index==0u?0x418u:0u));
        if(other!=0u && other!=4u)return;
        PE_StoreU32(0x800A1838u,1u);
        PE_StoreU8(record+1u,state==13u?14u:PE_LoadU8(record+11u));return;
    }
    case 2: {
        /* Directory enumeration.  The original dirent output is the caller's
         * frame+20h; a context-free live call has no frame, so reuse the
         * card's own transient filename buffer (not a fabricated stack
         * address).  Not a matching leaf: translated from 0x800412C0. */
        pe_addr_t dirent = incoming ? frame + 0x20u : 0x8009EE70u;
        pe_addr_t dirspec = PE_LoadU32(0x80092230u);
        pe_addr_t fname = PE_LoadU32(0x80092224u);
        int have_name = PE_RangeIsRam(fname + 6u, 12u);
        uint32_t blocks, i;
        pe_addr_t de;
        int can_continue = 0;

        PE_StoreU8(PE_LoadU32(0x80092230u)+2u,index+0x30u);
        PE_StoreU8(record+2u,0u);PE_StoreU8(record+3u,0u);PE_StoreU8(record+6u,0u);
        PE_StoreU8(record+4u,0u);PE_StoreU8(record+7u,0u);PE_StoreU8(record+10u,0u);
        for(int i=14;i>=0;i--)PE_StoreU8(record+(uint32_t)i*0x44u+28u,2u);

        if ((de = func_800727B4(dirspec, dirent)) == 0u) {
            PE_StoreU16(record+0x16u,(uint16_t)(PE_LoadU16(record+0x16u)-1u));
        } else {
            do {
                if (have_name && func_80071A04(dirent, fname+6u, 12) == 0) {
                    uint32_t slot=PE_LoadU8(dirent+0x13u);
                    pe_addr_t entry=record+slot*0x44u-0x1128u;
                    PE_StoreU8(entry,1u);
                    PE_StoreU8(entry+1u,0u);
                    PE_StoreU8(entry+0x29u,PE_LoadU8(dirent+0x12u)!=0x30u?1u:0u);
                    PE_StoreU8(record+4u,1u);
                }
                {
                    int32_t size=(int32_t)PE_LoadU32(dirent+0x18u);
                    uint8_t b=(uint8_t)(PE_LoadU8(record+0x0Au)+(uint8_t)(size>>13));
                    PE_StoreU8(record+0x0Au,b);
                }
                de=func_80072794(dirent);
            } while (de != 0u);
            PE_StoreU16(record+0x16u,0u);
        }

        if ((int16_t)PE_LoadU16(record+0x16u) > 0)
            return;

        /* Account entries: type2 becomes read-marker1, others reduce blocks. */
        blocks = PE_LoadU8(record+0x0Au);
        for (i=0;i<15u;i++) {
            pe_addr_t e=record+i*0x44u;
            if (PE_LoadU8(e+0x1Cu)==2u) PE_StoreU8(e+0x1Du,1u);
            else blocks=(blocks-1u)&0xFFu;
        }
        for (i=0;i<15u && blocks!=0u;i++) {
            pe_addr_t e=record+(14u-i)*0x44u;
            if (PE_LoadU8(e+0x1Cu)==2u) { PE_StoreU8(e+0x1Cu,3u); blocks--; }
        }
        PE_StoreU8(record+2u,15u);

        if (PE_LoadU32(0x800A186Cu)==0u) {
            PE_StoreU8(record+1u,15u);
            PE_StoreU32(0x800A1838u,0u);
            return;
        }
        for (i=0;i<15u;i++) {
            if (PE_LoadU8(record+i*0x44u+0x1Cu)==1u) { can_continue=1; break; }
        }
        if (!can_continue && PE_LoadU8(record+0x0Au)<15u && func_8004D27C()!=0)
            can_continue=1;
        if (can_continue) {
            uint32_t idx;
            PE_StoreU8(record+1u,3u);
            PE_StoreU16(record+0x16u,10u);
            func_80062CE4();
            idx=(uint32_t)((int32_t)((record-0x800A0ED4u)*0xC9484E2Bu)>>3);
            /* 0x80041588 jal func_8004D4C4(idx, record+2); the return low
             * byte is stored at record+5 by the delay slot at 0x80041594.
             *
             * The slot-list constructor allocates from the menu node pool
             * (func_80062F9C's free list at D_8009D158).  The live route has
             * that pool because boot ran func_80062F9C; the card-operation
             * oracles replay a recorded execution whose pool state is not
             * captured, so with an empty pool we keep the recorded named
             * boundary instead of dereferencing a null free-list head.
             * (docs/evidence/pe-libcard-file-api/REPORT.md) */
            if (incoming || PE_LoadU32(0x8009D158u) == 0u) {
                operation_boundary("func_80041108",0x8004D4C4u,idx,
                    PE_LoadU8(record+2u),0u,0u,1u,0u);
                return;
            }
            PE_StoreU8(record+5u,
                (uint8_t)(uint32_t)func_8004D4C4(idx, PE_LoadU8(record+2u)));
            return;
        }
        func_80062CE4();
        /* 0x800415A0 jal func_8004D298(index), then 0x800417AC sets
         * state=12 and clears D_800A1838 before the epilogue. */
        if (incoming || PE_LoadU32(0x8009D158u) == 0u) {
            operation_boundary("func_80041108",0x8004D298u,index,0u,0u,0u,1u,0u);
            return;
        }
        func_8004D298(index);
        PE_StoreU8(record+1u,12u);
        PE_StoreU32(0x800A1838u,0u);
        return;
    }
    case 3: {
        uint32_t cursor=PE_LoadU8(record+6u),limit=PE_LoadU8(record+2u)*2u;
        if(cursor<limit) {
            uint32_t center=PE_LoadU8(record+5u);
            do {
                uint32_t slot=center+(((cursor&1u)*2u-1u)*(((cursor&255u)+1u)>>1));
                if(slot<15u && !PE_LoadU8(record+slot*0x44u+29u))break;
                cursor++;PE_StoreU8(record+6u,cursor);
            } while((cursor&255u)<limit);
        }
        cursor=PE_LoadU8(record+6u);limit=PE_LoadU8(record+2u)*2u;
        selected=cursor<limit?PE_LoadU8(record+5u)+(((cursor&1u)*2u-1u)*((cursor+1u)>>1)):255u;
        PE_StoreU8(record+3u,selected);selected&=255u;
        if(selected==255u) {
            PE_StoreU8(record+1u,12u);PE_StoreU32(0x800A1838u,0u);return;
        }
        goto format_name;
    }
    case 4:
        selected=PE_LoadU8(record+3u);
        PE_StoreU8(record+selected*0x44u+69u,PE_LoadU32(0x800A1704u));
        selected=PE_LoadU8(record+3u);goto format_name;
    case 5:case 6:case 11:
        selected=PE_LoadU8(record+3u);
    format_name:
        if(incoming) {
            uint32_t saved[9];for(unsigned i=0;i<8;i++)saved[i]=incoming[16u+i];
            saved[0]=record;saved[2]=index;
            if(state==4u || state==5u || state==6u)saved[1]=1u;
            if(state==11u) {saved[2]=record>0x800A0ED4u;saved[3]=1u;}
            saved[8]=state==3u?0x800416D8u:state==4u?0x80041834u:
                state==5u?0x80041908u:state==6u?0x800419D4u:0x80041E98u;
            pe_addr_t format=PE_LoadU32(0x80092224u);
            uint32_t variant=PE_LoadU8(record+selected*0x44u+69u)+0x30u;
            PE_StoreU32(frame+16u,selected+0x41u);
            unsigned epoch=PE_Port_StopEpoch();
            (void)PE_FormatterFrame(0x8009EE70u,format,record>0x800A0ED4u,variant,frame,saved);
            if(PE_Port_StopEpoch()!=epoch)return;
            operation_boundary("PE_CardOperationFrame",0x80072734u,0x8009EE70u,
                state==4u?0x10200u:state==6u?2u:1u,0u,0u,3u,0u);return;
        }
        /* Present card: run the implemented formatter + libcard continuation.
         * Absent card: the original code's first unresolved call is the
         * formatter itself (the card oracles pin that no-card state). */
        if (PE_Card_IsPresent()) {
            card_live_format(state, record, selected, index);
            return;
        }
        operation_boundary("func_80041108",0x80071A84u,0x8009EE70u,PE_LoadU32(0x80092224u),
            record>0x800A0ED4u,PE_LoadU8(record+selected*0x44u+69u)+0x30u,15u,selected+0x41u);return;
    case 7:case 8: {
        int32_t remaining=(int16_t)PE_LoadU16(record+20u),cap=state==8u?128:1024;
        if (incoming || !PE_Card_IsPresent()) {
            operation_boundary("func_80041108",0x80072754u,
                PE_LoadU32(record+12u),PE_LoadU32(record+24u),
                (uint32_t)(remaining<cap+1?remaining:cap),0u,7u,0u);return;
        }
        card_live_read(record,state,index);return;
    }
    case 9: {
        int32_t remaining=(int16_t)PE_LoadU16(record+20u),cap=1024;
        if (incoming || !PE_Card_IsPresent()) {
            operation_boundary("func_80041108",0x80072764u,
                PE_LoadU32(record+12u),PE_LoadU32(record+24u),
                (uint32_t)(remaining<cap+1?remaining:cap),0u,7u,0u);return;
        }
        card_live_write(record);return;
    }
    case 10: {
        if (incoming || !PE_Card_IsPresent()) {
            operation_boundary("func_80041108",0x80072774u,PE_LoadU32(record+12u),0u,0u,0u,1u,0u);return;
        }
        card_live_close(record,index);return;
    }
    case 12:return;
    }
}

void func_80041108(uint32_t index)
{
    card_operation(index,0u,0);
}

/* Memory/output adapter for a supplied original call context. Cleanup and BIOS
 * remain explicit boundaries; no return/register state is synthesized for them. */
void PE_CardOperationFrame(uint32_t index,pe_addr_t caller_sp,const uint32_t incoming[32])
{
    pe_addr_t frame=caller_sp-0x78u;
    PE_StoreU32(frame+0x68u,incoming[18]);PE_StoreU32(frame+0x60u,incoming[16]);
    PE_StoreU32(frame+0x70u,incoming[31]);PE_StoreU32(frame+0x6Cu,incoming[19]);
    PE_StoreU32(frame+0x64u,incoming[17]);
    card_operation(index,frame,incoming);
}

void func_80042228(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    const uint32_t ids[]={38,37,36};
    for(unsigned i=0;i<3;i++) {func_80062F3C(ids[i]);if(PE_Port_StopEpoch()!=epoch)return;}
    func_800512AC(12,0u);
}

void func_8004D5CC(uint32_t index)
{
    unsigned epoch=PE_Port_StopEpoch();pe_addr_t list=func_80062A34(2u,36u);
    func_80062F3C(40u);if(PE_Port_StopEpoch()!=epoch)return;
    pe_addr_t callback=PE_LoadU32(0x8009CFFCu);
    if(callback) {
        switch(callback) {
        case 0x80042910u:func_80042910();break;
        case 0x80042928u:func_80042928();break;
        case 0x8005C488u:func_8005C488();break;
        case 0x80062F9Cu:func_80062F9C();break;
        default:operation_boundary("func_8004D5CC",callback,0u,0u,0u,0u,0u,0u);return;
        }
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(0x8009CFFCu,0u);
    }
    if(index!=PE_LoadU32(0x8009CF44u))return;
    func_80062F3C(63u);if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(39u);if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F1C(func_80062A34(1u,41u));if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(31u);if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(index+37u);if(PE_Port_StopEpoch()!=epoch)return;
    if(list) {
        PE_StoreU32(list+68u,0u);
        if(!func_800631DC())func_80062CB8(list);
    }
}

void func_80040F80(pe_addr_t record)
{
    unsigned epoch=PE_Port_StopEpoch();
    if(PE_LoadU32(0x800A185Cu)) {func_80042228();return;}
    uint32_t handle=PE_LoadU32(record+12u);
    if((int32_t)handle>=0) {
        operation_boundary("func_80040F80",0x80072774u,handle,0u,0u,0u,1u,0u);return;
    }
    if(PE_LoadU8(record+1u)==9u) {
        uint32_t selected=PE_LoadU8(record+3u);
        operation_boundary("func_80040F80",0x80071A84u,0x8009EE70u,PE_LoadU32(0x80092224u),
            record>0x800A0ED4u,PE_LoadU8(record+selected*0x44u+69u)+0x30u,15u,selected+0x41u);return;
    }
    uint32_t product=(record-0x800A0ED4u)*0xC9484E2Bu;
    func_8004D5CC((uint32_t)((int32_t)product>>3));
    if(PE_Port_StopEpoch()!=epoch)return;
    PE_StoreU8(record+1u,0u);PE_StoreU8(record+4u,0u);
    PE_StoreU32(0x800A1854u,0u);PE_StoreU32(0x800A1838u,0u);func_80062CE4();
}

/* Original40F80 frame through the first unresolved callee. The direct native
 * entry above continues to own returning cleanup without a guest frame. */
void PE_CardCleanupFrame(pe_addr_t record,pe_addr_t caller_sp,const uint32_t incoming[32])
{
    uint32_t override=PE_LoadU32(0x800A185Cu);
    pe_addr_t frame=caller_sp-0x50u;
    PE_StoreU32(frame+0x40u,incoming[18]);PE_StoreU32(frame+0x48u,incoming[31]);
    PE_StoreU32(frame+0x44u,incoming[19]);PE_StoreU32(frame+0x3Cu,incoming[17]);
    PE_StoreU32(frame+0x38u,incoming[16]);
    if(override) {
        operation_boundary("PE_CardCleanupFrame",0x80042228u,0u,0u,0u,0u,0u,0u);return;
    }
    uint32_t handle=PE_LoadU32(record+12u);
    if((int32_t)handle>=0) {
        operation_boundary("PE_CardCleanupFrame",0x80072774u,handle,0u,0u,0u,1u,0u);return;
    }
    uint32_t index=(uint32_t)((int32_t)((record-0x800A0ED4u)*0xC9484E2Bu)>>3);
    if(PE_LoadU8(record+1u)!=9u) {
        operation_boundary("PE_CardCleanupFrame",0x8004D5CCu,index,0u,0u,0u,1u,0u);return;
    }
    uint32_t saved[9];for(unsigned i=0;i<8;i++)saved[i]=incoming[16u+i];
    saved[0]=index;saved[1]=0u;saved[2]=record;saved[3]=0xFFFFFFFFu;saved[8]=0x80041048u;
    uint32_t selected=PE_LoadU8(record+3u);
    pe_addr_t format=PE_LoadU32(0x80092224u);
    uint32_t variant=PE_LoadU8(record+selected*0x44u+69u)+0x30u;
    PE_StoreU32(frame+16u,selected+0x41u);
    unsigned epoch=PE_Port_StopEpoch();
    (void)PE_FormatterFrame(0x8009EE70u,format,record>0x800A0ED4u,variant,frame,saved);
    if(PE_Port_StopEpoch()!=epoch)return;
    /* Reload original callee-saved memory for the next formatter call. */
    for(unsigned i=0;i<8;i++)saved[i]=PE_LoadU32(frame-0x250u+0x228u+i*4u);
    saved[8]=0x80041064u;
    (void)PE_FormatterFrame(frame+24u,0x80010F4Cu,saved[0],0x8009EE70u,frame,saved);
    if(PE_Port_StopEpoch()!=epoch)return;
    operation_boundary("PE_CardCleanupFrame",0x80072734u,frame+24u,1u,0u,0u,3u,0u);
}
