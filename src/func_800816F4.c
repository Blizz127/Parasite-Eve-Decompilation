int func_80071A04(const void *left, const void *right, unsigned int size);

int func_800816F4(const void *left, const void *right) {
    return func_80071A04(left, right, 12) == 0;
}
