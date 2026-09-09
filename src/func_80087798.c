/* AkaoSpuVoice_SetVolume — VRAM 0x80087798 / file 0x77F98 / size 0x24. */
void func_80087798(unsigned int index, unsigned int left, unsigned int right)
{
    volatile unsigned short *ptr =
        (volatile unsigned short *)(0x1F801C00u + (index * 0x10u));
    ptr[0] = (unsigned short)(left & 0x7FFFu);
    ptr[1] = (unsigned short)(right & 0x7FFFu);
}
