/* M34 effect 8: original Disc 1 C2, LBA16597, 100 sectors.
 * Overlay SHA256 0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a.
 * Persistent state remains in guest RAM. Untranslated child callbacks still
 * stop through the shared callback registry; no effect is silently omitted. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

/* Retained words below the original F434 entry SP. The original field
 * call history (M34_FRAME_STACK_PRESERVATION.md) establishes the writers.
 * Preserve data from calls, never a captured tuple. Unverified callback
 * graphs invalidate the record; subsequent projectiles need fresh writers.
 * This does not model asynchronous BIOS/IRQ stack activity. */
static struct {
    uint64_t generation;
    unsigned field,vm,known,snaps;
    pe_addr_t actor,opcode,sound_actor,callback;
    int32_t words[6];
    int32_t sound_words[3];
    int32_t construct_word;
    int32_t command_words[2];
} m34_stack;

static void m34_stack_trace(const char *event,pe_addr_t detail)
{
    if(!getenv("PE_M34_STACK_TRACE") || !PE_M34BossEffectOverlay())return;
    fprintf(stderr,"M34_STACK %s detail=%08X tick=%08X mask=%02X words=%08X,%08X,%08X,%08X,%08X,%08X\n",
        event,detail,D_8009D250,m34_stack.known,(uint32_t)m34_stack.words[0],
        (uint32_t)m34_stack.words[1],(uint32_t)m34_stack.words[2],(uint32_t)m34_stack.words[3],
        (uint32_t)m34_stack.words[4],(uint32_t)m34_stack.words[5]);
}

