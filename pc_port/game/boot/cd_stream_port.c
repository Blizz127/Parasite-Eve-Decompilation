/* Original Disc1 EXE SHA1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 * 7B290..7B558 status poll; 7A488..7A4A8 wrapper;
 * 7C564..7CE80 stream assembly; 7CE80..7CEAC forward word copy.
 * Enabled-device physical FIFO/DMA3 paths are connected; unsupported
 * device states and unknown callbacks remain explicit stops.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_gpu.h"
#include "pe_mdec.h"
#include "pe_cdreg.h"
#include "pe_spu_dma.h"
#include "pe_callback.h"
#include "game_port.h"

static uint16_t CdSpuRead(pe_addr_t base,uint32_t offset)
{
    if((base&0x1FFFFFFFu)==0x1F801C00u) return PE_SpuRegister_LoadU16(offset);
    return PE_LoadU16(base+offset);
}
static void CdSpuWrite(pe_addr_t base,uint32_t offset,uint16_t value)
{
    if((base&0x1FFFFFFFu)==0x1F801C00u) PE_SpuRegister_StoreU16(offset,value);
    else PE_StoreU16(base+offset,value);
}

/* 7BAC0..7BBB0: SPU master/CD volumes, control and CD stereo matrix. */
int func_8007BAC0(void)
{
    pe_addr_t base=PE_LoadU32(0x8009B290u);
    if(!CdSpuRead(base,0x1B8u) && !CdSpuRead(base,0x1BAu)) {
        CdSpuWrite(base,0x180u,0x3FFFu);CdSpuWrite(base,0x182u,0x3FFFu);
        base=PE_LoadU32(0x8009B290u);
    }
    CdSpuWrite(base,0x1B0u,0x3FFFu);CdSpuWrite(base,0x1B2u,0x3FFFu);
    CdSpuWrite(base,0x1AAu,0xC001u);
    PE_CdStoreU8(PE_LoadU32(0x8009B27Cu),2u);
    PE_CdStoreU8(PE_LoadU32(0x8009B284u),0x80u);
    PE_CdStoreU8(PE_LoadU32(0x8009B288u),0u);
    PE_CdStoreU8(PE_LoadU32(0x8009B27Cu),3u);
    PE_CdStoreU8(PE_LoadU32(0x8009B280u),0x80u);
    PE_CdStoreU8(PE_LoadU32(0x8009B284u),0u);
    PE_CdStoreU8(PE_LoadU32(0x8009B288u),0x20u);
    return 0;
}

/* 812F4..81310: unsigned modes0/1 only. */
void func_800812F4(uint32_t mode)
{
    if(mode<2u) PE_StoreU32(0x8009B6B8u,mode);
}

/* 7BBFC..7BDDC: controller startup and source2 registration.
 * BIOS console diagnostics use the host log. Device commands remain owned
 * by the translated issuer; no completion status is synthesized here. */
int func_8007BBFC(void)
{
    fprintf(stderr,"CD_init:addr=%08x\n",0x8009B298u);
    PE_StoreU8(0x8009AFD5u,0u);PE_StoreU8(0x8009AFD4u,0u);
    PE_StoreU32(0x8009AFB8u,0u);PE_StoreU32(0x8009AFB4u,0u);
    PE_StoreU32(0x8009AFC8u,0u);PE_StoreU32(0x8009AFC4u,0u);
    func_80073C94();
    if(PE_Port_ShouldStop()) return -1;
    (void)func_80073CC4(2u,0x8007C13Cu);
    if(PE_Port_ShouldStop()) return -1;
    PE_CdStoreU8(PE_LoadU32(0x8009B27Cu),1u);
    while(PE_CdLoadU8(PE_LoadU32(0x8009B288u))&7u) {
        PE_CdStoreU8(PE_LoadU32(0x8009B27Cu),1u);
        PE_CdStoreU8(PE_LoadU32(0x8009B288u),7u);
        PE_CdStoreU8(PE_LoadU32(0x8009B284u),7u);
    }
    PE_StoreU8(0x8009B296u,0u);PE_StoreU8(0x8009B295u,PE_LoadU8(0x8009B296u));
    PE_StoreU8(0x8009B294u,2u);
    PE_CdStoreU8(PE_LoadU32(0x8009B27Cu),0u);
    PE_CdStoreU8(PE_LoadU32(0x8009B288u),0u);
    PE_CdStoreU32(PE_LoadU32(0x8009B28Cu),0x1325u);
    (void)func_8007B558(1u,0u,0u,0u);
    if(PE_Port_ShouldStop()) return -1;
    if(PE_LoadU32(0x8009AFC4u)&0x10u) {
        (void)func_8007B558(1u,0u,0u,0u);
        if(PE_Port_ShouldStop()) return -1;
    }
    int result=func_8007B558(10u,0u,0u,0u);
    if(PE_Port_ShouldStop() || result) return -1;
    result=func_8007B558(12u,0u,0u,0u);
    if(PE_Port_ShouldStop() || result) return -1;
    result=func_8007B010(0u,0u);
    if(PE_Port_ShouldStop()) return -1;
    return result==2?0:-1;
}

/* 7FA2C..7FB04: SDK startup state, including original80B44(0,B582). */
void func_8007FA2C(void)
{
    PE_StoreU32(0x8009B554u,0u);PE_StoreU8(0x8009B558u,0u);
    for(unsigned i=0;i<4;i++) PE_StoreU8(0x8009B55Cu-i,0u);
    PE_StoreU32(0x8009B560u,0u);
    for(unsigned i=0;i<8;i++) PE_StoreU8(0x8009B56Bu-i,0u);
    PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x8009B578u,14u);
    PE_StoreU8(0x8009B56Cu,0u);PE_StoreU32(0x8009B570u,0u);
    PE_StoreU32(0x8009B57Cu,21u);PE_StoreU8(0x8009B580u,0u);PE_StoreU8(0x8009B581u,0u);
    /* LBA0 plus the150-sector lead-in is00:02:00. Byte3 is untouched. */
    PE_StoreU8(0x8009B582u,0u);PE_StoreU8(0x8009B583u,2u);PE_StoreU8(0x8009B584u,0u);
    for(unsigned i=0;i<6;i++) PE_StoreU8(0x8009B586u+i,0u);
    PE_StoreU32(0x8009B58Cu,0u);PE_StoreU32(0x8009B590u,1u);
    PE_StoreU32(0x8009B594u,0u);PE_StoreU32(0x8009B59Cu,0u);
    PE_StoreU32(0x8009B598u,0u);PE_StoreU32(0x8009B5A0u,0u);
}

