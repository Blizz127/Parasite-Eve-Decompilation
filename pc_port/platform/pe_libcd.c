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
#include "game_port.h"
#include "pe_bootstrap.h"

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

/* ── Batch 4: CdlReadS queue issue (evidence noted per function) ─────── */

/* func_8007E6B0 — request-slot ring allocator (asm/disc1/6E6C0.s, 21
 * words).  Eight 0x18-byte descriptors at D_800A3540; slot index is
 * (D_800A3600 + D_800A3608) mod 8 with retail's single subtract (no
 * wrap-around loop — transcribed exactly).  Returns 0 when the queue
 * count D_800A3608 reaches 8 (signed compare). */
pe_addr_t func_8007E6B0(void)
{
    uint32_t count = PE_LoadU32(0x800A3608u);
    uint32_t idx;

    if ((int32_t)count >= 8)
        return 0;
    idx = PE_LoadU32(0x800A3600u) + count;
    if (idx >= 8u)
        idx -= 8u;
    return 0x800A3540u + idx * 0x18u;
}

/* func_80080950 — 4-byte copy-or-clear helper (asm/disc1/71150.s, 18
 * words).  src != 0: copies 4 bytes src -> dst when dst != 0, else
 * nothing.  src == 0: clears one byte at dst when dst != 0.  Callers
 * discard the result. */
void func_80080950(pe_addr_t dst, pe_addr_t src)
{
    uint32_t i;

    if (src == 0u) {
        if (dst != 0u)
            PE_StoreU8(dst, 0u);
        return;
    }
    if (dst == 0u)
        return;
    for (i = 0u; i < 4u; i++)
        PE_StoreU8(dst + i, PE_LoadU8(src + (pe_addr_t)i));
}

/* func_8007C214 — streaming DMA-completion callback (asm/disc1/6C93C.s,
 * 35 words).  Publishes status 2 to the active 32-byte stream record
 * (D_800C0DC8 + D_800BE9E4 * 32), copies the record's sector word to
 * D_800A3490, advances D_800BE9E4 to D_800BE998, chains the
 * D_800B0CC8 callback when set, then clears D_800B89F4.
 *
 * The D_800B0CC8 chain is a proven-dead arm on every path that reaches
 * here: func_8007C304 (the only writer in the translated tree) is
 * called with callback = 0 by the movie prefix, so the register reads
 * 0 and retail skips the jalr (beqz).  The nonzero arm is an honest
 * boundary, not a silent skip. */
