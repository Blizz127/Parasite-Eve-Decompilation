/*
 * pe_spu_init.c — Psy-Q SPU init / register-write translation.
 *
 * These are SDK functions (class 2 host adaptations): the retail bodies read
 * and write the SPU register window through the D_8009B3FC base pointer.  The
 * port owns one SPU register file (pe_spu_dma.c g_spu_registers) and one SPU
 * voice mixer (pe_spu.c), so every retail `sh/lhu off($base)` becomes a
 * PE_SpuRegister_StoreU16/LoadU16 call.  The bodies are otherwise transcribed
 * from asm/disc1/6D874.s and 6E4AC.s.
 *
 * Authority: retail SLUS_006.62 SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 *   func_8007DCAC  SPU busy-wait (60 iterations of a x13 multiply; no MMIO)
 *   func_8007D454  write `count` halfwords through the SPU transfer port
 *   func_8007D1D4  SPU init (SsInit's hardware stage); arg 0 = full reset
 *   func_8007DAE0  SPU register write through the D_8009B3FC base
 *
 * Host adaptation note: D_8009B3FC is loaded as 0x1F801C00 from the retail EXE
 * image (file offset 0x8BA90 = VRAM 0x8009B3FC), and the mixed 0x1F801C00 /
 * 0x8015xxxx base forms are handled by routing the model directly rather than
 * dereferencing the guest pointer.  No retail behavior is skipped: the init
 * key-on/key-off, main/reverb volumes, voice defaults, pitch and transfer
 * address writes all reach the model.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_spu_dma.h"
#include "pe_gpu.h"
#include "pe_port_compat.h"

/* SPU register offsets (relative to 0x1F801C00). */
#define SPU_MAIN_VOL_L   0x180u
#define SPU_MAIN_VOL_R   0x182u
#define SPU_REVERB_VOL_L 0x184u
#define SPU_REVERB_VOL_R 0x186u
#define SPU_KON_LO       0x188u
#define SPU_KON_HI       0x18Au
#define SPU_KOFF_LO      0x18Cu
#define SPU_KOFF_HI      0x18Eu
#define SPU_PITCHMOD_LO  0x190u
#define SPU_PITCHMOD_HI  0x192u
#define SPU_NOISE_LO     0x194u
#define SPU_NOISE_HI     0x196u
#define SPU_REVERB_LO    0x198u
#define SPU_REVERB_HI    0x19Au
#define SPU_ENDX_LO      0x19Cu
#define SPU_ENDX_HI      0x19Eu
#define SPU_IRQ_ADDR     0x1A4u
#define SPU_TRANSFER_ADDR 0x1A6u
#define SPU_TRANSFER_DATA 0x1A8u
#define SPU_SPUCNT       0x1AAu
#define SPU_TRANSFER_CTRL 0x1ACu
#define SPU_STATUS       0x1AEu

/* func_8007DCAC — 60-iteration busy loop.  No hardware access; transcribed so
 * callers keep their published timing shape. */
void func_8007DCAC(void)
{
    uint32_t inner = 13u;
    int outer;

    for (outer = 0; outer < 0x3C; outer++)
        inner = inner * 13u;            /* v0 = v1*2 + v1; <<2; + v1 */
    (void)inner;
}

/* func_8007D454(src, count) — SPU transfer-port write.
 *
 * Transcribed from asm/disc1/6D874.s.  The transfer address comes from
 * D_8009B414 (8-byte units); each 0x40-halfword chunk is pushed through
 * 0x1A8 with the SPUCNT bit4 handshake and the 0x1AE busy/status polls.  The
 * port's register file stores the port writes; the SPU RAM image update is
 * performed by PE_SpuRegister_StoreU16's transfer model (pe_spu_dma.c). */
