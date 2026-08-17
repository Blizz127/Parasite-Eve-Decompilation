/*
 * PE-BTL14 — dest-token decode and 6B4F8 12574 publish cut
 * (translated retail, not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 3F074 @ 0x8003F088 jals 6B4F8(D_8009D280) before the poll and
 * before 125E0. 6B4F8 is 540 words (0x8006B4F8..0x8006BD68) and
 * is the sole TEXT caller of 12574. This file is not that whole
 * body. It is the proven token→name→index path plus the chunk-2
 * header walk that publishes overlay+0x944..+0x954.
 *
 * func_8006E2D0 — 26 words 0x8006E2D0..0x8006E338, SHA-256
 * c0d67bd4…31d9. Zero jal. a0=7-byte dest, a1=packed token.
 * Six 5-bit fields (shifts 27,22,17,12,7,2) index the EXE
 * charset at D_800930B4. Lowercase is folded. Live
 * 0xA80002C8 → "M0005I".
 *
 * func_8006E454 — 17 words 0x8006E454..0x8006E498, SHA-256
 * 7c98a25d…f4f4. Zero jal. atoi of dest[2..4]. "M0005I" → 5.
 * 6B4F8 then uses index-1 into the 8-byte PE.IMG table at
 * D_80093378. Table[4] = {rel=0x266A, packed=0x0600A921}
 * (33+169+96 sectors). Chunk 2 lands at overlay+0x18C.
 *
 * func_8006B4F8_12574_publish_cut — ROM 0x8006B79C..0x8006B94C
 * after the three PE.IMG loads. s4 = overlay+0x18C dest.
 * s5 = s4 + (lw(s4+4) & 0x003FFFFF). 12574(s4 + (lw(s4 +
 * (hdr+0x14 & 0x3FFFFF))+4 & 0x00FFFFFF)) → overlay+0x944.
 * Sibling stores +0x948/+0x94C from hdr+0x18/+0x1C, and the
 * hdr+0x20 count loop writes overlay+0x950+id*4.
 *
 * Live m0005i 12574 list: count=7 type entries; 125E0 desc
 * lbu=2, actors type 1 then type 6. No type 0 in that desc.
 * CD issue / LoadImage / 72714/726C4/72724 / 6CDA4 are not
 * this cut.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_CHARSET    0x800930B4u
#define GA_OVERLAY    0x800B0CD8u
#define GA_TABLE      0x80093378u
#define GA_D_800B0DD8 0x800B0DD8u
#define GA_NAME       0x80122180u
#define MASK_22       0x003FFFFFu
#define MASK_24       0x00FFFFFFu

extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);
extern int func_8006E1C0(pe_addr_t entry, pe_addr_t base);
extern int func_80072714(void);
extern void func_800726C4(void);
extern void func_80072724(void);

void func_8006E2D0(pe_addr_t dest, uint32_t token)
{
    unsigned int i;
    unsigned int shift;
    unsigned int idx;
    uint8_t ch;

    for (i = 0; i < 6u; i++) {
        shift = (5u - i) * 5u + 2u;
        idx = (token >> shift) & 0x1Fu;
        ch = PE_LoadU8(GA_CHARSET + idx);
        if ((unsigned int)(ch - 97u) < 26u)
            ch = (uint8_t)(ch - 32u);
        PE_StoreU8(dest + i, ch);
    }
    PE_StoreU8(dest + 6u, 0u);
}

int func_8006E454(pe_addr_t name)
{
    int hundreds;
    int tens;
    int ones;

    hundreds = (int)(int8_t)PE_LoadU8(name + 2u) - 48;
    tens = (int)(int8_t)PE_LoadU8(name + 3u) - 48;
    ones = (int)(int8_t)PE_LoadU8(name + 4u);
    return hundreds * 100 + tens * 10 + ones - 48;
}

static pe_addr_t pe_6b4f8_rel(pe_addr_t base, uint32_t packed)
{
    return base + (PE_LoadU32(base + (packed & MASK_22) + 4u) & MASK_24);
}

void func_8006B4F8_12574_publish_cut(void)
{
    pe_addr_t s4;
    pe_addr_t s5;
    pe_addr_t rec;
    uint32_t word;
    uint32_t count;
    uint32_t i;
    uint8_t id;

    s4 = PE_LoadU32(GA_OVERLAY + 0x18Cu);
    s5 = s4 + (PE_LoadU32(s4 + 4u) & MASK_22);

    PE_StoreU32(GA_OVERLAY + 0x944u,
                func_80012574(pe_6b4f8_rel(s4, PE_LoadU32(s5 + 0x14u))));
    PE_StoreU32(GA_OVERLAY + 0x948u, pe_6b4f8_rel(s4, PE_LoadU32(s5 + 0x18u)));
    PE_StoreU32(GA_OVERLAY + 0x94Cu, pe_6b4f8_rel(s4, PE_LoadU32(s5 + 0x1Cu)));

    word = PE_LoadU32(s5 + 0x20u);
    count = word >> 22;
    rec = s4 + (word & MASK_22);
    for (i = 0; i < count; i++) {
        id = PE_LoadU8(rec + 7u);
        PE_StoreU32(GA_OVERLAY + 0x950u + (uint32_t)id * 4u,
                    s4 + (PE_LoadU32(rec + 4u) & MASK_24));
        rec += 8u;
    }
}

static int pe_6b4f8_issue_poll(int lba, pe_addr_t dest, int sectors)
{
    int status;

    do {
        status = func_8006E6A8(lba, dest, sectors);
    } while (status == -1);
    for (;;) {
        status = func_8006E7E8();
        if (status == 0)
            return 1;
        if (status != -1) {
            do {
                status = func_8006E7E8();
            } while (status != 0 && status != -1);
            if (status == 0)
                return 1;
        }
    }
}

static void pe_6b4f8_e1c0_loop(pe_addr_t base)
{
    pe_addr_t metadata;
    uint32_t header;
    uint32_t count;
    pe_addr_t entry;
    uint32_t issued;

    metadata = base + PE_LoadU32(base + 4u);
    header = PE_LoadU32(metadata + 0x28u);
    count = header >> 22;
    entry = base + (header & MASK_22);
    issued = 0u;
    if (count == 0u)
        return;
    for (;;) {
        func_8006E1C0(entry, base);
        header = PE_LoadU32(metadata + 0x28u);
        issued += 1u;
        count = header >> 22;
        entry += 0x14u;
        if (issued >= count)
            break;
    }
}

/*
 * PE-BTL63 — 6B4F8 dest-token PE.IMG three-chunk load.
 *
 * ROM 0x8006B4F8..0x8006B79C before publish: 6E2D0 / 6E454 /
 * table[atoi-1] at D_80093378, then three 6E6A8 into overlay
 * +0x194 / +0x168 / +0x18C with sec0 / sec1 / sec2 from the
 * packed word. 6E1C0 counted loops after chunk1 issue (on
 * dest0) and after chunk2 issue (on dest1). 72714/726C4/72724
 * then the existing 12574 publish cut. 6CDA4 tail is not this
 * cut. Live Watch 0x31 token 0xA80663C8 is M0367I table[366]
 * rel=0x15050 packed=0x04E0AA21 (33+170+78).
 */
