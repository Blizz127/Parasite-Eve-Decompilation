typedef struct {
    int unk0;
    unsigned int flags;
    int unk8;
    int unkC;
    int unk10;
    int value;
} BufferInfo;

void *func_800719C4(BufferInfo *arg0) {
    if (arg0->flags & 8) {
        return &arg0->value;
    }
    return 0;
}