/* 7F994..7FA2C: low-level initializer, original helper order.
 * An ordinary7BBFC failure is ignored by retail; an unresolved native stop
 * must unwind before any subsequent initialization is published. */
int func_8007F994(void)
{
    (void)func_8007BBFC();
    if(PE_Port_ShouldStop()) return 0;
    (void)func_8007BAC0();
    if(PE_Port_ShouldStop()) return 0;
    PE_StoreU32(0x800A36A8u,0u);PE_StoreU32(0x800A36A4u,PE_LoadU32(0x800A36A8u));
    PE_StoreU32(0x800A36A0u,0u);
    func_8007FA2C();func_800812F4(0u);
    PE_StoreU32(0x8009AFB4u,0x80080164u);PE_StoreU32(0x8009AFB8u,0x80080778u);
    /* Host binding gives the original guest identity its native body. */
    if(PE_Callback_Bind(0x8007FE24u,func_8007FE24)!=0) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
    }
    (void)func_80073D58(0u,0x8007FE24u);
    PE_StoreU32(0x8009AFD8u,1u);PE_StoreU32(0x8009B554u,1u);
    return 1;
}

/* 800F4..80164: timed-out command reset and asynchronous reissue. */
void func_800800F4(void)
{
    func_8007B9EC();
    if(PE_Port_ShouldStop()) return;
    PE_StoreU32(0x8009B59Cu,PE_LoadU32(0x8009B59Cu)+1u);
    uint8_t cmd=PE_LoadU8(0x8009B558u);
    PE_StoreU32(0x8009B598u,PE_LoadU32(0x8009B5A4u+cmd*4u)?960u:30u);
    (void)func_8007B558(PE_LoadU8(0x8009B558u),PE_LoadU32(0x8009B560u),0u,1u);
}

/* 7F7E8..7F88C: ready-lane queue restart callback. */
static void func_8007F7E8(void)
{
    if(func_8007FBF0(0)!=1 || (int32_t)PE_LoadU32(0x800A3608u)<=0 || func_8007FBF0(0)!=1) return;
    pe_addr_t record=0x800A3540u+PE_LoadU32(0x800A3604u)*24u;
    if(PE_LoadU32(record)) (void)func_8007FB44(PE_LoadU8(record+4u),PE_LoadU32(record+12u));
}

/* 7FE24..800F4: CD VBlank state-machine tick.
 * Retail's four-byte SetMode stack local is retained in guest scratch;
 * only byte0 is written here, exactly as in the original. */
void func_8007FE24(void)
{
    uint32_t pending=PE_LoadU32(0x8009B598u);
    if((int32_t)pending>0) {
        PE_StoreU32(0x8009B598u,pending-1u);
        if(pending==1u) {func_800800F4();return;}
    }
    for(unsigned i=0;i<2;i++) {
        pe_addr_t address=i?0x8009B5A0u:0x8009B594u;
        uint32_t value=PE_LoadU32(address);
        if((int32_t)value>0) PE_StoreU32(address,value-1u);
    }
    if(PE_LoadU32(0x8009B574u)==2u) {
        uint32_t state=PE_LoadU32(0x8009B578u);
        int issue=0;uint32_t command=1u;
        if(state==12u) {
            if(PE_LoadU32(0x8009B6A4u)) {
                PE_StoreU8(0x801FFE90u,0u);
                if((int32_t)PE_LoadU32(0x8009B598u)<=0) {
                    PE_StoreU32(0x8009B570u,32u);
                    (void)func_8007FCFC(14u,0x801FFE90u);
                    if(PE_Port_ShouldStop()) return;
                }
                PE_StoreU32(0x8009B6A4u,0u);
            } else {
                if((int32_t)PE_LoadU32(0x8009B598u)<=0) {
                    PE_StoreU32(0x8009B570u,32u);
                    (void)func_8007FCFC(1u,0u);
                    if(PE_Port_ShouldStop()) return;
                }
                PE_StoreU32(0x8009B6A4u,1u);
            }
        } else if(state==13u) {
            PE_StoreU32(0x8009B6A4u,0u);issue=1;
        } else if(state==14u) {
            uint32_t phase=PE_LoadU32(0x8009B57Cu);
            if(phase==21u || phase==24u) issue=1;
            else if(phase==22u) {PE_StoreU32(0x8009B58Cu,PE_LoadU32(0x8009B58Cu)+1u);issue=1;}
            else if(phase==23u) {issue=1;command=19u;}
        } else if(state==15u) {
            if(!PE_LoadU32(0x8009B594u)) {PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x8009B578u,11u);}
        } else if(state==16u || state==17u) {
            if(PE_LoadU8(state==16u?0x8009B588u:0x8009B58Au)) {
                PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x8009B578u,11u);
            } else issue=1;
        }
        if(issue && (int32_t)PE_LoadU32(0x8009B598u)<=0) {
            PE_StoreU32(0x8009B570u,32u);
            (void)func_8007FCFC(command,0u);
            if(PE_Port_ShouldStop()) return;
        }
    }
    if(PE_LoadU32(0x800A36A0u) && PE_LoadU32(0x8009B554u)) {
        pe_addr_t target=PE_LoadU32(0x800A36A0u);
        if(target==0x8007F7E8u) {
            func_8007F7E8();
            if(PE_Port_ShouldStop()) return;
        } else {
            Bootstrap_ReturnVoid4Indirect("CD_vblank_callback","func_8007FE24",target,0u,0u,0u,0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return;
        }
    }
    uint32_t lane=PE_LoadU32(0x8009B574u);
    if((lane==3u || (lane==1u && !PE_LoadU8(0x8009B58Bu))) && (int32_t)PE_LoadU32(0x8009B598u)<=0) {
        PE_StoreU32(0x8009B570u,33u);
        (void)func_8007FCFC(1u,0u);
    }
}