static int m34_stack_keep(void)
{
    /* Original F434 words live on the CPU stack across Aya's nested field/VM
     * work. Connected kite runs reached known=0x38/0x32 because those paths
     * wiped the bit-mask while ClipSound snapshot words survived. */
    return m34_stack.known && m34_stack.field && PE_M34BossEffectOverlay();
}
void PE_M34StackInvalidate(void) { m34_stack.known=0u; }
void PE_M34StackField(int begin)
{
    if(m34_stack.generation!=PE_RamGeneration()) {
        memset(&m34_stack,0,sizeof(m34_stack));
        m34_stack.generation=PE_RamGeneration();
    }
    if(!PE_M34BossEffectOverlay())m34_stack.known=0u;
    else if(begin && m34_stack.field && !m34_stack_keep())m34_stack.known=0u;
    m34_stack.field=begin!=0;
    m34_stack.actor=0u;m34_stack.opcode=0u;m34_stack.vm=0u;
    m34_stack.callback=0u;
}
void PE_M34StackActor(pe_addr_t actor) { m34_stack.actor=actor; }
void PE_M34StackVm(int begin)
{
    if(begin) {
        m34_stack.vm++;
        if((m34_stack.vm!=1u || !m34_stack.field) && !m34_stack_keep())
            m34_stack.known=0u;
    } else if(m34_stack.vm)m34_stack.vm--;
}
void PE_M34StackOpcode(pe_addr_t fn)
{
    m34_stack.opcode=fn;
    if(!fn)return;
    /* Original dispatches in the continuously traced M34 command history.
     * Sound/effect calls have additional writer hooks below. */
    switch(fn) {
    case 0x80012850u:case 0x8001731Cu:case 0x80017294u:case 0x800172E0u:
    case 0x80014DA0u:case 0x800179F8u:case 0x800173F4u:case 0x80018004u:
    case 0x80014694u:case 0x80017A50u:case 0x80014228u:case 0x80019154u:
    case 0x80018080u:case 0x80019C4Cu:case 0x80012E7Cu:case 0x80015648u:
    case 0x800143B0u:case 0x80019BE4u:case 0x800184ECu:case 0x800187C0u:
    case 0x80018774u:case 0x800172FCu:return;
    default:
        if(!m34_stack_keep())m34_stack.known=0u;
        return;
    }
}
static int m34_stack_script(pe_addr_t opcode)
{
    return m34_stack.generation==PE_RamGeneration() && m34_stack.field &&
        m34_stack.actor && m34_stack.vm==1u && m34_stack.opcode==opcode &&
        PE_M34BossEffectOverlay();
}
void PE_M34StackClipSound(pe_addr_t body,pe_addr_t actor,uint32_t volume)
{
    /* Once effect-8 construct+command are sampled, Aya's pistol 184EC must
     * not steal sound_actor (connected kite left boss snap then overwrote). */
    if(PE_M34BossEffectOverlay()) {
        if((m34_stack.known&7u)==7u)return;
        if((m34_stack.snaps&6u)==6u && m34_stack.sound_actor &&
           actor!=m34_stack.sound_actor)
            return;
    }
    if(!m34_stack_script(0x800184ECu) || volume>127u) {
        if(!m34_stack_keep())m34_stack.known=0u;
        return;
    }
    /* 6DED8/6DEE0 save the 2FAF8 caller's body/target registers;
     * 6E184 writes the computed volume into the preceding word. */
    m34_stack.words[0]=(int32_t)volume;m34_stack.words[1]=(int32_t)body;
    m34_stack.words[2]=(int32_t)actor;m34_stack.sound_actor=actor;
    m34_stack.sound_words[0]=(int32_t)volume;m34_stack.sound_words[1]=(int32_t)body;
    m34_stack.sound_words[2]=(int32_t)actor;
    m34_stack.snaps|=1u;
    m34_stack.known|=7u;
    m34_stack_trace("clip-sound",actor);
}
void PE_M34StackConstruct(uint32_t code)
{
    if(!m34_stack_script(0x80018774u) || code!=8u ||
       PE_LoadU8(0x800B0DC7u)!=0u || !(D_8009D1A0&0x80u) ||
       (PE_LoadU32(0x800B0CD8u)&8u)) {
        if(!m34_stack_keep())m34_stack.known=0u;
        return;
    }
    /* 17020/17024 initialize S1 to this original decode table. 6916C
     * saves that register through 18774 -> 6F39C -> 6914C. */
    m34_stack.words[3]=(int32_t)0x80010690u;
    m34_stack.construct_word=(int32_t)0x80010690u;
    m34_stack.snaps|=2u;
    m34_stack.known|=8u;
    m34_stack_trace("constructor",code);
}
void PE_M34StackCallback(pe_addr_t fn)
{
    m34_stack.callback=fn;
    if(!PE_M34BossEffectOverlay()) {
        if(m34_stack.known)m34_stack_trace("unknown-callback",fn);
        m34_stack.known=0u;
        return;
    }
    switch(fn) {
    case 0x8018F0E4u:case 0x8018F12Cu:case 0x8018F1B8u:
    case 0x8018F23Cu:case 0x8018F244u:case 0x8018F24Cu:
    case 0x8018F36Cu:case 0x8018F374u:case 0x8018F380u:
    case 0x8018F3BCu:case 0x8018F3C4u:case 0x8018F434u:
    case 0x8018F830u:case 0x8018FC54u:case 0x8018FDD4u:
    case 0x8018FDE4u:case 0x8018FEE0u:
        return;
    /* Translated PE_WeaponCallback leaves. Aya's pistol draw/update during
     * M34 used to clear the F434 record (kite/reload pilot, f=60323). */
    case 0x800CE1FCu:case 0x800CE2B4u:case 0x800CE3B4u:case 0x800CE3ACu:
    case 0x800CE464u:case 0x800CE470u:
    case 0x800CD980u:case 0x800CDA5Cu:case 0x800CDC24u:
    case 0x800CDD0Cu:case 0x800CDE90u:case 0x800CDD04u:
    case 0x800C9C20u:case 0x800C9C8Cu:case 0x800C9D9Cu:
    case 0x800C9EA8u:case 0x800C9FD8u:case 0x800C9EA0u:
    case 0x800CA4A8u:case 0x800CDF40u:case 0x800CA4B4u:
    case 0x800CA540u:case 0x800CDF4Cu:case 0x800CDFE0u:
        return;
    default:
        /* Connected known=0x32: an unlisted callback wiped the mask while
         * ClipSound snapshot + words[3..5] survived. Keep overlay-live data. */
        if(m34_stack_keep()) {
            m34_stack_trace("kept-callback",fn);
            return;
        }
        if(m34_stack.known)m34_stack_trace("unknown-callback",fn);
        m34_stack.known=0u;
        return;
    }
}
static int m34_stack_drawing(pe_addr_t callback)
{
    return m34_stack.generation==PE_RamGeneration() && m34_stack.field &&
        m34_stack.callback==callback && PE_M34BossEffectOverlay();
}
static void m34_stack_low_half(unsigned word,int16_t value)
{
    m34_stack.words[word]=(int32_t)(((uint32_t)m34_stack.words[word]&0xFFFF0000u)|(uint16_t)value);
    /* A partial store cannot establish the untouched high half. */
}
void PE_M34StackPointQuad(const int16_t vertices[4][4])
{
    if(!m34_stack_drawing(0x8018F830u))return;
    /* C62A0/C6264/C625C/C62B8 at the F830 -> C61A8 depth.
     * The fourth vertex's Y is zero; the surrounding padding is retained. */
    m34_stack_low_half(0,vertices[2][2]);
    m34_stack.words[1]=(int32_t)((uint16_t)vertices[3][0]|((uint32_t)(uint16_t)vertices[3][1]<<16u));
    m34_stack_low_half(2,vertices[3][2]);m34_stack.known|=2u;
}
int PE_M34StackRead(int32_t retained[6])
{
    if(m34_stack.generation!=PE_RamGeneration() || !m34_stack.field ||
       !PE_M34BossEffectOverlay())return 0;
    if((m34_stack.known&0x38u)==0x38u && (m34_stack.known&7u)!=7u &&
       m34_stack.sound_actor &&
       (uint32_t)m34_stack.words[2]==(uint32_t)m34_stack.sound_actor)
        m34_stack.known|=7u;
    if(m34_stack.known!=63u)return 0;
    memcpy(retained,m34_stack.words,sizeof(m34_stack.words));return 1;
}

