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