int func_8007B290(uint32_t mode, pe_addr_t result)
{
    func_80073A44(-1);
    if (PE_Port_ShouldStop()) return 0;
    PE_StoreU32(0x800A3478u,PE_GPU_VSyncQuery()+0x3C0u);
    PE_StoreU32(0x800A347Cu,0u);
    PE_StoreU32(0x800A3480u,0x80011BA8u);
    for (;;) {
        int timeout;
        func_80073A44(-1);
        if (PE_Port_ShouldStop()) return 0;
        timeout=(int32_t)PE_LoadU32(0x800A3478u)<(int32_t)PE_GPU_VSyncQuery();
        if (!timeout) {
            uint32_t count=PE_LoadU32(0x800A347Cu);
            PE_StoreU32(0x800A347Cu,count+1u);
            timeout=(int32_t)count>0x3C0000;
        }
        if (timeout) {
            Bootstrap_ReturnVoid1("func_80073C5C","func_8007B290",0x80011B18u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }
        if (func_80073DE8()) {
            uint8_t index=PE_CdLoadU8(PE_LoadU32(0x8009B27Cu))&3u;
            for (;;) {
                uint32_t pending=(uint32_t)func_8007AAB4();
                if (PE_Port_ShouldStop()) return 0;
                if (!pending) break;
                if ((pending&4u) && PE_LoadU32(0x8009AFB8u)) {
                    Bootstrap_ReturnVoid4Indirect("func_8007B290_afb8_callback",
                        "func_8007B290",PE_LoadU32(0x8009AFB8u),
                        PE_LoadU8(0x8009B295u),0x800A3468u,0u,0u);
                    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                    return 0;
                }
                if ((pending&2u) && PE_LoadU32(0x8009AFB4u)) {
                    Bootstrap_ReturnVoid4Indirect("func_8007B290_afb4_callback",
                        "func_8007B290",PE_LoadU32(0x8009AFB4u),
                        PE_LoadU8(0x8009B294u),0x800A3460u,0u,0u);
                    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                    return 0;
                }
            }
            PE_CdStoreU8(PE_LoadU32(0x8009B27Cu),index);
        }
        for (unsigned lane=0;lane<2;lane++) {
            pe_addr_t tag=0x8009B296u-lane;
            unsigned status=PE_LoadU8(tag);
            if (!status) continue;
            PE_StoreU8(tag,0u);
            if (result)
                for(unsigned i=0;i<8;i++)
                    PE_StoreU8(result+i,PE_LoadU8(0x800A3470u-lane*8u+i));
            return (int)status;
        }
        if (mode) return 0;
    }
}

int func_8007A488(uint32_t mode, pe_addr_t result)
{
    return func_8007B290(mode,result);
}

static void StreamStop(const char *name)
{
    Bootstrap_ReturnVoid(name,"func_8007C564");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

/* The original helper copies forward even on overlap; memcpy/memmove
 * would not preserve that behavior. a3 is ignored by the original leaf. */
static void func_8007CE80(pe_addr_t dst,pe_addr_t src,uint32_t words)
{
    for(uint32_t i=0;i<words;i++) PE_StoreU32(dst+i*4u,PE_LoadU32(src+i*4u));
}

static void StreamAdvanceMemory(void)
{
    if(PE_LoadU32(0x800C0DB8u))
        PE_StoreU32(0x800BCD7Cu,PE_LoadU32(0x800BCD7Cu)+1u);
}

/* 7CEAC..7D054: channel setup, interrupt selection, request wait, DMA issue.
 * Channel3 is the CD path; other channels remain explicit boundaries. */
void func_8007CEAC(uint32_t channel,pe_addr_t address,uint32_t blocks,uint32_t words,
                    uint32_t control,uint32_t interrupt,uint32_t unused)
{
    (void)unused;
    if(channel!=3u) {StreamStop("func_8007CEAC_channel");return;}
    if(PE_CdReg_ReadU32(PE_CDREG_DMA3+8u)&0x01000000u) {
        PE_CdReg_ServiceDMA3();
        if(PE_Port_ShouldStop()) return;
        if(PE_CdReg_ReadU32(PE_CDREG_DMA3+8u)&0x01000000u) {StreamStop("func_8007CEAC_busy");return;}
    }
    pe_addr_t dicr=PE_LoadU32(0x8009B348u),dpcr=PE_LoadU32(0x8009B344u);
    uint8_t enables=dicr==0x1F8010F4u?(uint8_t)(PE_GPU_ReadDICR()>>16u):PE_LoadU8(dicr+2u);
    enables=(uint8_t)interrupt==1u?(uint8_t)(enables|8u):(uint8_t)(enables&~8u);
    /* Byte write must not acknowledge unrelated write-one-to-clear flags. */
    if(dicr==0x1F8010F4u) PE_GPU_WriteDICR((PE_GPU_ReadStoredDICR()&0x0000FFFFu)|((uint32_t)enables<<16u));
    else PE_StoreU8(dicr+2u,enables);
    if(dpcr==0x1F8010F0u) PE_GPU_WriteDPCR(PE_GPU_ReadDPCR()|0x8000u);
    else PE_StoreU32(dpcr,PE_LoadU32(dpcr)|0x8000u);
    PE_CdReg_WriteU32(PE_CDREG_DMA3,address);
    PE_CdReg_WriteU32(PE_CDREG_DMA3+4u,(blocks<<16u)|words);
    if(!(PE_CdLoadU8(PE_LoadU32(0x8009B32Cu))&0x40u)) {StreamStop("func_8007CEAC_data_wait");return;}
    PE_CdReg_WriteU32(PE_CDREG_DMA3+8u,control);
}

static void StreamResetPartial(void)
{
    uint32_t begin=PE_LoadU32(0x800BE9E4u);
    uint32_t count=PE_LoadU32(0x800BE998u)-begin;
    PE_StoreU32(0x800A5D54u,0u);
    PE_StoreU16(0x800A8018u,0u);
    /* Original 7C444 clears a word, not the whole32-byte slot. */
    for(uint32_t i=0;i<count;i++)
        PE_StoreU32(PE_LoadU32(0x800C0DC8u)+((begin+i)<<5u),0u);
    PE_StoreU32(0x800BE998u,PE_LoadU32(0x800BE9E4u));
    PE_StoreU16(PE_LoadU32(0x800A34A0u),0u);
}

static int StreamAuxiliary(void)
{
    pe_addr_t cb=PE_LoadU32(0x800B0CCCu);
    if(!cb) return 1;
    Bootstrap_ReturnVoid4Indirect("func_8007C564_auxiliary_callback",
        "func_8007C564",cb,0u,0u,0u,0u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}

static uint32_t StreamOutputChcr(void)
{
    pe_addr_t reg=PE_LoadU32(0x8009B34Cu);
    if(reg==0x1F801098u) {
        PeMdecState state;PE_MDEC_GetState(&state);return state.dma1_chcr;
    }
    return PE_LoadU32(reg);
}

void func_8007C564(void)
{
    /* Explicit host-stack adaptation. Response bytes and sector location
     * retain guest scratch, including the original no-write/no-result path.
     * No equivalence to arbitrary original caller stack residue is claimed. */
    const pe_addr_t response=0x801FFE80u,location=0x801FFE88u;
    pe_addr_t rec;
    uint32_t memory,control;
    int last;
    if(PE_LoadU32(0x800B89F4u)==1u) return;
    if(PE_LoadU32(0x800A801Cu) && (StreamOutputChcr()&0x01000000u)) {
        PE_StoreU32(0x800B0CD0u,1u);
        StreamAdvanceMemory();
        PE_StoreU32(0x8009B374u,1u);
        return;
    }
    if(func_8007A488(1u,response)==5 || PE_Port_ShouldStop()) return;
    if(PE_LoadU8(response)&4u) { PE_StoreU32(0x8009B374u,3u);return; }
    rec=PE_LoadU32(0x800C0DC8u)+(PE_LoadU32(0x800BE998u)<<5u);
    PE_StoreU32(0x800A34A0u,rec);
    if(PE_LoadU16(rec)) {
        StreamAdvanceMemory();PE_StoreU32(0x8009B374u,4u);return;
    }
    PE_CdStoreU8(PE_LoadU32(0x8009B32Cu),0u);
    PE_CdStoreU8(PE_LoadU32(0x8009B338u),0u);
    PE_CdStoreU8(PE_LoadU32(0x8009B32Cu),0u);
    PE_CdStoreU8(PE_LoadU32(0x8009B338u),0x80u);
    PE_CdStoreU32(PE_LoadU32(0x8009B33Cu),0x20943u);
    PE_CdStoreU32(PE_LoadU32(0x8009B340u),0x1323u);
    if(!PE_LoadU32(0x800A8020u)) {
        /* Reading a real FIFO must consume distinct bytes. The existing
         * command shadow cannot substitute one latched byte twelve times. */
        if(!PE_CdReg_DeviceEnabled()) {StreamStop("func_8007C564_sector_fifo");return;}
        for(unsigned i=0;i<12u;i++) {
            uint8_t byte=PE_CdLoadU8(PE_LoadU32(0x8009B334u));
            if(PE_Port_ShouldStop()) return;
            if(i<4u) PE_StoreU8(location+i,byte);
        }
    }
    memory=PE_LoadU32(0x800C0DB8u);
    if(!memory) {
        if(!PE_CdReg_DeviceEnabled()) {
            Bootstrap_ReturnVoid5("func_8007CEAC","func_8007C564",3u,rec,0u,8u,0x11000000u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return;
        }
        func_8007CEAC(3u,rec,0u,8u,0x11000000u,0u,0u);
        if(PE_Port_ShouldStop()) return;
    } else func_8007CE80(rec,memory+(PE_LoadU32(0x800BCD7Cu)<<11u),8u);
    if(PE_CdLoadU32(PE_LoadU32(0x8009B35Cu))&0x01000000u) {
        StreamStop("func_8007C564_dma3_wait");return;
    }
    rec=PE_LoadU32(0x800A34A0u);
    PE_StoreU32(rec+28u,PE_LoadU32(location));
    PE_CdStoreU32(PE_LoadU32(0x8009B33Cu),0x20843u);
    PE_CdStoreU32(PE_LoadU32(0x8009B340u),0x1325u);
    if(PE_LoadU32(0x800C0DC0u)==1u && PE_LoadU32(0x800B6918u)) {
        if(PE_LoadU32(0x800B6918u)!=PE_LoadU16(rec+8u)) {
            PE_StoreU16(rec,0u);StreamAdvanceMemory();return;
        }
        PE_StoreU32(0x800C0DC0u,0u);
    }
    if(PE_LoadU16(rec)!=0x160u ||
        ((PE_LoadU16(rec+2u)>>10u)&31u)!=PE_LoadU32(0x800B8620u)) {
        if(PE_LoadU32(0x800C0DB8u)) PE_StoreU32(0x800BCD7Cu,0u);
        PE_StoreU32(0x8009B374u,5u);PE_StoreU16(rec,0u);return;
    }
    if((int32_t)(int16_t)PE_LoadU16(0x800A8018u)!=(int32_t)PE_LoadU16(rec+4u) ||
        (PE_LoadU32(0x800A5D54u) && PE_LoadU32(0x800A5D54u)!=PE_LoadU16(rec+8u))) {
        StreamResetPartial();StreamAdvanceMemory();PE_StoreU32(0x8009B374u,6u);return;
    }
    if(!PE_LoadU16(rec+4u)) {
        uint32_t frame=PE_LoadU16(rec+8u);
        PE_StoreU16(0x800A8018u,0u);PE_StoreU32(0x800A5D54u,frame);
        if(PE_LoadU32(0x800C0DBCu) && frame>=PE_LoadU32(0x800C0DBCu)) {
            StreamResetPartial();PE_StoreU32(0x800C0DC0u,1u);
            if(!StreamAuxiliary()) return;
            StreamAdvanceMemory();PE_StoreU32(0x8009B374u,7u);return;
        }
        if(PE_LoadU32(0x800C20C4u)-PE_LoadU32(0x800BE998u)-1u<PE_LoadU16(rec+6u)) {
            if(!PE_LoadU32(0x800C0DBCu)) {
                PE_StoreU16(rec,1u);PE_StoreU32(0x800C0DC0u,1u);
                if(!StreamAuxiliary()) return;
                StreamAdvanceMemory();PE_StoreU32(0x8009B374u,8u);return;
            }
            if((int16_t)PE_LoadU16(PE_LoadU32(0x800C0DC8u))) {
                PE_StoreU16(rec,0u);StreamAdvanceMemory();PE_StoreU32(0x8009B374u,9u);return;
            }
            PE_StoreU16(rec,1u);
            PE_StoreU32(0x800BE998u,0u);
            func_8007CE80(PE_LoadU32(0x800C0DC8u),rec,8u);
            rec=PE_LoadU32(0x800C0DC8u);PE_StoreU32(0x800A34A0u,rec);
        }
        PE_StoreU32(0x800BE9E4u,PE_LoadU32(0x800BE998u));
    }
    PE_StoreU32(0x8009B374u,10u);
    PE_StoreU16(0x800A8018u,(uint16_t)(PE_LoadU16(0x800A8018u)+1u));
    PE_StoreU32(0x800C0DC4u,PE_LoadU32(0x800C0DC8u)+
        (PE_LoadU32(0x800C20C4u)<<5u)+PE_LoadU32(0x800BE998u)*2016u);
    control=0x11000000u;
    if(PE_LoadU32(0x800A801Cu)) {
        PE_CdStoreU32(PE_LoadU32(0x8009B33Cu),0x20943u);
        PE_CdStoreU32(PE_LoadU32(0x8009B340u),0x1323u);
    } else {
        control=0x11400100u;
        PE_CdStoreU32(PE_LoadU32(0x8009B33Cu),0x21020843u);
    }
    last=(uint32_t)PE_LoadU16(rec+6u)-1u==PE_LoadU16(rec+4u);
    if(last) PE_StoreU32(0x800B89F4u,1u);
    memory=PE_LoadU32(0x800C0DB8u);
    if(!memory) {
        if(!PE_CdReg_DeviceEnabled()) {
            Bootstrap_ReturnVoid5("func_8007CEAC","func_8007C564",3u,
                PE_LoadU32(0x800C0DC4u),0u,0x1F8u,control);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return;
        }
        func_8007CEAC(3u,PE_LoadU32(0x800C0DC4u),0u,0x1F8u,control,(uint32_t)last,0u);
        if(PE_Port_ShouldStop()) return;
    } else {
        func_8007CE80(PE_LoadU32(0x800C0DC4u),
            memory+(PE_LoadU32(0x800BCD7Cu)<<11u)+32u,0x1F8u);
        StreamAdvanceMemory();
    }
    if(last) {
        PE_StoreU16(0x800A8018u,0u);PE_StoreU32(0x800A5D54u,0u);
        PE_StoreU32(0x800B8620u,PE_LoadU32(0x800B6914u));
    }
    PE_CdStoreU32(PE_LoadU32(0x8009B340u),0x1325u);
    PE_StoreU16(PE_LoadU32(0x800A34A0u),3u);
    PE_StoreU32(0x800BE998u,PE_LoadU32(0x800BE998u)+1u);
    if(PE_LoadU32(0x800C0DB8u) && PE_LoadU32(0x800B89F4u)) func_8007C214();
}

/* Data-ready callback chain:7C13C ->80778 ->7F88C ->813E8 ->7C564. */
static void CdCopyResponse(pe_addr_t dst,pe_addr_t src)
{
    if(!dst) return;
    if(!src) { PE_StoreU8(dst,0u);return; }
    for(unsigned i=0;i<8;i++) PE_StoreU8(dst+i,PE_LoadU8(src+i));
}

static void CdDataCallback(pe_addr_t target,uint32_t status,pe_addr_t response);

static void func_8008080C(uint32_t status,pe_addr_t response)
{
    uint32_t offset=0u;
    if((uint8_t)status!=5u) {
        offset=PE_LoadU32(0x8009B624u+PE_LoadU8(0x8009B558u)*4u)-1u;
        if((int32_t)offset<0) return;
    }
    uint8_t value=PE_LoadU8(response+offset);
    PE_StoreU8(0x8009B588u,value>>7u);
    PE_StoreU8(0x8009B589u,(value>>6u)&1u);
    PE_StoreU8(0x8009B58Au,(value>>5u)&1u);
    PE_StoreU8(0x8009B58Bu,(value>>1u)&1u);
    PE_StoreU8(0x8009B56Cu,value);
    CdCopyResponse(0x8009B564u,response);
}

/* 80220..80404: acknowledged public command and callback publication. */
static void func_80080220(uint32_t status,pe_addr_t response)
{
    status=(uint8_t)status;
    if(status==2u) {
        uint8_t cmd=PE_LoadU8(0x8009B558u);
        if(cmd==14u) {
            if((PE_LoadU8(0x8009B581u)^PE_LoadU8(0x8009B559u))&0x80u) {
                PE_StoreU32(0x8009B578u,15u);PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x8009B594u,3u);
            } else {PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x8009B578u,11u);}
            PE_StoreU8(0x8009B581u,PE_LoadU8(0x8009B559u));
        } else if(cmd==3u || cmd==6u || cmd==27u) {
            PE_StoreU32(0x8009B578u,cmd==3u?16u:17u);PE_StoreU32(0x8009B574u,2u);
            PE_StoreU8(0x8009B587u,cmd);PE_StoreU32(0x8009B5A0u,1200u);
        } else {
            uint32_t offset=(uint32_t)cmd-2u;
            if(offset<26u) {
                pe_addr_t target=PE_LoadU32(0x80011D0Cu+offset*4u);
                switch(target) {
                case 0x80080318u:PE_StoreU32(0x8009B582u,PE_LoadU32(0x8009B559u));break;
                case 0x80080340u:PE_StoreU8(0x8009B586u,PE_LoadU8(0x8009B558u));break;
                case 0x80080354u:PE_StoreU8(0x8009B587u,PE_LoadU8(0x8009B558u));break;
                case 0x80080368u:break;
                default:
                    Bootstrap_ReturnVoid4Indirect("CD_command_table","func_80080220",target,status,response,0u,0u);
                    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return;
                }
            }
            PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x8009B578u,11u);
        }
    } else {
        int error=(PE_LoadU8(0x8009B56Cu)&0x10u)!=0;
        PE_StoreU32(0x8009B574u,error?2u:1u);PE_StoreU32(0x8009B578u,error?12u:11u);
    }
    if(PE_LoadU32(0x800A36A4u) && PE_LoadU32(0x8009B554u))
        CdDataCallback(PE_LoadU32(0x800A36A4u),status,response);
}

/* 80404..8068C: internal command progression, including startup retries. */
static void func_80080404(uint32_t status,pe_addr_t response)
{
    if(PE_LoadU32(0x8009B578u)==12u) PE_StoreU32(0x8009B578u,13u);
    uint32_t state=PE_LoadU32(0x8009B578u);
    uint8_t flags=PE_LoadU8(0x8009B56Cu);
    if((uint8_t)status!=2u) {
        if(flags&0x10u) {PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x8009B578u,12u);}
        else if(state-16u<2u) {PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x8009B578u,11u);}
        return;
    }
    if(state==13u) {
        if(!(flags&0x10u)) {
            PE_StoreU32(0x8009B578u,14u);PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x8009B57Cu,21u);
            PE_StoreU32(0x8009B590u,PE_LoadU32(0x8009B590u)+1u);
        }
        return;
    }
    if(state==14u) {
        uint32_t phase=PE_LoadU32(0x8009B57Cu);
        if(phase==21u) {
            if(!(flags&0x10u)) {
                PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x8009B578u,14u);
                PE_StoreU32(0x8009B57Cu,22u);PE_StoreU32(0x8009B58Cu,0u);
            }
        } else if(phase==22u && !(flags&2u)) {
            if((int32_t)PE_LoadU32(0x8009B58Cu)>=301) {
                PE_StoreU32(0x8009B574u,3u);
                if(PE_LoadU32(0x800A36ACu)) CdDataCallback(PE_LoadU32(0x800A36ACu),5u,response);
            }
        } else if(phase==22u || phase==23u) {
            PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x8009B578u,14u);PE_StoreU32(0x8009B57Cu,phase==22u?23u:24u);
        } else if(phase==24u && flags==2u) {
            PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x8009B578u,11u);PE_StoreU32(0x8009B57Cu,0u);
            if(PE_LoadU32(0x800A36ACu)) CdDataCallback(PE_LoadU32(0x800A36ACu),2u,response);
        }
        return;
    }
    if(state-16u<2u && !PE_LoadU32(0x8009B5A0u) && !(flags&2u)) {
        PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x8009B578u,11u);
        if(PE_LoadU32(0x800A36A8u) && PE_LoadU32(0x8009B554u)) {
            CdDataCallback(PE_LoadU32(0x800A36A8u),5u,response);
            if(PE_Port_ShouldStop()) return;
        }
        if(PE_LoadU32(0x800A36A4u) && PE_LoadU32(0x8009B554u))
            CdDataCallback(PE_LoadU32(0x800A36A4u),5u,response);
    }
}

