#include "tiles.h"
#include "bitbox.h"

uint8_t tile_map[TILE_MAP_MEMORY] CCM_MEMORY;
uint8_t tile_translator[16] CCM_MEMORY;
//uint16_t tile_properties[256] CCM_MEMORY;
uint8_t tile_draw[16][16][8] CCM_MEMORY;
int16_t tile_map_x CCM_MEMORY, tile_map_y CCM_MEMORY;
uint16_t tile_map_width CCM_MEMORY, tile_map_height CCM_MEMORY;
// tile_map_width * tile_map_height <= TILE_MAP_MEMORY
// tile_map_x < tile_map_width - 320
// tile_map_y < tile_map_height - 240

void map_line()
{
    if (tile_map_x % 16)
    {
        uint16_t *dst = draw_buffer;
        int tile_j = tile_map_y + vga_line;
        int index = (tile_j/16)*(tile_map_width) + tile_map_x / 16;
        tile_j %= 16;
        uint8_t *tile = &tile_map[index/2];
        if (index % 2)
        {
            // draw the first tile (it's somewhat off-screen)
            uint8_t trans = tile_translator[((*tile)>>4)]; // first one is odd
                
            uint8_t *tile_color = &tile_draw[trans][tile_j][(tile_map_x%16)/2] - 1;
            if (tile_map_x % 2)
            {
                ++tile_color;
                for (int l=(tile_map_x%16)/2; l<7; ++l)
                {
                    *dst++ = palette[(*tile_color)>>4];
                    *dst++ = palette[(*(++tile_color))&15];
                }
                *dst++ = palette[(*tile_color)>>4];
            }
            else
            {
                for (int l=(tile_map_x%16)/2; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }

            // draw 19 un-broken tiles (tile 2 to tile 21)
            for (int k=0; k<19/2; ++k)
            {
                // translate the tile into what tile it should be drawn as:
                // first one is even (not odd)
                trans = tile_translator[(*(++tile))&15];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }

                // second one is odd
                trans = tile_translator[((*tile)>>4)];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
            // 21st tile is even, but still unbroken
            trans = tile_translator[(*(++tile))&15];
            tile_color = &tile_draw[trans][tile_j][0] - 1;
            for (int l=0; l<8; ++l)
            {
                *dst++ = palette[(*(++tile_color))&15];
                *dst++ = palette[(*tile_color)>>4];
            }

            // draw 22'nd broken tile is odd
            trans = tile_translator[((*tile)>>4)&15];
            tile_color = &tile_draw[trans][tile_j][0]-1; 
            if (tile_map_x % 2)
            {
                *dst++ = palette[(*(++tile_color))&15];
                for (int l=0; l<(tile_map_x%16)/2; ++l)
                {
                    *dst++ = palette[(*tile_color)>>4];
                    *dst++ = palette[(*(++tile_color))&15];
                }
            }
            else
            {
                for (int l=0; l<(tile_map_x%16)/2; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
        }
        else 
        {
            // draw the first tile (it's somewhat off-screen)
            uint8_t trans = tile_translator[(*tile)&15]; // first one is even
            uint8_t *tile_color = &tile_draw[trans][tile_j][(tile_map_x%16)/2] - 1;
            if (tile_map_x % 2)
            {
                ++tile_color;
                for (int l=(tile_map_x%16)/2; l<7; ++l)
                {
                    *dst++ = palette[(*tile_color)>>4];
                    *dst++ = palette[(*(++tile_color))&15];
                }
                *dst++ = palette[(*tile_color)>>4];
            }
            else
            {
                for (int l=(tile_map_x%16)/2; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }

            // draw 19 un-broken tiles (tile 2 to tile 21)
            for (int k=0; k<19/2; ++k)
            {
                // translate the tile into what tile it should be drawn as:
                // first one is odd
                trans = tile_translator[((*tile)>>4)];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
                
                // second one is even
                trans = tile_translator[(*(++tile))&15];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
            // 21st tile is odd, but still unbroken
            trans = tile_translator[((*tile)>>4)];
            tile_color = &tile_draw[trans][tile_j][0] - 1;
            for (int l=0; l<8; ++l)
            {
                *dst++ = palette[(*(++tile_color))&15];
                *dst++ = palette[(*tile_color)>>4];
            }

            // draw 22'nd broken tile is even
            trans = tile_translator[(*(++tile))&15];
            tile_color = &tile_draw[trans][tile_j][0]-1; 
            if (tile_map_x % 2)
            {
                *dst++ = palette[(*(++tile_color))&15];
                for (int l=0; l<(tile_map_x%16)/2; ++l)
                {
                    *dst++ = palette[(*tile_color)>>4];
                    *dst++ = palette[(*(++tile_color))&15];
                }
            }
            else
            {
                for (int l=0; l<(tile_map_x%16)/2; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
        }
    }
    else // tile_map_x puts a tile exactly on the edge of the screen
    {
        uint16_t *dst = draw_buffer;
        int tile_j = tile_map_y + vga_line;
        int index = (tile_j/16)*(tile_map_width) + tile_map_x / 16;
        tile_j %= 16;
        uint8_t *tile = &tile_map[index/2]-1;
        if (index % 2)
        {
            ++tile;
            for (int k=0; k<10; ++k)
            {
                // translate the tile into what tile it should be drawn as:
                uint8_t trans = tile_translator[((*tile)>>4)];

                uint8_t *tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
                    
                trans = tile_translator[(*(++tile))&15];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
        }
        else // not odd
        {
            for (int k=0; k<10; ++k)
            {
                // translate the tile into what tile it should be drawn as:
                uint8_t trans = tile_translator[(*(++tile))&15];

                uint8_t *tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
                
                // translate the tile into what tile it should be drawn as:
                trans = tile_translator[((*tile)>>4)];

                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }

        }
    }
}

void map_controls()
{
    if (vga_frame % 2 == 0)
    {
        int moved = 0;
        if (GAMEPAD_PRESSED(0, left))
        {
            if (tile_map_x > 0)
            {
                --tile_map_x;
                moved = 1;
            }
        }
        else if (GAMEPAD_PRESSED(0, right))
        {
            if (tile_map_x + SCREEN_W < tile_map_width*16 - 1)
            {
                ++tile_map_x;
                moved = 1;
            }
        }
        if (GAMEPAD_PRESSED(0, up))
        {
            if (tile_map_y > 0)
            {
                --tile_map_y;
                moved = 1;
            }
        }
        else if (GAMEPAD_PRESSED(0, down))
        {
            if (tile_map_y + SCREEN_H < tile_map_height*16 - 1)
            {
                ++tile_map_y;
                moved = 1;
            }
        }

        if (moved)
            update_objects(); 
    }
    if (GAMEPAD_PRESS(0, select))
    {
        visual_mode = EditTile;
    }
}
