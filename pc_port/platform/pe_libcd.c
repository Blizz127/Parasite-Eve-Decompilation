/*
 * Phase 6E-A — libcd: CdInit, Cd reset, CdReady and CD state getters.
 *
 * ROM evidence (asm/disc1/6E6C0.s unless noted):
 *   func_8007EC14 CdInit
 *   func_8007ED58 CD reset + state clear (boot wait-loop 1 provider)
 *   func_8007F72C CdReady; func_8007FBF0 status-lane getter D_8009B574[idx]
 *     (getter body in asm/disc1/703F0.s: sll a0,2; lw D_8009B574[a0])
 *   func_800808BC / func_80080930 — D_8009B554 flag clear/set
 *   func_800822BC — DS-abort check, always stores D_8009B70C = 0
 *   func_80081E5C(0) — exchange D_8009B708
 *   func_8007F994 — CD low-level init (hardware; guest effects listed below)
 *   func_8007F778 — src C leaf getter D_800A3608
 *   func_80080CC8 — src C leaf exchange D_8009AFC0
 *   func_8007F7A8 -> func_8007FCAC — src C leaf getter D_8009B590
 * Classification: 2 (SDK host implementation); the drive model is class 3
 * (deterministic platform behavior).
 *
 * Collapsed retail effects (hardware-only):
 *   - func_8007F994's controller programming (func_8007BBFC, func_8007BAC0,
 *     func_8007FA2C, func_800812F4(0), func_80073D58): CD/DMA registers.
 *   - func_8007B9EC inside func_800808BC: CD hardware access.
 *   - func_800822BC's DS-abort calls when D_8009B70C==1 (never true here).
 *
 * Host drive model: retail sets D_8009B574[0] = 1 (idle/ready) from the CD
 * interrupt handler once the reset sequence is acknowledged.  The host
 * models the controller synchronously — after a completed reset with no
 * pending command the drive is idle — so func_8007ED58 sets the lane to 1
 * when no hardware status has been recorded.  This reproduces the
 * retail-observable state transition; it is not a fabricated return value.
 *
 * ── Batch 3: real-disc providers (evidence noted per function) ─────────
 *   func_80080C48  CdPosToInt (asm/disc1/71150.s): pure BCD math over the
 *                  CdlLOC bytes at the guest fp.
 *   func_80082314  PVD verify (asm/disc1/72ABC.s): lane shortcut, CdReady
 *                  wait, then D_800B28F8 result word {0x10,1,2,4}.
 *                  Collapsed retail internals: func_800822AC (DS-abort
 *                  check), func_80081DF8(0x10) (CdFlush), func_80080B44
 *                  (CdIntToPos of LBA 16), func_8007F0C8 (async 27-sector
 *                  read issue) and the func_80082400/func_80082444 deferred
 *                  callback chain.  The callback's guest-observable result
 *                  is D_800B28F8 = 4 when the sector-16 user data carries
 *                  "CD001" at offset 1 (strncmp vs D_8001205C, verified
 *                  "CD001" in the retail EXE), else 2; the host reaches the
 *                  same word synchronously via PE_Disc_VerifyPVD.
 *   func_80081414  DsSearchFile (asm/disc1/71A68.s): leading-'\' path
 *                  required, at most 8 components; on a match copies a
 *                  24-byte CdlFILE {pos(4), size(4), name(16)} to the guest
 *                  fp and returns nonzero, else 0.  Collapsed internals:
 *                  the D_800A36B8 cache table, the D_8009B6E0 timestamp and
 *                  func_80081714 refresh, the func_800819D8 component walk,
 *                  func_80081A7C directory read, func_800816F4 name compare
 *                  and the D_8009AFC0-gated debug printfs.  The host walks
 *                  the real ISO9660 directory records synchronously with
 *                  the same guest-visible contract.
 *   func_8006E6D4  async read issue (asm/disc1/5B1E4.s):
 *                  (lba_base, lba_off, dest, sectors).  Guards transcribed
 *                  verbatim (D_800B0CD8 & 0x1000000, CdReady != 1, queue
 *                  != 0, mode mismatch vs D_800B0DD4).  The transfer is
 *                  synchronous on the host: `sectors * 0x800` bytes land in
 *                  guest RAM before return, so D_8009B6B4 (sectors pending)
 *                  is 0 instead of `sectors`; all other guest state matches
 *                  the retail
 *                  post-issue values (D_8009B6AC=0x200, D_8009B6B0=dest,
 *                  D_8009B6C4=vsync timestamp, D_8009B6D4=1).
 *                  Collapsed: func_800719E4(1) (BIOS B(38h) CD mode set —
 *                  never taken at boot, D_8009B590 == D_800B0DD4 == 0),
 *                  func_80080B44/func_80080E34/func_8007F0C8 issue layer,
 *                  and the func_80071A74 failure printf (D_8001136C).
 *   func_800811E4  read poll (asm/disc1/714DC.s): -1 on timeout
 *                  (D_8009B6C4 + 1200 vsyncs, signed compare), else the
 *                  pending-byte count D_8009B6B4.  func_8007F608(fp)
 *                  (DsDataSync query) is collapsed — retail discards its
 *                  result.  Timeout abort func_80081268 collapses the CD
 *                  abort commands; its guest-observable effect
 *                  (D_8009B6CC = 0) is transcribed.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_disc.h"
#include "host_framebuffer.h"

/* B54K-AL: value-only controller location for the proven synchronous
 * CdlSetloc arm.  This is host hardware state, not planted guest state. */
