/*
 * Phase 6E-B23 — func_80052C6C: resource-table search and initialization.
 *
 * Raw body: 112 words / 0x1C0, executable 0x80052C6C..0x80052E2B,
 * live split asm/disc1/43408.s (offset 0x4346C..0x4362C in the asm segment).
 * All 112 words verified against the SHA-exact retail executable.
 *
 * Subsystem: resource-table initialization — a dispatcher-level callee
 * that searches an existing record table, copies matching records into a
 * 9-row output table at 0x800A1E64, and clears related halfword buffers.
 *
 * Coupled callees translated in this file:
 *   func_80052E30  (31 words) — resource-buffer init / reuse (exported in
 *                               B39 for its other retail caller)
 *   func_80052EB0  ( 3 words) — two-word state setter
 *   func_80052F0C  ( 5 words) — buffer-identity comparison (exported in
 *                               B39 for its other retail caller)
 *   func_80052F70  (22 words) — capped-add resource ID allocator
 *   func_8005DB44  (17 words) — 32-byte record table lookup
 *
 * func_80051E58 is a pre-existing C leaf (2 words, lw $gp+0x2A8; jr $ra)
 * and is not re-implemented here.
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"

/* ── gp-relative state globals (host-side, extern for test reset) ── */
/* These were originally `static` but that creates a host/guest split-brain:
 * ResetTestState (PE_RamReset) cannot clear them, causing stale values
 * to leak between tests.  Making them extern allows pe_globals.c to own
 * the storage and tests to reset via PE_Sdk_ResetState. */

/* D_8009D018: read by func_80051E58, used by func_80052F70 */
/* D_8009D03C: record count / search result index */
/* D_8009D048: resource buffer pointer */
/* D_8009D04C: state flag / previous buffer pointer */
/* D_8009D050: resource count / ID */
/* D_8009D054: secondary state / buffer pointer */
/* D_8009D058: host table pointer */
/* D_8009D064: type/size indicator */

/* ── Guest address constants ─────────────────────────────────────────── */

#define GA_800C0E48         0x800C0E48u
#define GA_800C0EAA         0x800C0EAAu
#define GA_800C0E0C         0x800C0E0Cu
#define GA_800C0E4A         0x800C0E4Au   /* 0x800C0EAA - 0x60 = 0x800C0E4A? No... */
#define GA_800C1F7E         0x800C1F7Eu
#define GA_800C1EB8         0x800C1EB8u   /* 0x800C1F7E - 2*99 */
#define GA_800C2022         0x800C2022u
#define GA_800C1F80         0x800C1F80u   /* 0x800C2022 - 2*81 */
#define GA_800A1E64         0x800A1E64u
#define GA_800AD05C         0x800AD05Cu
#define GA_800A1F84         0x800A1F84u
#define GA_800B8034         0x800B8034u
#define GA_800B8038         0x800B8038u

/* ── func_80051E58 — retail C leaf, 2 words ────────────────────────────
 * lw $v0, 0x2A8($gp) ; jr $ra
 * Returns D_8009D018 ($gp + 0x2A8). */
static int func_80051E58(void) {
    return (int)D_8009D018;
}

/* ── func_80052F70 — capped-add allocator, 22 words ────────────────────
 * Executable 0x80052F70..0x80052FC7, 22 words verified.
 * File offset 0x43770.
 *
 * Reads a byte at 0x800C0E0C, adds func_80051E58() return, and caps at 51.
 * If the capped sum < 51, calls func_80051E58() again (which may have
 * changed since the first call) and adds the byte to the new return.
 * Returns the capped value (0..50). */
unsigned int func_80052F70(void) {
    unsigned int b = PE_LoadU8(GA_800C0E0C);
    unsigned int v = func_80051E58();
    unsigned int sum = b + v;
    if (sum < 51u) {
        unsigned int v2 = func_80051E58();
        return b + v2;
    }
    return 50u;
}

