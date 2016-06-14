#include "bitbox.h"
#include "common.h"
#include "sprites.h"
#include "tiles.h"
#include "edit.h"
#include "save.h"
#include "fill.h"
#include "font.h"
#include <stdint.h>
#include <stdlib.h> // abs
#include <string.h> // memset

int16_t map_tile_y CCM_MEMORY, map_tile_x CCM_MEMORY;
uint8_t map_color[2] CCM_MEMORY, map_last_painted CCM_MEMORY;

#define MAP_HEADER 32 // and footer.  make it a multiple of 16

void map_init()
{
    map_tile_x = 0;
    map_tile_y = 0;
    map_color[0] = 0;
    map_color[1] = 1;
    map_last_painted = 0;
}

void map_switch()
{
    if (map_tile_x*16 < tile_map_x)
        tile_map_x = map_tile_x*16;
    else if (map_tile_x*16 >= tile_map_x + SCREEN_W)
        tile_map_x = map_tile_x*16 - SCREEN_W;
    else if (tile_map_x % 16)
        tile_map_x =16*(tile_map_x/16);
    
    if (map_tile_y*16 < tile_map_y + MAP_HEADER)
        tile_map_y = map_tile_y*16 - MAP_HEADER;
    else if (map_tile_y*16 >= tile_map_y + SCREEN_H - MAP_HEADER)
        tile_map_y = map_tile_y*16 - SCREEN_H + MAP_HEADER;
    else if (tile_map_y % 16)
        tile_map_y =16*(tile_map_y/16);
   
    update_objects(); 
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
        //create_object(k%16, rand()%(tile_map_width*16+16)-16, rand()%(tile_map_height*16+16)-16, rand()%256);
        create_object(k%16, 16*(rand()%tile_map_width), 16*(rand()%tile_map_height), rand()%256);
}

void map_line()
{
    if (vga_line < MAP_HEADER)
    {
        if (vga_line < MAP_HEADER - 20)
        {
            if (vga_line/2 == 0)
                memset(draw_buffer, 0, 2*SCREEN_W);
            return;
        }
        else if (vga_line >= MAP_HEADER - 4)
        {
            if (vga_line/2 == (MAP_HEADER-4)/2)
                memset(draw_buffer, 0, 2*SCREEN_W);
            return;
        }
        return;
    }
    else if (vga_line >= SCREEN_H-MAP_HEADER)
    {
        if (vga_line/2 == (SCREEN_H-MAP_HEADER)/2)
            memset(draw_buffer, 0, 2*SCREEN_W); 
        else if (vga_line >= SCREEN_H-MAP_HEADER+4)
        {
            if (vga_line < SCREEN_H-MAP_HEADER+4+8)
                font_render_line_doubled(game_message, 16, vga_line - (SCREEN_H-MAP_HEADER+4), 
                    65535, 0);
            else if (vga_line/2 == (SCREEN_H-MAP_HEADER+4+8)/2)
                memset(draw_buffer, 0, 2*SCREEN_W); 
            return;
        }
        return;
    }

    tiles_line();
    sprites_line();
    
    int tile_j = tile_map_y + vga_line;
    
    if (tile_j/16 == map_tile_y)
    switch (tile_j%16)
    {
    case 7:
    case 8:
    {
        uint32_t *dst = (uint32_t *)draw_buffer + (map_tile_x*16 - tile_map_x + 6)/2;
        *dst = ~(*dst);
        ++dst;
        *dst = ~(*dst);
        break;
    }
    case 6:
    case 9:
    {
        uint16_t *dst = draw_buffer + (map_tile_x)*16 - tile_map_x + 7;
        *dst = ~(*dst);
        ++dst;
        *dst = ~(*dst);
        break;
    }
    }
}

void map_spot_paint(uint8_t p)
{
    map_last_painted = p;

    int index = map_tile_y * tile_map_width + map_tile_x;
    uint8_t *memory = &tile_map[index/2];

    if (index % 2)
        *memory = ((*memory)&15) | (map_color[p]<<4);
    else
        *memory = (map_color[p]) | ((*memory) & 240);
}

