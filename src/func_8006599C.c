/* VRAM 0x8006599C / file 0x5619C / size 0x2C. */
typedef struct {
    char pad[0x10];
    int field_10;
} Header;

extern Header *volatile D_800B1624;

int func_8006599C(int arg0) {
    return *(short *)((char *)D_800B1624 + D_800B1624->field_10 + (arg0 << 4) + 6);
}