static void boss_projectile_update(pe_addr_t slot,pe_addr_t rec,pe_addr_t data);
static void boss_trail_draw(pe_addr_t data);
static void boss_projectile_draw(pe_addr_t slot,pe_addr_t rec,pe_addr_t data);

int PE_M34BossEffectOverlay(void)
{
    return PE_LoadU32(0x8018F014u)==0x0C0308BEu &&
        PE_LoadU32(0x8018F020u)==0x2463FF98u &&
        PE_LoadU32(0x8018FF80u)==0x8018F00Cu &&
        PE_LoadU32(0x8018FF3Cu)==0x8018F434u;
}

/* Full 43 words 8018F00C..8018F0B8. */
int PE_M34BossEffectInit(pe_addr_t slot)
{
    PE_StoreU32(func_800C22F8(slot),0x8018FF98u);
    PE_StoreU8(0x80190054u,32u);PE_StoreU8(0x80190055u,3u);
    PE_StoreU8(0x80190064u,43u);PE_StoreU8(0x80190065u,2u);
    PE_StoreU16(0x8019006Au,128u);
    PE_StoreU16(0x80190058u,0u);PE_StoreU16(0x8019005Au,0u);
    PE_StoreU8(0x80190056u,0u);PE_StoreU16(0x80190068u,0u);
    PE_StoreU8(0x80190060u,128u);PE_StoreU8(0x80190061u,128u);
    PE_StoreU8(0x80190062u,128u);PE_StoreU8(0x80190066u,0u);
    return 0;
}

/* Full 11 words F0B8..F0E4. Args five/six are copied by F0C8/F0D0;
 * C2AF0 ignores them, but F434 later observes those retained stack words. */
int PE_M34BossEffectCommand(pe_addr_t slot,uint32_t mode,uint32_t index,uint32_t value,uint32_t extra0,uint32_t extra1)
{
    if(m34_stack_script(0x800187C0u)) {
        m34_stack.words[4]=(int32_t)extra0;m34_stack.words[5]=(int32_t)extra1;
        m34_stack.command_words[0]=(int32_t)extra0;
        m34_stack.command_words[1]=(int32_t)extra1;
        m34_stack.snaps|=4u;
        m34_stack.known|=48u;
        m34_stack_trace("command",slot);
    } else if(!m34_stack_keep()) {
        m34_stack.known=0u;
    }
    (void)func_800C2AF0(slot,(int32_t)mode,(int32_t)index,value);
    return 0;
}

/* Full 33 words 8018F1B8..8018F23C. */
int PE_M34BossEffectCleanup(pe_addr_t slot)
{
    PE_StoreU8(slot,4u);
    if ((unsigned)func_800C6CE0(slot)>=2u) {
        pe_addr_t body=PE_LoadU32(PE_LoadU32(slot+8u));
        if(body<0x200000u)body|=0x80000000u;
        PE_StoreU32(body,PE_LoadU32(body)&0xC0FFFFFFu);
        body=PE_LoadU32(PE_LoadU32(slot+8u));
        pe_addr_t action=PE_LoadU32((body<0x200000u?body|0x80000000u:body)+24u);
        PE_StoreU8(action<0x200000u?action|0x80000000u:action,4u);
    }
    return 0;
}