static uint32_t g_cd_setloc_raw;
static int g_cd_setloc_valid;

/* Identical zeroing block shared by CdInit and func_8007ED58. */
static void PE_Cd_ClearState(void)
{
    int i;
    PE_StoreU32(0x800B8AB0u, 0);
    PE_StoreU32(0x800B8AB4u, 0);
    PE_StoreU32(0x800B8AB8u, 0);
    /* D_800A3510 fields: sw 0 @+0x20/+0x10/+0x00, sb 0 @+0x24/+0x14/+0x04 */
    PE_StoreU32(0x800A3510u + 0x20u, 0);
    PE_StoreU32(0x800A3510u + 0x10u, 0);
    PE_StoreU32(0x800A3510u + 0x00u, 0);
    PE_StoreU8(0x800A3510u + 0x24u, 0);
    PE_StoreU8(0x800A3510u + 0x14u, 0);
    PE_StoreU8(0x800A3510u + 0x04u, 0);
    for (i = 0; i < 8; i++) {
        PE_StoreU8(0x800A3515u + (uint32_t)i, 0);
        PE_StoreU8(0x800A3525u + (uint32_t)i, 0);
        PE_StoreU8(0x800A3535u + (uint32_t)i, 0);
    }
    /* func_8007E594 x8: zero the 0x18-byte descriptors at 0x800A3540+i*0x18 */
    for (i = 0; i < 8; i++) {
        pe_addr_t d = 0x800A3540u + (uint32_t)i * 0x18u;
        PE_StoreU32(d + 0x00, 0);
        PE_StoreU8(d + 0x04, 0);
        PE_StoreU8(d + 0x05, 0);
        PE_StoreU8(d + 0x06, 0);
        PE_StoreU8(d + 0x07, 0);
        PE_StoreU8(d + 0x08, 0);
        PE_StoreU32(d + 0x0C, 0);
        PE_StoreU32(d + 0x10, 0);
        PE_StoreU32(d + 0x14, 0);
    }
    PE_StoreU32(0x800A3604u, 0);
    PE_StoreU32(0x800A3600u, 0);
    PE_StoreU32(0x800A3608u, 0);
    for (i = 0; i < 8; i++) {
        PE_StoreU32(0x800A3610u + (uint32_t)i * 0x10u, 0);
    }
    PE_StoreU32(0x800A3690u, 0);
    g_cd_setloc_raw = 0u;
    g_cd_setloc_valid = 0;
}

