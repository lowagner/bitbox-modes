#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

// default palettes

void graph_frame() {}
// --------------------------------------------------------------
//uint16_t vram[N_y][N_x];
extern const uint8_t n_x;
extern const uint8_t n_y;

void graph_line() 
{
    if (vga_odd) return;
    // letterbox (black envelope):
    if (vga_line<20+OFFSET_Y || vga_line >= 220-(32-OFFSET_Y)) 
    {
        if (vga_line/2 == 0 || vga_line/2 == 110+SQUARE_PIXELS/2-(32-OFFSET_Y)/2)
            memset(draw_buffer, 0, SCREEN_W*2);
        else if (vga_line/2 == 10-SQUARE_PIXELS/2+OFFSET_Y/2 ||
            vga_line/2 == 110-(32-OFFSET_Y)/2)
            memset(draw_buffer, 10, SCREEN_W*2);
        return;
    }

    uint16_t *dst=draw_buffer+OFFSET_X;
    uint16_t *src=&vram[(vga_line-20-OFFSET_Y)/SQUARE_PIXELS][0];

    for (uint8_t i=0; i<n_x; ++i)
    { 
        for (uint8_t j=0; j<SQUARE_PIXELS; ++j)
            *dst++ = *src;
        ++src;
    }
}

// --------------------------------------------------------------
// utilities 

void clear() 
{
   memset(vram, 0, sizeof(vram));
}