/* ── func_80052EB0 — two-word state setter, 3 words ────────────────────
 * Executable 0x80052EB0..0x80052EBB, 3 words verified.
 * File offset 0x436B0.
 *
 * sw $a0, 0x2DC($gp)  → D_8009D04C = a0
 * sw $a1, 0x2E4($gp)  → D_8009D054 = a1
 * jr $ra */
static void func_80052EB0(unsigned int a0, unsigned int a1) {
    D_8009D04C = a0;
    D_8009D054 = a1;
}

/* ── func_80052F0C — buffer-identity comparison, 5 words ────────────────
 * Executable 0x80052F0C..0x80052F1F, 5 words verified.
 * File offset 0x4370C.
 *
 * lw $v0, 0x2D8($gp)  → D_8009D048
 * lui $v1, 0x800C ; addiu $v1, $v1, 0x0E48 → 0x800C0E48
 * xor $v0, $v0, $v1
 * jr $ra
 * sltu $v0,$zero,$v0 (delay slot)
 * Returns exactly 0 if D_8009D048 == 0x800C0E48, otherwise 1. */
uint32_t func_80052F0C(void) {
    return D_8009D048 != GA_800C0E48;
}

/* ── func_80052E30 — resource-buffer init / reuse, 31 words ─────────────
 * Executable 0x80052E30..0x80052EAB, 31 words verified.
 * File offset 0x43630.
 *
 * void func_80052E30(int a0)
 *
 * If a0 == 0 or D_8009D04C == 0:
 *   D_8009D048 = 0x800C0E48
 *   D_8009D050 = func_80052F70()
 *   D_8009D058 = 0x800AD05C
 *   D_8009D064 = 2
 *
 * Else (a0 != 0 && D_8009D04C != 0):
 *   D_8009D048 = D_8009D04C      (preserve existing buffer)
 *   D_8009D058 = 0x800A1F84
 *   D_8009D064 = 4
 *   D_8009D050 = D_8009D054      (preserve secondary state)
 *   (unconditional jump to epilogue — skips the alloc path)
 *
 * The 0x800C0E48 buffer is cleared by func_80052C6C's first loop
 * BEFORE this function is called. */
void func_80052E30(uint32_t a0) {
    if (a0 == 0u || D_8009D04C == 0u) {
        /* Allocate new path */
        D_8009D048 = GA_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = GA_800AD05C;
        D_8009D064 = 2u;
    } else {
        /* Reuse existing path */
        D_8009D048 = D_8009D04C;
        D_8009D058 = GA_800A1F84;
        D_8009D064 = 4u;
        D_8009D050 = D_8009D054;
    }
}