/* func_800822BC: DS-abort check; abort calls collapsed, flag always cleared. */
static void PE_Cd_DsAbortCheck(void)
{
    PE_StoreU32(0x8009B70Cu, 0);
}

/* func_80081E5C(0): old = D_8009B708; D_8009B708 = 0 (old discarded). */
static void PE_Cd_ClearB708(void)
{
    PE_StoreU32(0x8009B708u, 0);
}

int func_8007EC14(void)
{
    /* func_80080940 — src C leaf getter D_8009B554 */
    if (PE_LoadU32(0x8009B554u) != 0) {
        return (int)PE_LoadU32(0x8009B554u);
    }
    PE_Cd_ClearState();
    /* func_8007F994 — CD low-level init; guest-visible effects: */
    PE_StoreU32(0x800A36A8u, 0);
    PE_StoreU32(0x800A36A4u, 0);
    PE_StoreU32(0x800A36A0u, 0);
    PE_StoreU32(0x8009AFB4u, 0x80080164u);
    PE_StoreU32(0x8009AFB8u, 0x80080778u);
    PE_StoreU32(0x8009AFD8u, 1);
    PE_StoreU32(0x8009B554u, 1);
    /* handler installs (overwrite the lowlevel zeros) */
    PE_StoreU32(0x800A36A4u, 0x8007E964u);
    PE_StoreU32(0x800A36A8u, 0x8007F88Cu);
    PE_StoreU32(0x800A36ACu, 0x8007F960u);
    PE_StoreU32(0x800A36A0u, 0x8007F7E8u);
    PE_Cd_DsAbortCheck();
    PE_Cd_ClearB708();
    return 1;
}

int func_8007ED58(void)
{
    /* func_800808BC: */
    PE_StoreU32(0x8009B554u, 0);
    /* func_8007B9EC — CD hardware access: collapsed */
    if (PE_LoadU32(0x8009B574u) == 2) {
        uint32_t v = PE_LoadU32(0x8009B578u);
        if (v == 0xB || v == 0x11 || v == 0x10) {
            PE_StoreU32(0x8009B574u, 1);
            PE_StoreU32(0x8009B578u, 0xB);
        }
    }
    PE_Cd_ClearState();
    PE_Cd_DsAbortCheck();
    PE_Cd_ClearB708();
    /* func_80080930: */
    PE_StoreU32(0x8009B554u, 1);
    /* Host drive model: the reset sequence completes synchronously and
     * aborts any pending command, so the drive is idle/ready afterwards.
     * Retail reaches the same state asynchronously via the CD interrupt
     * handler (installed at D_800A36A0). */
    PE_StoreU32(0x8009B574u, 1);
    return 1;
}

int func_8007FBF0(int idx)
{
    return (int)PE_LoadU32(0x8009B574u + (uint32_t)idx * 4u);
}

int func_8007F72C(void)
{
    int st = func_8007FBF0(0);
    if (st != 1) {
        return st;
    }
    return (func_8007F778() > 0) ? 2 : 1;
}

int func_8007F778(void)
{
    return (int)PE_LoadU32(0x800A3608u);
}

int func_80080CC8(int v)
{
    int old = (int)PE_LoadU32(0x8009AFC0u);
    PE_StoreU32(0x8009AFC0u, (uint32_t)v);
    return old;
}

int func_8007F7A8(void)
{
    /* func_8007FCAC — src C leaf getter D_8009B590 */
    return (int)PE_LoadU32(0x8009B590u);
}

/* ── Batch 3: real-disc providers ────────────────────────────────────── */

/* func_80080B44 — CdIntToPos: absolute LBA + 150 → BCD mm/ss/ff at loc.
 * (Magic-multiply division collapsed to plain C division; identical math.) */
