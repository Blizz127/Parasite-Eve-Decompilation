/*
 * Overlay-local func_80192934 — media worker called from the 80192CE8
 * post-E08 loop.
 *
 * Retail span: [0x80192934,0x80192C48), 197 words, SHA-256
 * 9a88d5065fe2f4dc9fa860771a755eec3c53dfef824fc1bbec0df7a8b3991780.
 * Siblings through 0x80192CE8 (abort helper 80192C48 + DC0 latch
 * 80192C9C): docs/evidence/pe-92934-media-worker.
 *
 * Entry gate: D_800B0DBA < 2 returns 0 (boot path after 91FB8).
 * Body from 0x80192960: 918F8 flip / BFA0 / C01C / 91B64 poll /
 * C89C+7C394 on got-frame / CD reissue / 1494 wait / abort teardown.
 *
 * LIVE C89C WIRED (DAY2-159c): got-frame mirrors func_801924F8's
 * authenticated tail — E0-style delivery pump on the device path, and the
 * same complete-frame gate (`PE_Movie_LastPublishComplete()`) before
 * entering func_8010C89C + func_8007C394.  A surrogate publish keeps the
 * named boundary instead of decoding an unpopulated body (MV1d trap).
 *
 * Documented callees:
 *   918F8 / 0BFA0 / C01C / 91B64 / C89C / 7C394 — ported
 *   CD reissue — 7C2A0 / 7F72C / 7F778 / 80D5C / 81314
 *   abort teardown — C0D8 / 7A2A4 / 80DC4; sibling 92C48 adds 870F0
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_cdreg.h"
#include "pe_port_compat.h"
#include "host_framebuffer.h"

/* Same Stage-1b / MV1d honesty as func_801924F8_port.c: the decoder tail
 * is not entered until a full STR frame is delivered at the published
 * cursor.  See the got-frame note in the body. */

extern int func_8010C89C(uint32_t a0, pe_addr_t a1, pe_addr_t a2, uint32_t a3);
extern void func_8007C394(uint32_t stream);
extern void func_8010BFA0(pe_addr_t command_block, uint32_t mode);
extern void func_8010C01C(pe_addr_t destination, uint32_t words);
extern int func_80191B64(pe_addr_t state);
extern int32_t func_8007C2A0(pe_addr_t location);
extern void func_8007A2A4(void);
extern int func_80080DC4(int command, pe_addr_t param, pe_addr_t response);

