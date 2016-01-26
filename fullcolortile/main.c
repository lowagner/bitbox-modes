#include <stdlib.h> // rand
#include <math.h>
#include "nonsimple.h"

#include <string.h> // memset

void game_init()
{ 
    bg_color = RGB(30,100,200);

    clear();
    tile_map_width = 26;
    tile_map_height = 20;


    for (int k=0; k<tile_map_width*tile_map_height; ++k)
    {
        tile_map[k] = k%16;
    }

/*
    // tile 0
    uint16_t *tc = tile_draw[0][0];
    for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
    {
        if (((j)%2 + i) % 2)
            *tc++ = RGB(10,10,50);
        else
            *tc++ = RGB(10,50,30);
    }
    // tile 1
    for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
    {
        if (((j/2)%2 + i/2) % 2)
            *tc++ = RGB(200,100,50);
        else
            *tc++ = RGB(150,30,50);
    }
    // etc.
    for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
    {
        if (((j/4)%2 + i/4) % 2)
            *tc++ = RGB(150,100,50);
        else
            *tc++ = RGB(20,30,50);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
    {
        if (((j/8)%2 + i/8) % 2)
            *tc++ = RGB(20,10,250);
        else
            *tc++ = RGB(15,150,50);
    }
    // splitting a tile up
    for (int i=0; i<16; ++i)
    {
        if (((0/8)%2 + i/8) % 2)
            *tc++ = RGB(200,100,250);
        else
            *tc++ = RGB(105,150,130);
    }
    for (int j=1; j<15; ++j)
    {
        if (((j/8)%2 + 0/8) % 2)
            *tc++ = RGB(200,100,250);
        else
            *tc++ = RGB(105,150,130);
        for (int i=1; i<15; ++i)
        {
            if (((j/8)%2 + i/8) % 2)
                *tc++ = RGB(180,80,220);
            else
                *tc++ = RGB(85,130,100);
        }
        if (((j/8)%2 + 15/8) % 2)
            *tc++ = RGB(120,50,200);
        else
            *tc++ = RGB(45,100,50);
    }
    for (int i=0; i<16; ++i)
    {
        if (((15/8)%2 + i/8) % 2)
            *tc++ = RGB(120,50,200);
        else
            *tc++ = RGB(45,100,50);
    }
    // next tiles are random
    for (int k=0; k<9; ++k)
    for (int l=0; l<256; ++l)
    {
        *tc++ = rand()%(1<<16);
    }
*/
    uint32_t *tc = (uint32_t*) tile_draw[0][0];
    uint8_t color_index = 0;
    for (int tile=0; tile<16; ++tile)
    {
        for (int line=0; line<16; ++line)
        {
            uint32_t color2 = color_index | (color_index<<8);
            color2 = color2 | (color2 << 16);
            for (int col=0; col<8; ++col)
                *tc++ = color2;
            ++color_index;
        }
    }
}

void game_frame()
{
    kbd_emulate_gamepad();

    if (vga_frame % 2 == 1)
    {
        if (GAMEPAD_PRESSED(0, left))
        {
            if (tile_map_x > 0)
                --tile_map_x;
        }
        else if (GAMEPAD_PRESSED(0, right))
        {
            if (tile_map_x + SCREEN_W < tile_map_width*16 - 1)
                ++tile_map_x;
        }
        if (GAMEPAD_PRESSED(0, up))
        {
            if (tile_map_y > 0)
                --tile_map_y;
        }
        else if (GAMEPAD_PRESSED(0, down))
        {
            if (tile_map_y + SCREEN_H < tile_map_height*16 - 1)
                ++tile_map_y;
        }
    } 
    else if (vga_frame % 60 == 0)
    {
        uint16_t *tc = tile_draw[15][0];
        for (int k=0; k<256; ++k)
        {
            *tc++ = rand()%(1<<16);
        }
    }
}
