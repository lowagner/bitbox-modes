#include "bitbox.h"
#include "nonsimple.h"
#include "sprites.h"
#include "tiles.h"
#include "edit.h"
#include <stdint.h>
#include <stdlib.h> // abs
#include <string.h> // memset

VisualMode visual_mode CCM_MEMORY; 
uint16_t palette[16] CCM_MEMORY; 
uint16_t old_gamepad[2];
uint8_t gamepad_press_wait CCM_MEMORY;

void graph_frame() 
{
    switch (visual_mode)
    {
    case (TilesAndSprites):
        sprite_frame();
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
        sprite_line();
        break;
    case EditTile:
        edit_tile_line();
        break;
    case EditSprite:
        edit_sprite_line();
        break;
    default:
        break;
    }
}

void reset_colors_and_map() 
{
    static const uint16_t colors[16] = {
        [BLACK]=RGB(0, 0, 0),
        [GRAY]=RGB(157, 157, 157),
        [WHITE]=(1<<15) - 1,
        [PINK]=RGB(224, 111, 139),
        [RED]=RGB(190, 38, 51),
        [BLAZE]=RGB(235, 137, 49),
        [BROWN]=RGB(164, 100, 34),
        [DULLBROWN]=RGB(73, 60, 43),
        [GINGER]=RGB(247, 226, 107),
        [SLIME]=RGB(163, 206, 39),
        [GREEN]=RGB(68, 137, 26),
        [BLUEGREEN]=RGB(47, 72, 78),
        [CLOUDBLUE]=RGB(178, 220, 239),
        [SKYBLUE]=RGB(49, 162, 242),
        [SEABLUE]=RGB(0, 87, 132),
        [INDIGO]=RGB(28, 20, 40),
    };
    memcpy(palette, colors, sizeof(colors));
    memset(tile_map, 0, sizeof(tile_map));
    tile_map_x = 0;
    tile_map_y = 0;
    for (int j=0; j<16; ++j)
        tile_translator[j] = j; 
}
