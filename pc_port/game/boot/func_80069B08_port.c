/*
 * func_80069B08 — boot disc/status screen state machine (translated retail,
 * not matching src/).  Authority: retail SLUS_006.62 SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b; body
 * asm/disc1/56438.s 0x80069B08..0x8006A0E8, 0x5E0 bytes / 376 words.
 *
 * Sole caller: func_8001220C @0x800122A0, guarded by the sticky dispatch
 * bit 0x00100000 in D_800B0CD8; a0 is the caller's dispatch mode (1 or 2).
 * Returns 0 (retail v0 is not consumed by the caller).
 *
 * Structure, instruction-for-instruction:
 *
 *  1. Two blocking PE.IMG sector reads from the base LBA at D_800B0DD8,
 *     each with a start/end halfword pair from the D_800930D8 table and a
 *     destination pointer: chunk 0 = D_800930D8[0..2) -> [D_800B0E64],
 *     chunk 1 = D_800930DA[0..2) -> [D_800B0E6C].  The issue/poll shape is
 *     the same restart-on-timeout loop the neighbouring 6B4F8/6AD40 loaders
 *     use: reissue while func_8006E6A8 returns -1, then poll func_8006E7E8
 *     (0 done, -1 reissue the read, else keep polling).
 *  2. Bring the loaded UI up: func_8003E974 (bit-table init),
 *     func_800371B0([D_800B0E6C]) (rebind message data), func_800718D0 on
 *     the chunk-0 TIM, ClearImage (0,0,320,448) to black, and the
 *     double-buffered PutDispEnv for bank D_8009CDDC.
 *  3. Run the CD-status state machine (cases 0..9 via jtbl_80011388) one
 *     frame at a time until the done flag (s6) is set in case 4.  The first
 *     command is CdlStop (8) via func_8007EE84, polled with func_8007F418.
 *     Case 2 then holds until the CD status byte reports CdlStatShellOpen
 *     (0x10 — "once shell open", PsyQ libcd.h), i.e. until the player has
 *     opened the lid to swap discs; case 3/4 then re-check CdReady
 *     (func_8007F72C) and the mounted disc (func_800698D4), and the
 *     0xB4-frame timer (s1) opens message records (func_800375E0 ids 1..5).
 *     Case 4 finishes when the caller's mode matches D_800B0DCD bit
 *     (mode==1 -> bit0, mode==2 -> bit1), which func_800698D4 sets from the
 *     inserted disc's PEDISC01/02.IDF identity files.
 *
 *     This makes the function the retail disc-change screen: it is only
 *     entered when the inserted disc does NOT match the wanted mode (the
 *     func_8001220C gate skips it when D_800B0DCD already has the mode bit),
 *     and case 4 cannot set done until the matching disc is inserted after a
 *     shell-open event.  The host drive model has no lid/cover input, so on
 *     the Disc 2 image (mode 1 wanted, disc 2 inserted) the state machine is
 *     a genuine retail wait loop.  The host bound below (the same precedent
 *     as func_8001220C's PE_PORT_DISC_WAIT_LIMIT) returns on a host stop
 *     request instead of spinning forever; it never manufactures a return
 *     value or a screen effect.
 *  4. Per-frame draw tail: func_80037870, DrawSync, VSync, ResetGraph(1),
 *     PutDispEnv/PutDrawEnv for the current bank, MoveImage of the
 *     (0x140,0x100,0x140,0xE0) rect, DrawOTag, then toggle D_8009CDDC.
 *  5. On exit: DrawSync, SetDispMask(0), clear (0,0,320,448), publish
 *     D_800B0DCD = mode, spin on CdReady == 1, and store
 *     D_800B0DD4 = func_8007F7A8() (the CD status getter).
 *
 * Guest scratch: retail keeps the 8-byte poll response at sp+0x18 and the
 * single -1 message list terminator at sp+0x20.  The port has no guest
 * stack, so both live in the free 0x801FF040..0x801FFE00 scratch band
 * (distinct from func_80042020/func_80041108's 0x801FF600 formatter slot).
 * func_8007F418 stores the response and func_800375E0 reads the terminator
 * list, so both must be real guest addresses.
 *
 * No callee is unported: every call below reaches an existing native port
 * or SDK translation, so this file contains no bootstrap boundary.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GA_D_800B0DD8  0x800B0DD8u  /* PE.IMG base LBA                     */
