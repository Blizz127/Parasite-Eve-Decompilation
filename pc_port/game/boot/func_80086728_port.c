/*
 * Phase 6E-B38 — func_80086728: streaming-mode command-byte switch.
 *
 * Retail body: 18 words / 0x48 bytes,
 * 0x80086728..0x8008676C (exclusive end 0x80086770), file offset
 * 0x76F28 (PS-X EXE).  All 18 words exe-verified against
 * SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Maps the caller's argument to a CD streaming command byte and
 * dispatches via func_8008CBA8 (the streaming command dispatcher,
 * already translated in pe_stream.c):
 *
 *   a0 == 1 → cmd = 0x81
 *   a0 == 2 → cmd = 0x82
 *   else    → cmd = 0x80
 *
 * Stores the command at D_800BCD80, calls func_8008CBA8(), returns void.
 *
 * Retail delay-slot semantics:
 *   beq $a0,1 case_1  →  sw $ra,0x10($sp) (always)
 *   beq $a0,2 common  →  addiu $v0,$zero,0x82 (always; sets cmd for case 2)
 *   j   common        →  addiu $v0,$zero,0x80 (always; sets cmd for default)
 *   jal func_8008CBA8 →  nop
 *   jr  $ra           →  nop
 *
 * Sole call site: func_80052790 @ 0x8005279C.
 * func_80052790 passes a0 = (original_a0 < 1) ? 1 : 0, so the
 * reachable commands from that caller are 0x81 (true) and 0x80 (false).
 * The a0 == 2 path (cmd 0x82) is unreachable from func_80052790 but
 * exists in the retail binary for potential other callers.
 *
 * Classification: 1 — translated retail logic over already-real provider.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

#define GA_D_800BCD80 0x800BCD80u

void func_80086728(int a0)
{
    uint32_t cmd;

    if (a0 == 1)
        cmd = 0x81;
    else if (a0 == 2)
        cmd = 0x82;
    else
        cmd = 0x80;

    PE_StoreU32(GA_D_800BCD80, cmd);
    func_8008CBA8();
}

#define GA_D_800BCD84 0x800BCD84u
#define GA_D_800BCD90 0x800BCD90u

/*
 * func_80086464 is 13 words (0x80086464..0x80086498):
 * D_800BCD80=0x10, D_800BCD84=a0, jal 8CBA8. Cmd 0x10 is
 * 85084(*CD84); magic fail returns -1. Stream-complete is
 * not invented.
 */
void func_80086464(pe_addr_t a0)
{
    PE_StoreU32(GA_D_800BCD80, 0x10u);
    PE_StoreU32(GA_D_800BCD84, a0);
    (void)func_8008CBA8();
}

/*
 * func_80086C1C is 16 words (0x80086C1C..0x80086C5C):
 * D_800BCD80=0xC0, D_800BCD84=a1&0x7F, D_800BCD90=a0, jal 8CBA8.
 * 0xC0 is the default ring-enqueue path.
 */
void func_80086C1C(int a0, int a1)
{
    PE_StoreU32(GA_D_800BCD80, 0xC0u);
    PE_StoreU32(GA_D_800BCD84, (unsigned int)a1 & 0x7Fu);
    PE_StoreU32(GA_D_800BCD90, (unsigned int)a0);
    (void)func_8008CBA8();
}
