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
 * 0x2F / 0x30 / 0x5E / 0x77 / 0x9B / 0x0C / 0xD9. 0x1C and 0x1F are the already-ported
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
/* Host stand-in for ROM sp+16. APPROXIMATION: native has no guest $sp. */
#define GA_VM_FRAME   0x80120F80u

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
