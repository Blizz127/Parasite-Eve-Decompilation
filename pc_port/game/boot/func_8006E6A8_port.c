/*
 * Phase 6E-B16 — func_8006E6A8: sector-read issue wrapper.
 *
 * Raw body: 11 words / 0x2C, exe 0x8006E6A8–0x8006E6D3, file offset
 * 0x5EEA8, sole live split asm/disc1/5B1E4.s:4429; all 11 instruction
 * words verified exact against the SHA-exact retail executable.
 *
 * Retail semantics: a pure argument shuffle with a stack frame —
 *   func_8006E6A8(a0, a1, a2) → func_8006E6D4(a0, 0, a1, a2)
 * forwarding the return value.  No guest-memory access, no state.
 *
 * UNIT ADAPTATION (evidence-based): retail func_8006E6D4 passes its 4th
 * argument verbatim into func_80080E34 (DsRead family), whose size is a
 * SECTOR count.  The callers in func_8006A9E4 prove it: read cycle B is
 * 34 units followed by a 0x10A50-byte (67792) copy from the destination
 * buffer — 34 sectors = 69632 ≥ 67792 > 34 bytes; cycle D is 3 units
 * followed by a 0x1400-byte (5120) copy — 3 sectors = 6144 ≥ 5120 > 3
 * bytes.  The D_800930DC halfword table values are LBAs relative to
 * D_800B0DD8 (PE.IMG LBA).  The host provider func_8006E6D4 is the 6E-A
 * host adaptation whose documented contract takes a BYTE count
 * (synchronous model, exercised by its own tests), so the sector count
 * is converted at this boundary with exact uint32 wraparound (<< 11).
 * func_8006E834's own verbatim byte-convention call (settled 6E-A
 * behavior) is deliberately not touched by this rung.
 *
 * Classification: 1 — translated retail logic over an already-real
 * provider.  First/repeated/reset behavior: stateless; idempotent.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

int func_8006E6A8(int lba, pe_addr_t dest, int sectors)
{
    return func_8006E6D4(lba, 0, dest, (int)((uint32_t)sectors << 11));
}
