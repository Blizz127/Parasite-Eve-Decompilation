typedef struct {
    short x;
    short y;
    short w;
    short h;
    short screen_x;
    short screen_y;
    short screen_w;
    short screen_h;
    unsigned char isinter;
    unsigned char isrgb24;
    unsigned char pad0;
    unsigned char pad1;
} DisplayEnv;

DisplayEnv *func_800749D8(DisplayEnv *env, int x, int y, int w, int h) {
    env->x = x;
    env->y = y;
    env->w = w;
    env->h = h;
    env->screen_x = 0;
    env->screen_y = 0;
    env->screen_w = 0;
    env->screen_h = 0;
    env->isrgb24 = 0;
    env->isinter = 0;
    env->pad1 = 0;
    env->pad0 = 0;
    return env;
}
