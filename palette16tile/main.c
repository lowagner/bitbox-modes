#include "bitbox.h"
#include "common.h"
#include "sprites.h"
#include "tiles.h"
#include "edit.h"
#include "save.h"
#include "fill.h"
#include "font.h"
#include "io.h"

#include "string.h" // memcpy

VisualMode visual_mode CCM_MEMORY; 
uint16_t old_gamepad[2] CCM_MEMORY;
uint8_t gamepad_press_wait CCM_MEMORY;
uint8_t game_message[32] CCM_MEMORY;

void game_init()
{ 
    // init font
    memcpy(font, font_cache, sizeof(font_cache));

    // init game mode
    visual_mode = TilesAndSprites;
    save_not_load = 1;
    save_only = 0;
   
    // init tile mapping
    for (int j=0; j<16; ++j)
        tile_translator[j] = j; 
    
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
            // had troubles loading tiles...
            tiles_reset();
        }
        if (io_load_map())
        {
            // etc...
            map_reset();
        }
        if (io_load_sprite(16))
        {
            // and so on...
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
    case EditTileOrSprite:
        edit_tile_controls();
        break;
    case SaveLoadScreen:
        save_controls();
        break;
    }
    
    old_gamepad[0] = gamepad_buttons[0];
    old_gamepad[1] = gamepad_buttons[1];

    fill_frame();
    
    if (gamepad_press_wait)
        --gamepad_press_wait;
}

void graph_frame() 
{
    switch (visual_mode)
    {
    case (TilesAndSprites):
        sprites_frame();
        break;
    default:
        break;
    }
}

void graph_line() 
{
    if (vga_odd)
        return;
    switch (visual_mode)
    {
    case TilesAndSprites:
        tiles_line();
        sprites_line();
        break;
    case EditTileOrSprite:
        edit_tile_line();
        break;
    case SaveLoadScreen:
        save_line();
        break;
    default:
        break;
    }
}
