/* VRAM 0x80084F8C / file 0x7578C / size 0x2C.
 * Controller-record readiness: return 1 when the halfword at +0xE6 is zero,
 * else 1 when the byte at +0x46 is not 0xFF, else 0.
 *
 * The short-circuit form with an assignment side effect (recovered by m2c
 * from retail's `addu $v0,$zero,$zero` in the beq delay slot) is required:
 * a plain `if/else if/else` returning result makes era cc1 fold the tail
 * into `xori/sltu` (branchless), losing the second `beq`. era -O2 -G0. */
int func_80084F8C(unsigned char *record) {
    int result;

    if (*(unsigned short *)(record + 0xE6) == 0 || (result = 0, record[0x46] != 0xFF)) {
        return 1;
    }
    return result;
}
