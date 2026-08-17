/*
 * PE-BTL12 — func_80017018 task VM (translated retail, not matching
 * src/). Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 159 words 0x80017018..0x80017294, SHA-256 0b2a2f69…fcd8.
 * Sole TEXT jal 361F4 @ 0x80036224. One jalr of
 * D_800910A0[word & 0x1FFF].
 *
 *   task = D_8009D300
 *   if (task+8) & 0x50: walk +0x24
 *   actor = D_8009D2F0
 *   if actor+0x98 & 0x1000 and !(task+8 & 0x80): walk
 *   if D1A0 & 0x100 and actor != D254 and !(task+8 & 0x80): walk
 *   if task+0x10 == 0: walk
 *   task+0x10-- ; if still != 0: walk
 *   fetch word at task+0; argc=(word>>13)&0xF; op=word&0x1FFF
 *   kinds in word>>17 (3 bits each; word+4 after 5 args)
 *   kind 0: args[i] = &imm[i]
 *   kind 1: args[i] = actor + 0xAC + imm[i]*4
 *   kind 2: args[i] = D_800A77F0 + imm[i]*4
 *   kind 3: args[i] = D_8009DF70 + imm[i]*4
 *   kind 4: args[i] = D_800B6A80 + imm[i]*4
 *   jalr table[op](args); v0!=0 re-fetch; v0==0 store PC, walk +0x24
 *
 * v0!=0 re-fetches from gp+0x90 (already advanced, or rewritten
 * by op 0). v0==0 stores that PC to task+0 and walks +0x24.
 *
 * Handlers ported here: 0 / 1 / 2 / 0x20 / 0xCE / 0xEA-nop /
 * 0xA / 0x1D / 0x09 / 0x05 / 0x14 / 0x40 / 0x3F / 0xED-2900 /
 * 0xE1 / 0x84 / 0x88 / 0x08 / 0x0B / 0x41 / 0x2E / 0x4E /
 * 0x2F / 0x30 / 0x5E / 0x77 / 0x9B / 0x0C / 0xD9 / 0x24 / 0x11 / 0x86 / 0x04 / 0xAA / 0x65 / 0x82 / 0x9C / 0xAB / 0x1E / 0x79 / 0x85 / 0xDC / 0x1A / 0x6F / 0x5A / 0xB7 / 0x70 / 0x59 / 0x12 / 0x6A / 0x4B / 0x54 / 0x6B / 0x64 / 0x0E / 0x0D / 0x22 / 0x43 / 0x52 / 0x53 / 0xA6 / 0x2A / 0x87 / 0x31 / 0x94 / 0xC7 / 0xAD / 0x8B / 0x89 / 0x03 / 0xB8. 0x1C and 0x1F are the already-ported
 * mailbox leaves. Other table slots are not this cut (return 0
 * = advance). Not M2.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009CE00 0x8009CE00u
#define GA_D_800910A0 0x800910A0u
#define GA_D_800A77F0 0x800A77F0u
#define GA_D_8009DF70 0x8009DF70u
#define GA_D_800B6A80 0x800B6A80u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_8009D28C 0x8009D28Cu
#define GA_OP0        0x80017294u
#define GA_OP1        0x800172BCu
#define GA_OP2        0x800172E0u
#define GA_OP20       0x800172FCu
#define GA_OP1C       0x80017764u
#define GA_OP1F       0x800177ACu
#define GA_OPCE       0x800181CCu
#define GA_OPEA       0x80015DACu
#define GA_OPA        0x800173F4u
#define GA_OP1D       0x80017E20u
#define GA_OP9        0x80012850u
#define GA_OP5        0x8001731Cu
#define GA_OP14       0x80017588u
#define GA_OP40       0x80017D7Cu
#define GA_OP3F       0x80017D5Cu
#define GA_OPED       0x80016910u
#define GA_OPE1       0x8001A374u
#define GA_OP84       0x80018E84u
#define GA_OP88       0x80018F54u
#define GA_OP8        0x8001735Cu
#define GA_OPB        0x80012C20u
#define GA_OP41       0x80017D9Cu
#define GA_OP2E       0x80017AE8u
#define GA_OP4E       0x80017EC4u
#define GA_OP2F       0x80017B34u
#define GA_OP30       0x80017B74u
#define GA_OP5E       0x80014694u
#define GA_OP77       0x80014DA0u
#define GA_OP9B       0x80015240u
#define GA_OPC        0x80012E7Cu
#define GA_OPD9       0x8001A15Cu
#define GA_OP24       0x8001784Cu
#define GA_OP11       0x800130B4u
#define GA_OP86       0x80018EE0u
#define GA_OP4        0x80017988u
#define GA_OPAA       0x80019618u
#define GA_OP65       0x8001856Cu
#define GA_OP82       0x80018E58u
#define GA_OP9C       0x80019410u
#define GA_OPAB       0x80019638u
#define GA_OP1E       0x80019658u
#define GA_OP79       0x80018BECu
#define GA_OP85       0x80018EB4u
#define GA_OPDC       0x8001A1F0u
#define GA_OP1A       0x800176FCu
#define GA_OP6F       0x80018954u
#define GA_OP5A       0x80018164u
#define GA_OPB7       0x80018A48u
#define GA_OP70       0x8001897Cu
#define GA_OP59       0x80018004u
#define GA_OP12       0x800131E8u
#define GA_OP6A       0x80018774u
#define GA_OP4B       0x80013C34u
#define GA_OP54       0x800143B0u
#define GA_OP6B       0x800187C0u
#define GA_OP64       0x800184ECu
#define GA_OP0E       0x80014228u
#define GA_OP0D       0x80017410u
#define GA_OP22       0x800177C8u
#define GA_OP43       0x80017DE4u
#define GA_OP52       0x80017F88u
#define GA_OP53       0x80017FB0u
#define GA_OPA6       0x80019484u
#define GA_OP2A       0x80017A50u
#define GA_OP87       0x80018F0Cu
#define GA_OP31       0x80017BB4u
#define GA_OP94       0x80019154u
#define GA_OPC7       0x80019BE4u
#define GA_OPAD       0x80019748u
#define GA_OP8B       0x80018080u
#define GA_OP89       0x80017FF0u
#define GA_OP3        0x80017C54u
#define GA_OPB8       0x80013514u
/* Host stand-in for ROM sp+16. APPROXIMATION: native has no guest $sp. */
#define GA_VM_FRAME   0x80120F80u
/* Host stand-in for 17410's stack s16 = -1. APPROXIMATION. */
#define GA_375E0_LIST 0x80120F20u

