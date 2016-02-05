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

    for (int j=0; j<tile_map_height; ++j)
    for (int i=0; i<tile_map_width; ++i)
    {
        if (j < (tile_map_height-16))
            tile_map[j*tile_map_width+i] = 0;
        else
            tile_map[j*tile_map_width+i] = (j - tile_map_height + 16)%16;
    }

    // create some random sprites...
    uint32_t *sc = (uint32_t*) sprite_draw[0][0];
    uint8_t color_index = 0;
    for (int tile=0; tile<15; ++tile)
    {
        for (int line=0; line<16; ++line)
        {
            uint32_t color2 = color_index | (color_index<<8);
            color2 = color2 | (color2 << 16);
            for (int col=0; col<8; ++col)
                *sc++ = color2;
            ++color_index;
        }
    }
    // 16th sprite is random
    for (int l=0; l<256/2; ++l)
    {
        *sc++ = rand();
    }
    
    // tile 0
    uint16_t *tc = tile_draw[0][0];
    for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
    {
        *tc++ = 65535;
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
    {
        if (((j/8)%2 + i/8) % 2)
            *tc++ = 65535;
        else
            *tc++ = RGB(255,255,255);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
    {
        if (((j)%2 + i) % 2)
            *tc++ = RGB(10,10,50);
        else
            *tc++ = RGB(10,50,30);
    }
    // next tile
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
    for (int k=0; k<8; ++k)
    for (int l=0; l<256; ++l)
    {
        *tc++ = rand()%(1<<16);
    }

    // setup the objects and the linked list:
    for (int i=0; i<MAX_OBJECTS; ++i)
    {
        object[i] = (struct object) {
            .next_free_object = i+1,
            .next_used_object = 255,
        };
    }
    object[MAX_OBJECTS-1].next_free_object = 255;
    first_free_object = 0;
    first_used_object = 255;

    for (int k=0; k<MAX_OBJECTS; ++k)
        create_object(k%16, rand()%(tile_map_width*16+16)-16, rand()%(tile_map_height*16+16)-16, rand()%256);
}

void game_frame()
{
    static int darkening = 0;
    kbd_emulate_gamepad();

    if (vga_frame % 2 == 1)
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
    else if (vga_frame % 60 == 0)
    {
        uint16_t *tc = tile_draw[15][0];
        for (int k=0; k<256; ++k)
        {
            *tc++ = rand()%(1<<16);
        }
     
        // modify the background color:
        uint8_t bg_r = (bg_color >> 10) & 31;
        uint8_t bg_g = (bg_color >> 5) & 31;
        uint8_t bg_b = bg_color & 31;
        if (darkening)
        {
            if (bg_r)
                --bg_r;
            else
                darkening = 0;
            if (bg_g)
                --bg_g;
            if (bg_b)
                --bg_b;
        }
        else
        {
            if (bg_r < 31)
                ++bg_r;
            if (bg_g < 31)
                ++bg_g;
            if (bg_b < 31)
                ++bg_b;
            else
                darkening = 1;
        }

        bg_color = (bg_r << 10) | (bg_g << 5) | (bg_b);
    }
    
}
