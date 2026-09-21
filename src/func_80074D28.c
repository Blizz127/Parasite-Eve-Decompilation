/*
 * func_80074D28 — boot/main-loop scene-transition trampoline.
 *
 * VRAM 0x80074D28 / file 0x65528 / size 0x98 (38 words).
 * Boot-spine tail: pairs with func_80074CC8 (fan-in 16..32 on the
 * boot -> Day-1 -> Day-2 route, callers in the 0x8001xxxx / 0x8003xxxx
 * clusters).
 *
 * Shape (asm/disc1/654C8.s):
 *   - if the global state byte D_8009574E >= 2, call the debug-log vector
 *     D_80095748(D_80011870, a0);
 *   - if a0 == 0, clear the 0x14-byte region at D_8009574E + 0x6A via
 *     func_80077A28(..., -1, 0x14);
 *   - tail-call the renderer slot D_80095744->f(0x10) with the packed command
 *     0x3000000 when a0 != 0, else 0x3000001 (the +1 is set in the branch
 *     delay slot of the `beqz`).
 *
 * Levers:
 *   - `s0`/`s1` are pinned to `$16`/`$17` so cc1 keeps the incoming argument
 *     and the D_8009574E base there (retail's saved-register order is
 *     s1 then s0);
 *   - the condition is written `s0 == 0 ? <one> : <other>` to reproduce
 *     retail's `beqz s0` / `ori a0,a0,1` (the delay-slot set) / second
 *     `lui a0,0x300` fall-through;
 *   - MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 (patch 3) moves `addiu sp,sp,0x20`
 *     into the `jr $31` slot.
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern unsigned char D_8009574E;
extern int (*D_80095748)(char *, int);
extern struct D44 { char pad[0x10]; void (*f)(int); } *D_80095744;
extern void func_80077A28(char *a0, int a1, int a2);
extern char D_80011870;

void func_80074D28(int a0) {
    register unsigned char *s1 asm("$17") = &D_8009574E;
    register int s0 asm("$16") = a0;
    char *p;

    if (s1[0] >= 2)
        D_80095748(&D_80011870, s0);
    if (s0 == 0)
        func_80077A28((char *)s1 + 0x6A, -1, 0x14);
    p = (char *)D_80095744;
    (*(void (**)(int))(p + 0x10))(s0 ? 0x3000000 : 0x3000001);
}
