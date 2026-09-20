/* VRAM 0x8004BCB4 / file 0x3C4B4 / size 0x34.
 * Set a message option then submit text id 0x19. */
extern void func_8005E8A4(int a0, int a1);
extern int func_8005DC4C(int a0);
extern void func_8005F594(int a0);
void func_8004BCB4(void) {
    func_8005E8A4(0, 4);
    func_8005F594(func_8005DC4C(0x19));
}
