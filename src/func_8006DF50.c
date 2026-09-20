/*
 * func_8006DF50 — VRAM 0x8006DF50, size 0x58, file 0x5E750-0x5E7A8.
 *
 * Sound lookup + queue: func_8006E514(package, id); when a sound is found
 * call func_80086608(sound, key, pan, volume), else return -1.
 * era -O2 -G0. The if/else assignment (rather than two returns) is what
 * makes cc1 place the call on the fall-through path and -1 in the branch.
 */
int func_8006E514(unsigned char *package, int id);
int func_80086608(unsigned char *sound, int key, int pan, int volume);

int func_8006DF50(unsigned char *package, int id, int key, int pan, int volume) {
    unsigned char *sound = (unsigned char *)func_8006E514(package, id);
    int result;
    if (sound != 0)
        result = func_80086608(sound, key, pan, volume);
    else
        result = -1;
    return result;
}
