// 70D6C variant A: literal bases (PARK.md attempt-2 shape) + extern cursor store form
extern int D_80070E0C[];
extern int D_80070E04;
extern int D_80070E08;

unsigned int func_80070D6C(void) {
    unsigned int table = (unsigned int)D_80070E0C;
    int *cursor1 = &D_80070E04;
    int *cursor2 = &D_80070E08;
    int index1 = *cursor1;
    int index2 = *cursor2;
    unsigned int *slot1 = (unsigned int *)(table + index1);
    unsigned int *slot2 = (unsigned int *)(table + index2);
    unsigned int value1 = *slot1;
    unsigned int value2 = *slot2;
    unsigned int value = value1 + value2;

    *slot1 = value;
    index1 -= 4;
    index2 -= 4;
    if (index1 < 0) {
        index1 = 0x40;
    }
    if (index2 < 0) {
        index2 |= 0x40;
    }
    *cursor1 = index1;
    *cursor2 = index2;
    return value;
}
