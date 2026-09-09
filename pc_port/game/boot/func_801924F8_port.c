/*
 * Phase 6E-B54K-AF — authenticated prefix of overlay func_801924F8.
 *
 * Complete retail function: [0x801924F8,0x80192934), 271 words, SHA-256
 * ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a.
 * Translated prefix: [0x801924F8,0x801927B0), 174 words, SHA-256
 * 6cdaa4c16368ca81f53aeb6d194eca170cb13116b7476ad2e1e42f24d5aa9918.
 * Supersedes strict frontier func_801924F8_80192790_cut.
 * Supersedes strict frontier func_801924F8_801927A0_cut.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

/* Stage-1b / MV1d readiness: admit a stream cursor that is safe for
 * live C89C without clamping a1.
 *
 * Immediate pad-exit header (CDQ2d synthetic plant / MV1D_PAD):
 * count-3 < 0 → t5==0, bits>>22 == 0x1FF; or PAD3FF bits>>22 == 0x3FF.
 *
 * Live last-chunk frames are admitted separately via
 * PE_Port_TakeStreamFrameReady (set when E0 promotes on B89F4). Demuxed
 * bodies at s1 are VLC bitstreams — STR magic 0x80010160 lives in the
 * 32-byte sector header, not at the published payload cursor.
 */
static int c89c_stream_is_immediate_pad(uint32_t stream)
{
    uint16_t count_hw;
    uint16_t bits_hi;
    uint16_t bits_lo;
    uint32_t v0;
    uint32_t sym;
    int32_t t2;

    if (stream == 0u || !PE_RangeIsRam((pe_addr_t)stream, 12u))
        return 0;
    count_hw = PE_LoadU16((pe_addr_t)(stream + 6u));
    bits_hi = PE_LoadU16((pe_addr_t)(stream + 8u));
    bits_lo = PE_LoadU16((pe_addr_t)(stream + 10u));
    t2 = (int32_t)count_hw - 3;
    v0 = ((uint32_t)bits_hi << 16) | (uint32_t)bits_lo;
    sym = v0 >> 22;
    if (t2 < 0)
        return (sym ^ 0x1FFu) == 0u;
    return (sym ^ 0x3FFu) == 0u;
}