/* 8068C..80778: remaining command completions and SetMode transitions. */
static void func_8008068C(uint32_t status,pe_addr_t response)
{
    status=(uint8_t)status;
    if(status==2u && PE_LoadU8(0x8009B558u)==14u) {
        uint8_t mode=PE_LoadU8(0x8009B559u);
        if((PE_LoadU8(0x8009B581u)^mode)&0x80u) {
            PE_StoreU32(0x8009B578u,15u);PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x8009B594u,3u);
        }
        PE_StoreU8(0x8009B581u,mode);
    }
    if(status==5u) {
        if(PE_LoadU8(0x8009B56Cu)&0x10u) {
            PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x8009B578u,12u);
            if(PE_LoadU32(0x800A36A4u) && PE_LoadU32(0x8009B554u))
                CdDataCallback(PE_LoadU32(0x800A36A4u),5u,response);
        } else {PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x8009B578u,11u);}
    }
}

/* 80164..80220: registered CD command-completion callback. */
void func_80080164(uint32_t status,pe_addr_t response)
{
    func_8008080C((uint8_t)status,response);
    PE_StoreU32(0x8009B59Cu,0u);PE_StoreU32(0x8009B598u,0u);
    if(PE_LoadU8(0x8009B56Cu)&0x10u) status=5u;
    uint32_t kind=PE_LoadU32(0x8009B570u);
    if(kind==31u) func_80080220((uint8_t)status,response);
    else if(kind==32u) func_80080404((uint8_t)status,response);
    else func_8008068C((uint8_t)status,response);
    if(PE_Port_ShouldStop()) return;
    if(!PE_LoadU32(0x8009B598u)) PE_StoreU32(0x8009B570u,33u);
}