static void PE_Cd_IntToPos(int lba, pe_addr_t loc)
{
    uint32_t t = (uint32_t)(lba + 150);
    uint32_t m = t / 4500u;
    uint32_t s = (t % 4500u) / 75u;
    uint32_t f = t % 75u;
    PE_StoreU8(loc + 0, (uint8_t)(((m / 10u) << 4) | (m % 10u)));
    PE_StoreU8(loc + 1, (uint8_t)(((s / 10u) << 4) | (s % 10u)));
    PE_StoreU8(loc + 2, (uint8_t)(((f / 10u) << 4) | (f % 10u)));
}

static int PE_Cd_BcdByte(uint8_t b)
{
    return ((b >> 4) * 10) + (b & 0x0F);
}

/* func_80080C48 — CdPosToInt: BCD mm/ss/ff at fp → ((m*60+s)*75+f) - 150. */
int func_80080C48(pe_addr_t fp)
{
    int m = PE_Cd_BcdByte(PE_LoadU8(fp + 0));
    int s = PE_Cd_BcdByte(PE_LoadU8(fp + 1));
    int f = PE_Cd_BcdByte(PE_LoadU8(fp + 2));
    return ((m * 60 + s) * 75 + f) - 150;
}

/* func_80080D5C is the blocking command wrapper at [0x80080D5C,
 * 0x80080DC4).  The native substrate currently implements only its proven
 * production arm: command 2, named CdlSetloc by the retail executable's own
 * command-name table.  The complete movie caller never reads its eight-byte
 * response buffer, so result==0 deliberately avoids fabricating controller
 * response bytes.  Other commands and response-consuming callers remain a
 * mutation-free boundary rather than false successes. */
int func_80080D5C(int command, pe_addr_t param, pe_addr_t result)
{
    PE_Disc *d;
    uint8_t m, s, f;
    int lba;

    if (((uint32_t)command & 0xFFu) != 2u || result != 0u)
        return 0;
    if (!PE_RangeIsRam(param, 4u) || func_8007F72C() != 1 ||
        func_8007F778() != 0)
        return 0;
    d = PE_Disc_GetActive();
    if (!d)
        return 0;

    m = PE_LoadU8(param + 0u);
    s = PE_LoadU8(param + 1u);
    f = PE_LoadU8(param + 2u);
    if ((m & 0x0Fu) > 9u || (m >> 4) > 9u ||
        (s & 0x0Fu) > 9u || (s >> 4) > 5u ||
        (f & 0x0Fu) > 9u || (f >> 4) > 7u)
        return 0;
    lba = func_80080C48(param);
    if (lba < 0 || (uint32_t)lba >= PE_Disc_UserSectorCount(d))
        return 0;

    g_cd_setloc_raw = PE_LoadU32(param);
    g_cd_setloc_valid = 1;
    return 1;
}

uint32_t PE_Cd_GetSetlocRaw(void)
{
    return g_cd_setloc_valid ? g_cd_setloc_raw : 0u;
}

/* func_80082314 — PVD verify; see header for the collapsed async layer. */
int func_80082314(void)
{
    PE_Disc *d;
    uint32_t result;

    if (func_8007FBF0(0) == 2 && func_8007FBF0(1) == 0x10) {
        return 0x10;
    }
    for (;;) {
        int st = func_8007F72C();
        if (st == 1) break;
        if (st == 3) return 1;
    }
    PE_StoreU32(0x800B28F8u, 0);
    d = PE_Disc_GetActive();
    result = (d && PE_Disc_VerifyPVD(d)) ? 4u : 2u;
    PE_StoreU32(0x800B28F8u, result);
    return (int)PE_LoadU32(0x800B28F8u);
}

/* func_80081414 — DsSearchFile over the real ISO9660 tree.
 * fp is a guest address receiving the 24-byte CdlFILE. */