extern unsigned int D_8009D1A0;

int func_80017294(pe_addr_t args)
{
    uint32_t imm;
    pe_addr_t actor;
    pe_addr_t base;

    imm = PE_LoadU32(PE_LoadU32(args));
    actor = PE_LoadU32(GA_D_8009D2F0);
    base = PE_LoadU32(actor + 0x9Cu);
    PE_StoreU32(GA_D_8009CE00, base + (imm << 1));
    return 1;
}

int func_800172BC(pe_addr_t args)
{
    pe_addr_t actor;

    (void)args;
    actor = PE_LoadU32(GA_D_8009D2F0);
    PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) | 0x10u);
    return 0;
}

int func_800172E0(pe_addr_t args)
{
    pe_addr_t task;

    task = PE_LoadU32(GA_D_8009D300);
    PE_StoreU32(task + 0x10u, PE_LoadU16(PE_LoadU32(args)));
    return 0;
}

int func_800172FC(pe_addr_t args)
{
    pe_addr_t task;

    (void)args;
    task = PE_LoadU32(GA_D_8009D300);
    PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) | 0x10u));
    return 0;
}

/*
 * PE-BTL29 — opcode 0x24 task-word copy 1784C.
 *
 * 12 words 0x8001784C..0x8001787C, SHA-256 0a67c25d…17e9.
 * D_800910A0[0x24]. Zero jal. Always v0=1.
 * gp+0x590 is D_8009D300. *arg0 = task+0x18; *arg1 = task+0x1C.
 * Live type-5 +0x2C4: local[2], local[3]. 12700 does not write
 * those words; 124F8 zeros the pool.
 */
int func_8001784C(pe_addr_t args)
{
    pe_addr_t task;

    task = PE_LoadU32(GA_D_8009D300);
    PE_StoreU32(PE_LoadU32(args), PE_LoadU32(task + 0x18u));
    PE_StoreU32(PE_LoadU32(args + 4u), PE_LoadU32(task + 0x1Cu));
    return 1;
}

/*
 * PE-BTL32 — opcode 0x04 task-flag walk 17988.
 *
 * 28 words 0x80017988..0x800179F8, SHA-256 2ddb182b…5c0a.
 * D_800910A0[0x04]. Zero jal. Always v0=1.
 * For i in 0..2: walk D2F0+0xA0[i] via +0x24; if task!=D300
 * then task+8 |= 0x10. Live type-0 after 65400 deliver of
 * type-1 0x1C payload 0xFF.
 */
int func_80017988(pe_addr_t args)
{
    unsigned int i;
    pe_addr_t actor;
    pe_addr_t cur;
    pe_addr_t skip;

    (void)args;
    actor = PE_LoadU32(GA_D_8009D2F0);
    skip = PE_LoadU32(GA_D_8009D300);
    for (i = 0u; i < 3u; i++) {
        cur = PE_LoadU32(actor + 0xA0u + i * 4u);
        while (cur != 0u) {
            if (cur != skip)
                PE_StoreU16(cur + 8u,
                            (uint16_t)(PE_LoadU16(cur + 8u) | 0x10u));
            cur = PE_LoadU32(cur + 0x24u);
        }
    }
    return 1;
}

/*
 * PE-BTL34 — opcode 0x65 pose-group clear 1856C.
 *
 * 11 words 0x8001856C..0x80018598, SHA-256 309d956f…c25f.
 * D_800910A0[0x65]. Zero jal. Always v0=1. argc 0.
 * Zeros D2F0 +0x68/+0x6C/+0x70 and +0x78/+0x7C/+0x80
 * (12C20 codes 2 and 3). Live type-0 after 0xAA/0x40.
 */
int func_8001856C(pe_addr_t args)
{
    pe_addr_t actor;

    (void)args;
    actor = PE_LoadU32(GA_D_8009D2F0);
    PE_StoreU32(actor + 0x68u, 0u);
    PE_StoreU32(actor + 0x6Cu, 0u);
    PE_StoreU32(actor + 0x70u, 0u);
    PE_StoreU32(actor + 0x78u, 0u);
    PE_StoreU32(actor + 0x7Cu, 0u);
    PE_StoreU32(actor + 0x80u, 0u);
    return 1;
}

