#include "bitbox.h"
#include "nonsimple.h"
#include "sprites.h"
#include "tiles.h"
#include "edit.h"
#include "fill.h"

#include <stdlib.h> // rand

void game_init()
{ 
    old_gamepad[0] = 65535;
    old_gamepad[1] = 65535;
    visual_mode = TilesAndSprites;

    reset_colors_and_map();
    tile_map_width = 26;
    tile_map_height = 20;

    for (int j=0; j<tile_map_height; ++j)
    for (int i=0; i<tile_map_width/2; ++i)
    {
        if (j < (tile_map_height-16))
            tile_map[j*(tile_map_width/2)+i] = 0;
        else
            tile_map[j*(tile_map_width/2)+i] = ((j - tile_map_height + 16)%16)|(((j - tile_map_height + 16)%16)<<4);
    }

    // create some random sprites...
    uint8_t *sc = sprite_draw[0][0][0];
    int color_index = 0;
    for (int tile=0; tile<15; ++tile)
    for (int frame=0; frame<8; ++frame)
    for (int line=0; line<16; ++line)
    {
        for (int col=0; col<8; ++col)
            *sc++ = (((color_index+1)>>(tile/2))&15)|(((color_index>>(tile/2))&15)<<4);
        ++color_index;
    }
    // 16th sprite is random
    for (int l=0; l<256/2; ++l)
    {
        *sc++ = rand()%256;
    }
    
    // tile 0
    uint8_t *tc = tile_draw[0][0];
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        *tc++ = SKYBLUE|(SKYBLUE<<4);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j/8)%2 + i/4) % 2)
            *tc++ = BLAZE|(BLAZE<<4);
        else
            *tc++ = DULLBROWN|(DULLBROWN<<4);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j)%2 + i) % 2)
            *tc++ = CLOUDBLUE|(SEABLUE<<4);
        else
            *tc++ = BLUEGREEN|(BLUEGREEN<<4);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j/2)%2 + i) % 2)
            *tc++ = BROWN|(BROWN<<4);
        else
            *tc++ = DULLBROWN|(DULLBROWN<<4);
    }
    // etc.
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j/4)%2 + i/2) % 2)
            *tc++ = RED|(RED<<4);
        else
            *tc++ = GINGER|(GINGER<<4);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j/8)%2 + i/4) % 2)
            *tc++ = GREEN|(GREEN<<4);
        else
            *tc++ = INDIGO|(INDIGO<<4);
    }
    // splitting a tile up
    for (int i=0; i<8; ++i)
    {
        if (((0/8)%2 + i/4) % 2)
            *tc++ = CLOUDBLUE|(CLOUDBLUE<<4);
        else
            *tc++ = SLIME|(SLIME<<4);
    }
    for (int j=1; j<15; ++j)
    {
        if (((j/8)%2 + 0/8) % 2)
            *tc++ = CLOUDBLUE|(SKYBLUE<<4);
        else
            *tc++ = SLIME|(GREEN<<4);
        for (int i=1; i<7; ++i)
        {
            if (((j/8)%2 + i/4) % 2)
                *tc++ = SKYBLUE|(SKYBLUE<<4);
            else
                *tc++ = GREEN|(GREEN<<4);
        }
        if (((j/8)%2 + 15/8) % 2)
            *tc++ = SKYBLUE|(SEABLUE<<4);
        else
            *tc++ = GREEN|(BLUEGREEN<<4);
    }
    for (int i=0; i<8; ++i)
    {
        if (((15/8)%2 + i/4) % 2)
            *tc++ = SEABLUE|(SEABLUE<<4);
        else
            *tc++ = BLUEGREEN|(BLUEGREEN<<4);
    }
    // next tiles are random
    for (int k=0; k<8; ++k)
    for (int l=0; l<128; ++l)
    {
        *tc++ = rand()%256;
    }

    // setup the objects and the linked list:
    for (int i=0; i<MAX_OBJECTS; ++i)
    {
        object[i] = (struct object) {
            .next_free_object = i+1,
            .next_used_object = 255,
            .invisible_color = 16 // won't match anything
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
    kbd_emulate_gamepad();
    switch (visual_mode)
    {
    case TilesAndSprites:
        map_controls();
        break;
    case EditTile:
        edit_tile_controls();
        break;
    }
    
    if (vga_frame % 60 == 0)
    {
        uint8_t *tc = tile_draw[15][0];
        for (int k=0; k<128; ++k)
        {
            *tc++ = rand()%256;
        }
    } 
    
    old_gamepad[0] = gamepad_buttons[0];
    old_gamepad[1] = gamepad_buttons[1];

    fill_frame();
}
