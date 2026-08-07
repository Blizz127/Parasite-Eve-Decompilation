/*
 * Phase 6E-B50 — func_8006AD40: streaming subsystem multiplexer.
 *
 * 391 words / 1564 bytes, exe 0x8006AD40–0x8006B35C, file offset 0x5B540.
 *
 * Six streaming channels gated on D_800B0CD8 bit 0, each with
 * issue (func_8006E6A8) / poll (func_8006E7E8) pattern, followed by
 * DrawSync/VSync/PutDispEnv/SetDispMask finalization and flag-bit clear.
 *
 * Dependency boundary:
 *   func_8006E6A8  TRANSLATED (B16)
 *   func_8006E7E8  TRANSLATED (B16)
 *   func_8006E1C0  UNRESOLVED (68 words, GPU packet entry processor)
 *   func_8007506C  UNRESOLVED (24 words, Psy-Q callback dispatch)
 *   func_8006E498  TRANSLATED (archive directory lookup)
 *   func_800718D0  UNRESOLVED (29 words, archive data processor)
 *   func_80030894  UNRESOLVED (788 words, GPU rendering pipeline)
 *   func_80087024  TRANSLATED (streaming shutdown)
 *   func_80074DC0  TRANSLATED (DrawSync)
 *   func_80074A44  TRANSLATED (ResetGraph)
 *   func_80073A44  TRANSLATED (VSync)
 *   func_800755F0  TRANSLATED (PutDispEnv / Present)
 *   func_80074D28  TRANSLATED (SetDispMask)
 *
 * Classification: 1 — translated retail logic with four unresolved
 * callees routed through the centralized bootstrap boundary.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_bootstrap.h"

/* ── Guest addresses ───────────────────────────────────────────────── */
#define GA_D_800930EA  0x800930EAu
#define GA_D_800930EC  0x800930ECu
#define GA_D_800930EE  0x800930EEu
#define GA_D_800930F0  0x800930F0u
#define GA_D_800930E0  0x800930E0u
#define GA_D_80093126  0x80093126u
#define GA_D_800B0CD8  0x800B0CD8u
#define GA_D_800B0DD8  0x800B0DD8u
#define GA_D_8009CDDC  0x8009CDDCu
#define GA_D_800CCE80  0x800CCE80u
#define GA_GPU_DISP_BASE  0x80090020u

/* ── Unresolved callees ────────────────────────────────────────────── */
extern void func_8006E1C0(pe_addr_t entry, pe_addr_t base);
extern void func_8007506C(pe_addr_t hdr, pe_addr_t data);
extern void func_800718D0(pe_addr_t desc);
extern void func_80030894(void);

/* ── Translated callees ────────────────────────────────────────────── */
extern int       func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int       func_8006E7E8(void);
extern pe_addr_t func_8006E498(pe_addr_t base, uint32_t key);
extern void      func_80087024(void);

/* ── GPU display rect processing ───────────────────────────────────── */
static void PE_6AD40_GPURect(pe_addr_t base_addr)
{
    uint16_t a0 = PE_LoadU16(base_addr + 0x164A);
    uint16_t v0 = PE_LoadU16(base_addr + 0x1648);
    uint16_t hw2 = PE_LoadU16(base_addr + 0x164E);
    uint16_t hw3 = PE_LoadU16(base_addr + 0x164C);

    uint32_t v1 = ((uint32_t)(a0 & 0x100) >> 4)
                | (((uint32_t)(v0 & 0x3FF) >> 6) | 0x20u)
                | ((uint32_t)(a0 & 0x200) << 2);
    uint32_t v2 = ((uint32_t)hw2 << 6)
                | ((uint32_t)(hw3 >> 4) & 0x3F);

    PE_StoreU16(base_addr + 0x1650, (uint16_t)v1);
    PE_StoreU16(base_addr + 0x1652, (uint16_t)v2);
}

/* ── Streaming issue helper ────────────────────────────────────────── */
static int PE_6AD40_Issue(pe_addr_t table, pe_addr_t lba_base,
                            pe_addr_t desc)
{
    uint32_t off = PE_LoadU16(table);
    uint32_t end = PE_LoadU16(table + 2);
    return func_8006E6A8((int)(lba_base + off), desc, (int)(end - off));
}

