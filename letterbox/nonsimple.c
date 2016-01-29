#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

// default palettes

void graph_frame() {}
// --------------------------------------------------------------
//uint16_t vram[N_y][N_x];

uint16_t vram[(SCREEN_H-72)/SQUARE_PIXELS][(SCREEN_W-64)/SQUARE_PIXELS];
const uint8_t n_x = (SCREEN_W-64)/SQUARE_PIXELS;
const uint8_t n_y = (SCREEN_H-72)/SQUARE_PIXELS;

uint8_t offset_x; // should not be more than 64.  also should probably be even.
uint8_t offset_y; // should not be more than 32.  also probably should make it even.
uint8_t bg_color;
uint8_t bg_wrap_color;


void graph_line() 
{
    if (vga_odd) return;
    // letterbox (black envelope):
    int y = vga_line-offset_y;
    if (y < 0)
    {
        if (offset_y <= 2*SQUARE_PIXELS)
        {
            if (vga_line/2 >= offset_y/2 - SQUARE_PIXELS/2)
                memset(draw_buffer, bg_wrap_color, SCREEN_W*2);
        }
        else
        {
            if (vga_line/2 == 0)
                memset(draw_buffer, bg_color, SCREEN_W*2);
            else if (y/2 == -SQUARE_PIXELS/2)
                memset(draw_buffer, bg_wrap_color, SCREEN_W*2);
        }
        return;
    }
    else if (y >= n_y*SQUARE_PIXELS) 
    {
        if (y/2 == (n_y+1)*SQUARE_PIXELS/2)
            memset(draw_buffer, bg_color, SCREEN_W*2);
        else if (y/2 == n_y*SQUARE_PIXELS/2)
            memset(draw_buffer, bg_wrap_color, SCREEN_W*2);
        return;
    }

    uint16_t *dst=draw_buffer+offset_x;
    uint16_t *src=&vram[y/SQUARE_PIXELS][0];

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