/*
 * PE-BTL35 — opcode 0x82 view-apply wrapper 18E58.
 *
 * 11 words 0x80018E58..0x80018E84, SHA-256 99b34ecf…def3.
 * D_800910A0[0x82]. jal 66800(*arg0); v0=1.
 * Live type-0 after 0x65/0x2E: imm 1.
 */
int func_80018E58(pe_addr_t args)
{
    func_80066800(PE_LoadU32(PE_LoadU32(args)));
    return 1;
}

/*
 * PE-BTL36 — opcode 0x9C fade-wait 19410.
 *
 * 16 words 0x80019410..0x80019450, SHA-256 ab0d665b…1ce8.
 * D_800910A0[0x9C]. Zero jal.
 * If (CFEE&3) < 2: v0=1 continue.
 * Else: CE00 -= 8, D300+0x10 = 1, v0=0 yield.
 * Live type-0 after 0x86 (CFEE=6) waits.
 */
int func_80019410(pe_addr_t args)
{
    (void)args;
    if ((PE_LoadU8(0x800BCFEEu) & 3u) < 2u)
        return 1;
    PE_StoreU32(GA_D_8009CE00, PE_LoadU32(GA_D_8009CE00) - 8u);
    PE_StoreU32(PE_LoadU32(GA_D_8009D300) + 0x10u, 1u);
    return 0;
}

/*
 * PE-BTL37 — opcode 0xAB overlay-bit clear 19638.
 *
 * 8 words 0x80019638..0x80019658, SHA-256 03ea9775…7afc.
 * D_800910A0[0xAB]. Zero jal. D_800B0CD8 &= ~0x2000; v0=1.
 * Inverse of 0xAA. Live type-0 after 0x9C fade-wait.
 */
int func_80019638(pe_addr_t args)
{
    (void)args;
    PE_StoreU32(0x800B0CD8u, PE_LoadU32(0x800B0CD8u) & ~0x2000u);
    return 1;
}

/*
 * PE-BTL39 — opcode 0x1E actor-flag 19658.
 *
 * 9 words 0x80019658..0x8001967C, SHA-256 cadffdfc…6fcd.
 * D_800910A0[0x1E]. Zero jal. D2F0+0x98 |= 0x80; v0=1.
 * Live type-2 after mailbox 0xB from type-0 0x1C.
 */
int func_80019658(pe_addr_t args)
{
    pe_addr_t actor;

    (void)args;
    actor = PE_LoadU32(GA_D_8009D2F0);
    PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) | 0x80u);
    return 1;
}

/*
 * PE-BTL40 — opcode 0x79 actor-flag 18BEC.
 *
 * 9 words 0x80018BEC..0x80018C10, SHA-256 d373eee2…000c.
 * D_800910A0[0x79]. Zero jal. D2F0+0x98 |= 0x20; v0=1.
 * Live type-2 after 0x1E.
 */
int func_80018BEC(pe_addr_t args)
{
    pe_addr_t actor;

    (void)args;
    actor = PE_LoadU32(GA_D_8009D2F0);
    PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) | 0x20u);
    return 1;
}

/*
 * PE-BTL41 — opcode 0x85 fade-in 18EB4.
 *
 * 11 words 0x80018EB4..0x80018EE0, SHA-256 1a18600c…9d43.
 * D_800910A0[0x85]. jal 66B60(lhu *arg0); v0=1.
 * Type-3 HIT imm 0x1E. Do not invent a region hit.
 * Boot 6E9A0 already jals 66B60(2).
 */
int func_80018EB4(pe_addr_t args)
{
    func_80066B60(PE_LoadU16(PE_LoadU32(args)));
    return 1;
}

/*
 * PE-BTL42 — opcode 0xDC actor-flag 1A1F0.
 *
 * 9 words 0x8001A1F0..0x8001A214, SHA-256 65cffdf8…d872.
 * D_800910A0[0xDC]. Zero jal. D2F0+0x98 |= 0x01000000; v0=1.
 * Live type-2 after mailbox 0xFB → 0x0B pose pair.
 */
int func_8001A1F0(pe_addr_t args)
{
    pe_addr_t actor;

    (void)args;
    actor = PE_LoadU32(GA_D_8009D2F0);
    PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) | 0x01000000u);
    return 1;
}

/*
 * PE-BTL43 — opcode 0x1A RNG write 176FC.
 *
 * 26 words 0x800176FC..0x80017764, SHA-256 ae4816ac…2ab2.
 * D_800910A0[0x1A]. jal 70D6C when *arg1==*arg2, else
 * jal 70DD0(*arg1,*arg2). *arg0 = v0; v0=1.
 * Live type-2 after 0xDC: dest local[0x18], 70DD0(0,0x64).
 */
int func_800176FC(pe_addr_t args)
{
    int32_t lo;
    int32_t hi;
    uint32_t result;

    lo = (int32_t)PE_LoadU32(PE_LoadU32(args + 4u));
    hi = (int32_t)PE_LoadU32(PE_LoadU32(args + 8u));
    if (lo == hi)
        result = func_80070D6C();
    else
        result = (uint32_t)func_80070DD0(lo, hi);
    PE_StoreU32(PE_LoadU32(args), result);
    return 1;
}

