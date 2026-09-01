extern char D_800A5B70[];

void *func_80084B20(int arg0) {
    char *result = D_800A5B70;

    if (arg0 & 0xF0) {
        result += 0xF0;
    }
    return result;
}
