#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

#define FASTMEM __attribute__ ((section (".ccm")))

uint16_t bg_color FASTMEM;
uint8_t tile_map[TILE_MAP_MEMORY] FASTMEM;
uint8_t tile_translator[16] FASTMEM;
uint16_t tile_draw[15][16][16] FASTMEM;
uint16_t tile_map_x FASTMEM, tile_map_y FASTMEM;
uint16_t tile_map_width FASTMEM, tile_map_height FASTMEM;
// tile_map_width * tile_map_height <= TILE_MAP_MEMORY
// tile_map_x < tile_map_width - 320
// tile_map_y < tile_map_height - 240


void graph_frame() {}
// --------------------------------------------------------------

void graph_line() 
{
    int tile_j = tile_map_y + vga_line/2;
    int tile_i = tile_map_x;
    uint8_t *tile = &tile_map[(tile_j/16)*tile_map_width + tile_i / 16];

    uint32_t *dst = (uint32_t*)draw_buffer;
    if (tile_map_x % 16)
    {

    }
    else
    {
        tile_j %= 16;
        for (int k=0; k<20; ++k)
        {
            // translate the tile into what tile it should be drawn as:
            uint8_t trans = tile_translator[(*tile++)&15];
            if (trans < 15)
            {
                for (int l=0; l<16; ++l)
                {
                    uint16_t color = tile_draw[trans][tile_j][l];
                    if (color < 32768)
                        *dst++ = (color) | (color<<16);
                    else
                        *dst++ = (bg_color) | (bg_color<<16);
                }
            }
            else // transparent
            {
                uint32_t double_bg = bg_color | (bg_color << 16);
                for (int l=0; l<16; ++l)
                {
                    *dst++ = double_bg;
                }
            }
        }
    }
}
// --------------------------------------------------------------
// utilities 

void clear() 
{
    memset(tile_map, 15, sizeof(tile_map));
    tile_map_x = 0;
    tile_map_y = 0;
    for (uint8_t i=0; i<16; ++i)
        tile_translator[i] = i; 
}
