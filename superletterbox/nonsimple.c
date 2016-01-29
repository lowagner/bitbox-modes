#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

// default palettes

void graph_frame() {}
// --------------------------------------------------------------
uint16_t vram[Ny][Nx];
uint16_t offset_y;

void graph_line() 
{
    // letterbox (black envelope):
    int y = vga_line - offset_y;
    if (y < 0 || y >= Ny) 
    {
        if (vga_line/2 == 0 || y/2 == Ny/2)
            memset(draw_buffer, 0, SCREEN_W*2);
        return;
    }

    memcpy(draw_buffer, vram[vga_line - offset_y], SCREEN_W*2);
}

// --------------------------------------------------------------
// utilities 

void clear() 
{
   memset(vram, 0, sizeof(vram));
}
