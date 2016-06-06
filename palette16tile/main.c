#include "bitbox.h"
#include "nonsimple.h"
#include "sprites.h"
#include "tiles.h"
#include "edit.h"
#include "save.h"
#include "fill.h"
#include "font.h"
#include "io.h"

#include "string.h" // memcpy

void game_init()
{ 
    // init font
    memcpy(font, font_cache, sizeof(font_cache));

    // init game mode
    visual_mode = TilesAndSprites;
   
    // init tile mapping
    for (int j=0; j<16; ++j)
        tile_translator[j] = j; 
    
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

    // now load everything else
    if (io_get_recent_filename())
    {
        // had troubles loading a filename
        base_filename[0] = 'T';
        base_filename[1] = 'M';
        base_filename[2] = 'P';
        base_filename[3] = 0;

        // need to reset everything
        palette_reset();
        tiles_reset();
        map_reset();
        sprites_reset();
    }
    else // there was a filename to look into
    {
        if (io_load_palette())
        {
            // had troubles loading a palette
            palette_reset();
        }
        if (io_load_tile(16))
        {
            // had troubles loading a palette
            tiles_reset();
        }
        //if (io_load_map())
        {
            map_reset();
        }
        //if (io_load_sprites())
        {
            sprites_reset();
        }
    }
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
    case SaveScreen:
        save_controls();
        break;
    }
    
    old_gamepad[0] = gamepad_buttons[0];
    old_gamepad[1] = gamepad_buttons[1];

    fill_frame();
    
    if (gamepad_press_wait)
        --gamepad_press_wait;
}