void func_8007D454(pe_addr_t src, int count)
{
    int remaining = count;
    uint16_t saved_ctrl;

    /* Entry: program 0x1A6 with D_8009B414, remember (0x1AE & 0x7FF). */
    PE_SpuRegister_StoreU16(SPU_TRANSFER_ADDR,
                            PE_LoadU16(0x8009B414u));
    saved_ctrl = (uint16_t)(PE_SpuRegister_LoadU16(SPU_STATUS) & 0x7FFu);

    while (remaining != 0) {
        int chunk = (remaining < 0x41) ? remaining : 0x40;
        int i;

        for (i = 0; i < chunk; i++) {
            PE_SpuRegister_StoreU16(SPU_TRANSFER_DATA, PE_LoadU16(src));
            src += 2u;
        }
        /* 0x6DCE0: SPUCNT = (SPUCNT & 0xFFCF) | 0x10, then wait for the
         * 0x400 status bit to clear (bounded retries as in retail). */
        PE_SpuRegister_StoreU16(
            SPU_SPUCNT,
            (uint16_t)((PE_SpuRegister_LoadU16(SPU_SPUCNT) & 0xFFCFu) | 0x10u));
        {
            unsigned guard = 0;
            while ((PE_SpuRegister_LoadU16(SPU_STATUS) & 0x400u) != 0u &&
                   guard++ < 0xF00u)
                ;
        }
        remaining -= chunk;
    }

    /* Exit: SPUCNT &= 0xFFCF, then poll 0x1AE & 0x7FF back to the entry value. */
    PE_SpuRegister_StoreU16(
        SPU_SPUCNT, (uint16_t)(PE_SpuRegister_LoadU16(SPU_SPUCNT) & 0xFFCFu));
    {
        unsigned guard = 0;
        while ((uint16_t)(PE_SpuRegister_LoadU16(SPU_STATUS) & 0x7FFu)
                   != saved_ctrl &&
               guard++ < 0xF00u)
            ;
    }
}

/* func_8007D1D4(arg) — SPU hardware init.
 *
 * arg == 0 performs the full reset: volumes/SPUCNT clear, SPU RAM transfer
 * address, 24 voice defaults (vol 0, pitch 0x3FFF, start 0x200, ADSR 0), the
 * all-voice key-on/key-off reset pulse, then SPUCNT = 0xC000.  arg != 0 keeps
 * the current voices (used by the re-init path). */
