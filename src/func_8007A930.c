/* VRAM 0x8007A930 / file 0x6B130 / size 0x104. */
unsigned char *func_8007A930(int arg0, unsigned char *arg1) {
    int t;
    int q;
    int s;
    int h;
    int m;

    t = arg0 + 0x96;
    q = t / 75;
    s = t % 75;
    h = q / 60;
    m = q % 60;
    arg1[2] = (s / 10) * 16 + s % 10;
    arg1[1] = (m / 10) * 16 + m % 10;
    arg1[0] = (h / 10) * 16 + h % 10;
    return arg1;
}