/* Full 18 words F0E4..F12C and 35 words F12C..F1B8. */
int PE_M34BossEffectDraw(pe_addr_t slot)
{
    if(func_800C6CE0(slot)==3)(void)func_800C2414(slot,0x8018FF48u);
    return 0;
}
int PE_M34BossEffectUpdate(pe_addr_t slot)
{
    int result=-1;
    if(func_800C6CE0(slot)==3) {
        result=func_800C251C(slot,0x8018FF5Cu);
        result|=func_800C2758(slot,0x8018FF34u,0x8018FF70u);
    }
    if(result==-1)(void)PE_M34BossEffectCleanup(slot);
    return 0;
}

/* Original grouped loads precede stores, including overlapping operands. */
static void copy_words(pe_addr_t dst,pe_addr_t src,unsigned count,unsigned batch)
{
    for(unsigned i=0;i<count;) {
        uint32_t words[4];unsigned n=count-i<batch?count-i:batch;
        for(unsigned j=0;j<n;j++)words[j]=PE_LoadU32(src+(i+j)*4u);
        for(unsigned j=0;j<n;j++)PE_StoreU32(dst+(i+j)*4u,words[j]);
        i+=n;
    }
}

/* Returns whether this original child entry is translated. */
int PE_M34BossEffectChild(pe_addr_t fn,pe_addr_t slot,pe_addr_t rec,pe_addr_t data)
{
    switch(fn) {
    case 0x8018F434u: {
        int32_t retained[6];
        pe_addr_t owner=PE_LoadU32(slot+8u);
        m34_stack_trace("projectile",data);
        /* Connected kite: words[] is mutated (PointQuad / other scripts) and
         * known bits are partial (0x32/0x38). F434 inputs are the three
         * writer snapshots — same data the original stack slots held. */
        if(owner!=m34_stack.sound_actor || m34_stack.snaps!=7u ||
           (uint32_t)m34_stack.sound_words[2]!=owner) {
            fprintf(stderr,
                    "[EFFECT] F434 stack not ready slot8=%08X sound=%08X known=%02X snaps=%u field=%u w2=%08X snap2=%08X w3=%08X cw=%08X\n",
                    owner,(unsigned)m34_stack.sound_actor,
                    m34_stack.known,m34_stack.snaps,m34_stack.field,
                    (uint32_t)m34_stack.words[2],(uint32_t)m34_stack.sound_words[2],
                    (uint32_t)m34_stack.words[3],(uint32_t)m34_stack.construct_word);
            return 0;
        }
        retained[0]=m34_stack.sound_words[0];
        retained[1]=m34_stack.sound_words[1];
        retained[2]=m34_stack.sound_words[2];
        retained[3]=m34_stack.construct_word;
        retained[4]=m34_stack.command_words[0];
        retained[5]=m34_stack.command_words[1];
        PE_M34BossProjectileInit(data,retained);
        /* The two 79754 rotation outputs stop before their translation
         * words. Full original F434 preserves all six retained inputs. */
        return 1;
    }
    case 0x8018F24Cu: {
        pe_addr_t actor=PE_LoadU32(slot+8u);
        PE_StoreU32(0x80190048u,actor);
        pe_addr_t matrix=PE_LoadU32(actor+0x238u);
        actor=PE_LoadU32(0x80190048u);
        copy_words(0x80190028u,matrix,8u,3u);
        matrix=PE_LoadU32(actor+0x238u);
        copy_words(0x80190070u,matrix+0x620u,8u,4u);
        if(PE_LoadU32(slot+8u)==PE_LoadU32(0x8009D254u))
            copy_words(0x80190070u,PE_LoadU32(PE_LoadU32(0x80190048u)+0x238u),8u,4u);
        return 1;
    }
    case 0x8018F36Cu:case 0x8018F3BCu:return 1;
    case 0x8018F374u:PE_StoreU8(rec+1u,2u);return 1;
    case 0x8018F380u:
        PE_StoreU16(data,0u);
        PE_StoreU16(data+2u,(uint16_t)PE_LoadU32(PE_LoadU32(0x800E2248u)+24u));return 1;
    case 0x8018F3C4u: {
        pe_addr_t value=PE_LoadU32(0x800E2248u)+24u;
        PE_StoreU32(value,(uint32_t)(int32_t)(int16_t)PE_LoadU16(data+2u));
        (void)func_800C2B90(slot,2u,0x8018FF70u,0x8018FF34u);
        PE_StoreU8(rec+1u,2u);return 1;
    }
    case 0x8018FDD4u:
        PE_StoreU8(data+2u,0u);PE_StoreU16(data+4u,128u);return 1;
    case 0x8018FC54u:boss_projectile_update(slot,rec,data);return 1;
    case 0x8018FDE4u:boss_trail_draw(data);return 1;
    case 0x8018F830u:boss_projectile_draw(slot,rec,data);return 1;
    case 0x8018FEE0u: {
        uint8_t time=(uint8_t)(PE_LoadU8(data+2u)+1u);
        PE_StoreU8(data+2u,time);
        if((int8_t)time>=21)PE_StoreU8(rec+1u,2u);
        int value=(int16_t)PE_LoadU16(data+4u);
        if(value>=9)PE_StoreU16(data+4u,(uint16_t)(value-8));
        return 1;
    }
    default:return 0;
    }
}

