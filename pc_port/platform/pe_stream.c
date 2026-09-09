/*
 * Phase 6E-A — streaming engine bring-up (func_8006A5BC's four setup calls).
 *
 * ROM evidence (asm/disc1/74FB0.s, 75F44.s, 7D284.s):
 *   func_80085644 — streaming bring-up sequence
 *   func_80085290 — bulk streaming state init (242 asm lines, transcribed)
 *   func_8008CB08 — command-ring allocator (0x800B8628 + D_8009D2F4*0x24)
 *   func_8008CBA8 — streaming command producer
 *   func_8008CA84 — consumer and voice controls in pe_stream_commands.c
 *   func_80086FF8/func_80087024/func_8008682C — command issuers
 *   func_80085A04/85F14/85EB4/850C0/850F4/85E54/85174/85F44/85098 — leaves
 * Classification: 1 for the guest-state transcription; the SPU/DMA hardware
 * effects are class 2 collapses enumerated below.
 *
 * Audio command RAM is now consumed through the enabled host event.
 * Score sequencing remains unported. Bounded host ADPCM synthesis and optional
 * PulseAudio output live in pe_spu_synth.c / pe_host_audio.c (AUD1-E0).
 *
 * Retained hardware/SDK adaptations:
 *   - func_8007D9F8 (SPU DMA upload in func_80085E54): B48A records the
 *     DMA4 issue.  The following retail 85174 barrier supplies the
 *     controlled completion-event pump.
 *   - func_8007DB24(-1) (SPU heap query in func_80085EB4): host SPU-heap
 *     model starts at 0x1010 after SsInit (deterministic); only the store
 *     to D_8009B414 is guest-visible and the caller discards the return.
 *   - func_80085F74 / func_800862F4 (voice attribute/parameter setters in
 *     func_80085290): write through the voice-table base D_8009B3FC, which
 *     is installed by the collapsed SPU hardware init; not reproducible on
 *     host (same situation as D_8009B784 in pe_save.c).
 *   - func_8008CB54/func_8008CF70/func_80085A64 (streaming-mode command +
 *     SPU voice key off/on in func_80085290): SPU configuration/voice hardware paths, still deferred. This does not
 *     include 8CA84, whose command and channel RAM effects are native.
 *   - func_80085C44 (SPU control-register wait with "SPU:T/O" diagnostic),
 *     func_80085DC4 (SPU transfer-mode sequence in func_80085D84).
 *   - func_80085814/func_800858E8 retry loops (DMA acquire/release): their
 *     state tables D_8009B7CC/D_8009B7D0/D_8009B7D4 have no writer outside
 *     collapsed libsnd internals; modeled as host-synchronous (first poll
 *     succeeds).  The OpenEvent/EnableEvent retry loops use the real host
 *     event shims and also terminate on the first iteration.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_spu_dma.h"
#include "stub_registry.h"
#include <stdio.h>
#include <stdlib.h>

static uint16_t stream_register_read(uint32_t offset)
{
    pe_addr_t base = PE_LoadU32(0x8009B3FCu);
    if ((base & 0x1FFFFFFFu) == 0x1F801C00u)
        return PE_SpuRegister_LoadU16(offset);
    return PE_LoadU16(base + offset);
}

static void stream_register_write(uint32_t offset, uint16_t value)
{
    pe_addr_t base = PE_LoadU32(0x8009B3FCu);
    if ((base & 0x1FFFFFFFu) == 0x1F801C00u)
        PE_SpuRegister_StoreU16(offset, value);
    else PE_StoreU16(base + offset, value);
}

/* Original 8D140..8D610: zero mask writes all 32 mode registers;
 * otherwise each mask bit selects its corresponding halfword. The
 * destination base is reloaded for every selected store, as in retail.
 * Hardware address handling belongs to the mode-switch caller. */