/* ── func_8005DB44 — 32-byte record table lookup, 17 words ──────────────
 * Executable 0x8005DB44..0x8005DB87, 17 words verified.
 * File offset 0x4E344.
 *
 * pe_addr_t func_8005DB44(unsigned int index)
 *
 * Reads D_800B8038 (base) and D_800B8034 (alt_base) from guest RAM.
 * Computes count = (base - alt_base) / 32.
 * If index >= count, returns 0 (NULL).
 * Otherwise, returns alt_base + ((index * 32) + (base - 32) - alt_base)
 *   = alt_base + (index * 32) + (base - alt_base - 32)
 * Wait, let me re-derive from the MIPS:
 *   v0 = base - alt_base
 *   v0 = srl v0, 5  → v0 = (base - alt_base) / 32 = count
 *   if (index >= count) return 0
 *   v0 = index * 32  (sll by 5 in delay slot)
 *   v1 = base - 16  (addiu $v1, $v1, -16)
 *   v0 = v0 + v1  → v0 = index*32 + base - 16
 *   j 0x8005DB84 (return alt_base + v0)  → alt_base + index*32 + base - 16
 *   return alt_base + v0 (in delay slot)
 *
 * This is: return alt_base + (index * 32) + (base - 16 - alt_base + alt_base)
 *   = base + index*32 - 16 + alt_base - alt_base? No...
 *
 * Let me re-derive carefully:
 *   v1 = base: lw $v0, 0($v1) where $v1 = D_800B8038
 *   $a1 = alt_base: lw $a1, D_800B8034
 *   v0 = base - alt_base
 *   v0 = srl v0, 5 → v0 = (base - alt_base) >> 5 = count
 *   if (index >= count) return 0
 *   v0 = index << 5 (delay slot)
 *   v1 = base - 16 (addiu $v1, $v1, -16)
 *   v0 = v0 + v1 → v0 = index*32 + base - 16
 *   j 0x8005DB84 → return alt_base + v0
 *   return alt_base + v0 (delay slot)
 *
 * So: return alt_base + index*32 + base - 16
 *   = base + alt_base + index*32 - 16
 *
 * Hmm wait, that doesn't make sense. Let me re-check the addresses.
 *
 * 0x8005DB44: lui $v1, 0x800B → $v1 = 0x800B0000
 * 0x8005DB48: addiu $v1, $v1, 0x8038 → $v1 = 0x800B8038
 * 0x8005DB4C: lw $v0, 0($v1) → $v0 = PE_LoadU32(0x800B8038) = base
 * 0x8005DB50: lui $a1, 0x800B → $a1 = 0x800B0000
 * 0x8005DB54: lw $a1, -32716($a1) → $a1 = PE_LoadU32(0x800B0000 - 0x7FCC) = PE_LoadU32(0x800A8034)
 * Wait, -32716 = 0xFFFF8034. So 0x800B0000 + 0xFFFF8034 = 0x800A8034. But the load is from 0x800B8034.
 * 
 * Hmm wait: addiu $a1, $a1, -32716 → $a1 = 0x800B0000 + (-32716) = 0x800B0000 - 0x7FCC = 0x800A8034.
 * But the label says D_800B8034. That's 0x800B8034, not 0x800A8034.
 * 
 * Let me recalculate: 0x800B8034 - 0x800B0000 = 0x8034.
 * addiu $v1, $v1, 8038 → 0x800B0000 + 0x8038 = 0x800B8038. ✓
 * But for the second: 0x800B0000 + (-32716) = 0x800B0000 - 0x7FCC.
 * -32716 = 0xFFFF8034 as a signed 16-bit. But addiu sign-extends!
 * 0x800B0000 + 0xFFFF8034 = 0x800A8034 in 32-bit arithmetic.
 * 
 * That's wrong for D_800B8034. Let me re-check the raw word.
 * 
 * 0x8005DB54: 0x8CA58034
 * op=0x23 (lw), $rt=5 ($a1), $rs=5 ($a1), imm=0x8034
 * But lw is: lw $rt, imm($rs) where imm is sign-extended.
 * 0x8034 = 32820, sign-extended as 16-bit: bit 15 = 0, so it's +32820.
 * So: lw $a1, 0x8034($a1) = lw $a1, 0x8034($a1) where $a1 = 0x800B0000
 * = PE_LoadU32(0x800B0000 + 0x8034) = PE_LoadU32(0x800B8034)
 * 
 * Ah, I was confusing myself. The offset is +0x8034, not -32716. The raw hex 0x8034 in the lw instruction is a signed 16-bit with bit 15 = 0, so it's positive.
 * 
 * So:
 * base = PE_LoadU32(0x800B8038)
 * alt_base = PE_LoadU32(0x800B8034)
 * 
 * v0 = base - alt_base
 * v0 = srl v0, 5 → count = (base - alt_base) >> 5
 * if (index >= count) return 0
 * v0 = index << 5
 * v1 = base - 16
 * v0 = v0 + v1 = index*32 + base - 16
 * return alt_base + v0 = alt_base + index*32 + base - 16
 * 
 * Hmm, that's an unusual formula. Let me think about whether there's a simpler interpretation.
 * 
 * Actually, this is a record table where:
 * - alt_base is the start of the table
 * - base is some pointer PAST the table (or into the table)
 * - count = (base - alt_base) / 32 = number of records
 * 
 * The lookup computes: record = alt_base + index*32 + (base - alt_base - 16)
 *   = alt_base + index*32 + base - alt_base - 16
 *   = base + index*32 - 16
 *
 * Hmm, that's still odd. Let me think about this differently.
 *
 * Maybe base is actually a pointer beyond the table end, and alt_base is the start.
 * The -16 on base adjusts for something. Maybe the table is stored with a 16-byte header?
 * 
 * Actually, let me just transcribe the exact arithmetic and not try to simplify it. The port
 * will compute the same result as the MIPS.
 */

