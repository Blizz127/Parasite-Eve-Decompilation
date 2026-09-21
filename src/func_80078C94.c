int *func_80078C94(int *a0, int *a1) {
    register int *d asm("$4") = a0;
    register int t0 asm("$8") = a1[0];
    register int t1 asm("$9") = a1[1];
    register int t2 asm("$10") = a1[2];
    d[5] = t0;
    d[6] = t1;
    d[7] = t2;
    return d;
}
