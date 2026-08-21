/* Opcode 0x6F enemy-body allocator.
 * VRAM 0x8002F7D8 / file 0x1FFD8 / size 0x198 (102 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li
 * + MASPSX_THREE_WORD_SYMBOL_STORE=1.
 *
 * Copies the 216-byte ROM template at D_800109B0 to stack, walks the
 * 7 x 220-byte SlotRecord table, claims the first free slot, copies the
 * template into the body, publishes the body pointer through *out.
 */
typedef struct {
    unsigned int inUse;
    unsigned char body[216];
} SlotRecord;

extern SlotRecord D_800A5D58[];
extern unsigned int D_800109B0[];
extern unsigned char D_8009D2EC;
extern unsigned char D_8009D2A0;

extern void func_8001A680(unsigned char *actor, int cmd);

void func_8002F7D8(unsigned char **out) {
    unsigned int tmp[54]; /* 216 bytes */
    unsigned char i;
    unsigned int *src;
    unsigned int *dst;
    unsigned int *end;
    unsigned char *body;

    src = D_800109B0;
    dst = tmp;
    end = src + 52; /* 208 bytes */
    while (src != end) {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        src += 4;
        dst += 4;
    }
    dst[0] = src[0];
    dst[1] = src[1];

    for (i = 0; i < 7; i++) {
        if (D_800A5D58[i].inUse == 0) {
            D_800A5D58[i].inUse = 1;
            body = D_800A5D58[i].body;
            src = tmp;
            dst = (unsigned int *)body;
            end = src + 52;
            while (src != end) {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = src[3];
                src += 4;
                dst += 4;
            }
            dst[0] = src[0];
            dst[1] = src[1];
            *out = body;
            D_8009D2EC = (unsigned char)(D_8009D2EC + 1);
            body[7] = D_8009D2EC;
            *(unsigned int *)(body + 8) = 1u << i;
            if (((*(unsigned int *)((unsigned char *)out + 0x98)) & 0x2000) == 0) {
                unsigned int *slot = *(unsigned int **)out;
                slot[6] = (unsigned int)(slot + 7); /* +0x18 = +0x1C */
                func_8001A680((unsigned char *)out, 2);
                D_8009D2A0 = (unsigned char)(D_8009D2A0 + 1);
            }
            return;
        }
    }
}