#define GA_D_800B0E64  0x800B0E64u  /* chunk-0 destination pointer          */
#define GA_D_800B0E6C  0x800B0E6Cu  /* chunk-1 destination pointer          */
#define GA_D_800930D8  0x800930D8u  /* {start,end} pair, chunk 0            */
#define GA_D_800930DA  0x800930DAu  /* {start,end} pair, chunk 1            */
#define GA_D_8009CDDC  0x8009CDDCu  /* display-bank index                   */
#define GA_D_800B0E38  0x800B0E38u  /* ordering-table pointers per bank     */
#define GA_D_800BCE80  0x800BCE80u  /* DISP_ENV array (stride 20)           */
#define GA_D_800BCDC8  0x800BCDC8u  /* DRAW_ENV array (stride 92)           */
#define GA_D_800B0DCD  0x800B0DCDu  /* published disc/mode byte             */
#define GA_D_800B0DD4  0x800B0DD4u  /* published CD status halfword         */
#define GA_OT_SIZE     0x1000       /* ClearOTagR entry count               */
#define GA_TIMER       0xB4         /* state-machine frame timer            */

#define GA_SM_RESPONSE   0x801FF4C0u /* 8-byte func_8007F418 snapshot      */
#define GA_SM_TERMINATOR 0x801FF4C8u /* -1 message-list terminator         */

extern unsigned char func_8007F788(void);

/* Retail .L80069B5C/.L80069B98 (chunk 0) and .L80069BBC/.L80069BF8 (chunk 1):
 * issue the [start,end) halfword range at `range` from the PE.IMG base LBA to
 * `dest_slot`, then poll.  A poll of 0 completes the read; -1 restarts it. */
static void pe_69b08_read_chunk(pe_addr_t range, pe_addr_t dest_slot)
{
    int status;

    for (;;) {
        do {
            uint16_t start = PE_LoadU16(range);
            uint16_t end = PE_LoadU16(range + 2u);

            status = func_8006E6A8((int)(PE_LoadU32(GA_D_800B0DD8) + start),
                                   PE_LoadU32(dest_slot), (int)(end - start));
        } while (status == -1);

        for (;;) {
            status = func_8006E7E8();
            if (status == 0)
                return;
            if (status == -1)
                break;
        }
    }
}

/* Retail .L80069F2C (also reached from .L80069F24): open the mode's message
 * record.  The list is the single -1 terminator prepared before the loop. */
static void pe_69b08_open_result_message(int mode)
{
    func_800375E0((mode == 1) ? 1 : 2, 0u, GA_SM_TERMINATOR);
}

