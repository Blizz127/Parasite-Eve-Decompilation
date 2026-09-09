/* AkaoSpuVoice_SetAdsrAttack — VRAM 0x8008780C / file 0x7800C / size 0x30. */
void func_8008780C(int voice, unsigned int rate, unsigned int mode)
{
    volatile unsigned short *ptr =
        (volatile unsigned short *)(0x1F801C08u + ((unsigned)voice << 4));
    unsigned int current = *(volatile unsigned char *)ptr;
    unsigned int value = ((mode >> 2) << 15) | (rate << 8);
    *ptr = (unsigned short)(current | value);
}