/*
 * PE-BTL44 — opcode 0x6F slot-alloc wrapper 18954.
 *
 * 10 words 0x80018954..0x8001897C, SHA-256 9dc14475…a5ff.
 * D_800910A0[0x6F]. jal 2F7D8(*D2F0); v0=1.
 * Live type-2 after 0x1A. 2F7D8 is already ported (CH1).
 */
int func_80018954(pe_addr_t args)
{
    (void)args;
    func_8002F7D8(PE_LoadU32(GA_D_8009D2F0));
    return 1;
}

/*
 * PE-BTL44 — opcode 0x5A tagged-set wrapper 18164.
 *
 * 26 words 0x80018164..0x800181CC, SHA-256 b2d7e11c…9c8f.
 * D_800910A0[0x5A]. If D2F0+0x0C==0: 2FF78(lbu *arg0, *arg1).
 * Else 30220(D2F0, lbu *arg0, *arg1). v0=1.
 * Live type-2 is type!=0 so 30220. Callees already ported (CH1).
 */
int func_80018164(pe_addr_t args)
{
    pe_addr_t actor;
    uint8_t tag;
    uint32_t value;

    actor = PE_LoadU32(GA_D_8009D2F0);
    tag = PE_LoadU8(PE_LoadU32(args));
    value = PE_LoadU32(PE_LoadU32(args + 4u));
    if (PE_LoadU8(actor + 0x0Cu) == 0u)
        func_8002FF78(tag, value);
    else
        func_80030220(actor, tag, value);
    return 1;
}

/*
 * PE-BTL45 — opcode 0xB7 formation wrapper 18A48.
 *
 * 21 words 0x80018A48..0x80018A9C, SHA-256 9218163e…d862.
 * D_800910A0[0xB7]. jal 2FAA4(D2F0, lbu*0, lbu*1, lbu*2,
 * lbu*3, lhu*4); v0=1. Live (3,0,6,7,cond[1]).
 */
int func_80018A48(pe_addr_t args)
{
    func_8002FAA4(PE_LoadU32(GA_D_8009D2F0),
                  PE_LoadU8(PE_LoadU32(args)),
                  PE_LoadU8(PE_LoadU32(args + 4u)),
                  PE_LoadU8(PE_LoadU32(args + 8u)),
                  PE_LoadU8(PE_LoadU32(args + 12u)),
                  PE_LoadU16(PE_LoadU32(args + 16u)));
    return 1;
}

/*
 * PE-BTL45 — opcode 0x70 formation wrapper 1897C.
 *
 * 51 words 0x8001897C..0x80018A48, SHA-256 64cd42b0…0ff3.
 * D_800910A0[0x70]. jal 2FA10(D2F0, lbu*0..2, lbu*3, lhu*4,
 * lb*5..8, lbu*9, lbu*10); v0=1.
 * Live (0,0,8,9,cond[1],5,-1,-1,-1,3,15).
 */
int func_8001897C(pe_addr_t args)
{
    func_8002FA10(PE_LoadU32(GA_D_8009D2F0),
                  PE_LoadU8(PE_LoadU32(args)),
                  PE_LoadU8(PE_LoadU32(args + 4u)),
                  PE_LoadU8(PE_LoadU32(args + 8u)),
                  PE_LoadU8(PE_LoadU32(args + 12u)),
                  PE_LoadU16(PE_LoadU32(args + 16u)),
                  (int)(int8_t)PE_LoadU8(PE_LoadU32(args + 20u)),
                  (int)(int8_t)PE_LoadU8(PE_LoadU32(args + 24u)),
                  (int)(int8_t)PE_LoadU8(PE_LoadU32(args + 28u)),
                  (int)(int8_t)PE_LoadU8(PE_LoadU32(args + 32u)),
                  PE_LoadU8(PE_LoadU32(args + 36u)),
                  PE_LoadU8(PE_LoadU32(args + 40u)));
    return 1;
}

/*
 * PE-BTL46 — opcode 0x59 tagged-read wrapper 18004.
 *
 * 31 words 0x80018004..0x80018080, SHA-256 2993bcdd…f4ac.
 * D_800910A0[0x59]. If D2F0+0x0C==0: 2FE78(lbu *arg0).
 * Else 3010C(D2F0, lbu *arg0). *arg1 = v0; v0=1.
 * Live type-2 tag 44 → local[0xC] via 3010C.
 */
int func_80018004(pe_addr_t args)
{
    pe_addr_t actor;
    uint8_t tag;
    int value;

    actor = PE_LoadU32(GA_D_8009D2F0);
    tag = PE_LoadU8(PE_LoadU32(args));
    if (PE_LoadU8(actor + 0x0Cu) == 0u)
        value = func_8002FE78(tag);
    else
        value = func_8003010C(actor, tag);
    PE_StoreU32(PE_LoadU32(args + 4u), (uint32_t)value);
    return 1;
}

/*
 * PE-BTL48 — opcode 0x6A event-start wrapper 18774.
 *
 * 19 words 0x80018774..0x800187C0, SHA-256 e32a2874…d822.
 * D_800910A0[0x6A]. jal 6F39C(*arg0, D2F0); *arg1=v0; v0=1.
 * Live (0x75, type-2 actor) → local[7].
 */
int func_80018774(pe_addr_t args)
{
    int value;

    value = func_8006F39C(PE_LoadU32(PE_LoadU32(args)),
                          PE_LoadU32(GA_D_8009D2F0));
    PE_StoreU32(PE_LoadU32(args + 4u), (uint32_t)value);
    return 1;
}