void func_8008D140(pe_addr_t attributes)
{
    uint32_t mask = PE_LoadU32(attributes);
    for (unsigned i = 0; i < 32u; i++) {
        if (!mask || (mask & (1u << i))) {
            pe_addr_t base = PE_LoadU32(0x8009B3FCu);
            uint16_t value = PE_LoadU16(attributes + 4u + i * 2u);
            if ((base & 0x1FFFFFFFu) == 0x1F801C00u)
                PE_SpuRegister_StoreU16(0x1C0u + i * 2u, value);
            else PE_StoreU16(base + 0x1C0u + i * 2u, value);
        }
    }
}

/* Original 85BB4 allocation-overlap query. */
static int stream_mode_allocated(uint32_t address)
{
    address <<= PE_LoadU32(0x8009B424u) & 31u;
    pe_addr_t row = PE_LoadU32(0x8009B464u);
    if (!row) return 0;
    for (;;) {
        uint32_t word = PE_LoadU32(row);
        if (!(word & 0x80000000u)) {
            if (word & 0x40000000u) return 0;
            uint32_t start = word & 0x0FFFFFFFu;
            if (start >= address || address < start + PE_LoadU32(row + 4u)) return 1;
        }
        row += 8u;
    }
}

/* Original 85A64: SPU reverb enable state and control bit7. */
static void stream_mode_enable(unsigned enabled)
{
    if (!enabled) {
        uint16_t control = stream_register_read(0x1AAu);
        PE_StoreU32(0x8009B390u, 0);
        stream_register_write(0x1AAu, control & 0xFF7Fu);
    } else if (enabled == 1u) {
        if (PE_LoadU32(0x8009B394u) != 1u &&
            stream_mode_allocated(PE_LoadU32(0x8009B398u))) {
            uint16_t control = stream_register_read(0x1AAu);
            PE_StoreU32(0x8009B390u, 0);
            stream_register_write(0x1AAu, control & 0xFF7Fu);
        } else {
            uint16_t control = stream_register_read(0x1AAu);
            PE_StoreU32(0x8009B390u, 1);
            stream_register_write(0x1AAu, control | 0x80u);
        }
    }
}

/* Original 8D610 with DMA4 and WaitEvent supplied by the host event model.
 * Each wait services the pending transfer before consuming its event. */
static int stream_mode_clear(uint32_t mode)
{
    if (mode >= 10u || stream_mode_allocated(PE_LoadU32(0x8009B46Cu + mode * 4u))) return -1;
    unsigned shift = PE_LoadU32(0x8009B424u) & 31u;
    uint32_t amount, destination;
    if (!mode) { amount = 0x10u << shift; destination = 0xFFF0u << shift; }
    else {
        uint32_t address = PE_LoadU32(0x8009B46Cu + mode * 4u);
        amount = (0x10000u - address) << shift; destination = address << shift;
    }
    uint32_t transfer = PE_LoadU32(0x8009B418u);
    if (transfer == 1u) PE_StoreU32(0x8009B418u, 0);
    uint32_t callback = PE_LoadU32(0x8009B434u);
    if (callback) PE_StoreU32(0x8009B434u, 0);
    int result = 0;
    for (;;) {
        uint32_t count = amount <= 0x400u ? amount : 0x400u;
        uint16_t tsa = (uint16_t)(destination >> shift);
        PE_StoreU16(0x8009B414u, tsa);
        stream_register_write(0x1A6u, tsa);
        PE_StoreU32(0x8009B44Cu, 0);
        stream_register_write(0x1AAu, (stream_register_read(0x1AAu) & 0xFFCFu) | 0x20u);
        if (!PE_SpuDma_Begin(0x8009C4C0u, (uint32_t)tsa << shift, count, 0) ||
            !PE_SpuDma_Service() ||
            !PE_Event_ConsumeSpuDma((int)PE_LoadU32(0x8009B384u))) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            result = -1;
            break;
        }
        if (amount <= 0x400u) break;
        amount -= 0x400u; destination += 0x400u;
    }
    if (transfer == 1u) PE_StoreU32(0x8009B418u, transfer);
    if (callback) PE_StoreU32(0x8009B434u, callback);
    return result;
}