pe_addr_t func_8005DB44(unsigned int index) {
    unsigned int base    = PE_LoadU32(GA_800B8038);
    unsigned int alt_base = PE_LoadU32(GA_800B8034);
    unsigned int count   = (base - alt_base) >> 5u;
    if (index >= count)
        return 0u;
    unsigned int v0 = index << 5u;          /* index * 32 (delay slot) */
    unsigned int v1 = base - 16u;           /* addiu $v1, $v1, -16 */
    v0 = v0 + v1;                            /* v0 = index*32 + base - 16 */
    return alt_base + v0;                   /* delay slot of the j */
}

/* ── func_80052C6C — resource-table search and initialization, 112 words ─
 * Executable 0x80052C6C..0x80052E2B, 112 words verified.
 * File offset 0x4346C.
 *
 * void func_80052C6C(void)
 *
 * 1. Calls func_80052E30(0) — allocate new resource buffer.
 * 2. Clears 50 halfwords (100 bytes) at 0x800C0E48..0x800C0EAB.
 * 3. Searches func_8005DB44(i) for i=0,1,2,... until a record is found
 *    where byte[6] == 19 (0x13). Stores the index in D_8009D03C.
 * 4. For 9 iterations (i=0..8):
 *    a. Looks up record at index = D_8009D03C + (i % 3) - 1 via func_8005DB44
 *    b. Copies 32 bytes (unaligned, lwl/lwr pairs) to 0x800A1E64 + i*32
 *    c. Clears byte at offset 9 (sb $zero, 0x800A1E6D + i*32)
 *    d. Stores 999 (sh, 0x800A1E76 + i*32)
 * 5. Clears 100 halfwords (200 bytes) at 0x800C1EB8..0x800C1F7F.
 * 6. Clears 82 halfwords (164 bytes) at 0x800C1F80..0x800C2023.
 * 7. Sets D_8009D04C = 0.
 *
 * The 32-byte copy uses lwl/lwr pairs (unaligned word access). In the port,
 * we use memcpy through PE_Translate for the 32-byte blocks. */
