/*
 * Phase 6E-A — libcard: InitCARD + StartCARD.
 *
 * func_800409B4 (asm/disc1/307CC.s @ file 0x311B4).
 * Classification: 2 (SDK host implementation).
 *
 * Retail structure:
 *   - word guard D_800A1850: skip init when already done.
 *   - EnterCriticalSection; 8x OpenEvent (BIOS B(08h) via func_800726E4)
 *     with classes 0xF4000001/0xF0000011, specs {4,0x8000,0x100,0x2000},
 *     mode 0x1000, handlers func_80042BD8..func_80042C64; handles stored to
 *     D_800BCDA8..D_800BCDC4 (word stride 4).
 *   - func_8007DDD4(0) (_card_init), func_8007DE40, func_800726D4 (A(70h)),
 *     func_8007DD64(0) (A(ADh)): memory-card hardware/kernel bring-up —
 *     collapsed no-ops (no guest-RAM effects).
 *   - 8x EnableEvent (func_80072704) over the stored handles;
 *     ExitCriticalSection.
 *   - unconditionally: sb 0 -> D_800A0ED4+0x418 and D_800A0ED4+0.
 *
 * Kernel Event Control Blocks live outside the 2 MiB guest window; the host
 * event shim (pe_libetc.c) supplies deterministic handles.  The retail
 * handler addresses are preserved verbatim as the OpenEvent argument even
 * though the host never invokes them.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

static const struct {
    uint32_t cls;
    uint32_t spec;
    pe_addr_t handler;
} kCardEvents[8] = {
    { 0xF4000001u, 0x0004u, 0x80042BD8u },
    { 0xF4000001u, 0x8000u, 0x80042BECu },
    { 0xF4000001u, 0x0100u, 0x80042C00u },
    { 0xF4000001u, 0x2000u, 0x80042C14u },
    { 0xF0000011u, 0x0004u, 0x80042C28u },
    { 0xF0000011u, 0x8000u, 0x80042C3Cu },
    { 0xF0000011u, 0x0100u, 0x80042C50u },
    { 0xF0000011u, 0x2000u, 0x80042C64u },
};

void func_800409B4(void)
{
    int i;
    if (PE_LoadU32(0x800A1850u) == 0) {
        PE_StoreU32(0x800A1850u, 1);
        func_80072714();                    /* EnterCriticalSection */
        for (i = 0; i < 8; i++) {
            PE_StoreU32(0x800BCDA8u + (uint32_t)i * 4u,
                        (uint32_t)PE_Event_Open(kCardEvents[i].cls,
                                                kCardEvents[i].spec,
                                                0x1000,
                                                kCardEvents[i].handler));
        }
        /* func_8007DDD4(0), func_8007DE40, func_800726D4, func_8007DD64(0):
         * card hardware/kernel bring-up — collapsed no-ops */
        for (i = 0; i < 8; i++) {
            PE_Event_Enable((int)PE_LoadU32(0x800BCDA8u + (uint32_t)i * 4u));
        }
        func_80072724();                    /* ExitCriticalSection */
    }
    PE_StoreU8(0x800A0ED4u + 0x418u, 0);
    PE_StoreU8(0x800A0ED4u, 0);
}

/* Open + enable the eight card events exactly as func_800409B4 does, without
 * the guard or the guest-RAM writes.  Tests that drive func_800405A4 from a
 * fixture use this to register the same callback events the retail boot path
 * installs, so the empty-slot completion runs its real callback. */
void PE_Card_OpenEvents(void)
{
    int i;
    for (i = 0; i < 8; i++) {
        PE_StoreU32(0x800BCDA8u + (uint32_t)i * 4u,
                    (uint32_t)PE_Event_Open(kCardEvents[i].cls,
                                            kCardEvents[i].spec,
                                            0x1000,
                                            kCardEvents[i].handler));
    }
    for (i = 0; i < 8; i++) {
        PE_Event_Enable((int)PE_LoadU32(0x800BCDA8u + (uint32_t)i * 4u));
    }
}

/*
 * ── Memory-card kernel operations: empty-slot model ──────────────────────
 *
 * func_8007DD44 / func_8007DD54 / func_8007DDC4 / func_8007DDB4 are the
 * BIOS veneers at asm/disc1/6E538.s (A0 ABh _card_info, A0 ACh _card_load,
 * B0 50h _new_card, B0 4Eh _card_write); func_8007DD74 is the retail
 * _new_card + _card_write(port, 3Fh, NULL) wrapper.  The port has no card
 * hardware or card image, so both slots are empty, and the documented
 * empty-slot completion is a timeout:
 *
 *   psx-spx B(5Ch) _card_status: 11h = failed/timeout (eg. when no
 *   cartridge inserted).
 *
 * The timeout is reported to the game through the same events the retail
 * kernel delivers (psx-spx BIOS Event Summary):
 *
 *   F4000001h,2000h  card err eject or unformatted (higher-level ops)
 *   F0000011h,2000h  lower-level hardware I/O err
 *
 * func_800409B4 opens those eight events (four per layer) with mode 1000h,
 * so delivery runs the registered callback (func_80042C14 -> A1828 for the
 * higher layer; func_80042C64 -> A1834 for the lower layer) exactly as the
 * retail IRQ would.  The card status machine in func_800405A4 polls those
 * flags, so synchronous host delivery at the veneer is behaviorally
 * identical for its consumer.
 *
 * No success is invented: a card-present model is not implemented, and the
 * veneers always take the empty-slot path.  _card_write/_card_info returns
 * follow psx-spx: _card_write accepts sectors 0..400h (the documented 400h
 * quirk) and returns 1, or returns 0 for an out-of-range sector; the
 * higher-level reads return 0 (no info) on an empty slot.
 */
int func_8007DD44(int port)
{
    (void)port;
    (void)PE_Event_Deliver(0xF4000001u, 0x2000u);   /* card err eject */
    return 0;
}

int func_8007DD54(int port)
{
    (void)port;
    (void)PE_Event_Deliver(0xF4000001u, 0x2000u);   /* card err eject */
    return 0;
}

int func_8007DDC4(pe_addr_t port)
{
    /* B0(50h) _new_card() only clears the BIOS card-change latch for the
     * next read/write; it has no guest-RAM effect and no completion event. */
    (void)port;
    return 0;
}

int func_8007DDB4(pe_addr_t port, int sector, pe_addr_t src)
{
    (void)port;
    (void)src;
    if ((uint32_t)sector > 0x400u)
        return 0;                                   /* rejected, no I/O */
    (void)PE_Event_Deliver(0xF0000011u, 0x2000u);   /* lower-level I/O err */
    return 1;                                       /* accepted, async fail */
}
