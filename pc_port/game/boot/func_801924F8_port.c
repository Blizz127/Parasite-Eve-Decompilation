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

static void func_8010BD4C(pe_addr_t a0, uint32_t a1)
{
    (void)a0;
    (void)a1;
    Bootstrap_ReturnVoid("func_8010BD4C", "func_801924F8");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
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
    /* D0: RLE frame decode (2000 records at D0DF8). Decoder is MV1b. */
    func_8010BD4C(PE_LoadU32(0x801D0DF8u), 2000u);
    if (PE_Port_ShouldStop())
        return 0;
e0_poll:
    /* E0: poll the streaming slot table; s0 = 2000 tries. */
    {
        uint32_t s0 = 2000u;
        int32_t s1;
        for (;;) {
            uint32_t left;
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
    /* 80192814: s1 nonzero. The frame path computes a1 = lw[s3 + ([146C]^1)*4]
     * with s3 = 0 (sole-caller value, see above): a KUSEG vector-range load
     * the port cannot execute, and hands (s1, a1) to the MV1b decoder.
     * Stop at the decoder entry; the post-decode half (EC: v0 = 1, clear
     * DBD, B0DBC = 1, B0DBA++) resumes in MV1b with the decoder. */
    Bootstrap_ReturnVoid("func_8010C89C", "func_801924F8");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
