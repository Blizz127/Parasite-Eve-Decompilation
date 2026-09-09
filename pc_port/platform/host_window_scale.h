#ifndef HOST_WINDOW_SCALE_H
#define HOST_WINDOW_SCALE_H
#include <stdint.h>
#include <string.h>

/* Fit the complete game image inside the current client area, preserving
 * its aspect ratio at arbitrary window sizes (including downscaling). */
static void HostWindow_ScaleBGRA(uint8_t *out, int width, int height,
                                 const uint8_t *rgb, int fb_w, int fb_h)
{
    int w=width,h=height,x0,y0,x,y;
    if ((int64_t)width*fb_h>(int64_t)height*fb_w)
        w=(int)((int64_t)height*fb_w/fb_h);
    else h=(int)((int64_t)width*fb_h/fb_w);
    x0=(width-w)/2;y0=(height-h)/2;
    memset(out,0,(size_t)width*(size_t)height*4u);
    for(y=0;y<h;y++) {
        int sy=(int)((int64_t)y*fb_h/h);
        uint8_t *dst=out+((size_t)(y+y0)*width+x0)*4u;
        for(x=0;x<w;x++) {
            int sx=(int)((int64_t)x*fb_w/w);
            const uint8_t *src=rgb+((size_t)sy*fb_w+sx)*3u;
            *dst++=src[2];*dst++=src[1];*dst++=src[0];*dst++=0;
        }
    }
}
#endif