int func_801924F8(int index)
{
    uint16_t record_index = (uint16_t)index;
    pe_addr_t record;
    pe_addr_t suffix;
    pe_addr_t active_pair;
    uint8_t kind;
    uint16_t movie_x;
    uint16_t movie_y;
    uint32_t display_buffer;
    char filename[32] = "";
    int status;
    int32_t s1 = 0; /* E0 poll result; live into got_frame (retail $s1) */

    if (record_index >= 47u)
        return 0;

    PE_StoreU8(0x800B0DBFu, (uint8_t)index);
    record = 0x801D0E00u + (uint32_t)record_index * 20u;
    PE_StoreU32(0x801D11ACu, record);
    kind = PE_LoadU8(record + 4u);
    PE_StoreU8(0x800B0DBBu, kind);

    func_801918F8(0, (int8_t)kind);
    func_801918F8(1, (int8_t)kind);

    (void)func_800719F4(filename,
                        record_index < 21u ? "\\FMV1" : "\\FMV2");
    suffix = PE_LoadU32(record);
    (void)func_800719F4(
        filename, (const char *)PE_TranslateConst(suffix, 16u));

    do {
        status = 0;
        if (func_8007F72C() == 1 && func_8007F778() == 0)
            status = func_80081414(0x801D0DC4u, filename);
    } while (status == 0 || status == -1);

    PE_StoreU32(0x801D0DDCu, PE_LoadU32(0x801D0DC4u));
    record = PE_LoadU32(0x801D11ACu);
    movie_x = PE_LoadU16(record + 10u);
    movie_y = PE_LoadU16(record + 12u);
    display_buffer = PE_LoadU32(0x800ACDDCu);

    PE_StoreU8(0x801D148Au, (uint8_t)display_buffer);
    PE_StoreU32(0x801D1464u, PE_LoadU32(0x801D0DE8u));
    PE_StoreU8(0x801D146Cu, 0u);
    PE_StoreU32(0x801D1470u, PE_LoadU32(0x801D0DF0u));
    PE_StoreU32(0x801D1474u, PE_LoadU32(0x801D0DF4u));
    PE_StoreU8(0x801D1478u, 0u);
    PE_StoreU16(0x801D147Au, movie_x);
    PE_StoreU16(0x801D1482u, movie_x);
    PE_StoreU32(0x801D1468u, PE_LoadU32(0x801D0DECu));
    PE_StoreU16(0x801D147Cu, (uint16_t)(movie_y + 240u));
    PE_StoreU16(0x801D1484u, movie_y);

    active_pair = 0x801D1464u + ((display_buffer & 0xFFu) << 3);
    PE_StoreU16(0x801D148Cu, PE_LoadU16(active_pair + 22u));
    PE_StoreU16(0x801D148Eu, PE_LoadU16(active_pair + 24u));
    PE_StoreU16(0x801D1490u,
                PE_LoadU8(0x800B0DBBu) != 0u ? 24u : 16u);
    PE_StoreU8(0x801D1494u, 0u);

    func_8010BE3C(0);
    if (PE_Port_ShouldStop())
        return 0;

    func_8010C0D8(0x80191DC8u);

    func_8007A214(PE_LoadU32(0x801D0DFCu), 0x40u);

    record = PE_LoadU32(0x801D11ACu);
    func_8007C304(1u, (int32_t)(int16_t)PE_LoadU16(record + 6u),
                  -1, 0u, 0u);

cdready_wait:
    do {
        while (func_8007F72C() != 1) {
        }
    } while (func_8007F778() != 0);

    /* Retail passes an eight-byte stack result to the blocking command, but
     * the complete 271-word CFG never reads it.  Passing null preserves the
     * live CdlSetloc effect without inventing unobserved response bytes. */
    if (func_80080D5C(2, 0x801D0DC4u, 0u) == 0)
        return 0;

    status = func_80081314(0x801D0DC4u, 0x1E0u);
    if (PE_Port_ShouldStop())
        return 0;
    if (status == 0)
        goto cdready_wait; /* B0-exhaustion: re-poll. */
    /* B8: v0 = [146C]; a1 = film id; mode = 3. */
    func_800870F0(PE_LoadU8(0x800B0DBEu));
    if (PE_Port_ShouldStop())
        return 0;
    /* D0: RLE frame decode (2000 records at D0DF8).  Relanded in
     * CDQ2d with the E0 delivery pump (see below). */
    func_8010BD4C(PE_LoadU32(0x801D0DF8u), 2000u);
    if (PE_Port_ShouldStop())
        return 0;
e0_poll:
    /* E0: poll the streaming slot table; s0 = 2000 tries.  The
     * delivery pump fires the DMA-completion callback once per
     * poll: retail populates streaming slots via DMA-completion
     * interrupts during this spin (7C214, installed by 81314's
     * streaming arm), and the port — with no async interrupts —
     * delivers synchronously instead (the 7ED58 synchronous-reset
     * precedent).  One completion per poll; E0 takes got_frame on
     * the first poll, so cadence beyond that is unobservable. */
    {
        uint32_t s0 = 2000u;
        for (;;) {
            uint32_t left;
            /* Stage-1b promote: retail arms DMA3 IRQ (→7C214) only on
             * the last video chunk (7CEAC interrupt=last → B89F4).
             * Unconditional 7C214 every poll published incomplete bodies
             * and tripped MV1d. Fixtures arm PE_Port_ArmStreamPromote
             * because 7A214 clears B89F4 after the plant. Live Disc1
             * never arms that — HostFB_PumpCdProgress advances one CD
             * sector per poll until 7C564 sets B89F4. (VSync(-1) alone
             * only moves 1024 device cycles; a sector needs ~225k–451k,
             * so E0's 2000-try window could not finish a multi-sector
             * STR frame and live Bazzite stayed nested under 1220C.)
             * A B89F4 promote also latches StreamFrameReady so got_frame
             * may run C89C on the demuxed VLC body (not an immediate pad). */
            {
                int last_chunk = (PE_LoadU32(0x800B89F4u) == 1u);
                if (last_chunk || PE_Port_ConsumeStreamPromote()) {
                    if (last_chunk)
                        PE_Port_NoteStreamFrameReady();
                    func_8007C214();
                } else {
                    HostFB_PumpCdProgress();
                }
            }
            s1 = func_80191B64(0x801D1464u);
            left = s0 - 1u;
            s0 = left;
            if (s1 != 0)
                goto got_frame;
            if (((left << 16) & 0xFFFFFFFFu) != 0u)
                continue;
            break;
        }
    }
    /* Give-up path: copy D0DC4 to D0DDC, CD-ready waits, re-issue Setloc
     * (loc word D0DDC, s2 = READY_FROM_CALLER -1) + ReadN(480), reset s0.
     * The beqz delay (li s0,2000) runs on both outcomes, so s0 is always
     * 2000 here: status 0 reaches the 9289C wait, nonzero reaches E0. */
    for (;;) {
        PE_StoreU32(0x801D0DDCu, PE_LoadU32(0x801D0DC4u));
        while (func_8007F72C() != -1) {
        }
        while (func_8007F778() != 0) {
        }
        func_80080D5C(2, 0x801D0DDCu, 0u);
        status = func_80081314(0x801D0DDCu, 480u);
        if (PE_Port_ShouldStop())
            return 0;
        if (status == 0)
            break;
        goto e0_poll;
    }
    goto cdready_wait;
got_frame:
    /* 80192814: s1 nonzero.  Retail got_frame tail (ov133 carve,
     * docs/evidence/pe-mv1c-c89c-map/NOTE.md): bump [B0DBC], load
     * a1 = [0x801D1464 + ([146C]^1)*4] then toggle [146C] (first pass
     * loads [0x801D1468]), a2 = [0x801D0DF8], jal func_8010C89C(a0=s1,
     * a1, a2, 0), then jal func_8007C394(s1) and the EC stores
     * (sb 0,[B0DBD] @801928F8; sh 1,[B0DBC] @80192908).  The decoder
     * itself was transcribed in MV1d; this wires the production call
     * path.  The intervening slice-wait between 7C394 and the EC
     * stores is not yet expanded here — EC runs immediately so the
     * control frontier can leave the old C89C stub.
     *
     * DAY2-158c gate (Bazzite PE_StoreU16@0x80200000): live Disc1 aborts
     * are the known MV1d trap — guest RAM ends at 0x80200000; wiring
     * C89C without a pad-terminated / Stage-1b-ready frame at s1 lets
     * the output cursor walk off the 2MiB window
     * (docs/evidence/pe-mv1d-c89c/REPORT.md).  Not a random 1220C buffer
     * bug.  Stage-1b: E0 promotes only when B89F4 marks a last-chunk frame
     * (or a fixture surrogate arm); C89C runs on immediate pad OR when
     * that last-chunk latch is set. Do not clamp a1. */
    {
        uint16_t count = PE_LoadU16(0x800B0DBCu);
        uint32_t flip = (uint32_t)PE_LoadU8(0x801D146Cu) ^ 1u;
        pe_addr_t out;
        pe_addr_t table;
        uint32_t stream = (uint32_t)s1;
        int frame_ready = PE_Port_TakeStreamFrameReady();

        PE_StoreU16(0x800B0DBCu, (uint16_t)(count + 1u));
        out = (pe_addr_t)PE_LoadU32(0x801D1464u + flip * 4u);
        PE_StoreU8(0x801D146Cu, (uint8_t)flip);
        table = (pe_addr_t)PE_LoadU32(0x801D0DF8u);
        if (!c89c_stream_is_immediate_pad(stream) && !frame_ready) {
            Bootstrap_ReturnVoid("Stage1b_pad_terminated_frame",
                                 "func_801924F8");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
        (void)func_8010C89C(stream, out, table, 0u);
        if (PE_Port_ShouldStop())
            return 0;
        func_8007C394(stream);
        PE_StoreU8(0x800B0DBDu, 0u);
        PE_StoreU16(0x800B0DBCu, 1u);
    }
    return 0;
}
