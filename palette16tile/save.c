#include "bitbox.h"
#include "common.h"
#include "font.h"
#include "name.h"
#include "io.h"

#include <string.h> // memset

#define SAVE_COLOR 5 // a uint8_t, uint16_t color is (SAVE_COLOR)|(SAVE_COLOR<<8)

uint8_t save_only CCM_MEMORY; // 0 - everything, 1 - tiles, 2 - sprites, 3 - map, 4 - palette

#define NUMBER_LINES 10
   
void save_init()
{
    save_only = 0;
}

void save_line()
{
    if (vga_line < 22)
    {
        if (vga_line/2 == 0)
            memset(draw_buffer, SAVE_COLOR, 2*SCREEN_W);
        return;
    }
    else if (vga_line/2 == (SCREEN_H - 20)/2)
    {
        memset(draw_buffer, SAVE_COLOR, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= 22 + NUMBER_LINES*10)
    {
        if (vga_line/2 == (22 + NUMBER_LINES*10)/2)
            memset(draw_buffer, SAVE_COLOR, 2*SCREEN_W);
        return;
    }
    int line = (vga_line-22) / 10;
    int internal_line = (vga_line-22) % 10;
    if (internal_line == 0 || internal_line == 9)
    {
        memset(draw_buffer, SAVE_COLOR, 2*SCREEN_W);
    }
    else
    {
        --internal_line;
        switch (line)
        {
        case 0:
        {
            int save_text_offset = 16 + 5*9;
            font_render_line_doubled((const uint8_t *)"scan", 16, internal_line, 65535, SAVE_COLOR*257);
            switch (save_only)
            {
            case 0:
                font_render_line_doubled((const uint8_t *)"all:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257);
                save_text_offset += 5*9;
                break;
            case 1:
                font_render_line_doubled((const uint8_t *)"tiles:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257);
                save_text_offset += 7*9;
                break;
            case 2:
                font_render_line_doubled((const uint8_t *)"sprites:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257);
                save_text_offset += 9*9;
                break;
            case 3:
                font_render_line_doubled((const uint8_t *)"map:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257);
                save_text_offset += 5*9;
                break;
            case 4:
                font_render_line_doubled((const uint8_t *)"palette:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257);
                save_text_offset += 9*9;
                break;
                
            }
            font_render_line_doubled((uint8_t *)base_filename, save_text_offset, internal_line, 65535, SAVE_COLOR*257);
            break;
        }
        case 2:
            font_render_line_doubled((const uint8_t *)"start:choose filename", 16, internal_line, 65535, SAVE_COLOR*257);
            break;
        case 4:
            font_render_line_doubled((const uint8_t *)"<L/R>:selective save/load", 16, internal_line, 65535, SAVE_COLOR*257);
            break;
        case 6:
            font_render_line_doubled((const uint8_t *)"A:save to file", 16, internal_line, 65535, SAVE_COLOR*257);
            break;
        case 7:
            font_render_line_doubled((const uint8_t *)"X:load from file", 16, internal_line, 65535, SAVE_COLOR*257);
            //font_render_line_doubled((const uint8_t *)"X:delete  Y:overwrite", 16, internal_line, 65535, SAVE_COLOR*257);
            break;
        case 9:
            font_render_line_doubled(game_message, 32, internal_line, 65535, SAVE_COLOR*257);
            break;
        }
    }
}


void save_controls()
{
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

    int save_or_load = 0;
    if (GAMEPAD_PRESS(0, A))
    {
        game_message[0] = 0;
        // save
        save_or_load = 1;
    }
    if (GAMEPAD_PRESS(0, X))
    {
        game_message[0] = 0;
        // load
        save_or_load = 2;
    }
    if (save_or_load)
    {
        FileError error = BotchedIt;
        int offset = 0;
        switch (save_only)
        {
        case 0: // save all
            error = (save_or_load == 1) ? io_save_tile(16) : io_load_tile(16);
            if (error != NoError)
            {
                strcpy((char *)game_message, "tiles ");
                offset = 6;
                break;
            }
            error = (save_or_load == 1) ? io_save_sprite(16, 8) : io_load_sprite(16, 8);
            if (error != NoError)
            {
                strcpy((char *)game_message, "sprites ");
                offset = 8;
                break;
            }
            error = (save_or_load == 1) ? io_save_map() : io_load_map();
            if (error != NoError)
            {
                strcpy((char *)game_message, "map ");
                offset = 4;
                break;
            }
            error = (save_or_load == 1) ? io_save_palette() : io_load_palette();
            if (error != NoError)
            {
                strcpy((char *)game_message, "palette ");
                offset = 8;
                break;
            }
            break;
        case 1:
            error = (save_or_load == 1) ? io_save_tile(16) : io_load_tile(16);
            if (error != NoError)
            {
                strcpy((char *)game_message, "tiles ");
                offset = 6;
            }
            break;
        case 2:
            error = (save_or_load == 1) ? io_save_sprite(16, 8) : io_load_sprite(16, 8);
            if (error != NoError)
            {
                strcpy((char *)game_message, "sprites ");
                offset = 8;
            }
            break;
        case 3:
            error = (save_or_load == 1) ? io_save_map() : io_load_map();
            if (error != NoError)
            {
                strcpy((char *)game_message, "map ");
                offset = 4;
            }
            break;
        case 4:
            error = (save_or_load == 1) ? io_save_palette() : io_load_palette();
            if (error != NoError)
            {
                strcpy((char *)game_message, "palette ");
                offset = 8;
            }
            break;
        }
       
        switch (error)
        {
        case NoError:
            if (save_or_load == 1)
                strcpy((char *)game_message + offset, "saved!");
            else
                strcpy((char *)game_message + offset, "loaded!");
            break;
        case MountError:
            strcpy((char *)game_message + offset, "fs unmounted!");
            break;
        case ConstraintError:
            strcpy((char *)game_message + offset, "unconstrained!");
            break;
        case OpenError:
            strcpy((char *)game_message + offset, "no open!");
            break;
        case ReadError:
            strcpy((char *)game_message + offset, "no read!");
            break;
        case WriteError:
            strcpy((char *)game_message + offset, "no write!");
            break;
        case NoDataError:
            strcpy((char *)game_message + offset, "no data!");
            break;
        case MissingDataError:
            strcpy((char *)game_message + offset, "miss data!");
            break;
        case BotchedIt:
            strcpy((char *)game_message + offset, "fully bungled.");
            break;
        }
        return;
    }
    if (GAMEPAD_PRESS(0, B))
    {
        game_message[0] = 0;
        // TODO:  add functionality?
        return;
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        game_message[0] = 0;
        // TODO:  add functionality?
        return;
    }
    if (GAMEPAD_PRESS(0, L))
    {
        game_message[0] = 0;
        if (save_only)
            --save_only;
        return;
    } 
    if (GAMEPAD_PRESS(0, R))
    {
        game_message[0] = 0;
        if (save_only < 4)
            ++save_only; 
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        // switch to next visual mode and ignore previous_visual_mode
        visual_mode = TilesAndSprites;
        previous_visual_mode = None;
        save_only = 0; // reset save for next time
        return;
    }
    if (GAMEPAD_PRESS(0, start))
    {
        game_message[0] = 0;
        // switch to choose name and hope to come back
        visual_mode = ChooseFilename;
        previous_visual_mode = SaveLoadScreen;
        return;
    }
}
