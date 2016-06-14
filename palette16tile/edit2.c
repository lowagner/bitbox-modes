#include "bitbox.h"
#include "common.h"
#include "font.h"
#include "edit.h"
#include "name.h"
#include "io.h"

#include <string.h> // memset

#define TILE_COLOR 136 // a uint8_t, uint16_t color is (_COLOR)|(_COLOR<<8)
#define SPRITE_COLOR 164
#define BG_COLOR ((TILE_COLOR + (SPRITE_COLOR-TILE_COLOR)*edit_sprite_not_tile))

#define NUMBER_LINES 12

uint8_t edit2_copying CCM_MEMORY; // 0 for not copying, 1 for sprite, 2 for tile
uint8_t edit2_copy_location CCM_MEMORY;

void edit2_init()
{
    edit2_copying = 0;
}

void edit2_line()
{
    if (vga_line < 22)
    {
        if (vga_line/2 == 0)
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }
    else if (vga_line/2 == (SCREEN_H - 20)/2)
    {
        memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= 22 + NUMBER_LINES*10)
    {
        draw_parade(vga_line - (22 + NUMBER_LINES*10), BG_COLOR);
        return;
    }
    int line = (vga_line-22) / 10;
    int internal_line = (vga_line-22) % 10;
    if (internal_line == 0 || internal_line == 9)
    {
        memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
    }
    else
    {
        --internal_line;
        switch (line)
        {
        case 0:
            if (edit_sprite_not_tile)
            {

                uint8_t label[] = { 's', 'p', 'r', 'i', 't', 'e', ' ', hex[edit_sprite/8], '.', 
                    direction[(edit_sprite%8)/2], hex[(edit_sprite%8)%2], ' ', 'i', 'n', 0 };
                font_render_line_doubled(label, 16, internal_line, 65535, SPRITE_COLOR*257);
                font_render_line_doubled((uint8_t *)base_filename, 16+9*15, internal_line, 65535, SPRITE_COLOR*257);
            }
            else
            {
                uint8_t label[] = { 't', 'i', 'l', 'e', ' ', hex[edit_tile], ' ', 'i', 'n', 0 };
                font_render_line_doubled(label, 16, internal_line, 65535, TILE_COLOR*257);
                font_render_line_doubled((uint8_t *)base_filename, 16+9*10, internal_line, 65535, TILE_COLOR*257);
            }
            break;
        case 1: 
            break;
        case 2:
            if (edit_sprite_not_tile)
                font_render_line_doubled((const uint8_t *)"L/R:cycle sprite", 16, internal_line, 65535, SPRITE_COLOR*257);
            else
                font_render_line_doubled((const uint8_t *)"L/R:cycle tile", 16, internal_line, 65535, TILE_COLOR*257);
            break;
        case 3:
            if (edit2_copying)
                font_render_line_doubled((const uint8_t *)"A:cancel copy", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            else
                font_render_line_doubled((const uint8_t *)"A:save to file", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            break;
        case 4:
            if (edit2_copying)
                font_render_line_doubled((const uint8_t *)"X:  \"     \"", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            else
                font_render_line_doubled((const uint8_t *)"X:load from file", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            break;
        case 5:
            if (edit2_copying)
                font_render_line_doubled((const uint8_t *)"B:  \"     \"", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            else
                font_render_line_doubled((const uint8_t *)"B:copy", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            break;
        case 6:
            if (edit2_copying)
                font_render_line_doubled((const uint8_t *)"Y:paste", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            else
                font_render_line_doubled((const uint8_t *)"Y:change filename", 16+2*9, internal_line, 65535, 257*BG_COLOR);
        case 7:
            break;
        case 8:
            font_render_line_doubled((const uint8_t *)"start:return", 16, internal_line, 65535, 257*BG_COLOR);
            break;
        case 9:
            if (edit_sprite_not_tile)
                font_render_line_doubled((const uint8_t *)"select:go to palette", 16, internal_line, 65535, SPRITE_COLOR*257);
            else
                font_render_line_doubled((const uint8_t *)"select:go to sprites", 16, internal_line, 65535, TILE_COLOR*257);
            break;
        case 10:
            break;
        case 11:
            font_render_line_doubled(game_message, 16, internal_line, 65535, BG_COLOR*257);
            break;
        }
    }
    if (vga_line < 22 + 2*16)
    {
        uint32_t *dst = (uint32_t *)draw_buffer + (SCREEN_W - 24 - 16*2*2)/2 - 1;
        internal_line = (vga_line - 22)/2;
        uint8_t *tile_color = edit_sprite_not_tile ?
            &sprite_draw[edit_sprite/8][edit_sprite%8][internal_line][0] - 1:
            &tile_draw[edit_tile][internal_line][0] - 1;
        for (int l=0; l<8; ++l) 
        {
            uint32_t color = palette[(*(++tile_color))&15];
            color |= color << 16;
            *(++dst) = color;
            
            color = palette[(*tile_color)>>4];
            color |= color << 16;
            *(++dst) = color;
        }
    }
    else if (vga_line/2 == (22 + 2*16)/2)
    {
        memset(draw_buffer+(SCREEN_W - 24 - 16*2*2), BG_COLOR, 64);
    }
    else if (vga_line < 22 + 2 + 2*16 + 2*16 && edit2_copying)
    {
        uint32_t *dst = (uint32_t *)draw_buffer + (SCREEN_W - 24 - 16*2)/2;
        internal_line = (vga_line - (22 + 2 + 2*16))/2;
        uint8_t *tile_color = edit2_copying == 1 ?
            &sprite_draw[edit2_copy_location/8][edit2_copy_location%8][internal_line][0] - 1:
            &tile_draw[edit2_copy_location][internal_line][0] - 1;
        for (int l=0; l<8; ++l) 
        {
            uint32_t color = palette[(*(++tile_color))&15];
            color |= color << 16;
            *(++dst) = color;
            
            color = palette[(*tile_color)>>4];
            color |= color << 16;
            *(++dst) = color;
        }
    }
}

void edit2_controls()
{
    int moved = 0;
    if (GAMEPAD_PRESSING(0, R))
    {
        ++moved;
    }
    if (GAMEPAD_PRESSING(0, L))
    {
        --moved;
    }
    if (moved)
    {
        game_message[0] = 0;
        if (edit_sprite_not_tile)
        {
            edit_sprite = (edit_sprite + moved)&127;
            gamepad_press_wait = GAMEPAD_PRESS_WAIT;
        }
        else
        {
            edit_tile = (edit_tile + moved)&15;
            gamepad_press_wait = GAMEPAD_PRESS_WAIT+GAMEPAD_PRESS_WAIT/2;
        }
        return;
    }
    int make_wait = 0;
    if (GAMEPAD_PRESSING(0, left))
    {
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, right))
    {
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, down))
    {
        make_wait = 1;
    }
    if (make_wait)
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;

    if (GAMEPAD_PRESS(0, B))
    {
        // copy or uncopy
        if (edit2_copying)
        {
            edit2_copying = 0;
        }
        else if (edit_sprite_not_tile)
        {
            edit2_copying = 1;
            edit2_copy_location = edit_sprite;
        }
        else
        {
            edit2_copying = 2;
            edit2_copy_location = edit_tile;
        }
        return;
    }
    int save_or_load = 0;
    if (GAMEPAD_PRESS(0, A))
    {
        // save
        save_or_load = 1;
    }
    else if (GAMEPAD_PRESS(0, X))
    {
        // load
        save_or_load = 2;
    }
    if (save_or_load)
    {
        if (edit2_copying)
        {
            // or cancel a copy
            edit2_copying = 0;
            return;
        }
            
        FileError error;
        if (save_or_load == 1) // save
        {
            if (edit_sprite_not_tile)
                error = io_save_sprite(edit_sprite/8, edit_sprite%8);
            else
                error = io_save_tile(edit_tile);
        }
        else // load
        {
            if (edit_sprite_not_tile)
                error = io_load_sprite(edit_sprite/8, edit_sprite%8);
            else
                error = io_load_tile(edit_tile);
        }
        switch (error)
        {
        case NoError:
            if (save_or_load == 1)
                strcpy((char *)game_message, "saved!");
            else
                strcpy((char *)game_message, "loaded!");
            break;
        case MountError:
            strcpy((char *)game_message, "fs unmounted!");
            break;
        case ConstraintError:
            strcpy((char *)game_message, "unconstrained!");
            break;
        case OpenError:
            strcpy((char *)game_message, "no open!");
            break;
        case ReadError:
            strcpy((char *)game_message, "no read!");
            break;
        case WriteError:
            strcpy((char *)game_message, "no write!");
            break;
        case NoDataError:
            strcpy((char *)game_message, "no data!");
            break;
        case MissingDataError:
            strcpy((char *)game_message, "miss data!");
            break;
        case BotchedIt:
            strcpy((char *)game_message, "fully bungled.");
            break;
        }
        return;
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        if (edit2_copying)
        {
            // paste
            uint8_t *src, *dst;
            if (edit2_copying == 1) // sprite
                src = sprite_draw[edit2_copy_location/8][edit2_copy_location%8][0];
            else
                src = tile_draw[edit2_copy_location][0];
            if (edit_sprite_not_tile)
                dst = sprite_draw[edit_sprite/8][edit_sprite%8][0];
            else
                dst = tile_draw[edit_tile][0];
            if (src == dst)
                strcpy((char *)game_message, "pasting to same thing");
            else
            {
                memcpy(dst, src, 16*8);
                strcpy((char *)game_message, "pasted.");
            }
            edit2_copying = 0;
        }
        else
        {
            previous_visual_mode = EditTileOrSpriteProperties;
            game_switch(ChooseFilename);
        }
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        if (edit_sprite_not_tile)
        {
            edit_sprite_not_tile = 0; 
            game_switch(EditPalette);
            previous_visual_mode = None;
        }
        else
        {
            edit_sprite_not_tile = 1;
        }
        return;
    }
    if (GAMEPAD_PRESS(0, start))
    {
        game_message[0] = 0;
        if (previous_visual_mode)
        {
            game_switch(previous_visual_mode);
            previous_visual_mode = None;
        }
        else
        {
            game_switch(EditTileOrSprite);
        }
        return;
    }
}
