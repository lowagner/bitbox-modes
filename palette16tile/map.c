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
        visual_mode = EditTileOrSprite;
    }
}


