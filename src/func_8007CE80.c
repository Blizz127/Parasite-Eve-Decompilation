void func_8007CE80(unsigned int *a0, unsigned int *a1, unsigned int a2) {
    unsigned int i = 0;
    if (a2 != 0) {
        do {
            unsigned int v = *a1;
            a1++;
            i++;
            *a0 = v;
            a0++;
        } while (i < a2);
    }
}
