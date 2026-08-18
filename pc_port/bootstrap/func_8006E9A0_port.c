/*
 * Phase 6D-S — Native adaptation of func_8006E9A0 (display setup + clear).
 *
 * HOST_ADAPTED: register pins removed, arena-initialization section
 * removed (func_8006A8D4 in the Boot Rung now handles that before this
 * is called).  Retains the display-init → ClearImage → bootstrap-post →
 * dispatch path.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include <string.h>

int func_8006E9A0(int arg)
{
    RECT rect;

    /* 1. Display init */
    func_80073A44(0);                           /* VSync(0) */
    func_80074D28(0);                           /* SetDispMask(0) */
    func_800755F0(D_800BCE80);                  /* PutDispEnv */

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;                             /* 320 */
    rect.h = 0x1C0;                             /* 448 (clamped to 240) */
    func_80074F44(&rect, 0, 0, 1);              /* ClearImage → THE BLACK FRAME */

    func_80074DC0(0);                           /* DrawSync(0) */

    /* 2. Post-arena calls */
    func_8005E588();
    func_80066B60(2);

    /* 3. Poll loop — bootstrap: single pass */
    {
        int poll_count = 0;
        do {
            func_800752AC(NULL, 0x1000);        /* ClearOTagR */
            func_80068E24();
            func_80070E54();
        } while (++poll_count < 1);
    }

    /* 4. Post-loop */
    D_800B0DC6 = 0;
    func_80038D1C();

    /* 5. Dispatch exit */
    if (arg == 1) {
        D_8009D280 = 0xA80830C8;
    } else if (arg == 3) {
        D_8009D280 = 0xA80651C8;
    }
    return 0;
}