/* Original 8CF70: mode table, depth state, reverb registers and RAM clear. */
static int stream_mode_configure(uint32_t mode)
{
    unsigned clear = !!(mode & 0x100u); mode &= ~0x100u;
    if (mode >= 10u || stream_mode_allocated(PE_LoadU32(0x8009B46Cu + mode * 4u))) return -1;
    const pe_addr_t attributes = 0x80122300u; /* guest temporary replacing stack record */
    PE_StoreU32(0x8009B3A0u, mode);
    PE_StoreU32(0x8009B398u, PE_LoadU32(0x8009B46Cu + mode * 4u));
    for (unsigned i = 0; i < 0x44u; i++) PE_StoreU8(attributes + i, PE_LoadU8(0x8009C8C0u + mode * 0x44u + i));
    PE_StoreU32(attributes, 0);
    PE_StoreU32(0x8009B3ACu, mode == 7u ? 0x7Fu : 0);
    PE_StoreU32(0x8009B3A8u, mode == 7u || mode == 8u ? 0x7Fu : 0);
    uint16_t enabled = stream_register_read(0x1AAu) & 0x80u;
    if (enabled) stream_register_write(0x1AAu, stream_register_read(0x1AAu) & 0xFF7Fu);
    stream_register_write(0x184u, 0); stream_register_write(0x186u, 0);
    PE_StoreU16(0x8009B3A4u, 0); PE_StoreU16(0x8009B3A6u, 0);
    func_8008D140(attributes);
    if (clear) (void)stream_mode_clear(mode);
    stream_register_write(0x1A2u, (uint16_t)PE_LoadU32(0x8009B398u));
    if (enabled) stream_register_write(0x1AAu, stream_register_read(0x1AAu) | 0x80u);
    return 0;
}

void func_8008CB54(uint32_t mode)
{
    if (PE_LoadU32(0x8009B3A0u) == mode) return;
    stream_mode_enable(0);
    (void)stream_mode_configure(mode | 0x100u);
    stream_mode_enable(1);
}

