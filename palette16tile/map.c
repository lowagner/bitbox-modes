#include "bitbox.h"
#include "common.h"
#include "sprites.h"
#include "tiles.h"
#include "edit.h"
#include "save.h"
#include "font.h"
#include <stdint.h>
#include <stdlib.h> // abs
#include <string.h> // memset

int16_t map_y, map_x CCM_MEMORY;
int16_t map_tile_y, map_tile_x CCM_MEMORY;

#define MAP_HEADER 32 // and footer

void map_init()
{
    map_y = 0;
    map_x = 0;
    map_tile_x = 0;
    map_tile_y = 0;
}

void map_reset()
{
    tile_map_width = 26;
    tile_map_height = 20;

    for (int j=0; j<tile_map_height; ++j)
    for (int i=0; i<tile_map_width/2; ++i)
    {
        if (j <= (tile_map_height-16))
            tile_map[j*(tile_map_width/2)+i] = 0;
        else
            tile_map[j*(tile_map_width/2)+i] = ((j - tile_map_height + 16)%16)|(((j - tile_map_height + 16)%16)<<4);
    }
   
    for (int k=0; k<MAX_OBJECTS; ++k)
        create_object(k%16, rand()%(tile_map_width*16+16)-16, rand()%(tile_map_height*16+16)-16, rand()%256);
}

void map_line()
{
    if (vga_line < MAP_HEADER)
    {
        if (vga_line/2 == 0)
            memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= SCREEN_H-MAP_HEADER)
    {
        if (vga_line/2 == (SCREEN_H-MAP_HEADER)/2)
            memset(draw_buffer, 0, 2*SCREEN_W); 
        return;
    }
    uint16_t *dst = draw_buffer;
    int tile_j = 16*map_y + vga_line - MAP_HEADER;
    int draw_crosshairs = (tile_j/16 == map_tile_y);
    int index = (tile_j/16)*(tile_map_width) + map_x;
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
    if (draw_crosshairs)
    switch (tile_j)
    {
    case 7:
    case 8:
        dst = draw_buffer + (map_tile_x - map_x)*16 + 6;
        *dst = ~(*dst);
        ++dst;
        *dst = ~(*dst);
        ++dst;
        *dst = ~(*dst);
        ++dst;
        *dst = ~(*dst);
        break;
    case 6:
    case 9:
        dst = draw_buffer + (map_tile_x - map_x)*16 + 7;
        *dst = ~(*dst);
        ++dst;
        *dst = ~(*dst);
        break;
    }
}

void map_controls()
{
    if (vga_frame % 2 == 0)
    {
        int moved = 0;
        int cursor_moved = 0;
        if (GAMEPAD_PRESSED(0, left))
        {
            if (map_tile_x > 0)
            {
                --map_tile_x;
                cursor_moved = 1;
                if (map_tile_x < map_x)
                {
                    --map_x;
                    moved = 1;
                }
            }
        }
        else if (GAMEPAD_PRESSED(0, right))
        {
            if (map_tile_x < tile_map_width - 1)
            {
                ++map_tile_x;
                cursor_moved = 1;
                if (map_tile_x >= map_x + SCREEN_W/16)
                {
                    ++map_x;
                    moved = 1;
                }
            }
        }
        if (GAMEPAD_PRESSED(0, up))
        {
            if (map_tile_y > 0)
            {
                --map_tile_y;
                cursor_moved = 1;
                if (map_tile_y < map_y)
                {
                    --map_y;
                    moved = 1;
                }
            }
        }
        else if (GAMEPAD_PRESSED(0, down))
        {
            if (map_tile_y < tile_map_height - 1)
            {
                ++map_tile_y;
                cursor_moved = 1;
                if (map_tile_y >= map_y + (SCREEN_H-2*MAP_HEADER)/16)
                {
                    ++map_y;
                    moved = 1;
                }
            }
        }
        if (cursor_moved)
            gamepad_press_wait = GAMEPAD_PRESS_WAIT;

        if (moved)
            update_objects(); 
    }
    if (GAMEPAD_PRESS(0, start))
    {
        // TODO: choose edit_sprite or edit_tile (and edit_sprite_not_tile)
        // based on the tile/sprite which is currently selected in the map
        if (previous_visual_mode)
        {
            visual_mode = previous_visual_mode;
            previous_visual_mode = None;
        }
        else
        {
            visual_mode = EditTileOrSprite;
        }
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        previous_visual_mode = None;
        visual_mode = EditTileOrSprite;
        edit_sprite_not_tile = 0;
        return;
    }
}
