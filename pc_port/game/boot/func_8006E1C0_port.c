/*
 * Phase 6E-B51 — func_8006E1C0: packed texture-entry LoadImage dispatcher.
 *
 * Raw body: 68 words / 0x110 bytes, executable 0x8006E1C0..0x8006E2CF
 * (exclusive end 0x8006E2D0), file offset 0x5E9C0, live split
 * asm/disc1/5B1E4.s:4047-4119 (yaml segment [0x5B1E4, asm]).  All 68 words
 * verified exact against the SHA-exact retail executable
 * (SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * True ABI: int func_8006E1C0(pe_addr_t entry, pe_addr_t base).
 * Retail returns v0=0 (addu $v0,$zero,$zero at 0x8006E2B0); all eight
 * executable call sites discard it (each reloads $v0 from the table header
 * immediately after the jal).  a2/a3 are never read.
 *
 * Caller census (all 8 executable sites; identical shape: a0=entry,
 * a1=base in the jal delay slot, return unconsumed, inside a counted
 * 0x14-stride entry loop):
 *   func_8006914C @0x8006949C   func_8006B4F8 @0x8006B65C
 *   func_8006AD40 @0x8006AE48   func_8006B4F8 @0x8006B740   (B50 site: AE48)
 *   func_8006AD40 @0x8006B1DC   func_8006BECC @0x8006C03C
 *   func_8006AD40 @0x8006B254   func_8006C5BC @0x8006C750
 *
 * Entry record layout (0x14-byte stride in the callers' loops):
 *   +0x4  u32  image data offset from base (low 24 bits used)
 *   +0x7  u8   image rect h; retail substitutes 0x100 when zero
 *   +0x8  u32  packed image rect: x = (w>>10)&0x7FF, y = w>>21, w = w&0x3FF
 *   +0xC  u32  CLUT data offset from base (low 24 bits; zero = no CLUT call)
 *   +0xF  u8   CLUT rect h (no zero substitution)
 *   +0x10 u32  packed CLUT rect (same decode as +0x8)
 *
 * ROM-order operation map (retail $gp = 0x8009CD70; this body never
 * touches $gp, rodata, or any global — its only persistent effects are the
 * two callee dispatches):
 *   prologue  addiu $sp,-0x28; save $s0/$s1/$s2/$ra; s0=a0, s2=a1
 *   1. lw [s0+8]  x3 (three separate retail loads) -> sh halfwords to the
 *      stack rect at sp+0x10/+0x12/+0x14.
 *   2. lbu [s0+7]; beqz with delay-slot addiu $v1,0x100: h = byte ? byte :
 *      0x100; sh -> sp+0x16.  s1 = 0x00FFFFFF (lui/ori).
 *   3. lw [s0+4] & s1; a0 = sp+0x10;
 *      jal func_8007506C @0x8006E238, delay slot a1 = s2 + a1.
 *   4. lw [s0+0xC] & s1; beqz -> epilogue (delay slot a0 = sp+0x10).
 *   5. CLUT block: lw [s0+0x10] x3 -> sh sp+0x10/+0x12/+0x14;
 *      lbu [s0+0xF] -> sh sp+0x16 (no 0x100 substitution);
 *      lw [s0+4] & s1 + s2;  lw [s0+0xC] & s1;
 *      jal func_8007506C @0x8006E2A8, delay slot a1 = v0 + a1
 *      (= base + image_off + clut_off).
 *   6. v0 = 0; restore; jr $ra; nop.
 *
 * Callback audit (func_8007506C = PsyQ LoadImage, string-proven in
 * docs/ai_context/sdk_map.md; 24 words at 0x8007506C, live split
 * asm/disc1/654C8.s:283-310):
 *   LoadImage(rect, data) calls the read-only debug validator
 *   func_80074E28("LoadImage", rect) — its 0x11C body stores only to its
 *   own stack frame — then dispatches through the libgpu jump table:
 *   a0 = lw(lw(D_80095744)+0x20), jalr lw(lw(D_80095744)+0x8), a2 = 8,
 *   a3 = data (delay slot).
 *   D_80095744 is statically initialized to 0x80095704 (the 16-word "jtb")
 *   and has no runtime store anywhere in the executable (all 25 references
 *   are lui/lw reads; ResetGraph — the only pre-B51 setup caller — never
 *   writes it or the jtb).  Therefore both dispatches resolve identically
 *   and statically:
 *     target  = jtb[2] = 0x80076C34 (libgpu queue/transfer manager)
 *     a0      = jtb[8] = 0x80076664 (immediate worker, inner jalr $s3)
 *   The callback's state effects (GPU command packet ring at D_800BD03C,
 *   queue indices D_80095874/78/7C, env flag D_80095754, VRAM transfer)
 *   are NOT consumed by this function: both returns are discarded, the
 *   stack rect is rebuilt from entry fields between the calls, and every
 *   post-call load reads the caller-supplied entry record.  An honest
 *   centralized boundary therefore preserves all required state.
 *
 * Boundary representation: the retail rect lives at guest sp+0x10 and is
 * visible only to the unresolved callee, so it has no guest authority in
 * the port.  The diagnostic record carries the rect's exact packed words
 * (arg0 = x|y<<16, arg2 = w|h<<16), the true retail a1 data address
 * (arg1), and arg3 = 0.  Nothing is written to guest RAM for it.
 *
 * Classification: 1 — translated retail logic with an honest centralized
 * LoadImage boundary.  func_8007506C itself is NOT translated (B51 scope
 * limit); strict execution stops at it as the next genuine provider.
 */