/* ── func_80085290 — bulk streaming state init (verbatim transcription) ── */
static void PE_Stream_StateInit(void)
{
    int i;
    pe_addr_t s0, v1;

    PE_StoreU32(0x8009D2C8u, 0x800B6980u);
    PE_StoreU32(0x800B6A30u, 0x7F0000u);
    PE_StoreU32(0x800B69C8u, 0x7F0000u);
    PE_StoreU32(0x8009D2B4u, 0x7FFF0000u);
    PE_StoreU32(0x8009D2C0u, 1);
    PE_StoreU32(0x800BCD50u, 0);
    PE_StoreU32(0x800B6984u, 0);
    PE_StoreU32(0x800B6988u, 0);
    PE_StoreU16(0x800B69D4u, 0);
    PE_StoreU32(0x800BCD60u, 0);
    PE_StoreU32(0x800B699Cu, 0);
    PE_StoreU32(0x800B6A04u, 0);
    PE_StoreU32(0x800B69ECu, 0);
    PE_StoreU32(0x800B69F0u, 0);
    PE_StoreU16(0x800B6A3Cu, 0);
    PE_StoreU16(0x800B6A38u, 0);
    PE_StoreU16(0x800B69D0u, 0);
    PE_StoreU16(0x8009D21Eu, 0);
    PE_StoreU32(0x8009D2CCu, 0);
    PE_StoreU16(0x8009D220u, 0);
    PE_StoreU32(0x8009D2D0u, 0);
    PE_StoreU16(0x8009D2A2u, 0);
    PE_StoreU32(0x800BCD6Cu, 0);
    PE_StoreU32(0x800B69B4u, 0);
    PE_StoreU32(0x800BCD70u, 0);
    PE_StoreU32(0x800B69B8u, 0);
    PE_StoreU32(0x800BCD74u, 0);
    PE_StoreU32(0x800B69BCu, 0);
    PE_StoreU32(0x800B6A24u, 0);
    PE_StoreU32(0x800B6A20u, 0);
    PE_StoreU32(0x800B6A1Cu, 0);
    PE_StoreU32(0x800B8628u, 0);
    PE_StoreU16(0x800B69E0u, 0);
    PE_StoreU16(0x800B69DEu, 0);
    PE_StoreU16(0x800B69DCu, 0);
    PE_StoreU16(0x800B69E4u, 0);
    /* D_800C0D90 attribute block */
    PE_StoreU32(0x800C0D90u, 0x3FCFu);
    PE_StoreU16(0x800C0D96u, 0x3FFFu);
    PE_StoreU16(0x800C0D94u, 0x3FFFu);
    PE_StoreU16(0x800C0D98u, 0);
    PE_StoreU16(0x800C0D9Au, 0);
    PE_StoreU16(0x800C0DA2u, 0x7FFFu);
    PE_StoreU16(0x800C0DA0u, 0x7FFFu);
    PE_StoreU32(0x800C0DA4u, 0);
    PE_StoreU32(0x800C0DA8u, 1);
    PE_StoreU16(0x800C0DAEu, 0);
    PE_StoreU16(0x800C0DACu, 0);
    PE_StoreU32(0x800C0DB0u, 0);
    PE_StoreU32(0x800C0DB4u, 0);
    /* func_80085F74 — voice-attribute applier: collapsed (see header) */
    PE_StoreU32(0x8009D268u, 0);
    PE_StoreU32(0x8009D22Cu, 0);
    PE_StoreU32(0x8009D2B8u, 0);
    PE_StoreU32(0x800C0DD8u, 0);
    PE_StoreU32(0x800C0DD4u, 0);
    PE_StoreU32(0x800C0DD0u, 0);
    PE_StoreU32(0x8009D2F4u, 0);   /* = old D_8009D268 (0) */
    PE_StoreU32(0x8009D2DCu, 0);
    PE_StoreU32(0x8009D2E0u, 0);
    /* two 24-entry loops over the 0x11C-stride voice-state tables */
    for (i = 0; i < 0x18; i++) {
        s0 = 0x800B8AC0u + 0x50u + (uint32_t)i * 0x11Cu;
        PE_StoreU32(s0 - 0x18u, 0);
        PE_StoreU32(s0 + 0xA0u, 0x18u);
        PE_StoreU16(s0 + 0x04u, 0);
        PE_StoreU32(s0 + 0x00u, 0);
        /* func_800862F4(i, 0, 0, 0, 0) — voice-param setter: collapsed */
    }
    for (i = 0; i < 0x18; i++) {
        s0 = 0x800B8AC0u + 0x50u + (uint32_t)(0x18 + i) * 0x11Cu;
        PE_StoreU32(s0 - 0x18u, 0);
        PE_StoreU32(s0 + 0xA0u, 0x18u);
        PE_StoreU16(s0 + 0x04u, 0);
        PE_StoreU32(s0 + 0x00u, 0);
        /* func_800862F4(0x18 + i, 0, 0, 0, 0) — collapsed */
    }
    /* 12-entry channel block at 0x800BC03C, i = 0xC..0x17 */
    for (i = 0xC; i < 0x18; i++) {
        v1 = 0x800BC03Cu + (uint32_t)(i - 0xC) * 0x11Cu;
        PE_StoreU32(v1 - 0x04u, 0);
        PE_StoreU32(v1 + 0xB4u, (uint32_t)i);
        PE_StoreU16(v1 + 0x18u, 1);
        PE_StoreU32(v1 + 0x14u, 0);
        PE_StoreU16(v1 + 0x9Cu, 0x7F00u);
        PE_StoreU16(v1 + 0x38u, 0);
        PE_StoreU16(v1 + 0x34u, 0);
        PE_StoreU32(v1 + 0x00u, 0);
    }
    v1 = PE_LoadU32(0x8009D2C8u);
    PE_StoreU32(v1 + 0x18u, 0);
    PE_StoreU32(v1 + 0x14u, 0);
    PE_StoreU32(v1 + 0x10u, 0);
    PE_StoreU32(v1 + 0x80u, 0);
    PE_StoreU32(v1 + 0x7Cu, 0);
    PE_StoreU32(v1 + 0x78u, 0);
    PE_StoreU32(0x800BCD68u, 1);
    PE_StoreU32(0x800BCD64u, 0x66A80000u);
    PE_StoreU32(0x800BCD5Cu, 0);
    PE_StoreU32(0x800BCD58u, 0);
    PE_StoreU32(0x800BCD54u, 0);
    PE_StoreU32(v1 + 0xA8u, 0x3FFF0000u);
    PE_StoreU32(v1 + 0x40u, 0x3FFF0000u);
    PE_StoreU32(v1 + 0xACu, 0);
    PE_StoreU32(v1 + 0x44u, 0);
    PE_StoreU16(v1 + 0xC0u, 0);
    PE_StoreU16(v1 + 0x58u, 0);
    PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | 0x80u);
    /* func_8008CB54(4) — streaming-mode command: collapsed (see header) */
    /* func_80085A64(1) — SPU voice key-on: collapsed (see header) */
}