/*
 * PE-BTL50 — opcode 0x6B event-tick wrapper 187C0.
 *
 * 22 words 0x800187C0..0x80018818, SHA-256 1e733223…da40.
 * D_800910A0[0x6B]. jal 6F6D4(*arg0, 0, *arg1, *arg2,
 * *arg3, *arg4); v0=1. Live (local[7], 1, 0, 0, 0).
 */
int func_800187C0(pe_addr_t args)
{
    (void)func_8006F6D4(PE_LoadU32(PE_LoadU32(args)),
                        0u,
                        PE_LoadU32(PE_LoadU32(args + 4u)),
                        PE_LoadU32(args + 8u),
                        PE_LoadU32(args + 12u),
                        PE_LoadU32(args + 16u));
    return 1;
}

/*
 * PE-BTL51 — opcode 0x64 clip-wait wrapper 184EC.
 *
 * 32 words 0x800184EC..0x8001856C, SHA-256 9b345fcb…631d.
 * D_800910A0[0x64]. jal 2FAF8(D2F0, lbu *arg0);
 * *arg1 = sign-extend byte of v0. v0==0 rewinds CE00 by
 * 0x10 and sets D300+0x10=1. Live (3, local[6]).
 */
int func_800184EC(pe_addr_t args)
{
    int value;

    value = (int)(int8_t)(uint8_t)func_8002FAF8(
        PE_LoadU32(GA_D_8009D2F0),
        PE_LoadU8(PE_LoadU32(args)));
    PE_StoreU32(PE_LoadU32(args + 4u), (uint32_t)value);
    if (value == 0) {
        PE_StoreU32(GA_D_8009CE00, PE_LoadU32(GA_D_8009CE00) - 0x10u);
        PE_StoreU32(PE_LoadU32(GA_D_8009D300) + 0x10u, 1u);
        return 0;
    }
    return 1;
}

/*
 * PE-BTL54 — opcode 0x0D message-open 17410.
 *
 * 13 words 0x80017410..0x80017444, SHA-256
 * f84d1f42908c9ba5b90a985acaaa36a54538d99d96ad731305bd82571f45518e.
 * D_800910A0[0x0D]. jal 375E0(lh *arg0, 0, &-1); v0=1.
 * Live type-0 FF arm +0x920 id 7 (persist[0]&4==0).
 */
int func_80017410(pe_addr_t args)
{
    PE_StoreU16(GA_375E0_LIST, 0xFFFFu);
    func_800375E0((int)(int16_t)PE_LoadU16(PE_LoadU32(args)), 0u,
                  GA_375E0_LIST);
    return 1;
}

/*
 * PE-BTL54 — opcode 0x22 message-poll 177C8.
 *
 * 22 words 0x800177C8..0x80017820, SHA-256 from oracle.
 * D_800910A0[0x22]. jal 37548(lh *arg0). byte0==0 → v0=1.
 * Else CE00-=0xC, D300+0x10=1, v0=0. Live after 0x0D id 7.
 */
int func_800177C8(pe_addr_t args)
{
    if (func_80037548((int)(int16_t)PE_LoadU16(PE_LoadU32(args))) == 0)
        return 1;
    PE_StoreU32(GA_D_8009CE00, PE_LoadU32(GA_D_8009CE00) - 0xCu);
    PE_StoreU32(PE_LoadU32(GA_D_8009D300) + 0x10u, 1u);
    return 0;
}

/*
 * PE-BTL57 — opcode 0x43 choice-read 17DE4.
 *
 * 15 words 0x80017DE4..0x80017E20, SHA-256
 * 71be0cb334c6b16f99d7e4e13fd8b3411fd488653645cdec22f87e14c76ac405.
 * D_800910A0[0x43]. jal 37864; *arg0 = signed byte. v0=1.
 * 37864 is lb 0x134($gp) = D_8009CEA4. Live type-0 local[5]
 * after 0x22. 375E0 writes -1; FB 09 copies cursor on 0x100.
 */
int func_80037864(void)
{
    return (int)(int8_t)PE_LoadU8(0x8009CEA4u);
}

int func_80017DE4(pe_addr_t args)
{
    PE_StoreU32(PE_LoadU32(args), (uint32_t)func_80037864());
    return 1;
}

/*
 * PE-BTL58 — opcode 0x52 D1A0-or 17F88.
 *
 * 10 words 0x80017F88..0x80017FB0, SHA-256
 * ed1c623197818f92f87ae1fc3b82baa3d14f84a7bb66e9472a1876bc1348bb2b.
 * D_800910A0[0x52]. D_8009D1A0 |= *arg0; v0=1.
 * Live type-0 Watch +0xC78 imm 0x800. Host scalar, not guest RAM.
 */
int func_80017F88(pe_addr_t args)
{
    D_8009D1A0 |= PE_LoadU32(PE_LoadU32(args));
    return 1;
}

/*
 * PE-BTL58 — opcode 0x53 D1A0-and-not 17FB0.
 *
 * 11 words 0x80017FB0..0x80017FDC, SHA-256
 * 3594551d87ad74ca023d6f16c06a295719ee38c4a07dc9edaa698b6e315f259f.
 * D_800910A0[0x53]. D_8009D1A0 &= ~*arg0; v0=1.
 * Live type-0 +0xCF0 imm 0x800.
 */