/* 7EB88..7EC14: publish a completion using the live ring index. */
void func_8007EB88(uint32_t sequence,uint32_t status,pe_addr_t response)
{
    PE_StoreU32(0x800A3610u+(PE_LoadU32(0x800A3690u)<<4u),sequence);
    PE_StoreU8(0x800A3614u+(PE_LoadU32(0x800A3690u)<<4u),(uint8_t)status);
    CdCopyResponse(0x800A3615u+(PE_LoadU32(0x800A3690u)<<4u),response);
    uint32_t next=PE_LoadU32(0x800A3690u)+1u;
    PE_StoreU32(0x800A3690u,next);
    if((int32_t)next>=8) PE_StoreU32(0x800A3690u,0u);
}

/* 7E5C4..7E6B0: remove the head's consecutive sequence group. */
void func_8007E5C4(void)
{
    uint32_t sequence=PE_LoadU32(0x800A3540u+PE_LoadU32(0x800A3600u)*24u);
    while((int32_t)PE_LoadU32(0x800A3608u)>0) {
        pe_addr_t record=0x800A3540u+PE_LoadU32(0x800A3600u)*24u;
        PE_StoreU32(record,0u);PE_StoreU8(record+4u,0u);
        for(unsigned j=0;j<4;j++) PE_StoreU8(record+8u-j,0u);
        PE_StoreU32(record+12u,0u);PE_StoreU32(record+16u,0u);PE_StoreU32(record+20u,0u);
        uint32_t next=PE_LoadU32(0x800A3600u)+1u;
        PE_StoreU32(0x800A3600u,next);
        if((int32_t)next>=8) PE_StoreU32(0x800A3600u,0u);
        PE_StoreU32(0x800A3608u,PE_LoadU32(0x800A3608u)-1u);
        if(PE_LoadU32(0x800A3540u+PE_LoadU32(0x800A3600u)*24u)!=sequence) break;
    }
    PE_StoreU32(0x800A3604u,PE_LoadU32(0x800A3600u));
}