int func_8006B4F8_dest_load_cut(uint32_t token)
{
    int idx;
    uint32_t rel;
    uint32_t packed;
    uint32_t sec0;
    uint32_t sec1;
    uint32_t sec2;
    int lba;
    pe_addr_t dest0;
    pe_addr_t dest1;
    pe_addr_t dest2;

    dest0 = PE_LoadU32(GA_OVERLAY + 0x194u);
    dest1 = PE_LoadU32(GA_OVERLAY + 0x168u);
    dest2 = PE_LoadU32(GA_OVERLAY + 0x18Cu);
    if (dest0 < 0x80000000u || dest1 < 0x80000000u || dest2 < 0x80000000u)
        return 0;
    func_8006E2D0(GA_NAME, token);
    idx = func_8006E454(GA_NAME) - 1;
    if (idx < 0)
        return 0;
    rel = PE_LoadU32(GA_TABLE + (uint32_t)idx * 8u);
    packed = PE_LoadU32(GA_TABLE + (uint32_t)idx * 8u + 4u);
    sec0 = packed & 0xFFu;
    sec1 = (packed >> 8) & 0xFFFu;
    sec2 = packed >> 20;
    lba = (int)(PE_LoadU32(GA_D_800B0DD8) + rel);
    if (!pe_6b4f8_issue_poll(lba, dest0, (int)sec0))
        return 0;
    if (!pe_6b4f8_issue_poll(lba + (int)sec0, dest1, (int)sec1))
        return 0;
    pe_6b4f8_e1c0_loop(dest0);
    if (!pe_6b4f8_issue_poll(lba + (int)sec0 + (int)sec1, dest2, (int)sec2))
        return 0;
    pe_6b4f8_e1c0_loop(dest1);
    func_80072714();
    func_800726C4();
    func_80072724();
    func_8006B4F8_12574_publish_cut();
    return 1;
}