int func_80017FB0(pe_addr_t args)
{
    D_8009D1A0 &= ~PE_LoadU32(PE_LoadU32(args));
    return 1;
}

/*
 * PE-BTL58 — opcode 0xA6 19484.
 *
 * 11 words 0x80019484..0x800194B0, SHA-256
 * 731901885ba4878c7e07bcaf57d1106489875f5a4dcafb7b08a690dd78b64a3f.
 * D_800910A0[0xA6]. jal 438C0(*arg0); v0=1.
 * Live type-0 persist[8]==0 arm imm 0x7F.
 */
int func_80019484(pe_addr_t args)
{
    func_800438C0((int)PE_LoadU32(PE_LoadU32(args)));
    return 1;
}

/*
 * PE-BTL59 — opcode 0x2A bit-or 17A50.
 *
 * 10 words 0x80017A50..0x80017A78, SHA-256
 * 2f58ffe89e4130506d67a6784ea268503a8d0a6dec26bbe01506e2cec926db68.
 * D_800910A0[0x2A]. *arg0 |= (1 << *arg1); v0=1.
 * Live type-0 +0x1150 kinds [4,0] imms [0,4] → scratch[0] |= 0x10.
 * That is bit 4, not the type-6 scratch[0]&4 wait.
 */
int func_80017A50(pe_addr_t args)
{
    pe_addr_t dest;
    uint32_t bit;

    dest = PE_LoadU32(args);
    bit = 1u << (PE_LoadU32(PE_LoadU32(args + 4u)) & 31u);
    PE_StoreU32(dest, PE_LoadU32(dest) | bit);
    return 1;
}

/*
 * PE-BTL60 — opcode 0x87 RGB-fade 18F0C.
 *
 * 18 words 0x80018F0C..0x80018F54, SHA-256
 * cb2c045fd48813b66f79f29a11c45d26c3394c02ac40601fb63740f02baed649.
 * D_800910A0[0x87]. jal 66BD8(lhu *arg0..*arg4); v0=1.
 * Live type-0 +0x1020 imms 0x3C / 0xFF / 0xFF / 0xFF / 1.
 */
int func_80018F0C(pe_addr_t args)
{
    func_80066BD8(PE_LoadU16(PE_LoadU32(args)),
                  PE_LoadU16(PE_LoadU32(args + 4u)),
                  PE_LoadU16(PE_LoadU32(args + 8u)),
                  PE_LoadU16(PE_LoadU32(args + 0xCu)),
                  PE_LoadU16(PE_LoadU32(args + 0x10u)));
    return 1;
}

/*
 * PE-BTL61 — opcode 0xC7 task-flag 19BE4.
 *
 * 8 words 0x80019BE4..0x80019C04, SHA-256
 * ad1774026537b3de2fff5adfa0b0815f0460a0d1b5de64e3bf860c7519fa043a.
 * `lw $v1, 0x590($gp)` is D300. D300+8 |= 0x80; v0=1.
 * Live type-6 after 0x12 when scratch[0]&4 is clear (the wait
 * is while the bit is set).
 */
int func_80019BE4(pe_addr_t args)
{
    pe_addr_t task;

    (void)args;
    task = PE_LoadU32(GA_D_8009D300);
    PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) | 0x80u));
    return 1;
}

/*
 * PE-BTL61 — opcode 0xAD D2E8-and-not-4 19748.
 *
 * 8 words 0x80019748..0x80019768, SHA-256
 * 6776e7ebd6583acc0c697c7b4f19ddcffcbbd726fe217f2a582e216d611e4327.
 * D_8009D2E8 &= ~4; v0=1.
 */
int func_80019748(pe_addr_t args)
{
    (void)args;
    PE_StoreU32(GA_D_8009D2E8, PE_LoadU32(GA_D_8009D2E8) & ~4u);
    return 1;
}

/*
 * PE-BTL61 — opcode 0x8B typed tagged-read 18080.
 *
 * 57 words 0x80018080..0x80018164, SHA-256
 * a896f3b9839e239002929b4239ef244ab7ecc2836c606f2e28ec7712a57bd1c7.
 * *arg0==0: 2FE78(lbu *arg2). Else walk D20C for type==*arg0 and
 * idB==*arg1 with +(0x98)&0x10 clear, then 3010C(actor, lbu *arg2).
 * *arg3 = v0 only on the type-0 or first matching walk hit;
 * exhausted D20C / bit-0x10-only matches skip the store.
 * Handler v0=1. Live type-6 [2,0,0x2C,local].
 */
int func_80018080(pe_addr_t args)
{
    uint32_t type;
    pe_addr_t actor;
    uint8_t tag;
    int value;

    type = PE_LoadU32(PE_LoadU32(args));
    tag = PE_LoadU8(PE_LoadU32(args + 8u));
    if (type == 0u) {
        value = func_8002FE78(tag);
        PE_StoreU32(PE_LoadU32(args + 0xCu), (uint32_t)value);
        return 1;
    }
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        if (PE_LoadU8(actor + 0x0Cu) == (uint8_t)type &&
            PE_LoadU8(actor + 0x0Du) ==
                (uint8_t)PE_LoadU32(PE_LoadU32(args + 4u)) &&
            (PE_LoadU32(actor + 0x98u) & 0x10u) == 0u) {
            value = func_8003010C(actor, tag);
            PE_StoreU32(PE_LoadU32(args + 0xCu), (uint32_t)value);
            return 1;
        }
        actor = PE_LoadU32(actor + 4u);
    }
    return 1;
}

