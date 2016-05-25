#include "nonsimple.h"

#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

uint8_t superpixel[SCREEN_H][SCREEN_W/2] CCM_MEMORY;

uint32_t palette[256] CCM_MEMORY;

void graph_frame() {}

void graph_line() 
{
    if (vga_odd)
        return;
   
    int j = vga_line;
    uint32_t *dst = (uint32_t *)draw_buffer;
    for (int i=0; i<SCREEN_W/2; ++i) 
    {
        *dst++ = palette[superpixel[j][i]];
    }
}

void clear_screen() 
{
    static const uint32_t colors[16] = {
        [BLACK]=RGB(0, 0, 0),
        [GRAY]=RGB(157, 157, 157),
        [WHITE]=(1<<15) - 1,
        [PINK]=RGB(224, 111, 139),
        [RED]=RGB(190, 38, 51),
        [BLAZE]=RGB(235, 137, 49),
        [BROWN]=RGB(164, 100, 34),
        [DULLBROWN]=RGB(73, 60, 43),
        [GINGER]=RGB(247, 226, 107),
        [SLIME]=RGB(163, 206, 39),
        [GREEN]=RGB(68, 137, 26),
        [BLUEGREEN]=RGB(47, 72, 78),
        [CLOUDBLUE]=RGB(178, 220, 239),
        [SKYBLUE]=RGB(49, 162, 242),
        [SEABLUE]=RGB(0, 87, 132),
        [INDIGO]=RGB(28, 20, 40),
    };
    for (int k=0; k<256; ++k) {
        // little endian stuff
        palette[k] = colors[k%16] | (colors[k/16]<<16);
    }
    memset(superpixel, 0, sizeof(superpixel));
}

void propagate() {
}

void set_color(int x, int y, int c) 
{
    if (x % 2) 
        superpixel[y][x/2] = (superpixel[y][x/2]&15)|((c&15)<<4);
    else 
        superpixel[y][x/2] = (c&15)|(superpixel[y][x/2]&(15<<4));
}

int get_color(int x, int y) 
{
    if (x % 2) 
        return (superpixel[y][x/2]>>4)&15;
    else 
        return superpixel[y][x/2]&15;
}
