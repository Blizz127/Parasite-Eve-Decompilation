/* AkaoSpuVoice_SetAdsrDecayRate — VRAM 0x8008783C / file 0x7803C / size 0x28. */
void func_8008783C(int voice, unsigned int rate)
{
    volatile unsigned short *ptr =
        (volatile unsigned short *)(0x1F801C08u + ((unsigned)voice << 4));
    unsigned int current = *ptr;
    *ptr = (unsigned short)((current & 0xFF0Fu) | (rate << 4));
}
