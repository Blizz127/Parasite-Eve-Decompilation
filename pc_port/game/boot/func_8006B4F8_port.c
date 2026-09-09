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
 * s5 = s4 + (lw(s4+4) & 0x003FFFFF).
 *
 * PE-BTL66 — hdr+0x10 Writer A @ 0x8006B828..0x8006B894 before
 * the 12574 stores. count = word>>22, rec = s4+(word&0x3FFFFF),
 * 12-byte records: lbu type rec+0x0B, lbu cmd rec+7, ptr =
 * rec+4 & 0x00FFFFFF. Stores chunk2+ptr at overlay+0x1C0 +
 * type*192 + cmd*4 (D_800B0E98). Live m0005i type-2 cmd 0x17
 * byte+2 = 51 so 1A680 sets +0x0F = 50. hdr+0x0C B0E70 bind
 * @ 0x8006B7C8 is still not this cut (3D050 tail gate).
 *
 * Then 12574(s4 + (lw(s4 + (hdr+0x14 & 0x3FFFFF))+4 &
 * 0x00FFFFFF)) → overlay+0x944. Sibling stores +0x948/+0x94C
 * from hdr+0x18/+0x1C, and the hdr+0x20 count loop writes
 * overlay+0x950+id*4.
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

/* 6B968..6BA08: publish the loaded room's effect-module descriptors.
 * Entries 8..84 use the main dispatch table; 85+ use E1044. Existing
 * descriptors take precedence. M0005 exports Eve's effect 0x75 here. */
void func_8006B4F8_bind_effects(pe_addr_t base)
{
    pe_addr_t header=base+(PE_LoadU32(base+4u)&MASK_22);
    uint32_t packed=PE_LoadU32(header+4u), i;
    pe_addr_t entry=base+(packed&MASK_22);
    for (i=0; i<(packed>>22); i++,entry+=12u) {
        uint32_t code=PE_LoadU8(entry+7u);
        pe_addr_t slot;
        if (code>=8u && code<0x55u)
            slot=PE_LoadU32(0x800942E0u)+code*4u;
        else if (code>=0x55u)
            slot=0x800E1044u+(code-0x55u)*4u;
        else continue;
        if (PE_LoadU32(slot)==0u)
            PE_StoreU32(slot,PE_LoadU32(entry+8u));
    }
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

    /* Writer A — ROM 0x8006B828. Do not add hdr+0x0C B0E70. */
    word = PE_LoadU32(s5 + 0x10u);
    count = word >> 22;
    rec = s4 + (word & MASK_22);
    if (rec < 0x80000000u)
        count = 0;
    for (i = 0; i < count; i++) {
        uint8_t type;
        uint8_t cmd;

        if (rec < 0x80000000u)
            break;
        type = PE_LoadU8(rec + 0x0Bu);
        cmd = PE_LoadU8(rec + 7u);
        PE_StoreU32(GA_OVERLAY + 0x1C0u
                        + (uint32_t)type * 192u
                        + (uint32_t)cmd * 4u,
                    s4 + (PE_LoadU32(rec + 4u) & MASK_24));
        rec += 12u;
    }

    PE_StoreU32(GA_OVERLAY + 0x944u,
                func_80012574(pe_6b4f8_rel(s4, PE_LoadU32(s5 + 0x14u))));
    PE_StoreU32(GA_OVERLAY + 0x948u, pe_6b4f8_rel(s4, PE_LoadU32(s5 + 0x18u)));
    PE_StoreU32(GA_OVERLAY + 0x94Cu, pe_6b4f8_rel(s4, PE_LoadU32(s5 + 0x1Cu)));

    word = PE_LoadU32(s5 + 0x20u);
    count = word >> 22;
    rec = s4 + (word & MASK_22);
    if (rec < 0x80000000u)
        count = 0;
    for (i = 0; i < count; i++) {
        if (rec < 0x80000000u)
            break;
        id = PE_LoadU8(rec + 7u);
        PE_StoreU32(GA_OVERLAY + 0x950u + (uint32_t)id * 4u,
                    s4 + (PE_LoadU32(rec + 4u) & MASK_24));
        rec += 8u;
    }
    func_8006B4F8_bind_effects(s4);
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
    /* Retail overlaps each next CD request with the preceding chunk's
     * texture walk (6B61C / 6B700), before polling completion. The host
     * 6E6D4 copies all sectors during issue, so consume the old chunk
     * before that synchronous copy can overwrite a shared staging arena. */
    pe_6b4f8_e1c0_loop(dest0);
    func_80074DC0(0); /* drain guest-backed GPU sources before CD reuse */
    if (!pe_6b4f8_issue_poll(lba + (int)sec0, dest1, (int)sec1))
        return 0;
    pe_6b4f8_e1c0_loop(dest1);
    func_80074DC0(0);
    if (!pe_6b4f8_issue_poll(lba + (int)sec0 + (int)sec1, dest2, (int)sec2))
        return 0;
    func_80072714();
    func_800726C4();
    func_80072724();
    func_8006B4F8_12574_publish_cut();
    return 1;
}