/* F434..F830, all 255 original words. The original retains translations
 * below the incoming stack pointer; callers must supply those six words.
 * The field dispatcher supplies a verified writer record when available. */
static void boss_rotate_vector(const int16_t rotation[9],const int16_t in[3],int16_t out[3])
{
    for(unsigned i=0;i<9;i++)g_pe_gte.rt[i/3u][i%3u]=rotation[i];
    PE_GTE_SetV0(in[0],in[1],in[2]);PE_GTE_MVMVA(0x486012u);
    for(unsigned i=0;i<3;i++)out[i]=(int16_t)g_pe_gte.ir[i];
}
static void boss_compose(pe_addr_t dest,const int16_t left[9],const int32_t translation[3],
                         const int16_t right[9],const int32_t position[3])
{
    for(unsigned i=0;i<9;i++)g_pe_gte.rt[i/3u][i%3u]=left[i];
    for(unsigned col=0;col<3;col++) {
        PE_GTE_SetIR(right[col],right[col+3u],right[col+6u]);
        PE_GTE_MVMVA(0x49E012u);
        for(unsigned row=0;row<3;row++)PE_StoreU16(dest+(row*3u+col)*2u,(uint16_t)g_pe_gte.ir[row]);
    }
    for(unsigned i=0;i<3;i++)g_pe_gte.tr[i]=translation[i];
    PE_GTE_SetV0((int16_t)position[0],(int16_t)position[1],(int16_t)position[2]);
    PE_GTE_MVMVA(0x480012u);
    for(unsigned i=0;i<3;i++)PE_StoreU32(dest+20u+i*4u,(uint32_t)g_pe_gte.ir[i]);
}
void PE_M34BossProjectileInit(pe_addr_t data,const int32_t retained[6])
{
    int16_t yaw[9],rotation[9],angles[3],in[3]={0,0,0},velocity[3];
    for(unsigned i=0;i<3;i++)angles[i]=(int16_t)PE_LoadU16(0x8018EFF4u+i*2u);
    pe_addr_t param=PE_LoadU32(0x800E2248u)+24u;
    pe_addr_t command=PE_LoadU32(0x800E2248u)+80u;
    int16_t turn[3]={0,(int16_t)(PE_LoadU32(param)+PE_LoadU32(command)),0};
    PE_RotMatrix79754_values(turn,yaw);
    command=PE_LoadU32(0x800E2248u)+72u;
    in[2]=(int16_t)(0u-PE_LoadU32(command));
    PeEffectMatrix matrix;PE_EffectReadMatrix(0x80190028u,&matrix);
    boss_rotate_vector(matrix.r,in,velocity);
    for(unsigned i=0;i<3;i++)PE_StoreU16(data+24u+i*2u,(uint16_t)velocity[i]);
    boss_rotate_vector(yaw,velocity,velocity);
    for(unsigned i=0;i<3;i++)PE_StoreU16(data+24u+i*2u,(uint16_t)velocity[i]);
    copy_words(data+36u,0x80190028u,8u,4u);
    PE_StoreU16(data+8u,(uint16_t)PE_LoadU32(0x80190084u));
    PE_StoreU16(data+10u,(uint16_t)(PE_LoadU16(0x800942ECu)-256u));
    uint32_t z=PE_LoadU32(0x8019008Cu);
    PE_StoreU16(data+4u,128u);
    PE_StoreU16(data+16u,0u);PE_StoreU16(data+18u,0u);PE_StoreU16(data+20u,0u);
    PE_StoreU16(data+6u,0u);PE_StoreU8(data+2u,0u);PE_StoreU8(data,1u);
    PE_StoreU16(data+12u,(uint16_t)z);
    PE_StoreU8(data+1u,(uint8_t)PE_LoadU32(PE_LoadU32(0x800E2248u)+20u));
    PE_RotMatrix79754_values(angles,rotation);
    PE_EffectReadMatrix(data+36u,&matrix);
    boss_compose(data+36u,matrix.r,matrix.t,rotation,retained);
    PE_EffectReadMatrix(data+36u,&matrix);
    boss_compose(data+36u,yaw,retained+3u,matrix.r,matrix.t);
}