/* ── func_8008CB08 — command-ring allocator ───────────────────────────── */
static pe_addr_t PE_Stream_RingAlloc(void)
{
    pe_addr_t e = 0x800B8628u + PE_LoadU32(0x8009D2F4u) * 0x24u;
    PE_StoreU32(0x8009D2F4u, PE_LoadU32(0x8009D2F4u) + 1u);
    return e;
}

/* ── func_8008CBA8 — streaming command dispatcher ─────────────────────── */
int func_8008CBA8(void)
{
    uint32_t cmd = PE_LoadU32(0x800BCD80u);
    uint32_t a84 = PE_LoadU32(0x800BCD84u);
    uint32_t a88 = PE_LoadU32(0x800BCD88u);
    uint32_t a8C = PE_LoadU32(0x800BCD8Cu);
    uint32_t a90 = PE_LoadU32(0x800BCD90u);
    int s1 = 0;
    pe_addr_t e;

    PE_StoreU32(0x8009D268u, 1);
    switch (cmd) {
    case 0x98:
        e = PE_Stream_RingAlloc(); PE_StoreU32(e, 0x9Au);
        e = PE_Stream_RingAlloc(); PE_StoreU32(e, 0x9Cu);
        break;
    case 0x99:
        e = PE_Stream_RingAlloc(); PE_StoreU32(e, 0x9Bu);
        e = PE_Stream_RingAlloc(); PE_StoreU32(e, 0x9Du);
        break;
    case 0xD8:
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD0u);
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD4u);
        break;
    case 0xD9:
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD1u); PE_StoreU32(e + 0x8u, a88);
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD5u); PE_StoreU32(e + 0x8u, a88);
        break;
    case 0xDA:
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD2u);
        PE_StoreU32(e + 0x8u, a88); PE_StoreU32(e + 0xCu, a8C);
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD6u);
        PE_StoreU32(e + 0x8u, a88); PE_StoreU32(e + 0xCu, a8C);
        break;
    case 0x24: {
        uint32_t old = PE_LoadU32(0x8009CDF0u);
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84);
        PE_StoreU32(e + 0x8u, a88);
        PE_StoreU32(e + 0xCu, a8C);
        s1 = (int)old;
        PE_StoreU32(e + 0x14u, old);
        PE_StoreU32(e + 0x10u, a90);
        PE_StoreU32(e, cmd);
        PE_StoreU32(0x8009CDF0u, ((old + 1u) & 0x1FFu) + 0x400u);
        break;
    }
    case 0x10:
    case 0x12:
    case 0x19:
        /* Original 8CC68..8CD04: valid AKAO payloads carry a song ID
         * at +4 and SPU mode at +8; queue data beginning at +16. */
        if (func_80085084(a84) != 0) {
            s1 = -1;
        } else {
            uint32_t song = PE_LoadU16(a84 + 4u);
            uint32_t mode = PE_LoadU16(a84 + 8u);
            if (PE_LoadU16(PE_LoadU32(0x8009D2C8u) + 0x54u) == song)
                break;
            /* 8CB54 returns without hardware work when the configured
             * mode matches. Its SPU mode-change graph remains unported. */
            func_8008CB54(mode);
            if (PE_Port_ShouldStop()) { s1 = -1; break; }
            e = PE_Stream_RingAlloc();
            PE_StoreU32(e + 4u, a84 + 16u);
            PE_StoreU32(e + 12u, song);
            if (cmd == 0x12u) PE_StoreU32(e + 16u, a88);
            PE_StoreU32(e, cmd);
            s1 = (int)song;
        }
        break;
    default:                        /* includes 0xF0 / 0xF1 */
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84);
        PE_StoreU32(e + 0x8u, a88);
        PE_StoreU32(e + 0xCu, a8C);
        PE_StoreU32(e + 0x10u, a90);
        PE_StoreU32(e, cmd);
        break;
    }
    PE_StoreU32(0x8009D268u, 0);
    return s1;
}