void func_8007D1D4(int arg)
{
    pe_addr_t ctrl;
    uint16_t status;
    int i;
    uint16_t saved_pitchmod_noise[4];

    (void)saved_pitchmod_noise;

    /* 0x8009B40C control word: set bit 0xB0000.  In the retail image this
     * holds 0x1F8010F0 (DMA DPCR), so the OR goes to the DMA model rather
     * than a guest-RAM dereference. */
    ctrl = PE_LoadU32(0x8009B40Cu);
    if ((ctrl & 0x1FFFFFFFu) == 0x1F8010F0u)
        PE_GPU_WriteDPCR(PE_GPU_ReadDPCR() | 0xB0000u);
    else if (PE_RangeIsRam(ctrl, 4u))
        PE_StoreU32(ctrl, PE_LoadU32(ctrl) | 0xB0000u);
    /* else: no control word installed (zeroed-RAM fixture) — hardware-only. */

    PE_StoreU32(0x8009B418u, 0u);
    PE_StoreU32(0x8009B41Cu, 0u);
    PE_StoreU16(0x8009B414u, 0u);

    /* 0x6D220..0x6D22C: main volumes and SPUCNT cleared, then wait. */
    PE_SpuRegister_StoreU16(SPU_MAIN_VOL_L, 0u);
    PE_SpuRegister_StoreU16(SPU_MAIN_VOL_R, 0u);
    PE_SpuRegister_StoreU16(SPU_SPUCNT, 0u);
    func_8007DCAC();

    /* 0x6D23C..0x6D2A4: wait for the transfer-status low 11 bits to clear. */
    PE_SpuRegister_StoreU16(SPU_MAIN_VOL_L, 0u);
    PE_SpuRegister_StoreU16(SPU_MAIN_VOL_R, 0u);
    status = (uint16_t)(PE_SpuRegister_LoadU16(SPU_STATUS) & 0x7FFu);
    if (status != 0u) {
        unsigned guard = 0;
        do {
            status = (uint16_t)(PE_SpuRegister_LoadU16(SPU_STATUS) & 0x7FFu);
            guard++;
        } while (status != 0u && guard < 0xF00u);
        /* A real timeout calls func_80071A74 with the "SPU:T/O" diagnostic;
         * the port has no such consumer and the model never times out. */
    }

    /* 0x6D2B0..0x6D30C: SPU configuration words and reverb/ENDX writes. */
    PE_StoreU32(0x8009B420u, 2u);
    PE_StoreU32(0x8009B424u, 3u);
    PE_StoreU32(0x8009B428u, 8u);
    PE_StoreU32(0x8009B42Cu, 7u);

    PE_SpuRegister_StoreU16(SPU_TRANSFER_CTRL, 4u);
    PE_SpuRegister_StoreU16(SPU_REVERB_VOL_L, 0u);
    PE_SpuRegister_StoreU16(SPU_REVERB_VOL_R, 0u);
    PE_SpuRegister_StoreU16(SPU_KOFF_LO, 0xFFFFu);
    PE_SpuRegister_StoreU16(SPU_KOFF_HI, 0xFFFFu);
    PE_SpuRegister_StoreU16(SPU_REVERB_LO, 0u);
    PE_SpuRegister_StoreU16(SPU_REVERB_HI, 0u);

    for (i = 0; i < 10; i++)
        PE_StoreU16(0x800B6900u + (uint32_t)i * 2u, 0u);

    if (arg != 0)
        goto tail;

    /* 0x6D328..0x6D3AC: transfer address 0x200, pitch/noise/reverb zero,
     * then the transfer-port write of 16 halfwords from D_8009B43C. */
    PE_StoreU16(0x8009B414u, 0x200u);
    PE_SpuRegister_StoreU16(SPU_PITCHMOD_LO, 0u);
    PE_SpuRegister_StoreU16(SPU_PITCHMOD_HI, 0u);
    PE_SpuRegister_StoreU16(SPU_NOISE_LO, 0u);
    PE_SpuRegister_StoreU16(SPU_NOISE_HI, 0u);
    PE_SpuRegister_StoreU16(0x1B0u, 0u);
    PE_SpuRegister_StoreU16(0x1B2u, 0u);
    PE_SpuRegister_StoreU16(0x1B4u, 0u);
    PE_SpuRegister_StoreU16(0x1B6u, 0u);
    func_8007D454(0x8009B43Cu, 0x10);

    /* 0x6D384..0x6D3A8: 24 voice defaults.  pitch 0x3FFF, start 0x200. */
    for (i = 0; i < 0x18; i++) {
        uint32_t base = (uint32_t)i * 0x10u;
        PE_SpuRegister_StoreU16(base + 0x0u, 0u);
        PE_SpuRegister_StoreU16(base + 0x2u, 0u);
        PE_SpuRegister_StoreU16(base + 0x4u, 0x3FFFu);
        PE_SpuRegister_StoreU16(base + 0x6u, 0x200u);
        PE_SpuRegister_StoreU16(base + 0x8u, 0u);
        PE_SpuRegister_StoreU16(base + 0xAu, 0u);
    }

    /* 0x6D3BC..0x6D408: all-voice key-on reset pulse, then key-off.  The
     * 0x18A write is the KON-high halfword (voices 16..23); 0x18C/0x18E are
     * KOFF low/high. */
    PE_SpuRegister_StoreU16(SPU_KON_LO, 0xFFFFu);
    func_8007DCAC();
    PE_SpuRegister_StoreU16(SPU_KON_HI, 0x00FFu);
    func_8007DCAC();
    func_8007DCAC();
    func_8007DCAC();
    PE_SpuRegister_StoreU16(SPU_KOFF_LO, 0xFFFFu);
    func_8007DCAC();
    PE_SpuRegister_StoreU16(SPU_KOFF_HI, 0x00FFu);
    func_8007DCAC();
    func_8007DCAC();
    func_8007DCAC();

tail:
    /* 0x6D414..0x6D43C: SPUCNT = 0xC000 (enable + unmute). */
    PE_StoreU32(0x8009B430u, 1u);
    PE_SpuRegister_StoreU16(SPU_SPUCNT, 0xC000u);
    PE_StoreU32(0x8009B434u, 0u);
    PE_StoreU32(0x8009B438u, 0u);
}

/* func_8007DAE0(reg, value, shifted) — SPU register write through the
 * D_8009B3FC base.  `shifted != 0` right-shifts the value by D_8009B424
 * before storing (the transfer-data path uses this). */
void func_8007DAE0(int reg, uint16_t value, int shifted)
{
    uint16_t out = value;

    if (shifted)
        out = (uint16_t)((uint32_t)value >> (PE_LoadU32(0x8009B424u) & 31u));
    PE_SpuRegister_StoreU16((uint32_t)reg * 2u, out);
}