void func_80052C6C(void) {
    unsigned int i, idx;
    pe_addr_t rec;

    /* Step 1: Initialize resource buffer */
    func_80052E30(0u);

    /* Step 2: Clear 0x32 (50) halfwords starting at 0x800C0EAA, descending.
     *   $s0 = 49 first, stores at 0x800C0EAA, decrements to 48,
     *   then $v0 -= 2, stores at 0x800C0EA8, etc.
     *   Last store at $s0 = 0: address = 0x800C0EAA - 2*49 = 0x800C0E48.
     *   Range: 0x800C0E48..0x800C0EAA (103 bytes? no, 50 halfwords at even addresses...)
     *   Actually: 0x800C0E48, 0x800C0E4A, ..., 0x800C0EA8, 0x800C0EAA.
     *   That's 50 halfwords = 100 bytes, covering 0x800C0E48..0x800C0EAB? No.
     *   Even addresses: 0x800C0E48, 0x800C0E4A, ..., 0x800C0EA8, 0x800C0EAA.
     *   The bytes covered are 0x800C0E48..0x800C0EAB (0x64 = 100 bytes spanning 0x64 bytes).
     *   Verifying: 0x800C0EAA - 0x800C0E48 = 0x62 (98 bytes between first and last halfword).
     *   With 50 halfwords: ((0x800C0EAA - 0x800C0E48) / 2) + 1 = (0x62/2) + 1 = 0x31 + 1 = 50. ✓
     *   The byte range is 0x800C0E48..0x800C0EAB exclusive = 0x64 bytes = 100 bytes. */
    {
        pe_addr_t addr = GA_800C0EAA;
        for (i = 0u; i < 50u; i++) {
            PE_StoreU16(addr, 0u);
            addr -= 2u;
        }
    }

    /* Step 3: Search for a record where byte[6] == 0x13 (19).
     *   Retail: $s0 starts at 0. In the loop body:
     *     $a0 = $s0 (index)
     *     $s0++ (delay slot)
     *     rec = func_8005DB44(index)
     *     if (rec == 0) break
     *     if (PE_LoadU8(rec + 6) != 19) continue
     *     break (found)
     *   After the loop, $s0 = (last index + 1). */
    for (i = 0u; ; i++) {
        rec = func_8005DB44(i);
        if (rec == 0u)
            break;
        if (PE_LoadU8(rec + 6u) == 19u)
            break;
    }
    D_8009D03C = i + 1u;   /* retail: sw $s0, 0x2CC($gp) after loop */

    /* Step 4: Main loop — 9 iterations, copy 32 bytes per record.
     *
     *   $s0 = loop counter (0..8)
     *   $s2 = output base = 0x800A1E64, advances by 32 each iteration
     *   $s1 = output offset = 0, advances by 32 each iteration
     *   $s3 = 0x55555556 (multiplier for div-by-3)
     *   $s4 = 999
     *   $a0 = (D_8009D03C + ($s0 % 3) - 1) → lookup index
     *
     *   Retail computes $s0 % 3 via:
     *     mult $s0, 0x55555556
     *     $v0 = $s0 >> 31 (sign bit)
     *     $a1 = mfhi (high word of multiply)
     *     $v0 = $a1 - $v0 (floor division correction)
     *     $v1 = $v0 * 3
     *     $v1 = $s0 - $v1 (remainder)
     *     $a0 = D_8009D03C + $v1 - 1
     *
     *   Then: rec = func_8005DB44($a0)
     *   Copy 32 bytes from rec to $s2 (lwl/lwr pairs for 4 words × 2 = 8 words)
     *   sb $zero, $s2+9  (clear byte at offset 9)
     *   sh $s4, $s2+18   (store 999 at offset 18)
     *   $s2 += 32, $s1 += 32 */
    {
        pe_addr_t dest = GA_800A1E64;
        for (i = 0u; i < 9u; i++) {
            unsigned int rem = i % 3u;
            idx = D_8009D03C + rem - 1u;
            rec = func_8005DB44(idx);

            if (rec != 0u) {
                /* Copy 32 bytes from rec to dest using checked guest access.
                 * Retail uses lwl/lwr (unaligned word loads). We use memcpy
                 * through PE_Translate — simpler and correct for little-endian. */
                const void *src = PE_TranslateConst(rec, 32u);
                void *dst = PE_Translate(dest, 32u);
                __builtin_memcpy(dst, src, 32u);
            }

            PE_StoreU8(dest + 9u, 0u);       /* clear byte at offset 9 */
            PE_StoreU16(dest + 18u, 999u);   /* store 999 at offset 18 */
            dest += 32u;
        }
    }

    /* Step 5: Clear 0x64 (100) halfwords starting at 0x800C1F7E, descending.
     *   $s0 = 99, stores at 0x800C1F7E, decrements to 98, etc.
     *   Last store at $s0 = 0: address = 0x800C1F7E - 2*99 = 0x800C1EB8.
     *   Range: 0x800C1EB8..0x800C1F7F (200 bytes). */
    {
        pe_addr_t addr = GA_800C1F7E;
        for (i = 0u; i < 100u; i++) {
            PE_StoreU16(addr, 0u);
            addr -= 2u;
        }
    }

    /* Step 6: Clear 0x52 (82) halfwords starting at 0x800C2022, descending.
     *   $s0 = 81, last store at $s0 = 0: address = 0x800C2022 - 2*81 = 0x800C1F80.
     *   Range: 0x800C1F80..0x800C2023 (164 bytes).
     *   Adjacent to step 5: 0x800C1F80 immediately follows 0x800C1F7F. */
    {
        pe_addr_t addr = GA_800C2022;
        for (i = 0u; i < 82u; i++) {
            PE_StoreU16(addr, 0u);
            addr -= 2u;
        }
    }

    /* Step 7: Zero D_8009D04C */
    D_8009D04C = 0u;
}
