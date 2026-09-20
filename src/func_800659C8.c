/* VRAM 0x800659C8 / file 0x561C8 / size 0x30. */
typedef struct {
    char pad[0x10];
    int field_10;
} Header;

extern Header *volatile D_800B1624;

int func_800659C8(int arg0, unsigned int arg1) {
    *(unsigned short *)((char *)D_800B1624 + D_800B1624->field_10 + (arg0 << 4) + 8) =
        arg1 >> 8;
    return 0;
}