extern pe_addr_t func_8006E498(pe_addr_t base, uint32_t key);
extern int func_8006C5BC(void);
extern int func_8006C4C4(int a0);
extern void func_800125E0(void);
extern void func_80034FC4(void);
extern void func_8001266C(void);
extern void func_8001A918(void);
extern int func_8006BECC(void);

/*
 * PE-BTL90 — func_8006B35C dest-enter clear (103 words,
 * 0x8006B35C..0x8006B4F8, SHA-256 1106cb2a…). 3F074 first jal.
 *
 * Proven stores: B0E70[0..9] countdown from overlay+0x1BC,
 * ten B0E98 rows of 48 words stride 192, then one-word zeros
 * at +0x940/+0x944/+0x948/+0x94C, +0x954 then +0x950, +0x958.
 * Suffix is the retail 6E498 lookup plus the documented overlay
 * byte/bit stores.  +0x14C==0 skips the lookup (host guard;
 * ROM would deref).
 */
void func_8006B35C(void)
{
    unsigned int i;
    unsigned int j;
    pe_addr_t row;
    pe_addr_t wordp;
    pe_addr_t base;
    uint32_t overlay;
    pe_addr_t lookup;

    for (i = 0; i < 10u; i++)
        PE_StoreU32(0x800B0E70u + (9u - i) * 4u, 0u);

    row = GA_OVERLAY;
    for (i = 0; i < 10u; i++) {
        wordp = row + 0x27Cu;
        for (j = 0; j < 48u; j++) {
            PE_StoreU32(wordp, 0u);
            wordp -= 4u;
        }
        row += 192u;
    }

    PE_StoreU32(GA_OVERLAY + 0x940u, 0u);
    PE_StoreU32(GA_OVERLAY + 0x944u, 0u);
    PE_StoreU32(GA_OVERLAY + 0x948u, 0u);
    PE_StoreU32(GA_OVERLAY + 0x94Cu, 0u);
    PE_StoreU32(GA_OVERLAY + 0x954u, 0u);
    PE_StoreU32(GA_OVERLAY + 0x950u, 0u);
    PE_StoreU32(GA_OVERLAY + 0x958u, 0u);

    base = PE_LoadU32(GA_OVERLAY + 0x14Cu);
    PE_StoreU32(GA_OVERLAY + 0x128u, PE_LoadU32(GA_OVERLAY + 0x150u));
    PE_StoreU32(GA_OVERLAY + 0x12Cu,
                PE_LoadU32(GA_OVERLAY + 0x150u) + 0x1400u);
    lookup = 0u;
    if (base >= 0x80000000u)
        lookup = func_8006E498(base, 0x5EAF6804u);
    PE_StoreU32(GA_OVERLAY + 0x124u, lookup);

    overlay = PE_LoadU32(GA_OVERLAY) & ~0x400000u;
    PE_StoreU32(GA_OVERLAY, overlay);
    PE_StoreU8(GA_OVERLAY + 0xE0u, 39u);
    PE_StoreU8(GA_OVERLAY + 0xE1u, 13u);
    PE_StoreU8(GA_OVERLAY + 0x11u, 0u);
    PE_StoreU8(GA_OVERLAY + 0x12u, 0u);
    PE_StoreU16(0x80094488u, 0u);
    for (i = 0; i < 4u; i++)
        PE_StoreU16(0x8009448Cu + i * 8u, 0u);
    PE_StoreU8(GA_OVERLAY + 0xF6u, 48u);
    PE_StoreU8(GA_OVERLAY + 0xF7u, 127u);
    PE_StoreU16(GA_OVERLAY + 0xF8u, 256u);
    PE_StoreU16(GA_OVERLAY + 0xFAu, 2048u);
}

/*
 * 6B4F8 after 72724: hdr+1 → CE2, hdr+3 → overlay+8, then
 * hdr+0x0C B0E70[idB] (0x8006B7B0..0x8006B820). dest_load_cut
 * deliberately stops before this window.
 */
