#include "bitbox.h"
#include "common.h"
#include "sprites.h"
#include "tiles.h"
#include "edit.h"
#include "edit2.h"
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
    font_init();
    save_init();
    edit_init();
    edit2_init();
    tiles_init();
    sprites_init();

    // init game mode
    visual_mode = TilesAndSprites;

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
        if (io_load_sprite(16, 8))
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
        edit_controls();
        break;
    case EditTileOrSpriteProperties:
        edit2_controls();
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
        edit_line();
        break;
    case EditTileOrSpriteProperties:
        edit2_line();
        break;
    case SaveLoadScreen:
        save_line();
        break;
    default:
        break;
    }
}