/*
 * PE-BTL62 — opcode 0x89 mode-6 store 17FF0.
 *
 * 5 words 0x80017FF0..0x80018004, SHA-256
 * 117504bf5c4fa6434b3703c4e26d350c077de797af2ab6dbbb56366c64e9115c.
 * D_800910A0[0x89]. Zero jal. addiu v0,6 / lui+sw D28C / jr /
 * addiu v0,1. Matching src/func_80017FF0.c writes the host
 * symbol; this cut writes guest RAM so 0x94/299CC/2CF24 agree.
 * Live type-6 +0x394 after persist[0xA]&1 clear and mode==0.
 * argc 0. Always v0=1.
 */
int func_80017FF0(pe_addr_t args)
{
    (void)args;
    PE_StoreU32(GA_D_8009D28C, 6u);
    return 1;
}

/*
 * PE-BTL64 — opcode 0x03 camera-request 17C54.
 *
 * 14 words 0x80017C54..0x80017C8C, SHA-256
 * 4d53aeb570e4a1a69243d19089312f97ae255ebe4f0e9d5c15023ab01d9afa24.
 * jal 661EC(lh *arg0, lh *arg1, lhu *arg2, 0); v0=1.
 * Live M0367I type-1 +0x1C8: (0x12B, 0x7C, 1).
 */
int func_80017C54(pe_addr_t args)
{
    int16_t x;
    int16_t y;
    uint16_t z;

    x = (int16_t)PE_LoadU16(PE_LoadU32(args));
    y = (int16_t)PE_LoadU16(PE_LoadU32(args + 4u));
    z = PE_LoadU16(PE_LoadU32(args + 8u));
    func_800661EC((int)x, (int)y, (unsigned int)z, 0u);
    return 1;
}

static int pe_17018_dispatch(pe_addr_t fn, pe_addr_t args)
{
    if (fn == GA_OP0)
        return func_80017294(args);
    if (fn == GA_OP1)
        return func_800172BC(args);
    if (fn == GA_OP2)
        return func_800172E0(args);
    if (fn == GA_OP20)
        return func_800172FC(args);
    if (fn == GA_OP1C)
        return func_80017764(args);
    if (fn == GA_OP1F)
        return func_800177AC(args);
    if (fn == GA_OPCE)
        return func_800181CC(args);
    if (fn == GA_OPEA)
        return func_80015DAC_default_cut(args);
    if (fn == GA_OPA)
        return func_800173F4(args);
    if (fn == GA_OP1D)
        return func_80017E20(args);
    if (fn == GA_OP9)
        return func_80012850(args);
    if (fn == GA_OP5)
        return func_8001731C(args);
    if (fn == GA_OP14)
        return func_80017588(args);
    if (fn == GA_OP40)
        return func_80017D7C(args);
    if (fn == GA_OP3F)
        return func_80017D5C(args);
    if (fn == GA_OPED)
        return func_80016910_key2900_cut(args);
    if (fn == GA_OPE1)
        return func_8001A374(args);
    if (fn == GA_OP84)
        return func_80018E84(args);
    if (fn == GA_OP88)
        return func_80018F54(args);
    if (fn == GA_OP8)
        return func_8001735C(args);
    if (fn == GA_OPB)
        return func_80012C20(args);
    if (fn == GA_OP41)
        return func_80017D9C(args);
    if (fn == GA_OP2E)
        return func_80017AE8(args);
    if (fn == GA_OP4E)
        return func_80017EC4(args);
    if (fn == GA_OP2F)
        return func_80017B34(args);
    if (fn == GA_OP30)
        return func_80017B74(args);
    if (fn == GA_OP5E)
        return func_80014694(args);
    if (fn == GA_OP77)
        return func_80014DA0(args);
    if (fn == GA_OP9B)
        return func_80015240(args);
    if (fn == GA_OPC)
        return func_80012E7C(args);
    if (fn == GA_OPD9)
        return func_8001A15C(args);
    if (fn == GA_OP24)
        return func_8001784C(args);
    if (fn == GA_OP11)
        return func_800130B4(args);
    if (fn == GA_OP86)
        return func_80018EE0(args);
    if (fn == GA_OP4)
        return func_80017988(args);
    if (fn == GA_OPAA)
        return func_80019618(args);
    if (fn == GA_OP65)
        return func_8001856C(args);
    if (fn == GA_OP82)
        return func_80018E58(args);
    if (fn == GA_OP9C)
        return func_80019410(args);
    if (fn == GA_OPAB)
        return func_80019638(args);
    if (fn == GA_OP1E)
        return func_80019658(args);
    if (fn == GA_OP79)
        return func_80018BEC(args);
    if (fn == GA_OP85)
        return func_80018EB4(args);
    if (fn == GA_OPDC)
        return func_8001A1F0(args);
    if (fn == GA_OP1A)
        return func_800176FC(args);
    if (fn == GA_OP6F)
        return func_80018954(args);
    if (fn == GA_OP5A)
        return func_80018164(args);
    if (fn == GA_OPB7)
        return func_80018A48(args);
    if (fn == GA_OP70)
        return func_8001897C(args);
    if (fn == GA_OP59)
        return func_80018004(args);
    if (fn == GA_OP12)
        return func_800131E8(args);
    if (fn == GA_OP6A)
        return func_80018774(args);
    if (fn == GA_OP4B)
        return func_80013C34(args);
    if (fn == GA_OP54)
        return func_800143B0(args);
    if (fn == GA_OP6B)
        return func_800187C0(args);
    if (fn == GA_OP64)
        return func_800184EC(args);
    if (fn == GA_OP0E)
        return func_80014228(args);
    if (fn == GA_OP0D)
        return func_80017410(args);
    if (fn == GA_OP22)
        return func_800177C8(args);
    if (fn == GA_OP43)
        return func_80017DE4(args);
    if (fn == GA_OP52)
        return func_80017F88(args);
    if (fn == GA_OP53)
        return func_80017FB0(args);
    if (fn == GA_OPA6)
        return func_80019484(args);
    if (fn == GA_OP2A)
        return func_80017A50(args);
    if (fn == GA_OP87)
        return func_80018F0C(args);
    if (fn == GA_OP31)
        return func_80017BB4_btl1_cut(args);
    if (fn == GA_OP94)
        return func_80019154(args);
    if (fn == GA_OPC7)
        return func_80019BE4(args);
    if (fn == GA_OPAD)
        return func_80019748(args);
    if (fn == GA_OP8B)
        return func_80018080(args);
    if (fn == GA_OP89)
        return func_80017FF0(args);
    if (fn == GA_OP3)
        return func_80017C54(args);
    if (fn == GA_OPB8)
        return func_80013514(args);
    /* Unported / empty table slot: not this cut. Advance. */
    return 0;
}

