/*
 * PARKED DRAFT — func_8004C608 (inventory help selection), not a matching leaf.
 *
 * VRAM 0x8004C608 / file 0x3CE08 / size 0x648 (402 words, 0x8004C608..0x8004CC50).
 * Retail authority: asm/disc1/3CE08.s (currently emitted as `[0x3CE08, asm]`
 * in configs/USA/disc1.yaml).  This file is a faithful C reconstruction of the
 * complete original control flow, including the 61-entry jump table
 * jtbl_8001104C (0x8001104C).  It is NOT registered in disc1.yaml because it
 * is not byte-exact; registering it would break the exact SHA-1 rebuild.
 *
 * Exact behavior is proven independently by the extended native oracle
 * pc_port/tools/pe_inventory_help_oracle.py: groups 36..39 now execute in the
 * original emulator and the port's func_8004C608 matches all 255 original
 * memory fingerprints (pc_port/tests/test_inventory_help.h).
 *
 * Help id -> jtbl_8001104C target:
 *   0  -> C6B8   1  -> C75C   5  -> C708   6  -> C954   7  -> C850
 *   8  -> C8A0   11 -> C998   12 -> C8CC   13 -> C8D4   14 -> C928
 *   16 -> C8D4   17 -> C9E4   27 -> C9F4   28 -> C9F4   29 -> C9FC
 *   32 -> CA14   33 -> CA28   35 -> CA30   36 -> CA38   37 -> CAAC
 *   38 -> CAAC   39 -> CB38   46 -> CB70   48 -> CB78   49 -> CB80
 *   50 -> CB88   51 -> C790   52 -> C7CC   56 -> CBBC   58 -> CBCC
 *   59 -> CBD4   60 -> CBEC   (all others -> CC10 epilogue)
 */

extern int func_80062CC4(void);
extern void func_8005E8C4(void);
extern void func_8005E8A4(int x, int y);
extern int func_80052894(int index);
extern void func_8006006C(int seconds, int small);
extern void func_8005EB64(int icon);
extern void func_8005E914(void);
extern int func_80062A34(int type, int id);
extern int func_80063428(int handle);

extern int func_8005DD3C(int id);
extern int func_8005DC4C(int id);
extern int func_8005F27C(int text);
extern void func_8005E968(int color);
extern void func_8005F5B8(int id);
extern void func_8005EB58(int dim);

extern int func_8005B89C(void);
extern int func_8005415C(int index);
extern int func_8005DCEC(int index);
extern int func_8005332C(int index);
extern int func_80058BBC(int index);
extern int func_80058E08(int index);
extern int func_80059F08(int index);
extern int func_800556E8(int index);
extern int func_80057ED8(int index);
extern int func_80054288(void);
extern void func_80052E30(int mode);
extern int func_80042770(int index);
extern int func_800404A8(void);
extern int func_8003FFBC(void);

extern unsigned int D_8009CEF0;
extern int          D_8009CF8C;
extern unsigned int D_8009CF18;
extern unsigned int D_8009CF20;
extern unsigned int D_8009CF1C;
extern unsigned int D_8009CEFC;
extern unsigned int D_8009CF0C;
extern unsigned int D_8009CF50;
extern unsigned int D_8009CFF8;
extern unsigned char D_8009234C[];

typedef struct {
    unsigned char pad0[0x34];
    int           f34;          /* +0x34 */
} HelpWindow;

typedef struct {
    unsigned char pad0[0x24];
    int           id;           /* +0x24 */
} MenuNode;

/* inventory record: +0x04 byte = type index, +0x15 byte = command. */
typedef struct {
    unsigned char pad0[0x15];
    unsigned char cmd;          /* +0x15 */
} InvRecord;

static int inventory_description(int record)
{
    return record ? func_8005DCEC(*(unsigned char *)(record + 4) - 1) : 0;
}

static int selected_inventory_help(int index, unsigned int source)
{
    int selected = D_8009CF8C;
    if (selected >= 0) {
        int kind = func_8005415C(selected);
        int text = func_8005DC4C((kind >= 19 && kind < 22) ? 14 : 15);
        func_8005E968(0x408040);
        return text;
    }
    if (source == 52u) return inventory_description(func_80058BBC(index));
    if (source == 51u) index = func_80058E08(index);
    return inventory_description(func_8005332C(index));
}