void func_8007C214(void)
{
    uint32_t idx = PE_LoadU32(0x800BE9E4u);
    pe_addr_t rec = PE_LoadU32(0x800C0DC8u) + idx * 32u;
    uint32_t cb;

    PE_StoreU16(rec, 2u);
    memcpy(PE_Translate(0x800A3490u, 4u),
           PE_TranslateConst(rec + 0x1Cu, 4u), 4u);
    PE_StoreU32(0x800A3494u, PE_LoadU32(rec + 8u));
    PE_StoreU32(0x800BE9E4u, PE_LoadU32(0x800BE998u));
    cb = PE_LoadU32(0x800B0CC8u);
    if (cb != 0u) {
        Bootstrap_ReturnVoid("func_8007C214_B0CC8_chain", "func_8007C214");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    PE_StoreU32(0x800B89F4u, 0u);
}

/* Guest scratch holding the CdlLOC copy for func_8007F0C8's slot word.
 * Retail keeps those 4 bytes on its own stack (sp+0x31) and stores the
 * stack address into the command packet; a host stack address has no
 * guest meaning, so the bytes live at this documented scratch address
 * (same pattern as PE_698D4_CDLFILE) and the packet carries its guest
 * address.  Content is the verbatim location bytes, never invented. */
#define PE_7F0C8_LOC 0x801FFE80u

/* Phase 6E-CDS1 completion-selector preliminaries, called by the
 * func_8007F0C8 tail below (asm/disc1/6E6C0.s:855, asm/disc1/7018C.s:144).
 *
 * func_8007E8F4 (28 words, 0x8007E8F4..0x8007E964):
 * completion-queue predicate.  Gate lane == 1 via func_8007FBF0(0),
 * address the head record (0x800A3540 + 24 * D_800A3604, the *24 built
 * as sll-1/addu/sll-3 exactly like the 7F0C8 tail), return 0 on a null
 * head, else (func_8007FB44(rec.byte4, rec.word+0xC) != 0). */
int func_8007E8F4(void)
{
    uint32_t head;
    pe_addr_t rec;

    if (func_8007FBF0(0) != 1)
        return 0;
    head = PE_LoadU32(0x800A3604u);
    rec = 0x800A3540u + head * 24u;
    if (PE_LoadU32(rec) == 0u)
        return 0;
    return func_8007FB44(PE_LoadU8(rec + 4u), PE_LoadU32(rec + 0xCu)) != 0;
}

/* Phase 6E-CDS1 — func_8007FB44 (31 words, 0x8007FB44..0x8007FBC0):
 * completion-dispatch predicate.  Returns 0 while the interrupt-flag
 * word D_8009B598 is positive, while the init word D_8009B554 is zero,
 * or while the lane word D_8009B574 is not 1.  Otherwise latches
 * 0x1F @ D_8009B570, lane 2 @ D_8009B574, 0xB @ D_8009B578 (the 0xB
 * store sits in the jal delay slot, ahead of the call) and issues
 * func_8007FCFC(cmd & 0xFF, data, 0x8009B598, 0x8009B554), returning
 * its value.  Incoming a2/a3 are passed through: 7FCFC ignores them
 * (it zeroes a2 itself ahead of the 7B558 call). */
int func_8007FB44(uint32_t cmd, uint32_t data)
{
    if ((int32_t)PE_LoadU32(0x8009B598u) > 0)
        return 0;
    if (PE_LoadU32(0x8009B554u) == 0u)
        return 0;
    if (PE_LoadU32(0x8009B574u) != 1u)
        return 0;
    PE_StoreU32(0x8009B570u, 0x1Fu);
    PE_StoreU32(0x8009B574u, 2u);
    PE_StoreU32(0x8009B578u, 0xBu);
    /* func_8007FCFC (74 words: 7B9EC/80950/7B558 controller dispatch
     * over drive-state bytes) is control-heavy, so it is the new named
     * stop.  Production stops here; the 0 below is never consumed past
     * the stop. */
    Bootstrap_ReturnVoid("func_8007FCFC", "func_8007FB44");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    (void)cmd;
    (void)data;
    return 0;
}

/* func_8007F0C8 — CdlReadS queue issue (asm/disc1/6F684.s, ~130 words).
 *
 * Retail args: a0 = mode byte, a1 = CdlLOC*, a2 = sector count,
 * a3 = per-packet word, stack = destination buffer (-1 = streaming,
 * no buffer).  Queues up to 4 command packets: validates the BCD
 * location (negative LBA fails), gates count to 3..27 via the retail
 * jump-table range check (both arms are instruction-identical past
 * their labels — mechanically diffed — so one transcription serves),
 * fills one 0x18 descriptor per packet (sequence from D_8009B53C with
 * the FFFFFFFF -> 1 wrap guard), bumps D_800A3608 per packet, then the
 * F394 completion selector checks lane == 1 and the head descriptor
 * against the sequence.
 *
 * Frame layout is transcribed exactly (four 16-byte packets at
 * sp+0x10, location bytes at sp+0x31, loc pointer at sp+0x38).
 * The per-iteration gate reads slot+8 (sp+0x48/0x58/0x68/0x78):
 * slot3+8 is a proven retail zero (cleared, never stored) and the
 * rest is retail's own spilled registers — zero-canonicalized here.
 * Hence the 80950 arm is structurally transcribed but never fires
 * in-port; the loc-pointer word at slot2+8 belongs to the Setloc
 * hardware packet and feeds only the collapsed controller chain.
 * All canonicalized bytes feed nothing translated readers consume.
 *
 * The selector tail runs the real Phase 6E-CDS1 preliminaries above:
 * both non-passing arms return the sequence (retail's delay slots move
 * $s5 to $v0), and the passing arm calls func_8007E8F4 (result
 * discarded) before returning the sequence.  Past 7FB44 the new named
 * stop is func_8007FCFC (the 7B558 controller dispatch and the 7C564
 * delivery state machine behind the 7F0C8 completion callback stay
 * unrepresented); lanes are untouched at the new boundary (still idle)
 * and completion stays pending, exactly like retail mid-stream. */
int func_8007F0C8(uint32_t mode, pe_addr_t loc, int count, uint32_t a3,
                  pe_addr_t buf)
{
    /* sp+0x10 base; packets at +0x00/+0x10/+0x20/+0x30. */
    uint8_t fr[0x60];
    uint32_t lba;
    uint32_t seq;
    uint32_t k;
    uint32_t i;

    for (i = 0u; i < sizeof(fr); i++)
        fr[i] = 0u;
    fr[0x00] = 9u;
    fr[0x10] = 0x0Eu;
    fr[0x11] = (uint8_t)(mode & 0xFFu);
    /* 0x8007F144/0x8007F150: the jal delay slot stores $v0 = sp+0x21
     * (the address of the mode byte) into packet 1's gate word
     * (sp+0x28 = fr[0x18]).  The queue loop only tests that word for
     * zero before the 80950 copy of bytes 1..4, so the port keeps a
     * nonzero marker: the stack address has no guest meaning.
     * (Previously left zero, which skipped the SetMode copy arm.) */
    fr[0x18] = 1u;
    lba = (uint32_t)func_80080C48(loc);
    if ((int32_t)lba < 0)
        return 0;
    fr[0x20] = 2u;
    /* Unaligned store pair (swl sp+0x34 / swr sp+0x31) lands the 4
     * location bytes at sp+0x31..sp+0x34 = fr[0x21..0x24]: bytes 1..4
     * of the SetLoc packet (fr base is sp+0x10).  Previously written at
     * fr[0x31..] (Phase FTE1 ASan audit). */
    for (i = 0u; i < 4u; i++) {
        uint8_t b = PE_LoadU8(loc + (pe_addr_t)i);
        fr[0x21u + i] = b;
        PE_StoreU8(PE_7F0C8_LOC + (pe_addr_t)i, b);
    }
    fr[0x28] = (uint8_t)(PE_7F0C8_LOC & 0xFFu);
    fr[0x29] = (uint8_t)((PE_7F0C8_LOC >> 8) & 0xFFu);
    fr[0x2A] = (uint8_t)((PE_7F0C8_LOC >> 16) & 0xFFu);
    fr[0x2B] = (uint8_t)((PE_7F0C8_LOC >> 24) & 0xFFu);
    /* Jump-table range gate: (count & 0xFF) - 3 < 25 unsigned. */
    if ((uint32_t)(((uint32_t)(count & 0xFF)) - 3u) >= 25u)
        return 0;
    fr[0x30] = (uint8_t)(count & 0xFF);
    fr[0x3C] = (uint8_t)(a3 & 0xFFu);
    fr[0x3D] = (uint8_t)((a3 >> 8) & 0xFFu);
    fr[0x3E] = (uint8_t)((a3 >> 16) & 0xFFu);
    fr[0x3F] = (uint8_t)((a3 >> 24) & 0xFFu);
    /* Queue capacity: D_800A3608 + 4 < 9. */
    if (PE_LoadU32(0x800A3608u) + 4u >= 9u)
        return 0;
    seq = PE_LoadU32(0x8009B53Cu) + 1u;
    if (seq == 0u)
        seq = 1u;
    PE_StoreU32(0x8009B53Cu, seq);
    for (k = 0u; k < 4u; k++) {
        /* 0x8007F1AC: $s4 = sp+0x10 = fr[0]; packets walk +0x10 for
         * $s6 = 4 entries (9, 0x0E, 2, command).  A +0x30 base here
         * read past the frame on the fourth pass (ASan, Phase FTE1). */
        uint8_t *sl = fr + k * 16u;
        pe_addr_t desc = func_8007E6B0();
        uint32_t w;

        if (desc == 0u)
            return 0;
        PE_StoreU32(desc, seq);
        PE_StoreU8(desc + 4u, sl[0]);
        w = (uint32_t)sl[8] | ((uint32_t)sl[9] << 8) |
            ((uint32_t)sl[10] << 16) | ((uint32_t)sl[11] << 24);
        if (w == 0u) {
            PE_StoreU32(desc + 0xCu, 0u);
        } else {
            /* func_80080950 copy arm, inlined: the source is a stack
             * address with no guest meaning, so the proven 4-byte
             * copy runs here; standalone 80950 stays exact for its
             * guest-addressed callers. */
            for (i = 0u; i < 4u; i++)
                PE_StoreU8(desc + 5u + (pe_addr_t)i, sl[1 + i]);
            PE_StoreU32(desc + 0xCu, desc + 5u);
        }
        w = (uint32_t)sl[12] | ((uint32_t)sl[13] << 8) |
            ((uint32_t)sl[14] << 16) | ((uint32_t)sl[15] << 24);
        PE_StoreU32(desc + 0x10u, w);
        PE_StoreU32(desc + 0x14u, (uint32_t)buf);
        PE_StoreU32(0x800A3608u, PE_LoadU32(0x800A3608u) + 1u);
    }
    /* F394 completion selector, transcribed exactly: both non-passing
     * arms fall through with seq in $v0 (the addu delay slots), and the
     * passing arm issues the real func_8007E8F4 (result discarded). */
    if (func_8007FBF0(0) != 1)
        return (int)seq;
    {
        uint32_t head = PE_LoadU32(0x800A3604u);
        if (PE_LoadU32(0x800A3540u + head * 24u) != seq)
            return (int)seq;
    }
    (void)func_8007E8F4();
    return (int)seq;
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