static void pe_6b4f8_ce2_hdr0c(void)
{
    pe_addr_t s4;
    pe_addr_t s5;
    uint32_t word;
    uint32_t count;
    uint32_t i;
    pe_addr_t rec;

    s4 = PE_LoadU32(GA_OVERLAY + 0x18Cu);
    if (s4 < 0x80000000u)
        return;
    s5 = s4 + (PE_LoadU32(s4 + 4u) & MASK_22);
    PE_StoreU8(GA_OVERLAY + 0x0Au, PE_LoadU8(s5 + 1u));
    PE_StoreU8(GA_OVERLAY + 0x08u, PE_LoadU8(s5 + 3u));

    word = PE_LoadU32(s5 + 0x0Cu);
    count = word >> 22;
    rec = s4 + (word & MASK_22);
    if (rec < 0x80000000u)
        count = 0;
    for (i = 0; i < count; i++) {
        uint8_t idb;

        if (rec < 0x80000000u)
            break;
        idb = PE_LoadU8(rec + 7u);
        uint32_t ptr = PE_LoadU32(rec + 4u) & MASK_24;

        PE_StoreU32(0x800B0E70u + (uint32_t)idb * 4u, s4 + ptr);
        rec += 12u;
    }
}

/*
 * PE-BTL90 named dest-ready cut, PE-BTL128 retail 3F074 order:
 * 6B35C + 6B4F8 (CE2 / hdr+0x0C / Writer A) then the 3F09C..3F27C
 * tail: D224/D308 serials, 34FC4, 1266C, 6BE4C, 6BECC until 0,
 * 6C4C4(CE4), 6C5BC until 0, 1A918, 125E0.
 * 6BE4C selects 6BECC's texture/model reload through the retail flag;
 * bypassing its texture states leaves Aya transparent. 68B94 is the
 * 677FC tile-publish cut (65B70/655D4/RTPS suffix deferred).
 * 6BD68 / 3F758 / E0060 stay deferred. Does not plant
 * mode 7/9/10.
 */
int func_8003F074_dest_ready_cut(uint32_t token)
{
    int guard;

    func_8006B35C();
    if (!func_8006B4F8_dest_load_cut(token))
        return 0;
    if (PE_LoadU32(GA_OVERLAY + 0x18Cu) >= 0x80000000u)
        pe_6b4f8_ce2_hdr0c();
    PE_StoreU32(0x8009D224u, 1u);
    PE_StoreU16(0x8009D308u, 1u);
    func_80034FC4();
    func_8001266C();
    /* 3F1D8 jal 68B94 — publishes rec+0x30/+0x34 SPRT lists. */
    (void)func_80068B94();
    (void)func_8006BE4C();
    guard = 0;
    while (func_8006BECC() == 1 && guard < 32)
        guard++;
    (void)func_8006C4C4((int)(int8_t)PE_LoadU8(GA_OVERLAY + 0x0Cu));
    guard = 0;
    while (func_8006C5BC() == 1 && guard < 16)
        guard++;
    func_8001A918();
    /* Retail 3F244..3F278: rebind message data at every field entry.
     * Keeping CE90 from the prior field can point into a new NPC model. */
    func_800371B0(func_8003F074_371b0_a0());
    if (PE_LoadU32(GA_OVERLAY + 0x944u) >= 0x80000000u)
        func_800125E0();
    /* 3F074 after-poll: DrawSync already ran; SetDispMask(1) so
     * PutDispEnv copies the VRAM display window instead of blanking. */
    func_80074D28(1);
    return 1;
}

/*
 * PE-BTL127 — func_8006BE4C dest-enter CE2/CE3 gate.
 * 32 words 0x8006BE4C..0x8006BECC, SHA-256 fb4091da…306b.
 * Sole TEXT jal 3F074@0x8003F204, after 1266C and before 6BECC.
 * CE2 in [10,15): if CE2!=CE3, overlay|=0x200000. Then if
 * (CE2-10)>>1 != ((CE3-10 srl 31)+CE3)>>1, overlay+0x0E|=4.
 * Always v0=0. Writes B0CD8/B0CE6, not D_800B6A80.
 */
int func_8006BE4C(void)
{
    uint8_t ce2;
    uint8_t ce3;
    uint32_t left;
    uint32_t right;
    int32_t delta;

    ce2 = PE_LoadU8(GA_OVERLAY + 0x0Au);
    if ((uint32_t)(ce2 - 10u) >= 5u)
        return 0;
    ce3 = PE_LoadU8(GA_OVERLAY + 0x0Bu);
    if (ce2 != ce3)
        PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) | 0x200000u);
    left = (uint32_t)(ce2 - 10u) >> 1;
    delta = (int32_t)ce3 - 10;
    right = ((uint32_t)delta >> 31) + (uint32_t)ce3;
    right = (uint32_t)((int32_t)right >> 1);
    if (left != right)
        PE_StoreU8(GA_OVERLAY + 0x0Eu,
                   (uint8_t)(PE_LoadU8(GA_OVERLAY + 0x0Eu) | 4u));
    return 0;
}