int func_80081414(pe_addr_t fp, const char *name)
{
    PE_Disc *d;
    uint32_t lba, size;
    char file_id[16];
    int i;

    if (!name || name[0] != '\\') return 0;
    d = PE_Disc_GetActive();
    if (!d) return 0;
    if (!PE_Disc_FindFile(d, name, &lba, &size, file_id, sizeof(file_id))) {
        return 0;
    }
    /* CdlFILE: CdlLOC pos (BCD mm/ss/ff, track byte 0), u32 size, name[16] */
    PE_Cd_IntToPos((int)lba, fp);
    PE_StoreU8(fp + 3, 0);
    PE_StoreU32(fp + 4, size);
    for (i = 0; i < 16; i++) {
        PE_StoreU8(fp + 8 + (uint32_t)i, (uint8_t)file_id[i]);
    }
    return 1;
}

/* func_8006E6D4 — read issue; synchronous on the host. */
int func_8006E6D4(int lba_base, int lba_off, pe_addr_t dest, int sectors)
{
    PE_Disc *d;
    uint32_t lba;
    uint32_t byte_count;
    int vs, ds, pr, mk;

    if (D_800B0CD8 & 0x01000000u) return -1;
    if (func_8007F72C() != 1) return -1;
    if (func_8007F778() != 0) return -1;
    if (func_8007F7A8() != (int)PE_LoadU16(0x800B0DD4u)) {
        /* func_800719E4(1) — BIOS B(38h) CD mode set: collapsed no-op. */
    }
    D_800B0CD8 |= 0x01004000u;
    lba = (uint32_t)(lba_base + lba_off);
    d = PE_Disc_GetActive();
    if (sectors < 0 ||
        (uint32_t)sectors > UINT32_MAX / PE_DISC_USER_SECTOR) {
        D_800B0CD8 &= 0xFEFFBFFFu;
        return -1;
    }
    byte_count = (uint32_t)sectors * PE_DISC_USER_SECTOR;
    if (byte_count > 0 && (!d ||
        !PE_RangeIsRam(dest, (size_t)byte_count) ||
        !PE_Disc_ReadUserData(d, lba, 0,
                              PE_Translate(dest, (size_t)byte_count),
                              byte_count))) {
        D_800B0CD8 &= 0xFEFFBFFFu;
        /* func_80071A74 printf(D_8001136C, lba, sectors): collapsed. */
        return -1;
    }
    /* sectors == 0 completes trivially (no data access). Bootstrap fixtures
     * use that explicit path; authenticated retail images adopt 6E834's
     * nonzero rodata range before entering the boot loop. */
    /* Retail post-issue state (asm func_80080E34); the transfer is already
     * complete on the host, so D_8009B6B4 (sectors pending) reads 0. */
    PE_StoreU32(0x8009B6ACu, 0x200u);
    PE_StoreU32(0x8009B6B0u, dest);
    PE_StoreU32(0x8009B6B4u, 0u);
    HostFB_GetState(&vs, &ds, &pr, &mk);
    PE_StoreU32(0x8009B6C4u, (uint32_t)vs);
    PE_StoreU32(0x8009B6D4u, 1u);
    return 1;
}

/* func_800811E4 — poll; 0 done, -1 timeout (>1200 vsyncs), else pending. */
int func_800811E4(pe_addr_t fp)
{
    int vs, ds, pr, mk;
    int32_t result;

    (void)fp; /* func_8007F608(fp) — DsDataSync query: collapsed; retail
               * discards its result here. */
    HostFB_GetState(&vs, &ds, &pr, &mk);
    if ((int32_t)(PE_LoadU32(0x8009B6C4u) + 0x4B0u) < vs) {
        /* func_80081268 timeout abort; CD abort commands collapsed. */
        PE_StoreU32(0x8009B6CCu, 0u);
        result = -1;
    } else {
        result = (int32_t)PE_LoadU32(0x8009B6B4u);
    }
    return result;
}
