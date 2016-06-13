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

#define MAP_HEADER 32 // and footer

void map_init()
{
    map_tile_x = 0;
    map_tile_y = 0;
    map_color[0] = 0;
    map_color[1] = 1;
    map_last_painted = 0;
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
        return;
    }
    uint16_t *dst = draw_buffer;
    int tile_j = tile_map_y + vga_line - MAP_HEADER;
    int draw_crosshairs = (tile_j/16 == map_tile_y);
    int index = (tile_j/16)*(tile_map_width) + tile_map_x/16;
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
    
    // draw sprites, too:
    if (!drawing_count) // none to draw...
        goto map_draw_crosshairs;
    int16_t vga16 = (int16_t) vga_line - MAP_HEADER;
    // add any new sprites to the draw line:
    while (last_drawing_index < drawing_count)
    {
        if (object[draw_order[last_drawing_index]].iy <= vga16) 
            ++last_drawing_index;
        else
            break;
    }
    // subtract any early sprites no longer on the line
    vga16-=16;
    while (first_drawing_index < drawing_count)
    {
        if (object[draw_order[first_drawing_index]].iy <= vga16) 
            ++first_drawing_index;
        else
            break;
    }
    // reset vga16
    vga16+=16;
    // draw visible sprites
    for (int k=first_drawing_index; k<last_drawing_index; ++k)
    {
        struct object *o = &object[draw_order[k]];
        int sprite_draw_row = vga16 - o->iy;
        
        uint16_t *dst = draw_buffer + o->ix;
        uint8_t *src = &sprite_draw[o->sprite_index][o->sprite_frame][sprite_draw_row][0]-1;
        uint8_t invisible_color = sprite_info[o->sprite_index][o->sprite_frame] & 31;
        for (int pxl=0; pxl<8; ++pxl)
        {
            uint8_t color = (*(++src))&15;
            if (color != invisible_color)
                *dst = palette[color]; // &65535; // unnecessary...
            ++dst; 
            color = ((*src)>>4); //&15; //unnecessary!
            if (color != invisible_color)
                *dst = palette[color]; // &65535; // unnecessary...
            ++dst; 
        }
    }
    
    map_draw_crosshairs:
    if (draw_crosshairs)
    switch (tile_j)
    {
    case 7:
    case 8:
        dst = draw_buffer + (map_tile_x)*16 - tile_map_x + 6;
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
        dst = draw_buffer + (map_tile_x)*16 - tile_map_x + 7;
        *dst = ~(*dst);
        ++dst;
        *dst = ~(*dst);
        break;
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
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
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
        make_wait = 1;
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
            if (map_tile_y < tile_map_y)
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
            if (map_tile_y >= tile_map_y/16 + (SCREEN_H-2*MAP_HEADER)/16)
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