void func_80069B08(int dispatch)
{
    int mode = dispatch;
    uint32_t state = 0u;
    int done = 0;
    uint32_t sequence = 0xFFFFFFFFu;
    int timer = GA_TIMER;
    RECT rect;
    int v1;
    uint32_t bank;

    /* Retail .L80069B08's `sh -1, 0x20(sp)` sits in the first VSync's delay
     * slot. */
    PE_StoreU16(GA_SM_TERMINATOR, 0xFFFFu);

    (void)func_80073A44(0);
    func_80074D28(0);

    pe_69b08_read_chunk(GA_D_800930D8, GA_D_800B0E64);
    pe_69b08_read_chunk(GA_D_800930DA, GA_D_800B0E6C);

    func_8003E974();
    func_800371B0(PE_LoadU32(GA_D_800B0E6C));

    rect.x = 0; rect.y = 0; rect.w = 0x140; rect.h = 0x1C0;
    (void)func_80074F44(&rect, 0u, 0u, 1u);
    (void)func_800718D0(PE_LoadU32(GA_D_800B0E64));

    /* Retail stages the per-frame MoveImage rect here (sp+0x10) and keeps it
     * across the state-machine loop. */
    rect.x = 0x140; rect.y = 0x100; rect.w = 0x140; rect.h = 0xE0;
    (void)func_80074DC0(0);

    (void)func_800755F0(GA_D_800BCE80 + PE_LoadU32(GA_D_8009CDDC) * 20u);
    func_80074D28(1);

    /* .L80069CC8 — one frame of the status machine, then the shared tail. */
    while (!done) {
        /* Host bound only (see the header note): retail has no stop check
         * here and leaves on the matching-disc/shell-open path. */
        if (PE_Port_ShouldStop())
            return;

        func_800752AC(PE_LoadU32(GA_D_800B0E38
                                 + PE_LoadU32(GA_D_8009CDDC) * 4u),
                      GA_OT_SIZE);

        if (state < 10u) {
            switch (state) {
            case 0u: /* .L80069D10: CdReady idle + empty queue -> issue cmd 8 */
                if (func_8007F72C() != 1)
                    break;
                if (func_8007F778() != 0)
                    break;
                sequence = func_8007EE84(8u, 0u, 0u, 0xFFFFFFFFu);
                state = 1u;
                break;

            case 1u: /* .L80069D4C: poll the issued command */
                v1 = func_8007F418(sequence, GA_SM_RESPONSE);
                if (v1 == 2) {
                    pe_69b08_open_result_message(mode);
                    state = 2u;
                    break;
                }
                if (v1 < 3)
                    break;
                if (v1 >= 7)
                    break;
                if (v1 < 5)
                    break;
                state = 0u; /* 5 or 6: restart the state machine */
                break;

            case 2u: /* .L80069D88: wait for the CD status bit */
                if ((func_8007F788() & 0x10u) != 0u)
                    state = 3u;
                break;

            case 3u: /* .L80069DA4: CdReady codes drive the next message */
                v1 = func_8007F72C();
                if (v1 == 2)
                    break;
                if (v1 < 3) {
                    if (v1 == 1) { /* .L80069DE8 */
                        func_800374E8();
                        func_800375E0((mode == 1) ? 3 : 4, 0u,
                                      GA_SM_TERMINATOR);
                        timer = GA_TIMER;
                        state = 5u;
                    }
                    break;
                }
                if (v1 == 3) { /* .L80069E14 */
                    timer = GA_TIMER;
                    state = 6u;
                }
                break;

            case 4u: /* .L80069E30: mounted-disc test */
                v1 = func_800698D4();
                if (v1 == -1) { /* .L80069E70 */
                    func_800374E8();
                    timer = GA_TIMER;
                    func_800375E0(5, 0u, GA_SM_TERMINATOR);
                    state = 7u;
                    break;
                }
                if (v1 >= 0) {
                    if (v1 != 0)
                        break;
                    /* .L80069EB0: done when the mounted disc matches the
                     * caller's mode bit in D_800B0DCD. */
                    if (mode == 1 && (PE_LoadU8(GA_D_800B0DCD) & 1u)) {
                        done = 1;
                        break;
                    }
                    if (mode == 2 && (PE_LoadU8(GA_D_800B0DCD) & 2u)) {
                        done = 1;
                        break;
                    }
                    timer = GA_TIMER;
                    state = 9u;
                    break;
                }
                if (v1 == -2) { /* .L80069E90 */
                    func_800374E8();
                    timer = GA_TIMER;
                    func_800375E0(5, 0u, GA_SM_TERMINATOR);
                    state = 8u;
                }
                break;

            case 5u: /* .L80069E20: timer then hand off to the disc test */
                if (timer != 0) {
                    timer--;
                    break;
                }
                state = 4u;
                break;

            case 6u: /* .L80069F04 */
            case 7u:
            case 8u:
            case 9u: /* .L80069F14: same countdown then reopen the result */
                if (timer != 0) {
                    timer--;
                    break;
                }
                func_800374E8();
                pe_69b08_open_result_message(mode);
                state = 2u;
                break;

            default:
                break;
            }
        }

        /* .L80069F48: draw the frame and flip the display bank. */
        func_80037870();
        (void)func_80074DC0(0);
        (void)func_80073A44(0);
        (void)func_80074A44(1);

        bank = PE_LoadU32(GA_D_8009CDDC);
        (void)func_800755F0(GA_D_800BCE80 + bank * 20u);

        bank = PE_LoadU32(GA_D_8009CDDC);
        (void)func_80075424(GA_D_800BCDC8 + bank * 92u);

        bank = PE_LoadU32(GA_D_8009CDDC);
        (void)func_8007512C(&rect, 0, bank ? 0xE0 : 0);

        bank = PE_LoadU32(GA_D_8009CDDC);
        (void)func_800753B4(PE_LoadU32(GA_D_800B0E38 + bank * 4u) + 0x3FFCu);

        PE_StoreU32(GA_D_8009CDDC, PE_LoadU32(GA_D_8009CDDC) ^ 1u);
    }

    /* .L8006A024: teardown, publish the mode, wait for CdReady, publish CD
     * status. */
    (void)func_80073A44(0);
    func_80074D28(0);

    rect.x = 0; rect.y = 0; rect.w = 0x140; rect.h = 0x1C0;
    (void)func_80074F44(&rect, 0u, 0u, 1u);
    (void)func_80074DC0(0);

    PE_StoreU8(GA_D_800B0DCD, (mode == 1) ? 1u : 2u);

    for (;;) {
        v1 = func_8007F72C();
        if (v1 == 1)
            break;
        (void)func_80073A44(0);
    }

    PE_StoreU16(GA_D_800B0DD4, (uint16_t)func_8007F7A8());
}
