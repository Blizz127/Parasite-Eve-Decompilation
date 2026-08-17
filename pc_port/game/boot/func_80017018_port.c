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
 * Handlers ported here: 0 / 1 / 2 / 0x20. 0x1C and 0x1F are the
 * already-ported mailbox leaves. Other table slots are not this cut
 * (return 0 = advance). Not M2.
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
        while (again) {
            pc = PE_LoadU32(task);
            PE_StoreU32(GA_D_8009CE00, pc);
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