/* ── Command issuers ──────────────────────────────────────────────────── */

void func_80086FF8(void)
{
    PE_StoreU32(0x800BCD80u, 0xF0u);
    func_8008CBA8();
}

void func_80087024(void)
{
    PE_StoreU32(0x800BCD80u, 0xF1u);
    func_8008CBA8();
}

void func_8008682C(int a)
{
    uint32_t v = (a == 1) ? 0x9Au : (a == 2) ? 0x9Cu : 0x98u;
    PE_StoreU32(0x800BCD80u, v);
    func_8008CBA8();
}

/* ── func_80085644 — streaming bring-up ───────────────────────────────── */

void func_80085644(void)
{
    func_8007D15C();                    /* SPU IRQ event (guard: no-op after SsInit) */

    /* SPU allocation globals (retail SsInit at 0x8007D2A8, collapsed).
     * Must be set before func_80085EB4 reads them. */
    PE_StoreU32(0x8009B420u, 2);    /* alignment mode flag */
    PE_StoreU32(0x8009B424u, 3);    /* SPU address shift (8-byte units) */
    PE_StoreU32(0x8009B428u, 8);    /* alignment divisor */
    PE_StoreU32(0x8009B42Cu, 7);    /* alignment mask */

    /* func_80085A04(4, &D_800B6958) — stream transfer config */
    PE_StoreU32(0x800B6958u, 0x40001010u);
    PE_StoreU32(0x8009B464u, 0x800B6958u);
    PE_StoreU32(0x8009B460u, 0);
    PE_StoreU32(0x8009B45Cu, 4);
    PE_StoreU32(0x800B6958u + 4u, (0x10000u << PE_LoadU32(0x8009B424u)) - 0x1010u);

    /* func_80085F14(0) — mode 0 -> both words 0 */
    PE_StoreU32(0x8009B38Cu, 0);
    PE_StoreU32(0x8009B418u, 0);

    /* func_80085EB4(0x1010): (0x1010-0x1010) <= 0x7EFE8 -> alloc path.
     * func_8007DB24(-1) SPU heap query: host model top = 0x1010. */
    PE_StoreU16(0x8009B414u, 0x1010u);

    /* func_800850F4(&D_8009B7FC, 0x20), then func_80085174. */
    func_800850F4(0x8009B7FCu, 0x20u);
    PE_SpuDma_WaitForCompletion();

    PE_Stream_StateInit();                  /* func_80085290 */

    /* func_80085C44(0) — SPU control-register wait: collapsed hardware */

    /* func_80085D84(0) — transfer-mode setter, no change at boot */
    if (PE_LoadU32(0x8009B438u) != 0) {
        PE_StoreU32(0x8009B438u, 0);
        /* func_80085DC4 — SPU transfer-mode sequence: collapsed */
    }

    /* func_80085814/func_800858E8 DMA acquire/release retry loops:
     * host-synchronous, succeed on first poll (see header) */

    /* OpenEvent retry loop: real host event handle, != -1 immediately */
    PE_StoreU32(0x8009CDE0u,
                (uint32_t)PE_Event_Open(0xF2000002u, 2, 0x1000, 0x8008E23Cu));
    /* EnableEvent retry loop: succeeds immediately */
    PE_Event_Enable((int)PE_LoadU32(0x8009CDE0u));
}