/* Full 96 words FC54..FDD4. Keep both polygon calls: allocation can write
 * guest storage between them. The signed byte remainder follows MIPS div. */
static void boss_projectile_update(pe_addr_t slot,pe_addr_t rec,pe_addr_t data)
{
    uint16_t x=PE_LoadU16(data+8u),vx=PE_LoadU16(data+24u);
    uint16_t vy=PE_LoadU16(data+26u),vz=PE_LoadU16(data+28u);
    PE_StoreU16(data+8u,(uint16_t)(x+vx));
    uint16_t y=PE_LoadU16(data+10u),z=PE_LoadU16(data+12u);
    PE_StoreU16(data+10u,(uint16_t)(y+vy));
    uint16_t size=PE_LoadU16(data+4u);
    PE_StoreU16(data+12u,(uint16_t)(z+vz));
    int fade=(int16_t)PE_LoadU16(data+6u);
    PE_StoreU16(data+4u,(uint16_t)(size+30u));
    if(fade<129)PE_StoreU16(data+6u,(uint16_t)(fade+10));
    PE_StoreU8(data+2u,(uint8_t)(PE_LoadU8(data+2u)+1u));
    if(func_8001CAB0((int32_t)((uint32_t)PE_LoadU16(data+8u)<<16u),
                    (int32_t)((uint32_t)PE_LoadU16(data+12u)<<16u),
                    PE_LoadU32(0x8009D248u),PE_LoadU16(0x8009D1CCu)) &&
       (int8_t)PE_LoadU8(data+2u)%3==0) {
        pe_addr_t trail=func_800C2B90(slot,3u,0x8018FF70u,0x8018FF34u);
        if(trail) {
            PE_StoreU16(trail+8u,PE_LoadU16(data+8u));
            PE_StoreU16(trail+10u,PE_LoadU16(0x800942ECu));
            PE_StoreU16(trail+12u,PE_LoadU16(data+12u));
        }
    }
    if(!func_8001CAB0((int32_t)((uint32_t)PE_LoadU16(data+8u)<<16u),
                     (int32_t)((uint32_t)PE_LoadU16(data+12u)<<16u),
                     PE_LoadU32(0x8009D248u),PE_LoadU16(0x8009D1CCu))) {
        PE_StoreU8(rec+1u,2u);PE_StoreU8(data,0u);
    }
}

/* 78CC4 scales columns with wrapping low-word multiplication. Its last
 * store also replaces the matrix padding with the upper product halfword. */
static void boss_scale(PeEffectMatrix *matrix,const int32_t scale[3])
{
    for(unsigned i=0;i<9;i++) {
        int32_t value=(int32_t)((uint32_t)(int32_t)matrix->r[i]*(uint32_t)scale[i%3u])>>12;
        matrix->r[i]=(int16_t)value;
        if(i==8)matrix->pad=(int16_t)(value>>16);
    }
}

/* Full 63 words FDE4..FEE0, including the complete shared quad renderer. */
static void boss_trail_draw(pe_addr_t data)
{
    func_800C2EAC(0u);func_800C3098(16);func_800C2FF0(32u,32u);func_800C3238(2u);
    PeEffectMatrix matrix={{4096,0,0,0,4096,0,0,0,4096},0,{0,0,0}};
    PE_StoreU16(0x8019006Au,PE_LoadU16(data+4u));
    for(unsigned i=0;i<3;i++)matrix.t[i]=(int16_t)PE_LoadU16(data+8u+i*2u);
    int32_t scale[3];
    for(unsigned i=0;i<3;i++)scale[i]=(int32_t)PE_LoadU32(0x8018EFFCu+i*4u);
    boss_scale(&matrix,scale);
    /* FDE4 -> C42A4's C4544 writes only the low half of retained word3,
     * before camera transformation changes this local matrix translation. */
    if(m34_stack_drawing(0x8018FDE4u))m34_stack_low_half(3,(int16_t)matrix.t[2]);
    PE_EffectQuadC42A4(0x80190060u,&matrix,1u);
}

