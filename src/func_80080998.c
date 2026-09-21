void func_80080998(unsigned char *a0, unsigned char *a1) {
    if (a1 != 0) {
        if (a0 != 0) {
            int i = 0;
            do {
                *a0 = *a1;
                a1++;
                i++;
                a0++;
            } while (i < 8);
        }
    } else if (a0 != 0) {
        *a0 = 0;
    }
}
