int func_80077CB4(unsigned char *head, unsigned char *tail) {
    int length = head[3] + tail[3] + 1;
    int result;

    if (length < 17) {
        head[3] = length;
        *(unsigned int *)tail = 0;
        result = 0;
    } else {
        result = -1;
    }

    return result;
}
