/*
 * Phase 6E-B54K-AS — executable helpers reached by the overlay title loop.
 *
 * Retail executable SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *   func_8005E038 [0x8005E038,0x8005E114) 55 words  pad-word remap of D_8009D26C
 *   func_80042770 [0x80042770,0x80042798) 10 words  card slot present bit
 *   func_8003FFCC [0x8003FFCC,0x8004006C) 40 words  valid-save scan
 *   func_800525EC [0x800525EC,0x80052634) 18 words  SE 0x44C through func_8006DF50
 *   func_8005267C [0x8005267C,0x800526C4) 18 words  SE 0x44E through func_8006DF50
 *   func_800425DC [0x800425DC,0x80042770) 101 words memory-card poll — NOT translated
 *   func_8005E6F0 [0x8005E6F0,0x8005E788)  38 words  frame-flip prepare (B54K-AU)
 *   func_8005E788 [0x8005E788,0x8005E84C)  49 words  frame-flip present (B54K-AU)
 * $gp = 0x8009CD70: +0x38C D_8009D0FC, +0x390 D_8009D100, +0x394 D_8009D104,
 * +0x398 D_8009D108, +0x3A8 D_8009D118, +0x3AC D_8009D11C, +0x3B0 D_8009D120,
 * +0x3C4 D_8009D134.  ClearOTagR (DMA6 OT clear) and DrawOTag (DMA2 OT walk)
 * are named cuts; both are reached only when D_8009D120 is nonzero.
 *
 * func_800425DC drives the libcard state machine (func_800405A4,
 * func_8004D9D8, func_8004CC50/8004D024 message windows, func_80041108 per
 * slot).  It is on the title loop every frame, so it is registered as a
 * BOOTSTRAP_RET boundary that records and continues: with the two slot
 * records of D_800A0ED4 left as func_80042538 initialised them, the title
 * behaves as with no memory card inserted.  Strict mode stops here.
 * The SE wrappers keep their retail guard (D_800B0E08 != 0) and record the
 * untranslated SPU call with its sequence id instead of playing it.
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_D_8009D26C 0x8009D26Cu
#define GA_CARD_SLOTS 0x800A0ED4u
#define GA_D_800B0E08 0x800B0E08u

uint32_t func_8005E038(void)
{
    uint32_t a0 = PE_LoadU32(GA_D_8009D26C);
    uint32_t v1 = (a0 << 9) & 0x1000u;

    if (a0 & 0x20u) v1 |= 0x4000u;
    if (a0 & 0x40u) v1 |= 0x8000u;
    if (a0 & 0x10u) v1 |= 0x2000u;
    if (a0 & 0x20000000u) v1 |= 0x20u;
    if (a0 & 0x40000000u) v1 |= 0x40u;
    if (a0 & 0x10000000u) v1 |= 0x10u;
    if (a0 & 0x80000000u) v1 |= 0x80u;
    if (a0 & 0x04000000u) v1 |= 0x4u;
    if (a0 & 0x08000000u) v1 |= 0x8u;
    if (a0 & 0x01000000u) v1 |= 0x1u;
    if (a0 & 0x02000000u) v1 |= 0x2u;
    if (a0 & 0x2u) v1 |= 0x100u;
    if (a0 & 0x4u) v1 |= 0x800u;
    return v1;
}

int func_80042770(int slot)
{
    return PE_LoadU8(GA_CARD_SLOTS + (uint32_t)slot * 0x418u) & 1u;
}

int func_8003FFCC(void)
{
    pe_addr_t record = GA_CARD_SLOTS;
    const pe_addr_t end = GA_CARD_SLOTS + 0x830u;
    int found = 0;

    while (record < end) {
        if (PE_LoadU8(record + 1u) == 0x0Fu) {
            pe_addr_t entry = record + 0x1Cu;
            const pe_addr_t limit = record + 0x418u;
            while (entry < limit) {
                found = 0;
                if (PE_LoadU8(entry) == 1u)
                    found = PE_LoadU8(entry + 0x29u) != 0u;
                if (found)
                    break;
                entry += 0x44u;
            }
        }
        record += 0x418u;
        if (found)
            break;
    }
    return found;
}

static void SoundEffect(uint32_t sequence, const char *caller)
{
    if (PE_LoadU32(GA_D_800B0E08) == 0u)
        return;
    /* func_8006DF50(handle, sequence, 0x100, 0x80, 0x7F): SPU effect. */
    Bootstrap_ReturnVoid1("func_8006DF50", caller, sequence);
}

void func_800525EC(void) { SoundEffect(0x44Cu, "func_800525EC"); }
void func_8005267C(void) { SoundEffect(0x44Eu, "func_8005267C"); }

void func_800425DC(void)
{
    Bootstrap_ReturnVoid("func_800425DC", "func_801909B4");
}

extern int func_8007506C(const RECT *rect, pe_addr_t data);
extern pe_addr_t func_80075424(pe_addr_t env);

void func_8005E6F0(void)
{
    uint32_t index;
    uint32_t stride;
    pe_addr_t draw, ot;

    if (PE_LoadU32(0x8009D120u) != 0u)
        index = PE_LoadU32(0x8009D108u) == 0u ? 1u : 0u;
    else
        index = PE_LoadU32(0x800ACDDCu);
    stride = ((index << 4) - index) << 3;              /* index * 120 */
    PE_StoreU32(0x8009D108u, index);
    draw = PE_LoadU32(0x800A21F4u + stride);
    ot = PE_LoadU32(0x800A21F0u + stride);
    PE_StoreU32(0x8009D0FCu, 0x800A2180u + stride);
    PE_StoreU32(0x8009D104u, draw);
    PE_StoreU32(0x8009D100u, draw);
    PE_StoreU32(0x8009D118u, ot);
    PE_StoreU32(0x8009D11Cu, ot + 4u);
    if (PE_LoadU32(0x8009D120u) != 0u) {
        /* 0x8005E770 jal func_800752AC (ClearOTagR, 0x1000 entries). */
        Bootstrap_ReturnVoid1("func_8005E6F0_func_800752AC_cut", "func_8005E6F0", ot);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    }
}

void func_8005E788(int mode)
{
    pe_addr_t env;
    RECT rect;

    if (PE_LoadU32(0x8009D120u) == 0u)
        return;
    func_80073A44(1);
    func_80074DC0(0);
    func_80073A44(mode == 1 ? 0 : mode);
    (void)func_80074A44(1);
    env = PE_LoadU32(0x8009D0FCu);
    (void)func_80075424(env);
    func_800755F0((void *)(uintptr_t)(env + 0x5Cu));
    if (PE_LoadU32(0x8009D134u) != 0u) {
        rect.x = 0;
        rect.y = PE_LoadU32(0x8009D108u) != 0u ? 0xEB : 0x0B;
        rect.w = 0x140;
        rect.h = 0xCC;
        (void)func_8007506C(&rect, PE_LoadU32(0x8009D134u));
        if (PE_Port_ShouldStop())
            return;
    }
    /* 0x8005E834 jal func_800753B4 (DrawOTag, ot + 0xFFF*4). */
    Bootstrap_ReturnVoid1("func_8005E788_func_800753B4_cut", "func_8005E788",
                          PE_LoadU32(0x8009D118u) + 0xFFFu * 4u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