/* 7E964..7EB88: queued completion, retry and pending-command restart. */
void func_8007E964(uint32_t status,pe_addr_t response)
{
    pe_addr_t record=0x800A3540u+PE_LoadU32(0x800A3604u)*24u;
    uint32_t sequence=PE_LoadU32(record);
    if(sequence) {
        PE_StoreU32(0x800A3510u,sequence);PE_StoreU8(0x800A3514u,(uint8_t)status);
        CdCopyResponse(0x800A3515u,response);
        int finish=0;
        if((uint8_t)status==2u) {
            uint32_t next=PE_LoadU32(0x800A3604u)+1u;
            uint32_t slot=(int32_t)next<8?next:0u;
            if(PE_LoadU32(0x800A3540u+slot*24u)==PE_LoadU32(record))
                PE_StoreU32(0x800A3604u,slot);
            else finish=1;
        } else if((uint8_t)status==5u) {
            int32_t retries=(int32_t)PE_LoadU32(record+20u);
            if(retries>0 || retries==-1) {
                PE_StoreU32(0x800A3604u,PE_LoadU32(0x800A3600u));
                uint32_t current=PE_LoadU32(record+20u);
                if(current!=0xFFFFFFFFu) PE_StoreU32(record+20u,current-1u);
            } else finish=1;
        }
        if(finish) {
            func_8007EB88(PE_LoadU32(record),(uint8_t)status,response);
            pe_addr_t callback=PE_LoadU32(record+16u);
            if(callback) {
                CdDataCallback(callback,(uint8_t)status,response);
                if(PE_Port_ShouldStop()) return;
            }
            func_8007E5C4();
        }
    }
    pe_addr_t callback=PE_LoadU32(0x800B8AB0u);
    if(callback) {
        CdDataCallback(callback,(uint8_t)status,response);
        if(PE_Port_ShouldStop()) return;
    }
    if(func_8007FBF0(0)==1 && (int32_t)PE_LoadU32(0x800A3608u)>0 && func_8007FBF0(0)==1) {
        pe_addr_t pending=0x800A3540u+PE_LoadU32(0x800A3604u)*24u;
        if(PE_LoadU32(pending)) (void)func_8007FB44(PE_LoadU8(pending+4u),PE_LoadU32(pending+12u));
    }
}

