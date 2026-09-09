/* AkaoSpuVoice_SetAdsrSustainLevel — VRAM 0x80087864 / file 0x78064 / size 0x28. */
void func_80087864(int voice, unsigned int level)
{
    volatile unsigned short *ptr =
        (volatile unsigned short *)(0x1F801C08u + ((unsigned)voice << 4));
    unsigned int current = *ptr;
    *ptr = (unsigned short)((current & 0xFFF0u) | level);
}
