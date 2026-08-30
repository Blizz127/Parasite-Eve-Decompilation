/*
 * Phase 6E-B54K-B2 — func_80030894: boot GPU-primitive builder,
 * prologue + bank-0 L2/L3, L4, and L5 packet groups (translated prefix).
 *
 * Full retail body:
 *   788 words / 0xC50 bytes, exe 0x80030894–0x800314E4 (exclusive),
 *   file offset 0x21094, window SHA-256
 *   a4dbd2cf130979a0f5db8ed532d0c559c5b3b90b2c5786e10fe91125311ed6e2.
 *   Sole caller: jal func_8006AD40 @ 0x8006B0AC.  ABI void(void)
 *   (backward liveness fixpoint in the B54J audit; $v0 at exit is a
 *   stale scratch discarded by the caller).
 *
 * Implemented prefix:
 *   0x80030894..0x80030D20 (291 words) — prologue, the bank record
 *   init at 0x800BE9F0, and the L2(j=0..9) × L3(k=0..3) sprite-array
 *   build at 0x800B01C0, followed by the complete L4 packet group
 *   (119 words / 0x1DC bytes; SHA-256
 *   592dc73fa08202d91812b463aa71ef93cafd3af7713a0724d61e0815b17bc3ec)
 *   and the five-entry L5 sprite array (32 words / 0x80 bytes; SHA-256
 *   b45f5a6c9a6d1565f3fcc6affce6edc91a679ebe2bd538734d75fd2c933575d0).
 *   The first excluded instruction is
 *
 *       lbu   s5, 24(sp)                # 0x80030D20
 *
 *   (next packet-group setup).  The named strict boundary is
 *   func_80030894_L5_cut.
 *
 * Word decode of the implemented window (verified against the
 * SHA-1-exact retail executable 452fb033f2eaa4b18aa20a5bca60b8125af3a37b):
 *
 *   prologue (0x80030894..0x8003090C):
 *     frame 88B; ra@84, fp@80, s7@76 .. s0@48 (sp-relative)
 *     sp+16/17/18 <- sign-extended lb 0x8009CD90+0/+1/+2 (font triple;
 *                    dead in this window — no reader before 0x80030AC4)
 *     fp  <- GetTPage(0,1,0x100,0x1E0) & 0xFFFF = 0x34   (delay slot of
 *            the GetClut jal at 0x800308FC consumes the GetTPage return)
 *     s7  <- 0x80 (vertex byte; first used by B54K-B groups)
 *     sp+32 (half) <- GetClut(0x130,0x1F8) = 0x7E13
 *     sp+24 (byte) <- 0 (outer bank counter i; ++ @0x80031484 and
 *                     test <2 @0x800314A8 live in B54K-B's epilogue)
 *
 *   bank record init (0x80030910..0x80030A14), per bank i (i = 0 here):
 *     s3  <- func_8005DADC(139)  == *(u32*)0x800A8030 + 0x800A8028 + 1112
 *     s0  <- 0x800BE9F0 + i*40  (record table; the audit's corrected
 *            lui 0x800C / addiu -0x1610 resolution)
 *     func_80077BA4(s0)                       SetPolyFT4 header
 *     sb s0+0x0C <- lbu(s3+0)                  u1
 *     sb s0+0x0D <- lbu(s3+1)                  v1
 *     sb s0+0x14 <- (lbu(s3+0)+lbu(s3+4))&0xFF u2
 *     sb s0+0x15 <- lbu(s3+1)                  v2
 *     sb s0+0x1C <- lbu(s3+0)                  u3
 *     sb s0+0x1D <- (lbu(s3+1)+lbu(s3+5))&0xFF v3
 *     sb s0+0x24 <- (lbu(s3+0)+lbu(s3+4))&0xFF u4
 *     sb s0+0x25 <- (lbu(s3+1)+lbu(s3+5))&0xFF v4 (delay slot of the
 *            second GetTPage jal — stores the PRE-call adder value)
 *     sh s0+0x16 <- GetTPage(0,0,0x1C0,0)
 *     sh s0+0x0E <- lhu(s3+2)                  clut id
 *     a1 <- 1 at 0x800309D8 — AFTER the second GetTPage call (which
 *           clobbers a1 to 0) and BEFORE SetSemiTrans, so the retail
 *           argument is abr=1
 *     sh zero -> s0+8,+0xA,+0x10,+0x12,+0x18,+0x1A,+0x20,+0x22
 *     sb zero -> s0+4,+5,+6
 *     func_80077B04(s0, 1)                    SetSemiTrans ON
 *
 *   L2/L3 sprite array (0x80030A18..0x80030AC0):
 *     s5 <- i*1400  (((i*3*4 - i)*16 - i)*8, instruction-exact)
 *     s6 <- 0 (j), s3 <- 0 (k)
 *     L3 body: s1 <- j*140 (((j*8+j)*4-j)*4)
 *              s0 <- k*28  (((k*8-k))*4... == (k*8-k)*4)
 *              a0 <- 0x800B01C0 + s5 + s1 + s0
 *              sp+40 word <- i*12 (v1 save; restored after the call;
 *                              host-stack only, not guest-observable)
 *              func_800370DC(a0, fp)          wrap_sprt twin
 *              sh 0x7E13 -> 0x800B0000 + (s5+s1+s0) + 0x1D6
 *                         == packet + 0x16   clut halfword
 *     L3: k < 4 (sltiu), L2: j < 10 (sltiu), counters &0xFF.
 *
 *   L5 sprite array (0x80030CA0..0x80030D1C):
 *     bank stride 140 (((i*8+i)*4-i)*4), item stride 28
 *     func_800370DC(0x8009E1D0 + i*140 + j*28, fp), j=0..4
 *     sh clut -> packet+0x16; tail dimensions <- 6 x 10
 *     sole back-edge uses literal sltiu <5; the next group starts at D20.
 *
 * All callees are native since B54I/GPU1 (GetTPage, GetClut,
 * SetPolyFT4, SetSemiTrans, func_8005DADC, func_800370DC); the L2L3
 * group is followed by B54K-B1/B2's native L4/L5 calls (wrap_tile,
 * SetTile, SetPolyG4, and wrap_sprt).  The L5 cut is the first
 * untranslated boundary inside this body.
 *
 * Classification: 1 — translated retail prefix.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GA_8009CD90      0x8009CD90u
#define GA_RECORD_TABLE  0x800BE9F0u
#define GA_SPRITE_BASE   0x800B01C0u
#define GA_L4_TILE_HEAD  0x8009E068u
#define GA_L4_TILE       0x8009E098u
#define GA_L4_POLYG4     0x800B00E8u
#define GA_L4_SPRITE     0x800B6920u
#define GA_L4_ARRAY      0x8009E0F0u
#define GA_L5_ARRAY      0x8009E1D0u

void func_80030894(void)
{
    uint32_t tpage_sprt;   /* s8: sprite tpage mode for wrap_sprt */
    uint16_t clut_id;      /* sp+32 half */
    uint8_t bank = 0u;     /* sp+24 byte: outer bank counter */

    /* 0x800308E0..0x8003090C prologue vectors.  The font triple loads
     * (sp+16..18) are dead in this window; sp+16..18 hold the
     * sign-extended bytes but nothing reads them before the cut. */
    tpage_sprt = func_80077A64(0u, 1u, 0x100u, 0x1E0u) & 0xFFFFu;
    clut_id = (uint16_t)func_80077AA4(0x130, 0x1F8);

    /* 0x80030910.. bank record init (bank 0). */
    {
        pe_addr_t rec = func_8005DADC(139u);            /* s3 */
        pe_addr_t rec_tab = GA_RECORD_TABLE +
                            (pe_addr_t)((uint32_t)bank * 40u); /* s0 */
        uint32_t u0v0_sum;                               /* delay-slot adder */
        uint32_t s5;                                     /* i*1400 */
        uint32_t j, k;

        func_80077BA4(rec_tab);                          /* SetPolyFT4 */

        PE_StoreU8(rec_tab + 0x0Cu, PE_LoadU8(rec + 0u));
        PE_StoreU8(rec_tab + 0x0Du, PE_LoadU8(rec + 1u));
        PE_StoreU8(rec_tab + 0x14u, (uint8_t)(
                   (uint32_t)PE_LoadU8(rec + 0u) + (uint32_t)PE_LoadU8(rec + 4u)));
        PE_StoreU8(rec_tab + 0x15u, PE_LoadU8(rec + 1u));
        PE_StoreU8(rec_tab + 0x1Cu, PE_LoadU8(rec + 0u));
        PE_StoreU8(rec_tab + 0x1Du, (uint8_t)(
                   (uint32_t)PE_LoadU8(rec + 1u) + (uint32_t)PE_LoadU8(rec + 5u)));
        PE_StoreU8(rec_tab + 0x24u, (uint8_t)(
                   (uint32_t)PE_LoadU8(rec + 0u) + (uint32_t)PE_LoadU8(rec + 4u)));
        u0v0_sum = (uint32_t)PE_LoadU8(rec + 1u) +
                   (uint32_t)PE_LoadU8(rec + 5u);
        PE_StoreU8(rec_tab + 0x25u, (uint8_t)u0v0_sum);  /* delay-slot store */

        PE_StoreU16(rec_tab + 0x16u,
                    (uint16_t)func_80077A64(0u, 0u, 0x1C0u, 0u));
        PE_StoreU16(rec_tab + 0x0Eu, PE_LoadU16(rec + 2u));

        /* 0x800309E8..0x80030A0C: zero the xy halfword pairs and the
         * u0/v0/clut-low bytes. */
        PE_StoreU16(rec_tab + 0x08u, 0u);
        PE_StoreU16(rec_tab + 0x0Au, 0u);
        PE_StoreU16(rec_tab + 0x10u, 0u);
        PE_StoreU16(rec_tab + 0x12u, 0u);
        PE_StoreU16(rec_tab + 0x18u, 0u);
        PE_StoreU16(rec_tab + 0x1Au, 0u);
        PE_StoreU16(rec_tab + 0x20u, 0u);
        PE_StoreU16(rec_tab + 0x22u, 0u);
        PE_StoreU8(rec_tab + 0x04u, 0u);
        PE_StoreU8(rec_tab + 0x05u, 0u);
        PE_StoreU8(rec_tab + 0x06u, 0u);
        func_80077B04(rec_tab, 1u);                      /* SetSemiTrans(.,1) */

        /* 0x80030A18..0x80030AC0: L2 (j<10) x L3 (k<4) sprite array. */
        s5 = (uint32_t)bank * 1400u;
        for (j = 0u; j < 10u; j++) {                     /* s6 */
            for (k = 0u; k < 4u; k++) {                  /* s3 */
                uint32_t off = s5 + j * 140u + k * 28u;

                func_800370DC(GA_SPRITE_BASE + off, tpage_sprt);
                /* 0x80030AA0: sh clut -> 0x800B0000 + off + 0x1D6. */
                PE_StoreU16(0x800B0000u + off + 0x1D6u, clut_id);
            }
        }
    }

    /* B54K-B1, retail 0x80030AC4..0x80030C9C: complete L4 group. */
    {
        uint32_t tile_mode =
            func_80077A64(0u, 0u, 0u, 0u) & 0xFFFFu;
        pe_addr_t tile_head = GA_L4_TILE_HEAD + (uint32_t)bank * 24u;
        pe_addr_t tile = tile_head + 8u;
        pe_addr_t standalone_tile = GA_L4_TILE + (uint32_t)bank * 16u;
        pe_addr_t poly = GA_L4_POLYG4 + (uint32_t)bank * 36u;
        pe_addr_t sprite_head = GA_L4_SPRITE + (uint32_t)bank * 28u;
        pe_addr_t sprite = sprite_head + 8u;
        uint32_t j;

        func_80037140(tile_head, tile_mode);
        PE_StoreU8(tile + 4u, 0x30u);
        PE_StoreU8(tile + 5u, 0x30u);
        PE_StoreU8(tile + 6u, 0x30u);
        func_80077B04(tile, 1u);

        func_80077C44(standalone_tile);
        PE_StoreU8(standalone_tile + 4u, 0x1Du);
        PE_StoreU8(standalone_tile + 5u, 0x3Eu);
        PE_StoreU8(standalone_tile + 6u, 0x32u);
        PE_StoreU16(standalone_tile + 0x0Cu, 0x38u);
        PE_StoreU16(standalone_tile + 0x0Eu, 3u);

        func_80077BC4(poly);
        func_800370DC(sprite_head, tpage_sprt);
        PE_StoreU8(sprite + 0x0Cu, 0xC8u);
        PE_StoreU8(sprite + 0x0Du, 0xE0u);
        PE_StoreU16(sprite_head + 0x16u, clut_id);
        PE_StoreU16(sprite + 0x10u, 4u);
        PE_StoreU16(sprite + 0x12u, 8u);

        PE_StoreU8(poly + 0x06u, 0x82u);
        PE_StoreU8(poly + 0x0Du, 0xFFu);
        PE_StoreU8(poly + 0x16u, 0x82u);
        PE_StoreU8(poly + 0x04u, 0u);
        PE_StoreU8(poly + 0x05u, 0x46u);
        PE_StoreU8(poly + 0x0Cu, 0x9Fu);
        PE_StoreU8(poly + 0x0Eu, 0xF9u);
        PE_StoreU8(poly + 0x14u, 0u);
        PE_StoreU8(poly + 0x15u, 0x46u);
        PE_StoreU8(poly + 0x1Cu, 0x9Fu);
        PE_StoreU8(poly + 0x1Du, 0xFFu);
        PE_StoreU8(poly + 0x1Eu, 0xF9u);

        PE_StoreU8(sprite + 0x04u, 0x9Fu);
        PE_StoreU8(sprite + 0x05u, 0xFFu);
        PE_StoreU8(sprite + 0x06u, 0xF9u);

        for (j = 0u; j < 4u; j++) {
            pe_addr_t head = GA_L4_ARRAY + (uint32_t)bank * 112u + j * 28u;
            pe_addr_t sprt = head + 8u;

            func_800370DC(head, tpage_sprt);
            PE_StoreU16(head + 0x16u, clut_id);
            PE_StoreU16(sprt + 0x10u, 6u);
            PE_StoreU16(sprt + 0x12u, 10u);
        }
    }

    /* B54K-B2, retail 0x80030CA0..0x80030D1C: five-entry L5 array. */
    {
        uint32_t j;

        for (j = 0u; j < 5u; j++) {
            pe_addr_t head = GA_L5_ARRAY + (uint32_t)bank * 140u + j * 28u;
            pe_addr_t sprt = head + 8u;

            func_800370DC(head, tpage_sprt);
            PE_StoreU16(head + 0x16u, clut_id);
            PE_StoreU16(sprt + 0x10u, 6u);
            PE_StoreU16(sprt + 0x12u, 10u);
        }
    }

    /* B54K-B2 cut: stop before the next group at retail 0x80030D20. */
    (void)Bootstrap_ReturnInt(
        "func_80030894_L5_cut", "func_80030894", 0);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