/* 7E704..7E8F4: cancel queued commands, then call saved callbacks.
 * The retail queue has eight slots. Corrupt larger positive counts would
 * overwrite its stack callback array and are an explicit native boundary. */
void func_8007E704(uint32_t status,pe_addr_t response)
{
    uint32_t sequences[8]={0};
    pe_addr_t callbacks[8]={0};
    uint32_t slot=PE_LoadU32(0x800A3600u),previous=0u;
    int saved=-1;
    if((int32_t)PE_LoadU32(0x800A3608u)>8) {
        Bootstrap_ReturnVoid1("func_8007E704_queue_count","func_8007E704",PE_LoadU32(0x800A3608u));
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return;
    }
    for(int i=0;i<(int32_t)PE_LoadU32(0x800A3608u);i++) {
        pe_addr_t record=0x800A3540u+slot*24u;
        uint32_t sequence=PE_LoadU32(record);
        if(sequence!=previous) {
            func_8007EB88(sequence,(uint8_t)status,response);
            previous=sequence;
        }
        pe_addr_t callback=PE_LoadU32(record+16u);
        if(callback && (saved<0 || sequences[saved]!=sequence)) {
            saved++;sequences[saved]=sequence;callbacks[saved]=callback;
        }
        slot++;
        if((int32_t)slot>=8) slot=0u;
    }
    PE_StoreU32(0x800A3608u,0u);PE_StoreU32(0x800A3604u,0u);PE_StoreU32(0x800A3600u,0u);
    for(unsigned i=0;i<8;i++) {
        pe_addr_t record=0x800A3540u+i*24u;
        PE_StoreU32(record,0u);PE_StoreU8(record+4u,0u);
        for(unsigned j=0;j<4;j++) PE_StoreU8(record+8u-j,0u);
        PE_StoreU32(record+12u,0u);PE_StoreU32(record+16u,0u);PE_StoreU32(record+20u,0u);
    }
    for(int i=0;i<=saved;i++) {
        CdDataCallback(callbacks[i],(uint8_t)status,response);
        if(PE_Port_ShouldStop()) return;
    }
}

/* 7F960..7F98C: optional startup notification, status byte only. */
void func_8007F960(uint32_t status,pe_addr_t response)
{
    pe_addr_t target=PE_LoadU32(0x800B8AB8u);
    if(target) CdDataCallback(target,(uint8_t)status,response);
}

static void func_8007F88C(uint32_t status,pe_addr_t response)
{
    unsigned low=(uint8_t)status;
    if(low==5u && (PE_LoadU8(response)&0x10u)) {
        func_8007E704(5u,response);
        if(PE_Port_ShouldStop()) return;
    }
    if(low==1u || low==5u || low==4u) {
        pe_addr_t slot=low==4u?0x800A3530u:0x800A3520u;
        PE_StoreU32(slot,1u);PE_StoreU8(slot+4u,(uint8_t)status);
        CdCopyResponse(slot+5u,response);
    }
    pe_addr_t target=PE_LoadU32(0x800B8AB4u);
    if(target) CdDataCallback(target,(uint8_t)status,response);
}

static void func_80080778(uint32_t status,pe_addr_t response)
{
    status=(uint8_t)status;
    func_8008080C(status,response);
    if(PE_LoadU8(0x8009B56Cu)&0x10u) {
        PE_StoreU32(0x8009B574u,2u);PE_StoreU32(0x8009B578u,12u);
    }
    if(PE_LoadU32(0x800A36A8u) && PE_LoadU32(0x8009B554u))
        CdDataCallback(PE_LoadU32(0x800A36A8u),status,response);
}

