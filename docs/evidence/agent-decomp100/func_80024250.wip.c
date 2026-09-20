/*
 * func_80024250 - apply one ability/command to the active actor record.
 *
 * VRAM 0x80024250 / file 0x14A50 / size 0x7EC (507 words). Jump-table
 * dispatch (jtbl_800107D4, 20 entries) on the ability index; the head
 * copies the 20-entry u16 ability table D_80010760 to the stack and plays
 * a sound/effect through func_8006DDCC.  Case bodies mutate the active
 * record pointer D_8009D278 and the global flag word D_8009D1AC.
 *
 * Build: era -O2 -G8 + dispatch fold (jtbl_800107D4).
 */
typedef unsigned char u8;
typedef unsigned short u16;

extern int D_8009D1A0;
extern int D_8009D1A8;
extern u16 D_8009D228;
extern int D_8009D1AC;
extern int D_8009D2E8;
extern u16 D_80010760[];
extern int *D_8009D254;
extern u8 *D_8009D278;

extern void func_8006DDCC(u16 a0, int a1, int a2, int a3, int a4);
extern int func_80071A54(void);
extern void func_8001A680(int a0, int a1);

#define REC (D_8009D278)
#define FLAGS (*(int *)(D_8009D278 + 0x4C))

void func_80024250(int index, int actor) {
    register int cost asm("$16");
    cost = 0;
    if (!(D_8009D1A0 & 2)) {
        u16 table[20];
        int body;
        memcpy(table, D_80010760, 40);
        body = (int)D_8009D254;
        func_8006DDCC(table[index], 1, *(short *)(body + 0x2A),
                      *(short *)(body + 0x2E), *(short *)(body + 0x32));
        D_8009D278 = (u8 *)*(int *)D_8009D254;
    }
    switch (index) {
    case 0:
        *(u16 *)(REC + 0x0C) += 30;
        cost = 0x3C0000;
        break;
    case 1:
        *(u16 *)(REC + 0x0C) += 60;
        cost = 0x780000;
        break;
    case 2:
        *(u16 *)(REC + 0x0C) += 280;
        cost = 0x1F40000;
        break;
    case 3:
        if ((FLAGS & 3) != 3) {
            FLAGS &= ~3;
        }
        cost = 0x640000;
        break;
    case 4:
        if ((FLAGS & 3) != 3) {
            FLAGS &= ~3;
        }
        if ((FLAGS & 0x0C) != 0x0C) {
            FLAGS &= ~0x0C;
            D_8009D2E8 &= ~0x10;
        }
        if ((FLAGS & 0x30) != 0x30) {
            FLAGS &= ~0x30;
        }
        if ((FLAGS & 0xC0) != 0xC0) {
            FLAGS &= ~0xC0;
        }
        cost = 0x2580000;
        {
            int rec = (int)REC;
            *(int *)(rec + 0x4C) &= ~0x1000;
        }
        break;
    case 5: {
        int rec = (int)REC;
        int div = *(int *)(rec + 0x28);
        FLAGS |= 0x200;
        *(int *)(rec + 0x08) -= div / 3;
        break;
    }
    case 6:
        break;
    case 7: {
        register int body asm("$6");
        int acc;
        int bit;
        register int m8 asm("$7");
        register int m10 asm("$5");
        register int m20 asm("$8");
        register int m40 asm("$9");
        m8 = ~0x8000;
        m10 = ~0x10000;
        m20 = ~0x20000;
        m40 = ~0x40000;
        acc = D_8009D1AC & ~0x1000;
        body = *(int *)actor;
        bit = (*(int *)(body + 0xCC) & 3) == 2;
        acc |= bit << 12;
        D_8009D1AC = acc;
        bit = (*(int *)(body + 0xCC) & 0x0C) == 8;
        acc = (acc & ~0x2000) | (bit << 13);
        D_8009D1AC = acc;
        bit = (*(int *)(body + 0xCC) & 0x30) == 0x20;
        acc = (acc & ~0x4000) | (bit << 14);
        D_8009D1AC = acc;
        bit = (*(int *)(body + 0xCC) & 0xC000) == 0x8000;
        acc = (acc & m8) | (bit << 15);
        D_8009D1AC = acc;
        bit = (*(int *)(body + 0xCC) & 0x30000) == 0x20000;
        acc = (acc & m10) | (bit << 16);
        D_8009D1AC = acc;
        bit = (*(int *)(body + 0xCC) & 0x3000) == 0x2000;
        acc = (acc & m20) | (bit << 17);
        D_8009D1AC = acc;
        bit = (*(int *)(body + 0xCC) & 0xC00) == 0x800;
        acc = (acc & m40) | (bit << 18);
        D_8009D1AC = acc;
        {
            int b2 = *(int *)(*(int *)actor + 0xCC);
            if (((b2 >> 6) & 3) == 2) {
                D_8009D1AC = (acc & ~0xC00) | 0x400;
            } else {
                D_8009D1AC = acc & ~0xC00;
            }
        }
        *(char *)D_8009D1AC = 0x4B;
        D_8009D1A8 = actor;
        D_8009D1AC = (D_8009D1AC & ~0x300) | 0x100;
        cost = 0x320000;
        break;
    }
    case 8: {
        int r = (*(int *)(*(int *)actor + 0xCC) >> 8) & 3;
        cost = 0x5A0000;
        if (r == 0) {
            if (func_80071A54() & 1) {
                break;
            }
        } else if (r != 2) {
            break;
        }
        {
            int body = *(int *)actor;
            *(int *)body |= 1;
        }
        break;
    }
    case 9: {
        int rec = (int)REC;
        int f;
        *(int *)(rec + 0x4C) |= 0x100;
        f = *(int *)(rec + 0x4C);
        D_8009D228 = 450;
        if ((f & 0xC0) == 0x40 || (f & 0xC0) == 0x80) {
            *(int *)(rec + 0x4C) = f & ~0xC0;
        }
        cost = 0xC80000;
        break;
    }
    case 10: {
        int r = (*(int *)(*(int *)actor + 0xCC) >> 10) & 3;
        cost = 0x960000;
        if (r == 0) {
            if (func_80071A54() & 1) {
                break;
            }
        } else if (r != 2) {
            break;
        }
        {
            int body = *(int *)actor;
            int random = func_80071A54();
            *(int *)body = (*(int *)body & ~0x0F) | (((random % 3 + 3) & 7) << 1);
            func_8001A680(actor, 4);
            *(int *)(actor + 0x98) |= 0x1000;
        }
        break;
    }
    case 11:
        FLAGS |= 0x400;
        cost = 0x1900000;
        break;
    case 12:
        FLAGS |= 0x800;
        cost = 0x3E80000;
        break;
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
        break;
    case 18:
        *(u16 *)(REC + 0x0C) = *(u16 *)(REC + 0x1C);
        if ((FLAGS & 3) != 3) {
            FLAGS &= ~3;
        }
        if ((FLAGS & 0x0C) != 0x0C) {
            FLAGS &= ~0x0C;
            D_8009D2E8 &= ~0x10;
        }
        if ((FLAGS & 0x30) != 0x30) {
            FLAGS &= ~0x30;
        }
        if ((FLAGS & 0xC0) != 0xC0) {
            FLAGS &= ~0xC0;
        }
        cost = 0x5140000;
        {
            int rec = (int)REC;
            *(int *)(rec + 0x4C) &= ~0x1000;
        }
        break;
    case 19: {
        int rec = (int)REC;
        *(int *)(rec + 0x4C) = (*(int *)(rec + 0x4C) | 0x80000) & ~0x200000;
        break;
    }
    default:
        break;
    }
    if ((short)*(u16 *)(REC + 0x1C) < (short)*(u16 *)(REC + 0x0C)) {
        *(u16 *)(REC + 0x0C) = *(u16 *)(REC + 0x1C);
    }
    if (*(int *)(*(int *)(REC + 0x6C) + 4) & 0x200) {
        *(int *)(REC + 0x08) -= (cost * 2) / 3;
    } else {
        *(int *)(REC + 0x08) -= cost;
    }
}
