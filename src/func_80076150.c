unsigned int func_80076150(int arg0, int arg1, unsigned int arg2) {
    return 0xE1000000 | (arg1 ? 0x200 : 0) | (arg0 ? 0x400 : 0) | (arg2 & 0x9FF);
}