#include "psx_compat.h"
#include "game_port.h"

int func_8006E1C0(pe_addr_t entry, pe_addr_t base)
{
    uint32_t dims;
    uint32_t rect0;
    uint32_t rect1;
    uint32_t h;
    uint32_t image_off;
    uint32_t clut_off;
    pe_addr_t data;

    /* 0x8006E1D8/0x8006E1EC/0x8006E1FC: three separate retail loads of
     * the packed dimension word at entry+8 (reproduced via PE_LoadU32). */
    dims  = PE_LoadU32(entry + 8u);
    rect0 = (dims >> 10) & 0x7FFu;              /* sh -> sp+0x10: x */
    dims  = PE_LoadU32(entry + 8u);
    rect0 |= (dims >> 21) << 16;                /* sh -> sp+0x12: y */
    dims  = PE_LoadU32(entry + 8u);
    rect1 = dims & 0x3FFu;                      /* sh -> sp+0x14: w */

    /* 0x8006E20C-0x8006E21C: lbu entry+7; beqz with delay-slot
     * addiu $v1,0x100; fall-through andi $v1,$v0,0xFF. */
    h = PE_LoadU8(entry + 7u);
    if (h == 0u)
        h = 0x100u;
    else
        h &= 0xFFu;
    rect1 |= h << 16;                           /* sh -> sp+0x16: h */

    /* 0x8006E22C-0x8006E234: a1 = lw [s0+4] & 0x00FFFFFF. */
    image_off = PE_LoadU32(entry + 4u) & 0x00FFFFFFu;
    data = base + image_off;                    /* delay slot 0x8006E23C */

    /* 0x8006E238: jal func_8007506C (PsyQ LoadImage) — centralized
     * unresolved boundary, callback #1 (image). */
    Bootstrap_ReturnVoid4("func_8007506C", "func_8006E1C0",
                          rect0, data, rect1, 0);

    /* 0x8006E240-0x8006E24C: v0 = lw [s0+0xC] & 0x00FFFFFF; beqz. */
    clut_off = PE_LoadU32(entry + 0xCu) & 0x00FFFFFFu;
    if (clut_off != 0u) {
        /* 0x8006E254-0x8006E290: CLUT rect from entry+0x10 and the raw
         * byte at entry+0xF (no 0x100 substitution here). */
        dims  = PE_LoadU32(entry + 0x10u);
        rect0 = (dims >> 10) & 0x7FFu;
        dims  = PE_LoadU32(entry + 0x10u);
        rect0 |= (dims >> 21) << 16;
        dims  = PE_LoadU32(entry + 0x10u);
        rect1 = dims & 0x3FFu;
        rect1 |= (uint32_t)PE_LoadU8(entry + 0xFu) << 16;

        /* 0x8006E294-0x8006E2A4: v0 = (lw [s0+4] & mask) + base;
         * a1 = lw [s0+0xC] & mask; delay slot a1 = v0 + a1. */
        data = base + (PE_LoadU32(entry + 4u) & 0x00FFFFFFu)
                    + (PE_LoadU32(entry + 0xCu) & 0x00FFFFFFu);

        /* 0x8006E2A8: jal func_8007506C — callback #2 (CLUT). */
        Bootstrap_ReturnVoid4("func_8007506C", "func_8006E1C0",
                              rect0, data, rect1, 0);
    }

    /* 0x8006E2B0: addu $v0,$zero,$zero — retail zero return. */
    return 0;
}