int map_spot_color()
{
    int index = map_tile_y * tile_map_width + map_tile_x;
    const uint8_t *memory = &tile_map[index/2];

    if (index % 2)
        return (*memory) >> 4;
    else
        return (*memory) & 15;
}

void map_spot_fill(uint8_t p)
{
    map_last_painted = p;

    if (!fill_can_start())
        fill_stop();
    uint8_t previous_canvas_color = map_spot_color();
    if (previous_canvas_color != map_color[p])
    {
        fill_init(tile_map, tile_map_width, tile_map_height, 
            previous_canvas_color, map_tile_x, map_tile_y, map_color[p]);
    }
}

void map_controls()
{
    int make_wait = 0;
    if (GAMEPAD_PRESSING(0, R))
    {
        game_message[0] = 0;
        map_color[map_last_painted] = (map_color[map_last_painted] + 1)&15;
        make_wait = 1;
    }
    else if (GAMEPAD_PRESSING(0, L))
    {
        game_message[0] = 0;
        map_color[map_last_painted] = (map_color[map_last_painted] - 1)&15;
        make_wait = 1;
    }
    
    if (GAMEPAD_PRESSING(0, A))
    {
        game_message[0] = 0;
        map_spot_fill(map_last_painted);
        make_wait = 1;
    }
    else if (GAMEPAD_PRESSING(0, X))
    {
        game_message[0] = 0;
        map_color[map_last_painted] = map_spot_color();
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
        previous_visual_mode = EditMap;
        game_switch(EditTileOrSprite);
        // TODO:  maybe go to sprite if a sprite was selected.
        edit_sprite_not_tile = 0;
        edit_tile = map_color[map_last_painted];
        return;
    }

    int paint_if_moved = 0; 
    if (GAMEPAD_PRESSING(0, Y))
    {
        game_message[0] = 0;
        map_spot_paint(0);
        paint_if_moved = 1;
    }
    else if (GAMEPAD_PRESSING(0, B))
    {
        game_message[0] = 0;
        map_spot_paint(1);
        paint_if_moved = 2;
    }
    
    int moved = 0;
    if (GAMEPAD_PRESSING(0, left))
    {
        if (map_tile_x > 0)
        {
            --map_tile_x;
            make_wait = 1;
            if (map_tile_x < tile_map_x/16)
            {
                tile_map_x -= 16;
                moved = 1;
            }
        }
    }
    else if (GAMEPAD_PRESSING(0, right))
    {
        if (map_tile_x < tile_map_width - 1)
        {
            ++map_tile_x;
            make_wait = 1;
            if (map_tile_x >= tile_map_x/16 + SCREEN_W/16)
            {
                tile_map_x += 16;
                moved = 1;
            }
        }
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        if (map_tile_y > 0)
        {
            --map_tile_y;
            make_wait = 1;
            if (map_tile_y < tile_map_y/16 + MAP_HEADER/16) // 0 < -32/16 + MAP_HEADER/16
            {
                tile_map_y -= 16;
                moved = 1;
            }
        }
    }
    else if (GAMEPAD_PRESSING(0, down))
    {
        if (map_tile_y < tile_map_height - 1)
        {
            ++map_tile_y;
            make_wait = 1;
            if (map_tile_y >= tile_map_y/16 + (SCREEN_H-MAP_HEADER)/16)
            {
                tile_map_y += 16;
                moved = 1;
            }
        }
    }
    if (moved)
    {
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
        if (paint_if_moved)
            map_spot_paint(paint_if_moved-1);
        update_objects(); 
        return;
    }
    else if (paint_if_moved || make_wait)
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
    
    if (GAMEPAD_PRESS(0, start))
    {
        if (previous_visual_mode)
        {
            game_switch(previous_visual_mode);
            previous_visual_mode = None;
        }
        else
        {
            // make a map menu here, but make it possible to go back to GameOn from there.
            game_switch(GameOn);
        }
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        previous_visual_mode = None;
        game_switch(EditTileOrSprite);
        edit_sprite_not_tile = 0;
        return;
    }
}