int func_80192934(void)
{
    pe_addr_t pair_base = 0x801D1464u;
    pe_addr_t flip_cell;
    pe_addr_t buf;
    int32_t words;
    int32_t poll_left;
    int frame;
    int16_t status16;
    uint8_t flip;

    if (PE_LoadU8(0x800B0DBAu) < 2u)
        return 0;

    /* Dig aid (pe-day2-158-74520-dma): entry had no TRACE, so live dumps
     * looked like "92934 skipped" while the post-DBA++ path stopped inside
     * the first BFA0/C01C → 74520 → 91DC8 cycle. */
    Trace_Direct("func_80192934_enter");

    /* ---- body @ 0x80192960 ---- */
    if (PE_LoadU8(0x801D0DC0u) == 2u) {
        int8_t buf_sel = (int8_t)((PE_LoadU32(0x800ACDDCu) ^ 1u) << 24 >> 24);
        func_801918F8((int)buf_sel, (int)(int8_t)PE_LoadU8(0x800B0DBBu));
        if (PE_Port_ShouldStop())
            return 0;
        PE_StoreU8(0x801D0DC0u, 0u);
    }

    /* Unaligned 4-byte copy 801D0DC4 -> 801D0DDC (retail lwl/lwr). */
    PE_StoreU32(0x801D0DDCu, PE_LoadU32(0x801D0DC4u));

    flip = PE_LoadU8(0x801D146Cu);
    buf = PE_LoadU32(pair_base + (uint32_t)flip * 4u);
    func_8010BFA0(buf, PE_LoadU8(0x801D0DBEu));
    if (PE_Port_ShouldStop())
        return 0;

    {
        int32_t w = (int32_t)(int16_t)PE_LoadU16(0x801D1490u);
        int32_t h = (int32_t)(int16_t)PE_LoadU16(0x801D1492u);
        int32_t product = w * h;
        words = (product + (product >> 31)) >> 1;
    }
    flip_cell = 0x801D146Cu + (uint32_t)PE_LoadU8(0x801D1478u) * 4u;
    buf = PE_LoadU32(flip_cell + 4u);
    func_8010C01C(buf, (uint32_t)words);
    if (PE_Port_ShouldStop())
        return 0;

    poll_left = 0x7d0; /* 2000 */
poll_again:
    for (;;) {
        /* Delivery pump mirrors func_801924F8's E0 poll.  On a real
         * drive the port advances the modeled CD device one poll
         * quantum at a time (HostFB_StreamTick -> PE_CdReg_ServiceDevice
         * + IRQ service), driving the retail chain data-ready IRQ ->
         * 813E8 -> 7C564 -> DMA3 -> 7C214, which publishes a complete
         * record as state 2.  Test fixtures with no device call
         * func_8007C214 directly (the documented synchronous-delivery
         * surrogate); that publish is not a last-chunk delivery, so the
         * got-frame gate below keeps the honest decoder boundary. */
        if (PE_CdReg_DeviceEnabled())
            HostFB_StreamTick();
        else
            func_8007C214();
        frame = func_80191B64(pair_base);
        if (PE_Port_ShouldStop())
            return 0;
        if (frame != 0)
            break;
        poll_left -= 1;
        if ((int32_t)(poll_left << 16) != 0)
            continue;
        status16 = -1;
        goto after_frame;
    }

    /* got-frame @ 0x80192A68 — authenticated C89C tail, same gate as
     * func_801924F8.  Retail bumps DBC, toggles 146C, loads
     * a1 = [0x801D1464 + (146C^1)<<2], a2 = [0x801D0DF8], then calls the
     * VLC decoder func_8010C89C(a0 = s1) and func_8007C394(s1).  The
     * decoder's output cursor is bounded only by the VLC stream's own
     * pad/terminator codes, so it must only ever see a complete STR
     * frame: a partial body walks a1 past the 2 MiB guest window (MV1d
     * trap).  PE_Movie_LastPublishComplete() is true only when the
     * published record came from a last video chunk; a surrogate publish
     * that did not keeps the named boundary instead of decoding an
     * unpopulated body.
     *
     * Unlike 924F8, retail 92934 has NO EC stores here: after 7C394 it
     * falls straight into the success path with status16 = 0. */
    {
        uint16_t count = PE_LoadU16(0x800B0DBCu);
        uint32_t next_flip = (uint32_t)PE_LoadU8(0x801D146Cu) ^ 1u;
        pe_addr_t out;
        pe_addr_t table;
        uint32_t stream = (uint32_t)frame;

        if (!PE_Movie_LastPublishComplete()) {
            Bootstrap_ReturnVoid("func_8010C89C", "func_80192934");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
        PE_StoreU16(0x800B0DBCu, (uint16_t)(count + 1u));
        PE_StoreU8(0x801D146Cu, (uint8_t)next_flip);
        out = (pe_addr_t)PE_LoadU32(pair_base + next_flip * 4u);
        table = (pe_addr_t)PE_LoadU32(0x801D0DF8u);
        if (!PE_RangeIsRam((pe_addr_t)stream, 2u) ||
            !PE_RangeIsRam(out, 0x11000u) ||
            !PE_RangeIsRam(table, 0x10800u)) {
            Bootstrap_ReturnVoid("func_8010C89C_arena", "func_80192934");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
        (void)func_8010C89C(stream, out, table, 0u);
        if (PE_Port_ShouldStop())
            return 0;
        func_8007C394(stream);
        status16 = 0;
        /* fall through to the success path at after_frame */
    }

after_frame:
    /* CD reissue when 91B64 poll exhausted (status16 == -1). */
    if (status16 == -1) {
        (void)func_8007C2A0(0x801D0DDCu);
        for (;;) {
            if (PE_Port_ShouldStop())
                return 0;
            if (func_8007F72C() != 1)
                continue;
            if (func_8007F778() != 0)
                continue;
            /* Retail a2 = sp+0x10; response unread (924F8 uses 0). */
            (void)func_80080D5C(2, 0x801D0DDCu, 0u);
            if (PE_Port_ShouldStop())
                return 0;
            if (func_80081314(0x801D0DDCu, 0x1E0u) == 0)
                continue;
            if (PE_Port_ShouldStop())
                return 0;
            break;
        }
        poll_left = 0x7d0;
        goto poll_again;
    }

    /* Success / 1494 wait / abort — retail after C89C+7C394 (cut above). */
    {
        int s3 = 0;
        uint32_t countdown = (uint32_t)(int32_t)status16;

        if (PE_LoadU8(0x801D1494u) == 0u) {
            for (;;) {
                countdown -= 1u;
                if (countdown == 0u) {
                    uint8_t disp = (uint8_t)(PE_LoadU8(0x801D148Au) ^ 1u);
                    pe_addr_t pair = pair_base + ((uint32_t)disp << 3);
                    PE_StoreU8(0x801D1494u, 1u);
                    PE_StoreU8(0x801D148Au, disp);
                    PE_StoreU16(0x801D148Cu, PE_LoadU16(pair + 0x16u));
                    PE_StoreU16(0x801D148Eu, PE_LoadU16(pair + 0x18u));
                }
                if (PE_LoadU8(0x801D1494u) != 0u)
                    break;
                HostFB_VSync(-1); /* host: interrupt-wait needs a pump */
                if (PE_Port_ShouldStop())
                    return 0;
            }
        }

        PE_StoreU8(0x801D1494u, 0u);
        if (PE_LoadU8(0x801D0DBDu) == 1u)
            s3 = 1;
        if (s3 == 0)
            return 1;

        /* Dig aid: sticky D0DBD from first 91B64 was ending FMV after the
         * first media-loop frame (pe-day2-158-909b4-early-title-cut). */
        Trace_Direct("func_80192934_dbd_abort");
        PE_StoreU8(0x800B0DBAu, (uint8_t)(PE_LoadU8(0x800B0DBAu) - 1u));
        func_8010C0D8(0u);
        if (PE_Port_ShouldStop())
            return 0;
        func_8007A2A4();
        if (PE_Port_ShouldStop())
            return 0;
        (void)func_80080DC4(9, 0u, 0u);
        return 0;
    }
}

/* Sibling @ 0x80192C48 — abort teardown with 870F0. */
void func_80192C48(void)
{
    PE_StoreU8(0x800B0DBAu, (uint8_t)(PE_LoadU8(0x800B0DBAu) - 1u));
    func_800870F0(0u);
    if (PE_Port_ShouldStop())
        return;
    func_8010C0D8(0u);
    if (PE_Port_ShouldStop())
        return;
    func_8007A2A4();
    if (PE_Port_ShouldStop())
        return;
    (void)func_80080DC4(9, 0u, 0u);
}

/* Sibling @ 0x80192C9C — latch 801D0DC0=1 when (a0!=0) XOR (DBB!=0). */
void func_80192C9C(int flag)
{
    int a0_nz = ((flag << 24) != 0);
    int dbb_nz = ((int8_t)PE_LoadU8(0x800B0DBBu) != 0);
    if (a0_nz != dbb_nz)
        PE_StoreU8(0x801D0DC0u, 1u);
}
