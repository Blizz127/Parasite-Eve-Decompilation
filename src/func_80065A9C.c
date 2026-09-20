/* VRAM 0x80065A9C / file 0x5629C / size 0x38. */
typedef struct {
    char pad[0x10];
    int field_10;
} Header;

extern Header *volatile D_800B1624;

int func_80065A9C(int arg0, int arg1) {
    unsigned char *p = (unsigned char *)D_800B1624 + D_800B1624->field_10 + (arg0 << 4);
    *p |= arg1 & 0x30;
    return 0;
}
