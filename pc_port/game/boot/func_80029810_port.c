/*
 * PE-BTL3 — ROM-word cuts from func_80029810, func_8001A680, and the
 * type-0 command-table writer at 0x8006C140 (func_8006BECC state 6).
 *
 * These are native translations, not matching src/ C.  Authority is
 * pc_port/tools/pe_btl3_29810_tail_oracle.py and
 * pc_port/tools/pe_btl3_command_table_oracle.py against the SHA-1-exact EXE.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_RECORD_P       0x8009D278u
#define GA_ACTOR_P        0x8009D254u
#define GA_COMMAND_TABLE  0x800B0E98u
#define GA_VALUE_D27C     0x8009D27Cu
#define GA_OVERLAY        0x800B0CD8u
#define CALLBACK_2D268    0x8002D268u

/*
 * ROM 0x8006C140..0x8006C174 (Writer B clip loop inside func_8006BECC
 * state 6). s4 = D_800B0CD8, s2 = overlay+0x154 package. Each 12-byte
 * directory record stores package+(rec+4 & 0x00FFFFFF) at
 * overlay+0x1C0+idB*4 = D_800B0E98[idB] (type-0 row). Directory setup
 * matches 0x8006C0E4/0xEC/0xF4 and 0x8006C11C..0x130 (section at
 * package+4, count = packed>>22, records at packed&0x3FFFFF). Model
 * bind 0x8006C118 (D_800B0E70) is not this cut.
 *
 * 0x55 / func_80029810 does not jal this writer. The table is a global
 * filled by 6BECC; 1A680 only reads it. Native tests invoke the writer
 * with a planted package so the consumer can use a ROM-shaped pointer.
 */
void func_8006C140_type0_clip_bind(pe_addr_t package)
{
    pe_addr_t dir;
    unsigned int packed;
    unsigned int count;
    unsigned int i;
    pe_addr_t rec;

    dir = package + PE_LoadU32(package + 4u);
    packed = PE_LoadU32(dir + 0x10u);
    count = packed >> 22;
    rec = package + (packed & 0x3FFFFFu);
    for (i = 0; i < count; i++) {
        unsigned int idb = PE_LoadU8(rec + 7u);
        unsigned int ptr = PE_LoadU32(rec + 4u) & 0x00FFFFFFu;
        PE_StoreU32(GA_OVERLAY + 0x1C0u + idb * 4u, package + ptr);
        rec += 12u;
    }
}

void func_8001A680_command_cut(pe_addr_t actor, unsigned int command)
{
    unsigned int type;
    pe_addr_t resource;
    unsigned int flags;

    type = PE_LoadU8(actor + 0x0Cu);
    resource = PE_LoadU32(GA_COMMAND_TABLE + type * 192u
                          + (command & 0xFFFFu) * 4u);
    PE_StoreU8(actor + 0x0Eu, (uint8_t)command);
    PE_StoreU32(actor + 0x14u, 0u);
    PE_StoreU32(actor + 0x18u, 0u);
    PE_StoreU32(actor + 0x1B0u, resource);
    flags = PE_LoadU32(actor + 0x98u) & ~0x200u;
    PE_StoreU32(actor + 0x98u, flags);
    PE_StoreU8(actor + 0x0Fu,
               (uint8_t)((PE_LoadU8(resource + 2u) - 1u) & 0xFFu));
}

void func_80029810_after_hp_cut(unsigned int encounter)
{
    pe_addr_t record;
    pe_addr_t actor;
    pe_addr_t source;
    int value;
    int cap;

    Bootstrap_ReturnVoid("func_800209F0", "func_80029810_after_hp_cut");
    record = PE_LoadU32(GA_RECORD_P);
    value = (int)PE_LoadU32(record + 0x08u);
    if (value <= 0) {
        PE_StoreU32(record + 0x08u, 0x00010000u);
    } else {
        cap = (int)PE_LoadU32(record + 0x28u);
        if (value >= cap) {
            PE_StoreU32(record + 0x08u, (unsigned int)cap);
            PE_StoreU32(record + 0x34u, 240u);
        }
    }

    Bootstrap_ReturnVoid("func_80030640", "func_80029810_after_hp_cut");
    actor = PE_LoadU32(GA_ACTOR_P);
    PE_StoreU32(actor + 0x194u, CALLBACK_2D268);
    source = PE_LoadU32(actor + 0x238u);
    PE_StoreU16(GA_VALUE_D27C,
                (uint16_t)(PE_LoadU32(source + 0x18u) - 100u));
    Bootstrap_ReturnVoid1("func_800339A0", "func_80029810_after_hp_cut",
                          encounter & 0xFFu);
    record = PE_LoadU32(GA_RECORD_P);
    func_8001A680_command_cut(actor, PE_LoadU8(record + 0x12u));
}

int func_800144FC_state3B_cut(void)
{
    pe_addr_t actor;
    unsigned int flags;

    if ((PE_LoadU8(GA_OVERLAY + 0x0Eu) & 3u) != 0u)
        return 0;
    actor = PE_LoadU32(GA_OVERLAY);
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0u);
    flags = PE_LoadU32(actor) & 0xFF7FFFFFu;
    PE_StoreU32(actor, flags);
    return 1;
}