/* Full 265 words F830..FC54. Drawing also performs the original collision
 * and hit-flag writes; all five quads remain in original submission order. */
static void boss_projectile_draw(pe_addr_t slot,pe_addr_t rec,pe_addr_t data)
{
    const pe_addr_t style=0x80190050u;
    func_800C2EAC(0u);func_800C3098(16);func_800C2FF0(64u,32u);func_800C3238(2u);
    if((int8_t)PE_LoadU8(data)!=1)return;
    PE_StoreU8(style,16u);PE_StoreU8(style+1u,16u);PE_StoreU8(style+2u,32u);
    PE_StoreU16(style+10u,PE_LoadU16(data+6u));
    PeEffectMatrix matrix;PE_EffectReadMatrix(data+36u,&matrix);
    /* Retail F918/F930 subtract X velocity from Y and Z as well. */
    for(unsigned i=0;i<3;i++)matrix.t[i]=(int16_t)PE_LoadU16(data+8u+i*2u)-(int16_t)PE_LoadU16(data+24u);
    int32_t scale[3]={(int16_t)PE_LoadU16(data+4u)>>1,593,593};
    boss_scale(&matrix,scale);PE_EffectQuadC42A4(style,&matrix,0u);
    PE_StoreU8(style,32u);PE_StoreU8(style+1u,32u);PE_StoreU8(style+2u,64u);
    PE_EffectReadMatrix(data+36u,&matrix);
    for(unsigned i=0;i<3;i++)matrix.t[i]=(int16_t)PE_LoadU16(data+8u+i*2u);
    scale[0]=(int16_t)PE_LoadU16(data+4u);scale[1]=1096;scale[2]=1096;
    boss_scale(&matrix,scale);
    /* F830's second 78CC4 call replaces these complete words at
     * 78D0C/78D4C. Later non-billboard quads leave this matrix intact. */
    if(m34_stack_drawing(0x8018F830u)) {
        for(unsigned i=0;i<2;i++)m34_stack.words[4u+i]=(int32_t)((uint16_t)matrix.r[i*2u]|
            ((uint32_t)(uint16_t)matrix.r[i*2u+1u]<<16u));
        m34_stack.known|=48u;
    }
    PE_EffectQuadC42A4(style,&matrix,0u);
    int16_t point[3];pe_addr_t aya=PE_LoadU32(0x8009D254u);
    for(unsigned i=0;i<3;i++)point[i]=(int16_t)PE_LoadU16(aya+42u+i*4u);
    if(PE_EffectPointQuadC61A8(point,&matrix) && func_800C6CE0(slot)==3) {
        pe_addr_t body=PE_LoadU32(PE_LoadU32(0x8009D254u));
        PE_StoreU32(body+76u,PE_LoadU32(body+76u)|0x4000u);
        pe_addr_t owner=PE_LoadU32(slot+8u);
        if(owner) {
            body=PE_LoadU32(owner);PE_StoreU32(body,PE_LoadU32(body)|0x80000000u);
        }
        PE_StoreU8(rec+1u,2u);
    }
    PE_StoreU8(style,120u);PE_StoreU8(style+1u,240u);PE_StoreU8(style+2u,120u);
    matrix.t[1]=(int16_t)PE_LoadU16(data+10u)-256;
    PE_EffectQuadC42A4(style,&matrix,0u);
    for(unsigned age=1;age<=2;age++) {
        for(unsigned i=0;i<3;i++)matrix.t[i]=(int16_t)PE_LoadU16(data+8u+i*2u)-
            (int32_t)age*(int16_t)PE_LoadU16(data+24u+i*2u);
        PE_StoreU8(style,(uint8_t)(age==1?60:30));
        PE_StoreU8(style+1u,(uint8_t)(age==1?120:60));
        PE_StoreU8(style+2u,(uint8_t)(age==1?60:30));
        matrix.t[1]=(int16_t)PE_LoadU16(data+10u)-256;
        PE_EffectQuadC42A4(style,&matrix,0u);
    }
}