void func_8004C608(HelpWindow *window)
{
    MenuNode *focused = (MenuNode *)func_80062CC4();
    InvRecord *record;
    int text = 0;
    unsigned int id;
    unsigned int index;
    int i;

    func_8005E8C4();
    func_8005E8A4(window->f34 - 0x27, 1);
    func_8006006C(func_80052894(2), 1);
    func_8005E8A4(-0x21, 0);
    func_8005EB64(0x8C);
    func_8005E914();
    if (!focused) return;

    id = (unsigned int)focused->id;
    index = (unsigned int)func_80063428(func_80062A34(2, focused->id));

    switch (id) {
    case 0: {
        unsigned int enabled = D_8009CEF0 & (func_8005B89C() ? 0x1Fu : 0x1EFu);
        int remaining = (int)index;
        int option = -1;
        while (remaining >= 0) {
            remaining -= (int)(enabled & 1u);
            option++;
            enabled >>= 1;
            if (option >= 9) break;
        }
        text = func_8005DD3C((unsigned int)(option + 40));
        break;
    }
    case 1:
    case 51:
    case 52:
        text = selected_inventory_help((int)index, id);
        break;
    case 5:
        if ((int)index >= 0)
            text = inventory_description(func_8005332C(
                (signed char)*(volatile unsigned char *)
                (D_8009CF18 ? 0x800C0E20u : 0x800C0E22u)));
        break;
    case 6:
    case 11: {
        int rec = (id == 6u) ? (int)D_8009CF20
                             : func_8005332C(func_80059F08(1));
        unsigned int command = *(volatile unsigned char *)(rec + (int)index + 21) & 31u;
        if (command)
            text = func_8005DD3C((int)(command + (1u - D_8009CF18) * 20u - 1u));
        break;
    }
    case 7:
        text = inventory_description(func_8005332C((int)(D_8009CF1C
            ? (unsigned int)func_80059F08(1)
            : (unsigned int)func_800556E8((int)index))));
        break;
    case 8:
        if ((int)index < func_80054288())
            text = func_8005DCEC((unsigned int)func_800556E8((int)index) + 235u);
        break;
    case 13:
    case 16:
        if (D_8009CEFC)
            text = func_8005DCEC((unsigned int)func_80057ED8((int)index) - 1u);
        else
            text = inventory_description(func_8005332C(func_800556E8((int)index)));
        break;
    case 14:
        func_80052E30(0);
        text = inventory_description(func_8005332C((int)index));
        break;
    case 17:
        text = func_8005DC4C(39);
        break;
    case 2: case 3: case 4: case 9: case 10: case 15: case 18: case 19:
    case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 30:
    case 31: case 34: case 40: case 41: case 42: case 43: case 44: case 45:
    case 47: case 53: case 54: case 55: case 57:
        break;
    case 12:
        text = func_8005DD3C((int)(index + 79u));
        break;
    case 27:
    case 28:
        text = func_8005DD3C(82);
        break;
    case 29:
        text = func_8005DD3C((int)(index + 86u - D_8009CF18 * 3u));
        break;
    case 32:
        if ((int)index < 4) text = func_8005DD3C((int)(index + 49u));
        break;
    case 33:
        text = func_8005DD3C((int)(index + 58u));
        break;
    case 35:
        text = func_8005DD3C((int)(index + 61u));
        break;
    case 36:
        if (func_800404A8()) {
            func_8005E8A4(6, 5);
            func_8005F5B8(0x4C);
            func_8005E8A4(0, 0x10);
            func_8005F5B8(0x4D);
        } else {
            unsigned int flag = (func_80042770(0) != 0 ||
                                 func_80042770(1) != 0) ? 1u : 0u;
            text = func_8005DC4C((int)(flag | 0x4Eu));
        }
        break;
    case 37:
    case 38:
        if (D_8009CF50) {
            if (D_8009CFF8) {
                func_8005EB58(0);
                func_8005E8A4(6, 5);
                func_8005F5B8(0x58);
                func_8005E8A4(0, 0x10);
                func_8005F5B8(0x59);
            } else {
                text = func_8005DC4C(0x5C);
            }
        } else {
            text = func_8005DC4C(func_8003FFBC() ? 0x57 : 0x5B);
        }
        break;
    case 39:
        func_8005EB58(0);
        func_8005E8A4(6, 5);
        func_8005F5B8(0x50);
        func_8005E8A4(0, 0x10);
        func_8005F5B8(0x51);
        break;
    case 46:
        text = func_8005DD3C((int)(index + 63u));
        break;
    case 48:
        text = func_8005DD3C((int)(index + 56u));
        break;
    case 49:
        text = func_8005DD3C((int)(index + 66u));
        break;
    case 50:
        text = func_8005DD3C(D_8009234C[
            (D_8009CF0C == 1u && index == 2u) ? 4u : index]);
        break;
    case 56:
        func_8005EB58(0);
        text = func_8005DD3C(68);
        break;
    case 58:
        text = func_8005DD3C((int)(index + 53u));
        break;
    case 59:
        text = func_8005DD3C((int)(index + (D_8009CF0C == 2u ? 77u : 75u)));
        break;
    case 60:
        text = func_8005DD3C((int)(index + 72u - D_8009CF18 * 3u));
        break;
    default:
        break;
    }

    if (text) {
        func_8005E8A4(6, 5);
        func_8005F27C(text);
        func_8005E968(0x808080);
    }
    (void)i;
}