static void CdDataCallback(pe_addr_t target,uint32_t status,pe_addr_t response)
{
    switch(target) {
    case 0x8007F960u:func_8007F960(status,response);return;
    case 0x8007E964u:func_8007E964(status,response);return;
    case 0x80080164u:func_80080164(status,response);return;
    case 0x80080778u:func_80080778(status,response);return;
    case 0x8007F88Cu:func_8007F88C(status,response);return;
    case 0x800813E8u:
        /* Original8-word wrapper ignores incoming arguments. */
        func_8007C564();return;
    default:
        Bootstrap_ReturnVoid4Indirect("CD_data_callback","CD_data_dispatch",
            target,status,response,0u,0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return;
    }
}

void func_8007C13C(void)
{
    uint8_t index=PE_CdLoadU8(PE_LoadU32(0x8009B27Cu))&3u;
    for(;;) {
        uint32_t events=(uint32_t)func_8007AAB4();
        if(PE_Port_ShouldStop()) return;
        if(!events) break;
        if((events&4u) && PE_LoadU32(0x8009AFB8u)) {
            CdDataCallback(PE_LoadU32(0x8009AFB8u),PE_LoadU8(0x8009B295u),0x800A3468u);
            if(PE_Port_ShouldStop()) return;
        }
        if((events&2u) && PE_LoadU32(0x8009AFB4u)) {
            CdDataCallback(PE_LoadU32(0x8009AFB4u),PE_LoadU8(0x8009B294u),0x800A3460u);
            if(PE_Port_ShouldStop()) return;
        }
    }
    PE_CdStoreU8(PE_LoadU32(0x8009B27Cu),index);
}

/* 7A930..7AAB4: signed LBA conversion and inverse BCD arithmetic.
 * Preserve wrapping input+150, signed division and frame/second/minute store
 * order. Invalid BCD digits are arithmetic inputs, not validation failures. */
pe_addr_t func_8007A930(int32_t lba,pe_addr_t location)
{
    int32_t absolute=(int32_t)((uint32_t)lba+150u);
    int32_t seconds=absolute/75,minutes=seconds/60;
    int32_t frame=absolute-seconds*75;
    seconds-=minutes*60;
    PE_StoreU8(location+2u,(uint8_t)(frame/10*16+frame%10));
    PE_StoreU8(location+1u,(uint8_t)(seconds/10*16+seconds%10));
    PE_StoreU8(location,(uint8_t)(minutes/10*16+minutes%10));
    return location;
}
int32_t func_8007AA34(pe_addr_t location)
{
    uint32_t minute=PE_LoadU8(location),second=PE_LoadU8(location+1u);
    uint32_t frame=PE_LoadU8(location+2u);
    return (int32_t)((((minute>>4u)*10u+(minute&15u))*60u+
        (second>>4u)*10u+(second&15u))*75u+(frame>>4u)*10u+(frame&15u))-150;
}
/* 7C2A0..7C2F8: retry from the sector after the last completed stream DMA.
 * Return the live word after writing the location: aliases affect the result. */
int32_t func_8007C2A0(pe_addr_t location)
{
    if(PE_LoadU32(0x800A8020u)) return -1;
    int32_t lba=func_8007AA34(0x800A3490u);
    func_8007A930((int32_t)((uint32_t)lba+1u),location);
    return (int32_t)PE_LoadU32(0x800A3494u);
}
pe_addr_t func_8007A4BC(pe_addr_t callback)
{
    pe_addr_t old=PE_LoadU32(0x8009AFB8u);
    PE_StoreU32(0x8009AFB8u,callback);return old;
}
pe_addr_t func_8007A8EC(pe_addr_t callback)
{
    return func_80073CF4(3u,callback);
}
/* 7A2A4..7A324: unregister the selected SDK stream callbacks, then clear
 * the original bank/request registers within the critical section. */
void func_8007A2A4(void)
{
    (void)func_80072714();
    if(PE_LoadU32(0x8009AFD8u)==1u) {
        (void)func_800824F0(0u);
        if(PE_Port_ShouldStop()) return;
        (void)func_800824C8(0u);
    } else {
        (void)func_8007A8EC(0u);
        if(PE_Port_ShouldStop()) return;
        (void)func_8007A4BC(0u);
    }
    PE_CdStoreU8(PE_LoadU32(0x8009AF1Cu),0u);
    if(PE_Port_ShouldStop()) return;
    PE_CdStoreU8(PE_LoadU32(0x8009AF28u),0u);
    if(PE_Port_ShouldStop()) return;
    func_80072724();
}

/* 7EE84..7F0C8: optional Setloc prefix followed by the requested command.
 * Capacity is checked independently for each descriptor; a prefix may remain
 * queued when the requested command cannot fit. Sequence zero is skipped. */
uint32_t func_8007EE84(uint32_t command,pe_addr_t param,uint32_t extra,uint32_t callback)
{
    unsigned prefix=PE_LoadU32(0x8009B4BCu+(command&255u)*4u)!=0u && param!=0u;
    for(unsigned pass=0;pass<=prefix;pass++) {
        if((int32_t)PE_LoadU32(0x800A3608u)>=8) return 0;
        uint32_t sequence=PE_LoadU32(0x8009B53Cu)+1u;
        PE_StoreU32(0x8009B53Cu,sequence);
        if(!sequence) PE_StoreU32(0x8009B53Cu,++sequence);
        pe_addr_t record=func_8007E6B0();
        unsigned is_prefix=prefix && pass==0u;
        PE_StoreU32(record,sequence);PE_StoreU8(record+4u,(uint8_t)(is_prefix?2u:command));
        if(param) func_80080950(record+5u,param);
        PE_StoreU32(record+12u,param);
        PE_StoreU32(record+16u,is_prefix?0u:extra);PE_StoreU32(record+20u,is_prefix?0u:callback);
        PE_StoreU32(0x800A3608u,PE_LoadU32(0x800A3608u)+1u);
        if(func_8007FBF0(0)==1 && PE_LoadU32(0x800A3540u+PE_LoadU32(0x800A3604u)*24u)==sequence)
            (void)func_8007E8F4();
        if(PE_Port_ShouldStop()) return 0;
        if(!is_prefix) return sequence;
    }
    return 0;
}
void func_80080998(pe_addr_t dest,pe_addr_t source)
{
    if(!dest) return;
    if(!source) {PE_StoreU8(dest,0u);return;}
    for(unsigned i=0;i<8u;i++) PE_StoreU8(dest+i,PE_LoadU8(source+i));
}
int func_8007FC64(pe_addr_t response)
{
    return func_8007B010(1u,response);
}
/* 7F418..7F608: completed-command ring search and eight response bytes.
 * The signed sequence comparison, reverse search and snapshot/copy order
 * preserve original wrap and alias behavior. */
int func_8007F418(uint32_t sequence,pe_addr_t response)
{
    uint32_t index;
    if(sequence) {
        (void)func_8007FC64(0u);
        if(PE_Port_ShouldStop()) return 0;
        index=PE_LoadU32(0x800A3690u);
        unsigned found=0;
        for(unsigned i=0;i<8u;i++) {
            if(PE_LoadU32(0x800A3610u+index*16u)==sequence) {found=1;break;}
            index++;
            if((int32_t)index>=8) index=0;
        }
        if(!found && !((int32_t)sequence<(int32_t)PE_LoadU32(0x800A3610u+PE_LoadU32(0x800A3690u)*16u))) return 0;
    }
    index=PE_LoadU32(0x800A3690u)-1u;
    if((int32_t)index<0) index=7;
    pe_addr_t record=0u;
    if(sequence) {
        for(unsigned i=0;i<8u;i++) {
            pe_addr_t candidate=0x800A3610u+index*16u;
            if(PE_LoadU32(candidate)==sequence) {record=candidate;break;}
            index--;
            if((int32_t)index<0) index=7;
        }
    } else {
        pe_addr_t candidate=0x800A3610u+index*16u;
        if(PE_LoadU32(candidate)) record=candidate;
    }
    if(!record) return 6;
    uint32_t first=PE_LoadU32(record),second=PE_LoadU32(record+4u),third=PE_LoadU32(record+8u);
    PE_StoreU32(0x800A3500u,first);PE_StoreU32(0x800A3504u,second);PE_StoreU32(0x800A3508u,third);
    PE_StoreU32(0x800A350Cu,PE_LoadU32(record+12u));
    func_80080998(response,0x800A3505u);
    return PE_LoadU8(0x800A3504u);
}
/* Identical original blocking wrappers80D5C and80DC4. Polling the existing
 * SDK worker advances the enabled device clock and dispatches real IRQs. */
int func_80080DC4(int command,pe_addr_t param,pe_addr_t response)
{
    uint32_t sequence=func_8007EE84((uint32_t)command&255u,param,0u,0u);
    if(!sequence || PE_Port_ShouldStop()) return 0;
    for(unsigned polls=0;;polls++) {
        unsigned status=(uint8_t)func_8007F418(sequence,response);
        if(PE_Port_ShouldStop()) return 0;
        if(status) return status==2u;
        if(!PE_CdReg_DeviceEnabled() || polls==0x100000u) {
            Bootstrap_ReturnVoid1("CD_command_wait","func_80080DC4",sequence);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
        }
    }
}
