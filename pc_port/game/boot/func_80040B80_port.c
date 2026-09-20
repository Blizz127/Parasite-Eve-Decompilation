/*
 * PE-SAVEWRITE — func_80040B80 save-buffer assembler (hand-translated).
 *
 * Authority: retail Disc1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b,
 * asm/disc1/307CC.s 0x80040B80..0x80040F80 (0x400, 256w).
 *
 * Builds the 0x2000-byte memory-card block in D_8009EED0: a 0x100-byte header
 * from D_800B8868 (BIOS version, formatted play-time string at +4, six status
 * words and the room/name block copied out of func_8005DE70()'s base), the
 * 0x12E0-byte body from D_800C0DE0, func_8003F800's extra fields, then a
 * CRC-16/CCITT (poly 0x1021) over the whole 0x2000 bytes stored as ~crc.
 *
 * The retail unaligned lwl/lwr copies are plain byte copies, so they are
 * expressed as memcpy here.  Not a matching leaf.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_SAVE_BLOCK   0x8009EED0u
#define GA_SAVE_CURSOR  0x800A0ED0u
#define GA_SAVE_HEADER  0x800B8868u
#define GA_SAVE_BODY    0x800C0DE0u

static void pe_st_memcpy(pe_addr_t dst, pe_addr_t src, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
        PE_StoreU8(dst + i, PE_LoadU8(src + i));
}

/* func_8005DE70 — return D_800A8044 plus the fixed base D_800A8028.  The
 * matching leaf computes `value + (base - 0x1C)` on &D_800A8044. */
int func_8005DE70(void)
{
    return (int)(PE_LoadU32(0x800A8044u) + 0x800A8028u);
}

void func_80040B80(pe_addr_t record)
{
    pe_addr_t base = (pe_addr_t)func_8005DE70();
    pe_addr_t dst, src, end;
    uint32_t crc = 0xFFFFu;

    func_80071A24(GA_SAVE_HEADER, 0x100u);
    PE_StoreU32(0x800A5D50u, 0x2000u);
    PE_StoreU8(0x800B886Au, 0x11u);
    PE_StoreU8(0x800B886Bu, 0x01u);
    PE_StoreU16(GA_SAVE_HEADER, PE_LoadU16(0x80010F48u));

    /* Play-time string -> header+4.  func_80040210 returns D_8009EE8C. */
    (void)func_80040210((int32_t)PE_LoadU8(record + 0x18u) - 0x40,
                        (int32_t)PE_LoadU32(0x800C0DE8u));
    func_80071A14(0x800B886Cu, 0x8009EE8Cu);

    /* Name/status words out of the func_8005DE70() base. */
    pe_st_memcpy(0x800B88C8u, base + 0x14u, 0x30u);
    pe_st_memcpy(0x800B88E8u, base + 0x40u, 0x80u);

    /* Assemble the block: header, body, extra fields. */
    func_80071A24(GA_SAVE_BLOCK, 0x2000u);
    PE_StoreU32(GA_SAVE_CURSOR, GA_SAVE_BLOCK);

    dst = GA_SAVE_BLOCK;
    src = GA_SAVE_HEADER;
    end = src + 0x100u;
    while (src != end) {
        pe_st_memcpy(dst, src, 0x10u);
        src += 0x10u;
        dst += 0x10u;
    }

    src = GA_SAVE_BODY;
    dst = PE_LoadU32(GA_SAVE_CURSOR) + 0x100u;
    PE_StoreU32(GA_SAVE_CURSOR, dst);
    end = src + 0x12E0u;
    while (src != end) {
        pe_st_memcpy(dst, src, 0x10u);
        src += 0x10u;
        dst += 0x10u;
    }
    pe_st_memcpy(dst, src, 4u);
    PE_StoreU32(GA_SAVE_CURSOR, PE_LoadU32(GA_SAVE_CURSOR) + 0x12E4u);

    func_8003F800();

    /* CRC-16/CCITT over the whole 0x2000-byte block; store ~crc as a word. */
    for (uint32_t i = 0; i < 0x2000u; i++) {
        crc ^= (uint32_t)PE_LoadU8(GA_SAVE_BLOCK + i) << 8;
        for (int b = 0; b < 8; b++) {
            if (crc & 0x8000u)
                crc = (crc << 1) ^ 0x1021u;
            else
                crc <<= 1;
        }
    }
    crc = (~crc) & 0xFFFFu;
    PE_StoreU32(PE_LoadU32(GA_SAVE_CURSOR), crc);
    PE_StoreU32(GA_SAVE_CURSOR, PE_LoadU32(GA_SAVE_CURSOR) + 4u);
}
