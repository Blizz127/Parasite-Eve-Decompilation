/* Original StoreImage worker, 66B54.s. CPU prefix and DMA suffix use the
 * same GPU authority as LoadImage; issuing DMA never completes it. */
#include "psx_compat.h"
#include "pe_gpu.h"
#include "game_port.h"

static int store_wait(uint32_t ready)
{
    while (!(PE_GPU_ReadStatus()&ready)) {
        uint32_t prior;
        if ((int32_t)PE_LoadU32(0x80095888u)<(int32_t)PE_GPU_VSyncQuery()) goto timeout;
        prior=PE_LoadU32(0x8009588Cu);PE_StoreU32(0x8009588Cu,prior+1u);
        if ((int32_t)prior>0xF0000) goto timeout;
    }
    return 1;
timeout:
    (void)Bootstrap_ReturnInt("func_80077404","func_800768A0",-1);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
}

static int store_image(RECT *rect,pe_addr_t guest_rect,pe_addr_t destination)
{
    int16_t limit;
    int32_t pixels,words,blocks;
    uint32_t remainder,i,value;
    limit=(int16_t)PE_LoadU16(0x80095750u);
    if (rect->w<0) rect->w=0;else if (rect->w>limit) rect->w=limit;
    if (guest_rect) PE_StoreU16(guest_rect+4u,(uint16_t)rect->w);
    limit=(int16_t)PE_LoadU16(0x80095752u);
    if (rect->h<0) rect->h=0;else if (rect->h>limit) rect->h=limit;
    if (guest_rect) PE_StoreU16(guest_rect+6u,(uint16_t)rect->h);
    pixels=(int32_t)rect->w*(int32_t)rect->h;
    value=(uint32_t)pixels+1u;value+=value>>31u;
    words=(int32_t)value>>1;blocks=(int32_t)value>>5;
    if (words<=0) return -1;
    /* Match the existing LoadImage adapter's native memory preflight. */
    if (!destination || (destination&3u) || !PE_RangeIsRam(destination,(size_t)words*4u)) return -1;
    if (!store_wait(PE_GPU_STATUS_READY_GP0) || !PE_GPU_CanBeginImageLoad(blocks!=0)) return -1;
    if (!PE_GPU_WriteGP1(0x04000000u) || !PE_GPU_WriteGP0(0x01000000u) ||
        !PE_GPU_WriteGP0(0xC0000000u) ||
        !PE_GPU_WriteGP0((uint32_t)(uint16_t)rect->x|((uint32_t)(uint16_t)rect->y<<16u)) ||
        !PE_GPU_WriteGP0((uint32_t)(uint16_t)rect->w|((uint32_t)(uint16_t)rect->h<<16u))) return -1;
    if (!store_wait(PE_GPU_STATUS_READY_READ)) return -1;
    remainder=(uint32_t)words-(uint32_t)blocks*16u;
    for (i=0;i<remainder;i++) {
        if (!PE_GPU_ReadGP0(&value)) return -1;
        PE_StoreU32(destination+i*4u,value);
    }
    if (blocks && (!PE_GPU_WriteGP1(0x04000003u) ||
        !PE_GPU_DMA2Issue(destination+remainder*4u,((uint32_t)blocks<<16u)|16u,PE_GPU_DMA2_CHCR_STORE))) return -1;
    return 0;
}

int func_800768A0(pe_addr_t address,pe_addr_t destination)
{
    RECT rect;
    (void)func_800773D0();
    if (!PE_RangeIsRam(address,sizeof(rect))) return -1;
    rect.x=(int16_t)PE_LoadU16(address);rect.y=(int16_t)PE_LoadU16(address+2u);
    rect.w=(int16_t)PE_LoadU16(address+4u);rect.h=(int16_t)PE_LoadU16(address+6u);
    return store_image(&rect,address,destination);
}

int PE_StoreImageInline8(uint32_t position,uint32_t size,pe_addr_t destination)
{
    RECT rect={(int16_t)position,(int16_t)(position>>16u),(int16_t)size,(int16_t)(size>>16u)};
    (void)func_800773D0();return store_image(&rect,0u,destination);
}