/* ── Main function ─────────────────────────────────────────────────── */
void func_8006AD40(void)
{
    uint32_t flags;
    pe_addr_t lba_base;
    int r;

    /* Guard: bit 0 of D_800B0CD8 */
    flags = PE_LoadU32(GA_D_800B0CD8);
    if (!(flags & 1))
        return;

    lba_base = PE_LoadU32(GA_D_800B0DD8);

    /* ── Channel 1: table D_800930EA ───────────────────────────────── */
    {
        pe_addr_t desc;
        do {
            desc = PE_LoadU32(GA_D_800B0CD8 + 0x160);
            {
                uint32_t off = PE_LoadU16(GA_D_800930EA);
                uint32_t end = PE_LoadU16(GA_D_800930EA + 2);
                r = func_8006E6A8((int)(lba_base + off), desc,
                                  (int)(end - off));
            }
        } while (r == -1);

        /* Poll: if returns -1, restart issue loop */
        for (;;) {
            int s2 = func_8006E7E8();
            if (s2 == 0)
                break;
            if (s2 == -1) {
                do {
                    desc = PE_LoadU32(GA_D_800B0CD8 + 0x160);
                    {
                        uint32_t off = PE_LoadU16(GA_D_800930EA);
                        uint32_t end = PE_LoadU16(GA_D_800930EA + 2);
                        r = func_8006E6A8((int)(lba_base + off), desc,
                                          (int)(end - off));
                    }
                } while (r == -1);
            }
        }
    }

    /* ── Channel 2: table D_800930EC, with GPU packet processing ──── */
    {
        pe_addr_t desc;
        int s0 = 0;  /* one-shot first-entry flag */

        /* Issue loop */
        do {
            desc = PE_LoadU32(GA_D_800B0CD8 + 0x174);
            {
                uint32_t off = PE_LoadU16(GA_D_800930EC);
                uint32_t end = PE_LoadU16(GA_D_800930EC + 2);
                r = func_8006E6A8((int)(lba_base + off), desc,
                                  (int)(end - off));
            }
        } while (r == -1);

        /* Poll loop with first-entry processing and restart */
        for (;;) {
            if (s0 == 0) {
                pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x160);
                uint32_t header, count, offset;
                pe_addr_t entry;
                uint32_t i;
                int idx;
                pe_addr_t result;

                /* GPU packet entry iteration */
                header = PE_LoadU32(base + 0x28);
                count = header >> 22;
                offset = header & 0x003FFFFF;
                entry = base + offset;
                for (i = 0; i < count; i++) {
                    func_8006E1C0(entry, base);
                    entry += 0x14;
                }

                /* GPU display rect: a1=0x20,0x30 */
                for (idx = 0x20; idx < 0x40; idx += 0x10)
                    PE_6AD40_GPURect(GA_GPU_DISP_BASE + (uint32_t)idx);

                /* Archive directory lookup: magic 0xABCDC06C */
                result = func_8006E498(base, 0xABCDC06Cu);

                /* Packet chain dispatch */
                if (result != 0) {
                    pe_addr_t p = result;
                    uint32_t cnt;
                    while ((cnt = PE_LoadU32(p)) != 0) {
                        func_8007506C(p + 4, p + 0xC);
                        p += cnt & ~3u;
                    }
                }

                s0 = 1;
            }

            {
                int s2 = func_8006E7E8();
                if (s2 == 0)
                    break;
                if (s2 == -1) {
                    /* Restart: re-issue (s0 stays 1 — retail does NOT
                     * reset $s0 on poll-restart; it resets on the
                     * explicit channel re-init at 0x8006ADD8 only when
                     * the whole channel restarts from -1 issue result) */
                    do {
                        desc = PE_LoadU32(GA_D_800B0CD8 + 0x174);
                        {
                            uint32_t off = PE_LoadU16(GA_D_800930EC);
                            uint32_t end = PE_LoadU16(GA_D_800930EC + 2);
                            r = func_8006E6A8((int)(lba_base + off), desc,
                                              (int)(end - off));
                        }
                    } while (r == -1);
                    /* After restart, s0 remains 1 — first-entry block
                     * will NOT re-execute (matches retail: $s0=1 survives
                     * poll-restart; only channel re-init resets it) */
                }
            }
        }
    }

    /* ── Channel 3: table D_800930EE, with display rect ────────────── */
    {
        pe_addr_t desc;
        int s0 = 0;

        do {
            desc = PE_LoadU32(GA_D_800B0CD8 + 0x180);
            {
                uint32_t off = PE_LoadU16(GA_D_800930EE);
                uint32_t end = PE_LoadU16(GA_D_800930EE + 2);
                r = func_8006E6A8((int)(lba_base + off), desc,
                                  (int)(end - off));
            }
        } while (r == -1);

        for (;;) {
            if (s0 == 0) {
                int idx;
                func_800718D0(PE_LoadU32(GA_D_800B0CD8 + 0x174));
                for (idx = 0; idx < 0x20; idx += 0x10)
                    PE_6AD40_GPURect(GA_GPU_DISP_BASE + (uint32_t)idx);
                s0 = 1;
            }
            {
                int s2 = func_8006E7E8();
                if (s2 == 0)
                    break;
                if (s2 == -1) {
                    do {
                        desc = PE_LoadU32(GA_D_800B0CD8 + 0x180);
                        {
                            uint32_t off = PE_LoadU16(GA_D_800930EE);
                            uint32_t end = PE_LoadU16(GA_D_800930EE + 2);
                            r = func_8006E6A8((int)(lba_base + off), desc,
                                              (int)(end - off));
                        }
                    } while (r == -1);
                    /* s0 stays 1 — first-entry won't re-run */
                }
            }
        }
    }

    /* ── Channel 4: table D_800930F0 ───────────────────────────────── */
    {
        pe_addr_t desc;
        int s0 = 0;

        do {
            desc = PE_LoadU32(GA_D_800B0CD8 + 0x14C);
            {
                uint32_t off = PE_LoadU16(GA_D_800930F0);
                uint32_t end = PE_LoadU16(GA_D_800930F0 + 2);
                r = func_8006E6A8((int)(lba_base + off), desc,
                                  (int)(end - off));
            }
        } while (r == -1);

        for (;;) {
            if (s0 == 0) {
                func_800718D0(PE_LoadU32(GA_D_800B0CD8 + 0x180));
                func_80030894();
                s0 = 1;
            }
            {
                int s2 = func_8006E7E8();
                if (s2 == 0)
                    break;
                if (s2 == -1) {
                    do {
                        desc = PE_LoadU32(GA_D_800B0CD8 + 0x14C);
                        {
                            uint32_t off = PE_LoadU16(GA_D_800930F0);
                            uint32_t end = PE_LoadU16(GA_D_800930F0 + 2);
                            r = func_8006E6A8((int)(lba_base + off), desc,
                                              (int)(end - off));
                        }
                    } while (r == -1);
                }
            }
        }
    }

    /* ── Channel 5: table D_800930E0, with archive lookups ─────────── */
    {
        pe_addr_t desc;
        int s0 = 0;

        do {
            desc = PE_LoadU32(GA_D_800B0CD8 + 0x16C);
            {
                uint32_t off = PE_LoadU16(GA_D_800930E0);
                uint32_t end = PE_LoadU16(GA_D_800930E0 + 2);
                r = func_8006E6A8((int)(lba_base + off), desc,
                                  (int)(end - off));
            }
        } while (r == -1);

        for (;;) {
            if (s0 == 0) {
                pe_addr_t ch4_desc = PE_LoadU32(GA_D_800B0CD8 + 0x14C);
                pe_addr_t r1, r2, r3;
                r1 = func_8006E498(ch4_desc, 0xC4B5BA04u);
                PE_StoreU32(GA_D_800B0CD8 + 0x11C, r1);
                r2 = func_8006E498(ch4_desc, 0xCAAD0704u);
                PE_StoreU32(GA_D_800B0CD8 + 0x120, r2);
                r3 = func_8006E498(ch4_desc, 0x5EAF6804u);
                PE_StoreU32(GA_D_800B0CD8 + 0x124, r3);
                s0 = 1;
            }
            {
                int s2 = func_8006E7E8();
                if (s2 == 0)
                    break;
                if (s2 == -1) {
                    do {
                        desc = PE_LoadU32(GA_D_800B0CD8 + 0x16C);
                        {
                            uint32_t off = PE_LoadU16(GA_D_800930E0);
                            uint32_t end = PE_LoadU16(GA_D_800930E0 + 2);
                            r = func_8006E6A8((int)(lba_base + off), desc,
                                              (int)(end - off));
                        }
                    } while (r == -1);
                }
            }
        }
    }

    /* ── Channel 6: table D_80093126, with GPU packet iteration ────── */
    {
        pe_addr_t desc;
        int s0 = 0;

        do {
            desc = PE_LoadU32(GA_D_800B0CD8 + 0x188);
            {
                uint32_t off = PE_LoadU16(GA_D_80093126);
                uint32_t end = PE_LoadU16(GA_D_80093126 + 2);
                r = func_8006E6A8((int)(lba_base + off), desc,
                                  (int)(end - off));
            }
        } while (r == -1);

        for (;;) {
            if (s0 == 0) {
                pe_addr_t ch5_desc = PE_LoadU32(GA_D_800B0CD8 + 0x16C);
                uint32_t header = PE_LoadU32(ch5_desc + 0x28);
                uint32_t count = header >> 22;
                uint32_t offset = header & 0x003FFFFF;
                pe_addr_t entry = ch5_desc + offset;
                uint32_t i;
                for (i = 0; i < count; i++) {
                    func_8006E1C0(entry, ch5_desc);
                    entry += 0x14;
                }
                s0 = 1;
            }
            {
                int s2 = func_8006E7E8();
                if (s2 == 0)
                    break;
                if (s2 == -1) {
                    do {
                        desc = PE_LoadU32(GA_D_800B0CD8 + 0x188);
                        {
                            uint32_t off = PE_LoadU16(GA_D_80093126);
                            uint32_t end = PE_LoadU16(GA_D_80093126 + 2);
                            r = func_8006E6A8((int)(lba_base + off), desc,
                                              (int)(end - off));
                        }
                    } while (r == -1);
                }
            }
        }
    }

    /* ── Finalization ──────────────────────────────────────────────── */
    func_80087024();
    func_80074DC0(0);       /* DrawSync(0) */
    func_80074A44(1);       /* ResetGraph(1) */
    func_80073A44(0);       /* VSync(0) */

    /* PutDispEnv: retail computes a0 = D_8009CDDC*20 + D_800CCE80 */
    {
        uint32_t idx = PE_LoadU32(GA_D_8009CDDC);
        pe_addr_t env_base = PE_LoadU32(GA_D_800CCE80);
        func_800755F0((void *)(uintptr_t)(env_base + idx * 20));
    }

    func_80074D28(1);       /* SetDispMask(1) */

    /* State writes */
    PE_StoreU16(GA_D_800B0CD8 + 6, 0xFFFF);
    PE_StoreU8(GA_D_800B0CD8 + 0x0B, 0);
    PE_StoreU8(GA_D_800B0CD8 + 0x0C, 0xFF);
    PE_StoreU8(GA_D_800B0CD8 + 0x09, 0xFF);
    PE_StoreU16(GA_D_800B0CD8 + 0xE8, 0xFFFF);
    PE_StoreU8(GA_D_800B0CD8 + 0xEB, 0);
    PE_StoreU8(GA_D_800B0CD8 + 0xEA, 0);

    /* Conditional: if bit 6 clear */
    flags = PE_LoadU32(GA_D_800B0CD8);
    if (!(flags & 0x40)) {
        PE_StoreU8(GA_D_800B0CD8 + 0xDA, 0xFF);
        PE_StoreU8(GA_D_800B0CD8 + 0xDD, 0xFF);
        PE_StoreU8(GA_D_800B0CD8 + 0xDC, 0xFF);
    }

    /* Conditional: if bit 7 clear */
    flags = PE_LoadU32(GA_D_800B0CD8);
    if (!(flags & 0x80)) {
        PE_StoreU8(GA_D_800B0CD8 + 0xDB, 0xFF);
        PE_StoreU8(GA_D_800B0CD8 + 0xDF, 0xFF);
        PE_StoreU8(GA_D_800B0CD8 + 0xDE, 0xFF);
    }

    /* Clear bit 0 */
    flags = PE_LoadU32(GA_D_800B0CD8);
    PE_StoreU32(GA_D_800B0CD8, flags & ~1u);
}
