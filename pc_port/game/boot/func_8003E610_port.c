/* PE_PORT from exact C (28 words, 31 lines). No register pins, no adaptations needed. */
#include "psx_compat.h"
extern void func_80073C94(void), func_8003E754(int,int), func_8007D054(void), func_80077F7C(void);
extern void func_80079004(int,int), func_80079024(int), func_800409B4(void), func_8003E944(void);
extern void func_8007EC14(void), func_80080CC8(int);
void func_8003E610(void) {
    func_80073C94();
    func_8003E754(0x140, 0xE0);
    func_8007D054();
    func_80077F7C();
    func_80079004(0xA0, 0x70);
    func_80079024(0xF0);
    func_800409B4();
    func_8003E944();
    func_8007EC14();
    func_80080CC8(0);
}