static void pe_17018_walk_next(void)
{
    pe_addr_t next;

    next = PE_LoadU32(PE_LoadU32(GA_D_8009D300) + 0x24u);
    PE_StoreU32(GA_D_8009D300, next);
}

void func_80017018(void)
{
    pe_addr_t task;
    pe_addr_t actor;
    pe_addr_t pc;
    pe_addr_t imm_base;
    uint32_t word;
    uint32_t word2;
    uint32_t kinds;
    uint32_t argc;
    uint32_t op;
    uint32_t delay;
    unsigned int i;
    unsigned int kind;
    int again;
    int first_fetch;

    for (;;) {
        task = PE_LoadU32(GA_D_8009D300);
        if (task == 0u)
            return;
        if ((PE_LoadU32(task + 8u) & 0x50u) != 0u) {
            pe_17018_walk_next();
            continue;
        }
        actor = PE_LoadU32(GA_D_8009D2F0);
        if ((PE_LoadU32(actor + 0x98u) & 0x1000u) != 0u &&
            (PE_LoadU16(task + 8u) & 0x80u) == 0u) {
            pe_17018_walk_next();
            continue;
        }
        if ((D_8009D1A0 & 0x100u) != 0u &&
            actor != PE_LoadU32(GA_D_8009D254) &&
            (PE_LoadU16(task + 8u) & 0x80u) == 0u) {
            pe_17018_walk_next();
            continue;
        }
        delay = PE_LoadU32(task + 0x10u);
        if (delay == 0u) {
            pe_17018_walk_next();
            continue;
        }
        delay -= 1u;
        PE_StoreU32(task + 0x10u, delay);
        if (delay != 0u) {
            pe_17018_walk_next();
            continue;
        }

        again = 1;
        first_fetch = 1;
        while (again) {
            if (first_fetch) {
                pc = PE_LoadU32(task);
                PE_StoreU32(GA_D_8009CE00, pc);
                first_fetch = 0;
            } else {
                pc = PE_LoadU32(GA_D_8009CE00);
            }
            word = PE_LoadU32(pc);
            word2 = PE_LoadU32(pc + 4u);
            argc = (word >> 13) & 0xFu;
            op = word & 0x1FFFu;
            kinds = word >> 17;
            imm_base = pc + 8u;
            PE_StoreU32(GA_D_8009CE00, imm_base + argc * 4u);
            for (i = 0; i < 16u; i++)
                PE_StoreU32(GA_VM_FRAME + i * 4u, 0u);
            for (i = 0; i < argc; i++) {
                kind = kinds & 7u;
                if (kind < 5u) {
                    uint32_t imm = PE_LoadU32(imm_base + i * 4u);
                    pe_addr_t decoded;

                    if (kind == 0u)
                        decoded = imm_base + i * 4u;
                    else if (kind == 1u)
                        decoded = actor + 0xACu + imm * 4u;
                    else if (kind == 2u)
                        decoded = GA_D_800A77F0 + imm * 4u;
                    else if (kind == 3u)
                        decoded = GA_D_8009DF70 + imm * 4u;
                    else
                        decoded = GA_D_800B6A80 + imm * 4u;
                    PE_StoreU32(GA_VM_FRAME + i * 4u, decoded);
                }
                kinds >>= 3;
                if (i == 4u)
                    kinds = word2;
            }
            again = pe_17018_dispatch(PE_LoadU32(GA_D_800910A0 + op * 4u),
                                      GA_VM_FRAME);
            if (again)
                task = PE_LoadU32(GA_D_8009D300);
        }
        PE_StoreU32(task, PE_LoadU32(GA_D_8009CE00));
        pe_17018_walk_next();
    }
}
